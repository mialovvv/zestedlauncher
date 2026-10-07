#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "fs_utils.hpp"
#include "http_client.hpp"
#include "mojang_api.hpp"
#include "auth_manager.hpp"
#include "settings_manager.hpp"
#include "instance_manager.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

class GameLauncher {
public:
    static GameLauncher& instance() {
        static GameLauncher inst;
        return inst;
    }

    struct LaunchStatus {
        std::string stage;      // "idle", "preparing", "downloading_manifest", "downloading_client", "downloading_libraries", "downloading_assets", "extracting_natives", "launching", "running", "error"
        int progress = 0;        // 0 to 100
        std::string details;
        bool is_running = false;
        std::string error_message;
    };

    LaunchStatus get_status() {
        std::lock_guard<std::mutex> lock(status_mutex_);
        return current_status_;
    }

    void set_status(const std::string& stage, int progress, const std::string& details, bool is_running = false, const std::string& error = "") {
        std::lock_guard<std::mutex> lock(status_mutex_);
        current_status_.stage = stage;
        current_status_.progress = progress;
        current_status_.details = details;
        current_status_.is_running = is_running;
        current_status_.error_message = error;
    }

    void launch(const std::string& instance_id, std::function<void(const LaunchStatus&)> on_progress = nullptr) {
        if (is_launching_.exchange(true)) {
            return;
        }

        std::thread([this, instance_id, on_progress]() {
            try {
                do_launch(instance_id, on_progress);
            } catch (const std::exception& e) {
                set_status("error", 0, e.what(), false, e.what());
                if (on_progress) on_progress(get_status());
            } catch (...) {
                set_status("error", 0, "Неизвестная критическая ошибка запуска", false, "Unknown error");
                if (on_progress) on_progress(get_status());
            }
            is_launching_ = false;
        }).detach();
    }

private:
    std::mutex status_mutex_;
    LaunchStatus current_status_{"idle", 0, "Готов к запуску", false, ""};
    std::atomic<bool> is_launching_{false};

    void do_launch(const std::string& instance_id, std::function<void(const LaunchStatus&)> cb) {
        auto notify = [this, &cb](const std::string& stage, int progress, const std::string& details, bool is_running = false, const std::string& error = "") {
            set_status(stage, progress, details, is_running, error);
            if (cb) cb(get_status());
        };

        notify("preparing", 2, "Подготовка игрового окружения...");

        InstanceConfig inst = InstanceManager::instance().get_instance_by_id(instance_id);
        Account acc = AuthManager::instance().get_active_account();
        json settings = SettingsManager::instance().get_json();

        std::string appdata = FsUtils::get_appdata_dir();
        std::string versions_dir = appdata + "\\versions";
        std::string libraries_dir = appdata + "\\libraries";
        std::string assets_dir = appdata + "\\assets";
        std::string game_dir = InstanceManager::instance().get_instance_path(inst.id);

        FsUtils::create_directories(versions_dir);
        FsUtils::create_directories(libraries_dir);
        FsUtils::create_directories(assets_dir);
        FsUtils::create_directories(assets_dir + "\\indexes");
        FsUtils::create_directories(assets_dir + "\\objects");
        FsUtils::create_directories(game_dir);
        FsUtils::create_directories(game_dir + "\\mods");
        FsUtils::create_directories(game_dir + "\\logs");

        notify("downloading_manifest", 5, "Загрузка манифеста Minecraft " + inst.game_version);

        // 1. Получение манифеста версии
        json version_json = MojangApi::instance().get_version_json(inst.game_version);
        if (version_json.empty()) {
            notify("error", 0, "Не удалось загрузить манифест версии Minecraft", false, "Manifest not found");
            return;
        }

        // 2. Скачивание Client JAR
        std::string client_jar_path = versions_dir + "\\" + inst.game_version + "\\" + inst.game_version + ".jar";
        if (!FsUtils::file_exists(client_jar_path)) {
            notify("downloading_client", 15, "Загрузка игрового клиента Minecraft...");
            if (version_json.contains("downloads") && version_json["downloads"].contains("client")) {
                std::string client_url = version_json["downloads"]["client"].value("url", "");
                if (!client_url.empty()) {
                    bool ok = HttpClient::instance().download_file(client_url, client_jar_path, [&](uint64_t dl, uint64_t total) {
                        if (total > 0) {
                            int p = 15 + (int)((dl * 15) / total);
                            notify("downloading_client", p, "Загрузка клиента: " + std::to_string(dl / 1024 / 1024) + " / " + std::to_string(total / 1024 / 1024) + " МБ");
                        }
                    });
                    if (!ok) {
                        notify("error", 0, "Не удалось скачать client.jar", false, "Client jar download failed");
                        return;
                    }
                }
            }
        }

        // 3. Скачивание библиотек
        notify("downloading_libraries", 30, "Синхронизация библиотек...");
        std::vector<std::string> classpath_entries;
        classpath_entries.push_back(client_jar_path);

        std::string natives_dir = versions_dir + "\\" + inst.game_version + "\\natives";
        FsUtils::create_directories(natives_dir);

        if (version_json.contains("libraries") && version_json["libraries"].is_array()) {
            const auto& libs = version_json["libraries"];
            size_t total_libs = libs.size();
            size_t idx = 0;

            for (const auto& lib : libs) {
                idx++;
                if (lib.contains("rules") && !MojangApi::check_rule_allowed(lib["rules"])) {
                    continue;
                }

                std::string lib_name = lib.value("name", "");
                if (lib_name.find("arm64") != std::string::npos || lib_name.find("x86") != std::string::npos) {
                    continue;
                }

                if (lib.contains("downloads")) {
                    const auto& dl = lib["downloads"];
                    if (dl.contains("artifact")) {
                        const auto& art = dl["artifact"];
                        std::string rel_path = art.value("path", "");
                        std::string url = art.value("url", "");
                        if (!rel_path.empty() && !url.empty()) {
                            std::replace(rel_path.begin(), rel_path.end(), '/', '\\');
                            std::string full_path = libraries_dir + "\\" + rel_path;
                            if (!FsUtils::file_exists(full_path)) {
                                HttpClient::instance().download_file(url, full_path);
                            }
                            classpath_entries.push_back(full_path);

                            if (rel_path.find("natives") != std::string::npos) {
                                extract_jar_natives(full_path, natives_dir);
                            }
                        }
                    }

                    if (dl.contains("classifiers") && dl["classifiers"].contains("natives-windows")) {
                        const auto& nat = dl["classifiers"]["natives-windows"];
                        std::string rel_path = nat.value("path", "");
                        std::string url = nat.value("url", "");
                        if (!rel_path.empty() && !url.empty()) {
                            std::replace(rel_path.begin(), rel_path.end(), '/', '\\');
                            std::string full_path = libraries_dir + "\\" + rel_path;
                            if (!FsUtils::file_exists(full_path)) {
                                HttpClient::instance().download_file(url, full_path);
                            }
                            extract_jar_natives(full_path, natives_dir);
                        }
                    }
                }

                int p = 30 + (int)((idx * 30) / total_libs);
                if (idx % 10 == 0) {
                    notify("downloading_libraries", p, "Загрузка библиотек (" + std::to_string(idx) + "/" + std::to_string(total_libs) + ")");
                }
            }
        }

        // 4. Загрузка и настройка Fabric Loader
        std::string main_class = version_json.value("mainClass", "net.minecraft.client.main.Main");
        if (inst.loader_type == "fabric") {
            notify("downloading_libraries", 62, "Инициализация Fabric Loader...");
            std::string fab_loader_ver = inst.loader_version;
            if (fab_loader_ver.empty()) {
                auto loaders = MojangApi::instance().get_fabric_loaders_for_game(inst.game_version);
                if (!loaders.empty()) fab_loader_ver = loaders[0];
            }

            if (!fab_loader_ver.empty()) {
                json fab_profile = MojangApi::instance().get_fabric_profile_json(inst.game_version, fab_loader_ver);
                if (!fab_profile.empty()) {
                    if (fab_profile.contains("mainClass")) {
                        main_class = fab_profile["mainClass"].get<std::string>();
                    }

                    if (fab_profile.contains("libraries") && fab_profile["libraries"].is_array()) {
                        for (const auto& flib : fab_profile["libraries"]) {
                            std::string name = flib.value("name", "");
                            std::string url = flib.value("url", "https://maven.fabricmc.net/");
                            std::string rel_path = maven_name_to_path(name);
                            if (!rel_path.empty()) {
                                std::replace(rel_path.begin(), rel_path.end(), '/', '\\');
                                std::string full_path = libraries_dir + "\\" + rel_path;
                                if (!FsUtils::file_exists(full_path)) {
                                    if (!url.empty() && url.back() != '/') url += '/';
                                    std::string url_sub = rel_path;
                                    std::replace(url_sub.begin(), url_sub.end(), '\\', '/');
                                    std::string full_url = url + url_sub;
                                    HttpClient::instance().download_file(full_url, full_path);
                                }
                                classpath_entries.push_back(full_path);
                            }
                        }
                    }
                }
            }
        }

        // 5. Синхронизация и загрузка всех необходимых ассетов Minecraft
        notify("downloading_assets", 68, "Проверка индексных ассетов...");
        std::string asset_index = "legacy";
        if (version_json.contains("assetIndex")) {
            asset_index = version_json["assetIndex"].value("id", "legacy");
            std::string index_url = version_json["assetIndex"].value("url", "");
            std::string index_dest = assets_dir + "\\indexes\\" + asset_index + ".json";
            if (!FsUtils::file_exists(index_dest) && !index_url.empty()) {
                HttpClient::instance().download_file(index_url, index_dest);
            }

            if (FsUtils::file_exists(index_dest)) {
                try {
                    json index_data = json::parse(FsUtils::read_file_string(index_dest));
                    if (index_data.contains("objects") && index_data["objects"].is_object()) {
                        // Alternate cache directories on user PC to pull assets from instantly
                        std::string userprofile = getenv("USERPROFILE") ? getenv("USERPROFILE") : "";
                        std::vector<std::string> alt_caches = {
                            appdata + "\\PrismLauncher\\assets\\objects",
                            userprofile + "\\AppData\\Roaming\\PrismLauncher\\assets\\objects",
                            userprofile + "\\AppData\\Roaming\\.minecraft\\assets\\objects"
                        };

                        struct AssetTask {
                            std::string url;
                            std::string dest;
                        };
                        std::vector<AssetTask> missing_tasks;

                        for (auto& [path_key, obj_info] : index_data["objects"].items()) {
                            std::string hash = obj_info.value("hash", "");
                            if (hash.size() < 2) continue;
                            std::string sub = hash.substr(0, 2);
                            std::string dest_file = assets_dir + "\\objects\\" + sub + "\\" + hash;

                            if (FsUtils::file_exists(dest_file)) {
                                continue;
                            }

                            // Check local alt caches
                            bool found_local = false;
                            for (const auto& alt_dir : alt_caches) {
                                std::string alt_file = alt_dir + "\\" + sub + "\\" + hash;
                                if (FsUtils::file_exists(alt_file)) {
                                    FsUtils::create_directories(assets_dir + "\\objects\\" + sub);
                                    std::error_code ec;
                                    fs::copy_file(FsUtils::u8path(alt_file), FsUtils::u8path(dest_file), fs::copy_options::overwrite_existing, ec);
                                    if (!ec) {
                                        found_local = true;
                                        break;
                                    }
                                }
                            }

                            if (!found_local) {
                                missing_tasks.push_back({
                                    "https://resources.download.minecraft.net/" + sub + "/" + hash,
                                    dest_file
                                });
                            }
                        }

                        // Multithreaded download of missing assets if any
                        if (!missing_tasks.empty()) {
                            size_t total_tasks = missing_tasks.size();
                            std::atomic<size_t> task_idx{0};
                            std::atomic<size_t> done_count{0};
                            std::vector<std::thread> workers;
                            int thread_count = (int)std::min<size_t>(8, total_tasks);

                            for (int t = 0; t < thread_count; ++t) {
                                workers.emplace_back([&]() {
                                    while (true) {
                                        size_t i = task_idx.fetch_add(1);
                                        if (i >= total_tasks) break;
                                        HttpClient::instance().download_file(missing_tasks[i].url, missing_tasks[i].dest);
                                        size_t d = done_count.fetch_add(1) + 1;
                                        if (d % 25 == 0 || d == total_tasks) {
                                            int p = 70 + (int)((d * 18) / total_tasks);
                                            notify("downloading_assets", p, "Загрузка ресурсов: " + std::to_string(d) + " / " + std::to_string(total_tasks));
                                        }
                                    }
                                });
                            }

                            for (auto& w : workers) {
                                if (w.joinable()) w.join();
                            }
                        }
                    }
                } catch (...) {}
            }
        }

        // 6. Формирование аргументов запуска Java
        notify("launching", 90, "Сборка аргументов запуска Java...");

        std::string java_exe = inst.java_path;
        if (java_exe.empty() || java_exe == "javaw.exe" || java_exe.find("javapath") != std::string::npos) {
            java_exe = settings.value("java_path", "");
        }
        if (java_exe.empty() || java_exe == "javaw.exe" || java_exe.find("javapath") != std::string::npos || !FsUtils::file_exists(java_exe)) {
            java_exe = SettingsManager::instance().get_recommended_java(inst.game_version);
        }
        if (java_exe.empty()) java_exe = "javaw.exe";

        int ram_min = inst.ram_min_mb > 0 ? inst.ram_min_mb : settings.value("ram_min_mb", 1024);
        int ram_max = inst.ram_max_mb > 0 ? inst.ram_max_mb : settings.value("ram_max_mb", 4096);
        std::string custom_jvm = !inst.jvm_args.empty() ? inst.jvm_args : settings.value("jvm_args", "");

        int res_w = settings.value("resolution_w", 1280);
        int res_h = settings.value("resolution_h", 720);
        bool fullscreen = settings.value("fullscreen", false);

        // Build classpath string
        std::string cp_string;
        for (size_t i = 0; i < classpath_entries.size(); ++i) {
            std::string p = classpath_entries[i];
            std::replace(p.begin(), p.end(), '/', '\\');
            cp_string += p;
            if (i + 1 < classpath_entries.size()) cp_string += ";";
        }

        // Write classpath to an args file to avoid Windows command-line 8192-char length limitation
        std::string cp_file = game_dir + "\\launch_cp.txt";
        {
            std::ofstream cp_out(cp_file);
            cp_out << "-cp\n" << cp_string << "\n";
        }

        std::ostringstream cmd;
        cmd << "\"" << java_exe << "\" ";
        cmd << "-Xms" << ram_min << "M ";
        cmd << "-Xmx" << ram_max << "M ";
        if (!custom_jvm.empty()) {
            cmd << custom_jvm << " ";
        }
        cmd << "-Djava.library.path=\"" << natives_dir << "\" ";
        cmd << "-Djna.tmpdir=\"" << natives_dir << "\" ";
        cmd << "-Dorg.lwjgl.system.SharedLibraryExtractPath=\"" << natives_dir << "\" ";
        cmd << "-Dio.netty.native.workdir=\"" << natives_dir << "\" ";
        cmd << "-Dminecraft.launcher.brand=\"zested launcher\" ";
        cmd << "-Dminecraft.launcher.version=\"2.0\" ";

        // Argument file for classpath
        cmd << "@\"" << cp_file << "\" ";

        // Main class
        cmd << main_class << " ";

        // Game args
        std::string user_name = acc.username.empty() ? "Player" : acc.username;
        std::string user_uuid = acc.uuid.empty() ? "00000000-0000-0000-0000-000000000000" : acc.uuid;
        std::string access_tok = acc.access_token.empty() ? "0" : acc.access_token;
        std::string user_type = (acc.type == "microsoft") ? "msa" : "mojang";

        cmd << "--username \"" << user_name << "\" ";
        cmd << "--version \"" << inst.game_version << "\" ";
        cmd << "--gameDir \"" << game_dir << "\" ";
        cmd << "--assetsDir \"" << assets_dir << "\" ";
        cmd << "--assetIndex \"" << asset_index << "\" ";
        cmd << "--uuid \"" << user_uuid << "\" ";
        cmd << "--accessToken \"" << access_tok << "\" ";
        cmd << "--userType \"" << user_type << "\" ";
        cmd << "--versionType \"zested launcher\" ";
        cmd << "--width " << res_w << " --height " << res_h << " ";
        if (fullscreen) {
            cmd << "--fullscreen ";
        }

        std::string final_cmd = cmd.str();

        // 7. Подготовка пайпов для захвата вывода и запуска процесса
        HANDLE hReadPipe = nullptr;
        HANDLE hWritePipe = nullptr;
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa, sizeof(sa));
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
        if (hReadPipe) {
            SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
        }

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        if (hWritePipe) {
            si.hStdOutput = hWritePipe;
            si.hStdError = hWritePipe;
            si.dwFlags |= STARTF_USESTDHANDLES;
        }
        ZeroMemory(&pi, sizeof(pi));

        std::vector<char> cmd_writable(final_cmd.begin(), final_cmd.end());
        cmd_writable.push_back('\0');

        BOOL success = CreateProcessA(
            nullptr,
            cmd_writable.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            game_dir.c_str(),
            &si,
            &pi
        );

        if (hWritePipe) {
            CloseHandle(hWritePipe);
        }

        if (!success) {
            DWORD err = GetLastError();
            if (hReadPipe) CloseHandle(hReadPipe);
            notify("error", 0, "Не удалось запустить процесс Java (код ошибки " + std::to_string(err) + ")", false, "Java launch failed");
            return;
        }

        // Обновляем дату запуска
        inst.last_played = (uint64_t)time(nullptr);
        InstanceManager::instance().save_instance_config(inst);

        // Поток чтения логов игры и запись в файл
        std::string log_file = game_dir + "\\logs\\launcher_game.log";
        std::vector<std::string> recent_lines;
        std::mutex lines_mutex;

        std::thread log_reader([hReadPipe, log_file, &recent_lines, &lines_mutex]() {
            if (!hReadPipe) return;
            std::ofstream out_log(log_file, std::ios::trunc);
            char buffer[2048];
            DWORD bytesRead = 0;
            std::string line_accum;

            while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                if (out_log.is_open()) {
                    out_log.write(buffer, bytesRead);
                    out_log.flush();
                }

                line_accum.append(buffer, bytesRead);
                size_t pos = 0;
                while ((pos = line_accum.find('\n')) != std::string::npos) {
                    std::string l = line_accum.substr(0, pos);
                    if (!l.empty() && l.back() == '\r') l.pop_back();
                    line_accum.erase(0, pos + 1);

                    std::lock_guard<std::mutex> lk(lines_mutex);
                    recent_lines.push_back(l);
                    if (recent_lines.size() > 50) recent_lines.erase(recent_lines.begin());
                }
            }
            CloseHandle(hReadPipe);
        });

        // Контроль первых 7 секунд работы для мгновенного отлова крашей
        DWORD wait_res = WaitForSingleObject(pi.hProcess, 6000);
        if (wait_res == WAIT_OBJECT_0) {
            // Процесс завершился раньше времени (краш)
            DWORD exit_code = 0;
            GetExitCodeProcess(pi.hProcess, &exit_code);

            if (log_reader.joinable()) log_reader.join();
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            std::string error_detail;
            {
                std::lock_guard<std::mutex> lk(lines_mutex);
                for (const auto& l : recent_lines) {
                    if (l.find("Exception") != std::string::npos ||
                        l.find("Error") != std::string::npos ||
                        l.find("FATAL") != std::string::npos ||
                        l.find("Caused by") != std::string::npos) {
                        error_detail = l;
                    }
                }
                if (error_detail.empty() && !recent_lines.empty()) {
                    error_detail = recent_lines.back();
                }
            }

            if (error_detail.empty()) {
                error_detail = "Процесс завершился с кодом " + std::to_string(exit_code);
            }

            notify("error", 0, "Minecraft завершился с ошибкой: " + error_detail, false, error_detail);
            return;
        }

        // Игра успешно стартовала и работает
        notify("running", 100, "Minecraft успешно запущен! Приятной игры.", true);

        // Ждем окончательного закрытия игры пользователем
        WaitForSingleObject(pi.hProcess, INFINITE);

        if (log_reader.joinable()) log_reader.join();
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        notify("idle", 0, "Игра завершена.", false);
    }

    static void extract_jar_natives(const std::string& jar_path, const std::string& out_dir) {
        // Windows bsdtar extracts archive without needing --wildcards
        std::string cmd = "tar.exe -xf \"" + jar_path + "\" -C \"" + out_dir + "\" 2>nul";
        system(cmd.c_str());
    }

    static std::string maven_name_to_path(const std::string& name) {
        std::vector<std::string> parts;
        std::stringstream ss(name);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }
        if (parts.size() < 3) return "";

        std::string group = parts[0];
        std::string artifact = parts[1];
        std::string version = parts[2];
        std::string classifier = parts.size() > 3 ? ("-" + parts[3]) : "";

        std::replace(group.begin(), group.end(), '.', '\\');
        return group + "\\" + artifact + "\\" + version + "\\" + artifact + "-" + version + classifier + ".jar";
    }
};

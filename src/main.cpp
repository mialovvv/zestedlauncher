#include <windows.h>
#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <nlohmann/json.hpp>
#include "webview.h"

#include "fs_utils.hpp"
#include "http_client.hpp"
#include "settings_manager.hpp"
#include "auth_manager.hpp"
#include "mojang_api.hpp"
#include "modrinth_api.hpp"
#include "instance_manager.hpp"
#include "game_launcher.hpp"
#include "ui_resources.hpp"

using json = nlohmann::json;

class LauncherApp {
public:
    LauncherApp() : w_(false, nullptr) {}

    void run() {
        w_.set_title("zested launcher");
        w_.set_size(1120, 740, WEBVIEW_HINT_NONE);

        // Customize window style: remove default Windows caption and thick borders for sleek frameless look
        auto win = w_.window();
        if (win.has_value()) {
            HWND hwnd = (HWND)win.value();
            if (hwnd) {
                LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
                style &= ~WS_CAPTION;
                style &= ~WS_THICKFRAME;
                style |= WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
                SetWindowLongPtr(hwnd, GWL_STYLE, style);

                // Center window on primary monitor
                int screenW = GetSystemMetrics(SM_CXSCREEN);
                int screenH = GetSystemMetrics(SM_CYSCREEN);
                int winW = 1120;
                int winH = 740;
                int posX = (screenW - winW) / 2;
                int posY = (screenH - winH) / 2;

                SetWindowPos(hwnd, nullptr, posX, posY, winW, winH, SWP_NOZORDER | SWP_FRAMECHANGED);

                // Round window corners (12px radius = 24px diameter)
                HRGN rgn = CreateRoundRectRgn(0, 0, winW + 1, winH + 1, 24, 24);
                SetWindowRgn(hwnd, rgn, TRUE);

                // Set Windows 11 DWM rounded corner preference if supported
                HMODULE hDwmapi = LoadLibraryA("dwmapi.dll");
                if (hDwmapi) {
                    typedef HRESULT (WINAPI *DwmSetWindowAttributeFunc)(HWND, DWORD, LPCVOID, DWORD);
                    auto pDwmSetWindowAttribute = (DwmSetWindowAttributeFunc)GetProcAddress(hDwmapi, "DwmSetWindowAttribute");
                    if (pDwmSetWindowAttribute) {
                        DWORD preference = 2; // DWMWCP_ROUND
                        pDwmSetWindowAttribute(hwnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &preference, sizeof(preference));
                    }
                    FreeLibrary(hDwmapi);
                }
            }
        }

        // Bind native bridge handler
        w_.bind("callNativeBridge", [this](const std::string& req) -> std::string {
            try {
                handle_client_message(req);
            } catch (const std::exception& e) {
                std::cerr << "Error handling message: " << e.what() << std::endl;
            }
            return "ok";
        });

        // Initialize webview HTML
        w_.set_html(UI::INDEX_HTML);

        w_.run();
    }

private:
    webview::webview w_;

    void post_to_ui(const std::string& type, const json& payload) {
        json msg;
        msg["type"] = type;
        msg["payload"] = payload;
        std::string script = "window.onNativeMessage(" + msg.dump() + ");";
        w_.dispatch([this, script]() {
            w_.eval(script);
        });
    }

    void send_instance_details(const std::string& id) {
        InstanceConfig inst = InstanceManager::instance().get_instance_by_id(id);
        json p;
        p["id"] = inst.id;
        p["name"] = inst.name;
        p["icon"] = inst.icon;
        p["game_version"] = inst.game_version;
        p["loader_type"] = inst.loader_type;
        p["loader_version"] = inst.loader_version;
        p["ram_min_mb"] = inst.ram_min_mb;
        p["ram_max_mb"] = inst.ram_max_mb;
        p["java_path"] = inst.java_path;
        p["jvm_args"] = inst.jvm_args;
        p["mods"] = InstanceManager::instance().get_instance_mods_json(id);

        std::thread([this, p, inst]() {
            auto loaders = MojangApi::instance().get_fabric_loaders_for_game(inst.game_version);
            json p_copy = p;
            p_copy["fabric_loaders"] = loaders;
            post_to_ui("instance_details", p_copy);
        }).detach();
    }

    void send_init_data() {
        json payload;
        payload["instances"] = InstanceManager::instance().get_instances_json();
        payload["accounts"] = AuthManager::instance().get_accounts_json();
        payload["settings"] = SettingsManager::instance().get_json();
        payload["system_ram_mb"] = SettingsManager::get_total_system_ram_mb();
        auto res = SettingsManager::get_primary_resolution();
        payload["system_res_w"] = res.first;
        payload["system_res_h"] = res.second;

        json javaList = json::array();
        for (const auto& j : SettingsManager::detect_java_installations()) {
            json item;
            item["path"] = j.path;
            item["version"] = j.version;
            javaList.push_back(item);
        }
        payload["javaList"] = javaList;

        post_to_ui("init_data", payload);
    }

    void handle_client_message(const std::string& req_str) {
        json req = json::parse(req_str);
        if (req.is_array() && !req.empty()) {
            if (req[0].is_string()) {
                req = json::parse(req[0].get<std::string>());
            } else {
                req = req[0];
            }
        }

        std::string action = req.value("action", "");
        json payload = req.value("payload", json::object());

        if (action == "request_init") {
            send_init_data();
        }
        else if (action == "window_drag") {
            auto win = w_.window();
            if (win.has_value()) {
                HWND hwnd = (HWND)win.value();
                if (hwnd) {
                    ReleaseCapture();
                    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                }
            }
        }
        else if (action == "window_minimize") {
            auto win = w_.window();
            if (win.has_value()) {
                HWND hwnd = (HWND)win.value();
                if (hwnd) ShowWindow(hwnd, SW_MINIMIZE);
            }
        }
        else if (action == "window_close") {
            auto win = w_.window();
            if (win.has_value()) {
                HWND hwnd = (HWND)win.value();
                if (hwnd) PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            w_.terminate();
        }
        else if (action == "select_instance") {
            std::string id = payload.value("id", "");
            SettingsManager::instance().update({{"active_instance", id}});
            send_init_data();
        }
        else if (action == "create_instance") {
            std::string name = payload.value("name", "New Instance");
            std::string ver = payload.value("game_version", "1.21.1");
            std::string loader = payload.value("loader_type", "vanilla");
            auto inst = InstanceManager::instance().create_instance(name, ver, loader);
            SettingsManager::instance().update({{"active_instance", inst.id}});
            send_init_data();
            post_to_ui("toast", {{"message", "Сборка '" + inst.name + "' создана!"}});
        }
        else if (action == "delete_instance") {
            std::string id = payload.value("id", "");
            InstanceManager::instance().delete_instance(id);
            send_init_data();
            post_to_ui("toast", {{"message", "Сборка удалена."}});
        }
        else if (action == "open_instance_folder") {
            std::string id = payload.value("id", "");
            std::string path = InstanceManager::instance().get_instance_path(id);
            FsUtils::open_in_explorer(path);
        }
        else if (action == "get_instance_details") {
            std::string id = payload.value("id", "");
            if (id.empty()) id = SettingsManager::instance().get_json().value("active_instance", "");
            send_instance_details(id);
        }
        else if (action == "save_instance_config") {
            std::string id = payload.value("id", "");
            if (!id.empty()) {
                InstanceConfig inst = InstanceManager::instance().get_instance_by_id(id);
                if (payload.contains("name")) inst.name = payload["name"].get<std::string>();
                if (payload.contains("game_version")) inst.game_version = payload["game_version"].get<std::string>();
                if (payload.contains("loader_type")) inst.loader_type = payload["loader_type"].get<std::string>();
                if (payload.contains("loader_version")) inst.loader_version = payload["loader_version"].get<std::string>();
                if (payload.contains("ram_min_mb")) inst.ram_min_mb = payload["ram_min_mb"].get<int>();
                if (payload.contains("ram_max_mb")) inst.ram_max_mb = payload["ram_max_mb"].get<int>();
                if (payload.contains("java_path")) inst.java_path = payload["java_path"].get<std::string>();
                if (payload.contains("jvm_args")) inst.jvm_args = payload["jvm_args"].get<std::string>();

                InstanceManager::instance().save_instance_config(inst);
                send_init_data();
                send_instance_details(id);
                post_to_ui("toast", {{"message", "Параметры сборки '" + inst.name + "' сохранены!"}});
            }
        }
        else if (action == "open_mods_folder") {
            std::string id = payload.value("id", "");
            if (id.empty()) id = SettingsManager::instance().get_json().value("active_instance", "");
            std::string path = InstanceManager::instance().get_instance_path(id) + "\\mods";
            FsUtils::create_directories(path);
            FsUtils::open_in_explorer(path);
        }
        else if (action == "delete_instance_mod") {
            std::string id = payload.value("id", "");
            std::string filename = payload.value("filename", "");
            if (!id.empty() && !filename.empty()) {
                InstanceManager::instance().delete_mod(id, filename);
                send_init_data();
                send_instance_details(id);
                post_to_ui("toast", {{"message", "Мод удален: " + filename}});
            }
        }
        else if (action == "toggle_instance_mod") {
            std::string id = payload.value("id", "");
            std::string filename = payload.value("filename", "");
            if (!id.empty() && !filename.empty()) {
                InstanceManager::instance().toggle_mod(id, filename);
                send_init_data();
                send_instance_details(id);
            }
        }
        else if (action == "add_mods_dialog") {
            std::string id = payload.value("id", "");
            if (id.empty()) id = SettingsManager::instance().get_json().value("active_instance", "");
            if (!id.empty()) {
                char filename_buf[8192] = {0};
                OPENFILENAMEA ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                HWND hwnd = nullptr;
                auto win = w_.window();
                if (win.has_value()) hwnd = (HWND)win.value();
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = "Minecraft Mods (*.jar;*.zip)\0*.jar;*.zip\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = filename_buf;
                ofn.nMaxFile = sizeof(filename_buf);
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

                if (GetOpenFileNameA(&ofn)) {
                    std::string target_dir = InstanceManager::instance().get_instance_path(id) + "\\mods";
                    FsUtils::create_directories(target_dir);

                    char* p = filename_buf;
                    std::string dir = p;
                    p += dir.length() + 1;
                    int added = 0;

                    if (*p == '\0') {
                        // Single file selected
                        std::string src = dir;
                        std::string fname = fs::path(FsUtils::u8path(src)).filename().string();
                        std::string dst = target_dir + "\\" + fname;
                        std::error_code ec;
                        fs::copy_file(FsUtils::u8path(src), FsUtils::u8path(dst), fs::copy_options::overwrite_existing, ec);
                        if (!ec) added++;
                    } else {
                        // Multiple files selected
                        while (*p != '\0') {
                            std::string fname = p;
                            std::string src = dir + "\\" + fname;
                            std::string dst = target_dir + "\\" + fname;
                            std::error_code ec;
                            fs::copy_file(FsUtils::u8path(src), FsUtils::u8path(dst), fs::copy_options::overwrite_existing, ec);
                            if (!ec) added++;
                            p += fname.length() + 1;
                        }
                    }

                    send_init_data();
                    send_instance_details(id);
                    post_to_ui("toast", {{"message", "Успешно добавлено модов: " + std::to_string(added)}});
                }
            }
        }
        else if (action == "get_fabric_loaders") {
            std::string gv = payload.value("game_version", "1.21.1");
            std::thread([this, gv]() {
                auto loaders = MojangApi::instance().get_fabric_loaders_for_game(gv);
                post_to_ui("fabric_loaders_result", {{"game_version", gv}, {"loaders", loaders}});
            }).detach();
        }
        else if (action == "save_settings") {
            SettingsManager::instance().update(payload);
            std::string active_id = SettingsManager::instance().get_json().value("active_instance", "");
            if (!active_id.empty() && payload.contains("ram_max_mb")) {
                InstanceConfig inst = InstanceManager::instance().get_instance_by_id(active_id);
                inst.ram_max_mb = payload["ram_max_mb"].get<int>();
                if (payload.contains("java_path")) inst.java_path = payload["java_path"].get<std::string>();
                if (payload.contains("jvm_args")) inst.jvm_args = payload["jvm_args"].get<std::string>();
                InstanceManager::instance().save_instance_config(inst);
            }
            send_init_data();
            post_to_ui("toast", {{"message", "Параметры успешно применены!"}});
        }
        else if (action == "browse_java") {
            char filename[MAX_PATH] = {0};
            OPENFILENAMEA ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            HWND hwnd = nullptr;
            auto win = w_.window();
            if (win.has_value()) hwnd = (HWND)win.value();
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = "Java Executable (javaw.exe; java.exe)\0javaw.exe;java.exe\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameA(&ofn)) {
                post_to_ui("java_selected", {{"path", std::string(filename)}});
            }
        }
        else if (action == "select_account") {
            std::string id = payload.value("id", "");
            AuthManager::instance().select_account(id);
            send_init_data();
        }
        else if (action == "delete_account") {
            std::string id = payload.value("id", "");
            AuthManager::instance().delete_account(id);
            send_init_data();
        }
        else if (action == "add_offline_account") {
            std::string name = payload.value("username", "Player");
            AuthManager::instance().add_offline_account(name);
            send_init_data();
            post_to_ui("toast", {{"message", "Аккаунт '" + name + "' добавлен."}});
        }
        else if (action == "start_ms_login") {
            static std::atomic<bool> is_logging_in{false};
            if (is_logging_in.load()) {
                post_to_ui("toast", {{"message", "Окно авторизации Microsoft уже открыто."}});
                return;
            }
            is_logging_in.store(true);
            std::thread([this]() {
                try {
                    bool captured = false;
                    std::string captured_code;

                    webview::webview login_w(false, nullptr);
                    login_w.set_title("Вход в аккаунт Microsoft");
                    login_w.set_size(520, 680, WEBVIEW_HINT_NONE);

                    auto win = login_w.window();
                    if (win.has_value()) {
                        HWND hwnd = (HWND)win.value();
                        if (hwnd) {
                            int screenW = GetSystemMetrics(SM_CXSCREEN);
                            int screenH = GetSystemMetrics(SM_CYSCREEN);
                            int winW = 520;
                            int winH = 680;
                            int posX = (screenW - winW) / 2;
                            int posY = (screenH - winH) / 2;
                            SetWindowPos(hwnd, nullptr, posX, posY, winW, winH, SWP_NOZORDER);
                        }
                    }

                    login_w.bind("onLoginCallback", [&](const std::string& req) -> std::string {
                        std::string url = req;
                        if (!url.empty() && url.front() == '"' && url.back() == '"') {
                            url = url.substr(1, url.length() - 2);
                        }
                        size_t pos = url.find("code=");
                        if (pos != std::string::npos) {
                            std::string code = url.substr(pos + 5);
                            size_t end_pos = code.find('&');
                            if (end_pos != std::string::npos) code = code.substr(0, end_pos);
                            captured_code = code;
                            captured = true;
                        }
                        login_w.terminate();
                        return "ok";
                    });

                    login_w.init(R"js(
                        function checkRedirect() {
                            try {
                                var href = window.location.href;
                                if (href.indexOf("oauth20_desktop.srf") !== -1 && href.indexOf("code=") !== -1) {
                                    window.onLoginCallback(href);
                                }
                            } catch(e) {}
                        }
                        window.addEventListener("DOMContentLoaded", checkRedirect);
                        setInterval(checkRedirect, 250);
                    )js");

                    std::string oauth_url = "https://login.live.com/oauth20_authorize.srf"
                                            "?client_id=00000000402b5328"
                                            "&response_type=code"
                                            "&redirect_uri=https://login.live.com/oauth20_desktop.srf"
                                            "&scope=service::user.auth.xboxlive.com::MBI_SSL";

                    login_w.navigate(oauth_url);
                    post_to_ui("toast", {{"message", "Открыто окно авторизации Microsoft..."}});
                    login_w.run();

                    if (captured && !captured_code.empty()) {
                        post_to_ui("toast", {{"message", "Авторизация в службах Minecraft..."}});
                        auto res = AuthManager::instance().authenticate_with_oauth_code(captured_code);
                        json p;
                        p["status"] = res.status;
                        p["message"] = res.message;
                        if (res.status == "success") {
                            p["account"]["username"] = res.account.username;
                            send_init_data();
                        }
                        post_to_ui("ms_auth_result", p);
                    } else {
                        post_to_ui("ms_auth_result", {{"status", "cancelled"}, {"message", "Окно входа закрыто."}});
                    }
                } catch (const std::exception& e) {
                    post_to_ui("ms_auth_result", {{"status", "error"}, {"message", e.what()}});
                }
                is_logging_in.store(false);
            }).detach();
        }
        else if (action == "poll_ms_login") {
            std::string device_code = payload.value("device_code", "");
            std::thread([this, device_code]() {
                auto res = AuthManager::instance().poll_microsoft_auth(device_code);
                json p;
                p["status"] = res.status;
                p["message"] = res.message;
                if (res.status == "success") {
                    p["account"]["username"] = res.account.username;
                    send_init_data();
                }
                post_to_ui("ms_auth_result", p);
            }).detach();
        }
        else if (action == "search_modpacks") {
            std::string q = payload.value("query", "");
            std::thread([this, q]() {
                auto packs = ModrinthApi::instance().search_modpacks(q);
                json list = json::array();
                for (const auto& item : packs) {
                    json j;
                    j["id"] = item.id;
                    j["slug"] = item.slug;
                    j["title"] = item.title;
                    j["description"] = item.description;
                    j["icon_url"] = item.icon_url;
                    j["author"] = item.author;
                    j["downloads"] = item.downloads;
                    j["categories"] = item.categories;
                    list.push_back(j);
                }
                post_to_ui("search_results", {{"target", "modpacks"}, {"results", list}});
            }).detach();
        }
        else if (action == "search_mods") {
            std::string q = payload.value("query", "");
            std::thread([this, q]() {
                auto mods = ModrinthApi::instance().search_mods(q);
                json list = json::array();
                for (const auto& item : mods) {
                    json j;
                    j["id"] = item.id;
                    j["slug"] = item.slug;
                    j["title"] = item.title;
                    j["description"] = item.description;
                    j["icon_url"] = item.icon_url;
                    j["author"] = item.author;
                    j["downloads"] = item.downloads;
                    j["categories"] = item.categories;
                    list.push_back(j);
                }
                post_to_ui("search_results", {{"target", "mods"}, {"results", list}});
            }).detach();
        }
        else if (action == "install_mod") {
            std::string project_id = payload.value("project_id", "");
            std::string title = payload.value("title", "Mod");
            std::string active_id = SettingsManager::instance().get_json().value("active_instance", "");
            InstanceConfig inst = InstanceManager::instance().get_instance_by_id(active_id);

            std::thread([this, project_id, title, inst]() {
                auto versions = ModrinthApi::instance().get_project_versions(project_id, inst.game_version, inst.loader_type);
                if (versions.empty()) {
                    versions = ModrinthApi::instance().get_project_versions(project_id, inst.game_version);
                }
                if (versions.empty()) {
                    versions = ModrinthApi::instance().get_project_versions(project_id);
                }

                if (!versions.empty() && !versions[0].files.empty()) {
                    auto file = versions[0].files[0];
                    std::string dest = InstanceManager::instance().get_instance_path(inst.id) + "\\mods\\" + file.filename;
                    bool ok = HttpClient::instance().download_file(file.url, dest);
                    if (ok) {
                        post_to_ui("toast", {{"message", "Мод '" + title + "' успешно установлен!"}});
                        send_init_data();
                    } else {
                        post_to_ui("toast", {{"message", "Ошибка скачивания файла мода."}});
                    }
                } else {
                    post_to_ui("toast", {{"message", "Не найдена совместимая версия мода."}});
                }
            }).detach();
        }
        else if (action == "install_modpack") {
            std::string project_id = payload.value("project_id", "");
            std::string title = payload.value("title", "Modpack");

            std::thread([this, project_id, title]() {
                post_to_ui("toast", {{"message", "Поиск версии сборки '" + title + "'..."}});
                auto versions = ModrinthApi::instance().get_project_versions(project_id);
                if (versions.empty()) {
                    post_to_ui("toast", {{"message", "Не удалось найти версии сборки."}});
                    return;
                }

                // Pick the first version that has files
                ModrinthVersion target_ver;
                bool found_ver = false;
                for (const auto& v : versions) {
                    if (!v.files.empty()) {
                        target_ver = v;
                        found_ver = true;
                        break;
                    }
                }

                if (!found_ver) {
                    post_to_ui("toast", {{"message", "Файлы сборки не найдены."}});
                    return;
                }

                std::string mrpack_url = target_ver.files[0].url;
                std::string temp_dir = FsUtils::get_appdata_dir() + "\\temp\\pack_" + std::to_string(time(nullptr));
                FsUtils::create_directories(temp_dir);
                std::string mrpack_file = temp_dir + "\\pack.mrpack";

                post_to_ui("toast", {{"message", "Скачивание архива сборки '" + title + "'..."}});
                bool dl_ok = HttpClient::instance().download_file(mrpack_url, mrpack_file);
                if (!dl_ok) {
                    FsUtils::delete_file_or_dir(temp_dir);
                    post_to_ui("toast", {{"message", "Ошибка скачивания файла сборки."}});
                    return;
                }

                // Extract with tar.exe
                std::string cmd = "tar.exe -xf \"" + mrpack_file + "\" -C \"" + temp_dir + "\" 2>nul";
                system(cmd.c_str());

                std::string index_path = temp_dir + "\\modrinth.index.json";
                if (!FsUtils::file_exists(index_path)) {
                    FsUtils::delete_file_or_dir(temp_dir);
                    post_to_ui("toast", {{"message", "Неверный формат пакета сборки."}});
                    return;
                }

                try {
                    std::string idx_str = FsUtils::read_file_string(index_path);
                    json idx = json::parse(idx_str);

                    std::string pack_name = idx.value("name", title);
                    std::string gv = "1.21.1";
                    std::string loader = "fabric";
                    std::string loader_ver = "";

                    if (idx.contains("dependencies")) {
                        auto deps = idx["dependencies"];
                        if (deps.contains("minecraft")) gv = deps["minecraft"].get<std::string>();
                        if (deps.contains("fabric-loader")) {
                            loader = "fabric";
                            loader_ver = deps["fabric-loader"].get<std::string>();
                        } else if (deps.contains("neoforge")) {
                            loader = "neoforge";
                            loader_ver = deps["neoforge"].get<std::string>();
                        } else if (deps.contains("forge")) {
                            loader = "forge";
                            loader_ver = deps["forge"].get<std::string>();
                        } else if (deps.contains("quilt-loader")) {
                            loader = "quilt";
                            loader_ver = deps["quilt-loader"].get<std::string>();
                        }
                    }

                    auto new_inst = InstanceManager::instance().create_instance(pack_name, gv, loader, loader_ver);
                    std::string inst_path = InstanceManager::instance().get_instance_path(new_inst.id);

                    // Copy overrides
                    std::string overrides_dir = temp_dir + "\\overrides";
                    if (FsUtils::file_exists(overrides_dir)) {
                        std::error_code ec;
                        fs::copy(FsUtils::u8path(overrides_dir), FsUtils::u8path(inst_path),
                                 fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
                    }
                    std::string client_overrides_dir = temp_dir + "\\client-overrides";
                    if (FsUtils::file_exists(client_overrides_dir)) {
                        std::error_code ec;
                        fs::copy(FsUtils::u8path(client_overrides_dir), FsUtils::u8path(inst_path),
                                 fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
                    }

                    // Download mods
                    if (idx.contains("files") && idx["files"].is_array()) {
                        auto files = idx["files"];
                        int total = (int)files.size();
                        int cur = 0;
                        post_to_ui("toast", {{"message", "Загрузка файлов сборки (0/" + std::to_string(total) + ")..."}});

                        for (const auto& f : files) {
                            cur++;
                            std::string rel_path = f.value("path", "");
                            if (rel_path.empty()) continue;

                            std::string file_url = "";
                            if (f.contains("downloads") && f["downloads"].is_array() && !f["downloads"].empty()) {
                                file_url = f["downloads"][0].get<std::string>();
                            }
                            if (file_url.empty()) continue;

                            std::string dest_path = inst_path + "\\" + rel_path;
                            std::error_code ec;
                            fs::create_directories(fs::path(FsUtils::u8path(dest_path)).parent_path(), ec);

                            HttpClient::instance().download_file(file_url, dest_path);

                            if (cur % 5 == 0 || cur == total) {
                                post_to_ui("toast", {{"message", "Загрузка файлов сборки (" + std::to_string(cur) + "/" + std::to_string(total) + ")..."}});
                            }
                        }
                    }

                    FsUtils::delete_file_or_dir(temp_dir);
                    send_init_data();
                    post_to_ui("toast", {{"message", "Сборка '" + pack_name + "' успешно установлена!"}});
                } catch (const std::exception& e) {
                    FsUtils::delete_file_or_dir(temp_dir);
                    post_to_ui("toast", {{"message", std::string("Ошибка установки: ") + e.what()}});
                }
            }).detach();
        }
        else if (action == "get_versions") {
            std::thread([this]() {
                auto versions = MojangApi::instance().get_versions();
                json list = json::array();
                for (const auto& v : versions) {
                    if (v.type == "release") {
                        json j;
                        j["id"] = v.id;
                        j["type"] = v.type;
                        j["release_time"] = v.release_time;
                        list.push_back(j);
                    }
                }
                post_to_ui("versions_list", list);
            }).detach();
        }
        else if (action == "launch_game") {
            std::string active_id = SettingsManager::instance().get_json().value("active_instance", "");
            GameLauncher::instance().launch(active_id, [this](const GameLauncher::LaunchStatus& status) {
                json p;
                p["stage"] = status.stage;
                p["progress"] = status.progress;
                p["details"] = status.details;
                p["is_running"] = status.is_running;
                p["error_message"] = status.error_message;
                post_to_ui("launch_status", p);
            });
        }
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LauncherApp app;
    app.run();
    return 0;
}

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include "fs_utils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct JavaInstallation {
    std::string path;
    std::string version;
    int major_version = 0;
};

class SettingsManager {
public:
    static SettingsManager& instance() {
        static SettingsManager inst;
        return inst;
    }

    SettingsManager() {
        config_path_ = FsUtils::get_appdata_dir() + "\\config.json";
        load();
    }

    void load() {
        if (!FsUtils::file_exists(config_path_)) {
            set_defaults();
            save();
            return;
        }

        try {
            std::string content = FsUtils::read_file_string(config_path_);
            data_ = json::parse(content);
        } catch (...) {
            set_defaults();
            save();
        }

        // Validate essential fields
        if (!data_.contains("ram_min_mb")) data_["ram_min_mb"] = 1024;
        if (!data_.contains("ram_max_mb")) data_["ram_max_mb"] = 4096;
        if (!data_.contains("java_path") || data_["java_path"].get<std::string>().empty()) {
            auto javas = detect_java_installations();
            if (!javas.empty()) {
                data_["java_path"] = javas[0].path;
            } else {
                data_["java_path"] = "javaw.exe";
            }
        }
        if (!data_.contains("jvm_args")) data_["jvm_args"] = "-XX:+UseG1GC -Dsun.rmi.dgc.server.gcInterval=2147483646 -XX:+UnlockExperimentalVMOptions -XX:+OptimizeStringConcat";
        if (!data_.contains("resolution_w")) data_["resolution_w"] = 1280;
        if (!data_.contains("resolution_h")) data_["resolution_h"] = 720;
        if (!data_.contains("fullscreen")) data_["fullscreen"] = false;
        if (!data_.contains("active_account_id")) data_["active_account_id"] = "";
        if (!data_.contains("active_instance")) data_["active_instance"] = "";
    }

    void save() {
        FsUtils::write_file_string(config_path_, data_.dump(4));
    }

    void set_defaults() {
        data_ = json::object();
        data_["ram_min_mb"] = 1024;
        data_["ram_max_mb"] = 4096;
        data_["jvm_args"] = "-XX:+UseG1GC -Dsun.rmi.dgc.server.gcInterval=2147483646 -XX:+UnlockExperimentalVMOptions -XX:+OptimizeStringConcat";
        data_["resolution_w"] = 1280;
        data_["resolution_h"] = 720;
        data_["fullscreen"] = false;
        data_["active_account_id"] = "";
        data_["active_instance"] = "";

        auto javas = detect_java_installations();
        if (!javas.empty()) {
            data_["java_path"] = javas[0].path;
        } else {
            data_["java_path"] = "javaw.exe";
        }
    }

    json get_json() const {
        return data_;
    }

    void update(const json& update_data) {
        for (auto it = update_data.begin(); it != update_data.end(); ++it) {
            data_[it.key()] = it.value();
        }
        save();
    }

    static int get_total_system_ram_mb() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            return (int)(memInfo.ullTotalPhys / (1024 * 1024));
        }
        return 8192;
    }

    static std::pair<int, int> get_primary_resolution() {
        int w = GetSystemMetrics(SM_CXSCREEN);
        int h = GetSystemMetrics(SM_CYSCREEN);
        if (w <= 0 || h <= 0) { w = 1920; h = 1080; }
        return {w, h};
    }

    static int parse_java_major(const std::string& text) {
        // Find patterns like 21, 17, 8 in text
        for (int v : {26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 11, 8}) {
            std::string s = std::to_string(v);
            if (text.find("-" + s) != std::string::npos ||
                text.find("jdk-" + s) != std::string::npos ||
                text.find("java-" + s) != std::string::npos ||
                text.find("temurin-" + s) != std::string::npos ||
                text.find("zulu-" + s) != std::string::npos ||
                text.find("/" + s + "/") != std::string::npos ||
                text.find("\\" + s + "\\") != std::string::npos ||
                text.find(" " + s) != std::string::npos) {
                return v;
            }
        }
        if (text.find("1.8.") != std::string::npos || text.find("jre1.8") != std::string::npos) return 8;
        return 0;
    }

    static std::vector<JavaInstallation> detect_java_installations() {
        std::vector<JavaInstallation> result;
        std::string userprofile = getenv("USERPROFILE") ? getenv("USERPROFILE") : "";

        std::vector<std::string> search_dirs = {
            userprofile + "\\.jdks",
            "C:\\Program Files\\Java",
            "C:\\Program Files (x86)\\Java",
            "C:\\Program Files\\Eclipse Adoptium",
            "C:\\Program Files\\BellSoft",
            "C:\\Program Files\\Zulu",
            "C:\\Program Files\\Microsoft\\jdk",
            "C:\\Program Files (x86)\\Minecraft Launcher\\runtime",
            "C:\\Program Files\\Common Files\\Oracle\\Java\\javapath"
        };

        // Check JAVA_HOME
        char* java_home = getenv("JAVA_HOME");
        if (java_home && strlen(java_home) > 0) {
            std::string javaw = std::string(java_home) + "\\bin\\javaw.exe";
            if (FsUtils::file_exists(javaw)) {
                JavaInstallation item;
                item.path = javaw;
                item.version = "JAVA_HOME (" + std::string(java_home) + ")";
                item.major_version = parse_java_major(item.version);
                result.push_back(item);
            }
        }

        // Scan common install directories (prioritize javaw.exe)
        for (const auto& dir : search_dirs) {
            if (dir.empty() || !fs::exists(FsUtils::u8path(dir))) continue;
            try {
                for (const auto& entry : fs::recursive_directory_iterator(FsUtils::u8path(dir), fs::directory_options::skip_permission_denied)) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        if (filename == "javaw.exe") {
                            std::string p = entry.path().string();
                            bool exists = false;
                            for (const auto& existing : result) {
                                if (existing.path == p) {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists) {
                                JavaInstallation item;
                                item.path = p;
                                std::string dirName = entry.path().parent_path().filename().string();
                                if (dirName == "bin") {
                                    dirName = entry.path().parent_path().parent_path().filename().string();
                                }
                                item.version = dirName.empty() ? "Java" : dirName;
                                item.major_version = parse_java_major(dirName + " " + p);
                                result.push_back(item);
                            }
                        }
                    }
                }
            } catch (...) {}
        }

        // Also check PATH javaw.exe
        char pathBuf[MAX_PATH];
        if (SearchPathA(nullptr, "javaw.exe", nullptr, MAX_PATH, pathBuf, nullptr)) {
            std::string p = pathBuf;
            bool exists = false;
            for (const auto& existing : result) {
                if (existing.path == p) { exists = true; break; }
            }
            if (!exists) {
                JavaInstallation item;
                item.path = p;
                item.version = "Системная Java (javaw.exe)";
                item.major_version = parse_java_major(p);
                result.push_back(item);
            }
        }

        return result;
    }

    std::string get_recommended_java(const std::string& game_version) {
        auto list = detect_java_installations();
        if (list.empty()) return "javaw.exe";

        bool is_modern_21 = (game_version.find("1.21") != std::string::npos ||
                             game_version.find("1.20.5") != std::string::npos ||
                             game_version.find("1.20.6") != std::string::npos);
        bool is_mid_17 = (game_version.find("1.17") != std::string::npos ||
                          game_version.find("1.18") != std::string::npos ||
                          game_version.find("1.19") != std::string::npos ||
                          game_version.find("1.20") != std::string::npos);

        if (is_modern_21) {
            for (const auto& j : list) {
                if (j.major_version == 21) return j.path;
            }
        }

        if (is_mid_17 || is_modern_21) {
            for (const auto& j : list) {
                if (j.major_version == 17) return j.path;
            }
        }

        // For older MC (1.16.5 etc) prefer 17 or 8 over 26
        for (const auto& j : list) {
            if (j.major_version == 8 || j.major_version == 17) return j.path;
        }

        // Return first non-26 if possible, or first available
        for (const auto& j : list) {
            if (j.major_version > 0 && j.major_version <= 21) return j.path;
        }

        return list[0].path;
    }

private:
    std::string config_path_;
    json data_;
};

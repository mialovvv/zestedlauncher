#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "http_client.hpp"
#include "fs_utils.hpp"

using json = nlohmann::json;

struct VersionInfo {
    std::string id;
    std::string type; // "release", "snapshot", "old_beta", "old_alpha"
    std::string url;
    std::string release_time;
};

struct LibraryDownload {
    std::string name;
    std::string path;
    std::string url;
    uint64_t size = 0;
    std::string sha1;
};

class MojangApi {
public:
    static MojangApi& instance() {
        static MojangApi inst;
        return inst;
    }

    MojangApi() {
        cache_dir_ = FsUtils::get_appdata_dir() + "\\meta";
        FsUtils::create_directories(cache_dir_);
    }

    std::vector<VersionInfo> get_versions(bool force_refresh = false) {
        std::string manifest_file = cache_dir_ + "\\version_manifest_v2.json";
        std::string manifest_content;

        if (!force_refresh && FsUtils::file_exists(manifest_file)) {
            manifest_content = FsUtils::read_file_string(manifest_file);
        }

        if (manifest_content.empty()) {
            auto resp = HttpClient::instance().get("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json");
            if (resp.success) {
                manifest_content = resp.body;
                FsUtils::write_file_string(manifest_file, manifest_content);
            }
        }

        std::vector<VersionInfo> versions;
        if (manifest_content.empty()) return versions;

        try {
            json j = json::parse(manifest_content);
            if (j.contains("versions") && j["versions"].is_array()) {
                for (const auto& v : j["versions"]) {
                    VersionInfo info;
                    info.id = v.value("id", "");
                    info.type = v.value("type", "");
                    info.url = v.value("url", "");
                    info.release_time = v.value("releaseTime", "");
                    versions.push_back(info);
                }
            }
        } catch (...) {}

        return versions;
    }

    json get_version_json(const std::string& version_id, const std::string& manifest_url = "") {
        std::string version_dir = FsUtils::get_appdata_dir() + "\\versions\\" + version_id;
        FsUtils::create_directories(version_dir);
        std::string json_path = version_dir + "\\" + version_id + ".json";

        if (FsUtils::file_exists(json_path)) {
            try {
                return json::parse(FsUtils::read_file_string(json_path));
            } catch (...) {}
        }

        std::string target_url = manifest_url;
        if (target_url.empty()) {
            auto versions = get_versions();
            for (const auto& v : versions) {
                if (v.id == version_id) {
                    target_url = v.url;
                    break;
                }
            }
        }

        if (target_url.empty()) return json();

        auto resp = HttpClient::instance().get(target_url);
        if (resp.success) {
            FsUtils::write_file_string(json_path, resp.body);
            try {
                return json::parse(resp.body);
            } catch (...) {}
        }

        return json();
    }

    // Fabric Loader
    std::vector<std::string> get_fabric_loaders_for_game(const std::string& game_version) {
        std::vector<std::string> loaders;
        std::string url = "https://meta.fabricmc.net/v2/versions/loader/" + game_version;
        auto resp = HttpClient::instance().get(url, {"User-Agent: zestedlauncher/1.0"});
        if (resp.success) {
            try {
                json j = json::parse(resp.body);
                for (const auto& item : j) {
                    if (item.contains("loader") && item["loader"].contains("version")) {
                        loaders.push_back(item["loader"]["version"].get<std::string>());
                    }
                }
            } catch (...) {}
        }
        return loaders;
    }

    json get_fabric_profile_json(const std::string& game_version, const std::string& loader_version) {
        std::string url = "https://meta.fabricmc.net/v2/versions/loader/" + game_version + "/" + loader_version + "/profile/json";
        auto resp = HttpClient::instance().get(url, {"User-Agent: zestedlauncher/1.0"});
        if (resp.success) {
            try {
                return json::parse(resp.body);
            } catch (...) {}
        }
        return json();
    }

    static bool check_rule_allowed(const json& rules) {
        if (!rules.is_array() || rules.empty()) return true;

        bool allowed = false;
        for (const auto& rule : rules) {
            std::string action = rule.value("action", "allow");
            bool matches = true;

            if (rule.contains("os")) {
                std::string os_name = rule["os"].value("name", "");
                if (os_name != "windows") {
                    matches = false;
                }
            }

            if (matches) {
                allowed = (action == "allow");
            }
        }
        return allowed;
    }

private:
    std::string cache_dir_;
};

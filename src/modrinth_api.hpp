#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "http_client.hpp"

using json = nlohmann::json;

struct ModrinthProject {
    std::string id;
    std::string slug;
    std::string title;
    std::string description;
    std::string icon_url;
    std::string author;
    uint64_t downloads = 0;
    std::vector<std::string> categories;
    std::vector<std::string> versions;
};

struct ModrinthVersionFile {
    std::string url;
    std::string filename;
    bool primary = false;
    uint64_t size = 0;
    std::string sha1;
};

struct ModrinthVersion {
    std::string id;
    std::string project_id;
    std::string name;
    std::string version_number;
    std::vector<std::string> game_versions;
    std::vector<std::string> loaders;
    std::vector<ModrinthVersionFile> files;
};

class ModrinthApi {
public:
    static ModrinthApi& instance() {
        static ModrinthApi inst;
        return inst;
    }

    std::vector<ModrinthProject> search_modpacks(const std::string& query = "", int limit = 20, int offset = 0) {
        return search(query, "modpack", limit, offset);
    }

    std::vector<ModrinthProject> search_mods(const std::string& query = "", int limit = 20, int offset = 0) {
        return search(query, "mod", limit, offset);
    }

    std::vector<ModrinthProject> search(const std::string& query, const std::string& project_type, int limit, int offset) {
        std::vector<ModrinthProject> results;
        std::string url = "https://api.modrinth.com/v2/search?query=" + url_encode(query) +
                          "&facets=[[%22project_type:" + project_type + "%22]]" +
                          "&limit=" + std::to_string(limit) +
                          "&offset=" + std::to_string(offset) +
                          "&index=downloads";

        auto resp = HttpClient::instance().get(url, {"User-Agent: zestedlauncher/1.0.0 (contact@zested.org)"});
        if (!resp.success) return results;

        try {
            json j = json::parse(resp.body);
            if (j.contains("hits") && j["hits"].is_array()) {
                for (const auto& item : j["hits"]) {
                    ModrinthProject p;
                    p.id = item.value("project_id", "");
                    p.slug = item.value("slug", "");
                    p.title = item.value("title", "");
                    p.description = item.value("description", "");
                    p.icon_url = item.value("icon_url", "");
                    p.author = item.value("author", "");
                    p.downloads = item.value("downloads", 0ULL);
                    if (item.contains("categories") && item["categories"].is_array()) {
                        for (const auto& cat : item["categories"]) {
                            p.categories.push_back(cat.get<std::string>());
                        }
                    }
                    results.push_back(p);
                }
            }
        } catch (...) {}

        return results;
    }

    std::vector<ModrinthVersion> get_project_versions(const std::string& project_id, const std::string& game_version = "", const std::string& loader = "") {
        std::vector<ModrinthVersion> versions;
        std::string url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
        bool hasParam = false;
        if (!game_version.empty()) {
            url += "?game_versions=[\"" + game_version + "\"]";
            hasParam = true;
        }
        if (!loader.empty()) {
            url += (hasParam ? "&" : "?") + std::string("loaders=[\"") + loader + "\"]";
        }

        auto resp = HttpClient::instance().get(url, {"User-Agent: zestedlauncher/1.0.0 (contact@zested.org)"});
        if (!resp.success) return versions;

        try {
            json j = json::parse(resp.body);
            if (j.is_array()) {
                for (const auto& item : j) {
                    ModrinthVersion v;
                    v.id = item.value("id", "");
                    v.project_id = item.value("project_id", "");
                    v.name = item.value("name", "");
                    v.version_number = item.value("version_number", "");

                    if (item.contains("game_versions") && item["game_versions"].is_array()) {
                        for (const auto& gv : item["game_versions"]) {
                            v.game_versions.push_back(gv.get<std::string>());
                        }
                    }

                    if (item.contains("loaders") && item["loaders"].is_array()) {
                        for (const auto& l : item["loaders"]) {
                            v.loaders.push_back(l.get<std::string>());
                        }
                    }

                    if (item.contains("files") && item["files"].is_array()) {
                        for (const auto& f : item["files"]) {
                            ModrinthVersionFile vf;
                            vf.url = f.value("url", "");
                            vf.filename = f.value("filename", "");
                            vf.primary = f.value("primary", false);
                            vf.size = f.value("size", 0ULL);
                            if (f.contains("hashes") && f["hashes"].contains("sha1")) {
                                vf.sha1 = f["hashes"].value("sha1", "");
                            }
                            v.files.push_back(vf);
                        }
                    }
                    versions.push_back(v);
                }
            }
        } catch (...) {}

        return versions;
    }

private:
    static std::string url_encode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        for (char c : value) {
            if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << ((int)(unsigned char)c);
            }
        }
        return escaped.str();
    }
};

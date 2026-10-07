#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "fs_utils.hpp"
#include "settings_manager.hpp"

using json = nlohmann::json;

struct InstanceConfig {
    std::string id;
    std::string name;
    std::string icon;
    std::string game_version; // e.g. "1.21.1"
    std::string loader_type;  // "vanilla", "fabric", "forge", "neoforge", "quilt"
    std::string loader_version;
    int ram_min_mb = 1024;
    int ram_max_mb = 4096;
    std::string java_path;
    std::string jvm_args;
    uint64_t created_at = 0;
    uint64_t last_played = 0;
};

class InstanceManager {
public:
    static InstanceManager& instance() {
        static InstanceManager inst;
        return inst;
    }

    InstanceManager() {
        instances_dir_ = FsUtils::get_appdata_dir() + "\\instances";
        FsUtils::create_directories(instances_dir_);
        load_instances();
    }

    std::string get_instances_dir() const {
        return instances_dir_;
    }

    std::string get_instance_path(const std::string& id) const {
        return instances_dir_ + "\\" + id;
    }

    void load_instances() {
        instances_.clear();
        if (!FsUtils::file_exists(instances_dir_)) return;
        for (const auto& entry : fs::directory_iterator(FsUtils::u8path(instances_dir_))) {
            if (entry.is_directory()) {
                std::string cfg_path = entry.path().string() + "\\instance.json";
                if (FsUtils::file_exists(cfg_path)) {
                    try {
                        std::string content = FsUtils::read_file_string(cfg_path);
                        json j = json::parse(content);
                        InstanceConfig cfg;
                        cfg.id = j.value("id", entry.path().filename().string());
                        cfg.name = j.value("name", cfg.id);
                        cfg.icon = j.value("icon", "grass");
                        cfg.game_version = j.value("game_version", "1.21.1");
                        cfg.loader_type = j.value("loader_type", "vanilla");
                        cfg.loader_version = j.value("loader_version", "");
                        cfg.ram_min_mb = j.value("ram_min_mb", 1024);
                        cfg.ram_max_mb = j.value("ram_max_mb", 4096);
                        cfg.java_path = j.value("java_path", "");
                        cfg.jvm_args = j.value("jvm_args", "");
                        cfg.created_at = j.value("created_at", 0ULL);
                        cfg.last_played = j.value("last_played", 0ULL);
                        instances_.push_back(cfg);
                    } catch (...) {}
                }
            }
        }

        // If no instance exists, do not auto-create
    }

    InstanceConfig create_instance(const std::string& name, const std::string& game_version,
                                  const std::string& loader_type = "vanilla",
                                  const std::string& loader_version = "") {
        std::string id = generate_instance_id(name);
        std::string unique_id = id;
        int counter = 1;
        while (FsUtils::file_exists(instances_dir_ + "\\" + unique_id)) {
            unique_id = id + "_" + std::to_string(counter++);
        }

        std::string inst_path = instances_dir_ + "\\" + unique_id;
        FsUtils::create_directories(inst_path);
        FsUtils::create_directories(inst_path + "\\mods");
        FsUtils::create_directories(inst_path + "\\resourcepacks");
        FsUtils::create_directories(inst_path + "\\shaderpacks");
        FsUtils::create_directories(inst_path + "\\saves");

        InstanceConfig cfg;
        cfg.id = unique_id;
        cfg.name = name.empty() ? ("Minecraft " + game_version) : name;
        cfg.icon = "grass";
        cfg.game_version = game_version.empty() ? "1.21.1" : game_version;
        cfg.loader_type = loader_type.empty() ? "vanilla" : loader_type;
        cfg.loader_version = loader_version;
        cfg.ram_min_mb = SettingsManager::instance().get_json().value("ram_min_mb", 1024);
        cfg.ram_max_mb = SettingsManager::instance().get_json().value("ram_max_mb", 4096);
        cfg.java_path = SettingsManager::instance().get_json().value("java_path", "javaw.exe");
        cfg.jvm_args = SettingsManager::instance().get_json().value("jvm_args", "");
        cfg.created_at = (uint64_t)time(nullptr);
        cfg.last_played = 0;

        save_instance_config(cfg);
        instances_.push_back(cfg);

        // Always switch to the newly created instance
        SettingsManager::instance().update({{"active_instance", unique_id}});

        return cfg;
    }

    bool save_instance_config(const InstanceConfig& cfg) {
        std::string cfg_path = instances_dir_ + "\\" + cfg.id + "\\instance.json";
        json j;
        j["id"] = cfg.id;
        j["name"] = cfg.name;
        j["icon"] = cfg.icon;
        j["game_version"] = cfg.game_version;
        j["loader_type"] = cfg.loader_type;
        j["loader_version"] = cfg.loader_version;
        j["ram_min_mb"] = cfg.ram_min_mb;
        j["ram_max_mb"] = cfg.ram_max_mb;
        j["java_path"] = cfg.java_path;
        j["jvm_args"] = cfg.jvm_args;
        j["created_at"] = cfg.created_at;
        j["last_played"] = cfg.last_played;

        for (auto& item : instances_) {
            if (item.id == cfg.id) {
                item = cfg;
                break;
            }
        }

        return FsUtils::write_file_string(cfg_path, j.dump(4));
    }

    bool delete_instance(const std::string& id) {
        std::string inst_path = instances_dir_ + "\\" + id;
        bool deleted = FsUtils::delete_file_or_dir(inst_path);
        if (deleted) {
            for (auto it = instances_.begin(); it != instances_.end(); ++it) {
                if (it->id == id) {
                    instances_.erase(it);
                    break;
                }
            }
            std::string active = SettingsManager::instance().get_json().value("active_instance", "");
            if (active == id && !instances_.empty()) {
                SettingsManager::instance().update({{"active_instance", instances_[0].id}});
            }
        }
        return deleted;
    }

    std::vector<std::string> get_installed_mods(const std::string& instance_id) {
        std::vector<std::string> mods;
        std::string mods_dir = instances_dir_ + "\\" + instance_id + "\\mods";
        if (FsUtils::file_exists(mods_dir)) {
            for (const auto& entry : fs::directory_iterator(FsUtils::u8path(mods_dir))) {
                if (entry.is_regular_file()) {
                    mods.push_back(entry.path().filename().string());
                }
            }
        }
        return mods;
    }

    json get_instance_mods_json(const std::string& instance_id) {
        json j = json::array();
        std::string mods_dir = instances_dir_ + "\\" + instance_id + "\\mods";
        if (FsUtils::file_exists(mods_dir)) {
            for (const auto& entry : fs::directory_iterator(FsUtils::u8path(mods_dir))) {
                if (entry.is_regular_file()) {
                    std::string fname = entry.path().filename().string();
                    bool enabled = true;
                    if (fname.size() > 9 && fname.substr(fname.size() - 9) == ".disabled") {
                        enabled = false;
                    }
                    json m;
                    m["filename"] = fname;
                    m["name"] = fname;
                    m["is_enabled"] = enabled;
                    m["size"] = entry.file_size();
                    j.push_back(m);
                }
            }
        }
        return j;
    }

    bool toggle_mod(const std::string& instance_id, const std::string& filename) {
        std::string mods_dir = instances_dir_ + "\\" + instance_id + "\\mods";
        std::string current_path = mods_dir + "\\" + filename;
        if (!FsUtils::file_exists(current_path)) return false;

        std::string new_name;
        if (filename.size() > 9 && filename.substr(filename.size() - 9) == ".disabled") {
            new_name = filename.substr(0, filename.size() - 9);
        } else {
            new_name = filename + ".disabled";
        }
        std::string new_path = mods_dir + "\\" + new_name;
        std::error_code ec;
        fs::rename(FsUtils::u8path(current_path), FsUtils::u8path(new_path), ec);
        return !ec;
    }

    bool delete_mod(const std::string& instance_id, const std::string& filename) {
        std::string mod_file = instances_dir_ + "\\" + instance_id + "\\mods\\" + filename;
        return FsUtils::delete_file_or_dir(mod_file);
    }

    json get_instances_json() {
        json j = json::array();
        std::string active_id = SettingsManager::instance().get_json().value("active_instance", "");
        if (active_id.empty() && !instances_.empty()) {
            active_id = instances_[0].id;
            SettingsManager::instance().update({{"active_instance", active_id}});
        }

        for (const auto& inst : instances_) {
            json item;
            item["id"] = inst.id;
            item["name"] = inst.name;
            item["icon"] = inst.icon;
            item["game_version"] = inst.game_version;
            item["loader_type"] = inst.loader_type;
            item["loader_version"] = inst.loader_version;
            item["ram_min_mb"] = inst.ram_min_mb;
            item["ram_max_mb"] = inst.ram_max_mb;
            item["java_path"] = inst.java_path;
            item["jvm_args"] = inst.jvm_args;
            item["created_at"] = inst.created_at;
            item["last_played"] = inst.last_played;
            item["is_active"] = (inst.id == active_id);

            // count mods
            int mod_count = 0;
            std::string mods_dir = instances_dir_ + "\\" + inst.id + "\\mods";
            if (FsUtils::file_exists(mods_dir)) {
                for (const auto& entry : fs::directory_iterator(FsUtils::u8path(mods_dir))) {
                    if (entry.is_regular_file()) mod_count++;
                }
            }
            item["mod_count"] = mod_count;
            j.push_back(item);
        }
        return j;
    }

    InstanceConfig get_instance_by_id(const std::string& id) {
        for (const auto& inst : instances_) {
            if (inst.id == id) return inst;
        }
        if (!instances_.empty()) return instances_[0];
        InstanceConfig empty_inst;
        return empty_inst;
    }

private:
    std::string instances_dir_;
    std::vector<InstanceConfig> instances_;

    static std::string generate_instance_id(const std::string& name) {
        std::string clean;
        for (char c : name) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
                clean += (char)tolower(c);
            } else if (c == ' ' || c == '.') {
                if (!clean.empty() && clean.back() != '_') {
                    clean += '_';
                }
            }
        }
        while (!clean.empty() && clean.back() == '_') clean.pop_back();
        if (clean.empty()) {
            clean = "inst_" + std::to_string(time(nullptr));
        }
        return clean;
    }
};

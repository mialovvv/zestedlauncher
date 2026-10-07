#pragma once

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

class FsUtils {
public:
    static std::string get_appdata_dir() {
        char path[MAX_PATH];
        std::string res;
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            res = path;
        } else {
            char* appdata = getenv("APPDATA");
            if (appdata) {
                res = appdata;
            } else {
                res = "C:\\Users\\Default\\AppData\\Roaming";
            }
        }
        std::string new_dir = res + "\\.zestedlauncher";
        std::string old_dir = res + "\\.arefulauncher";
        if (!file_exists(new_dir) && file_exists(old_dir)) {
            std::error_code ec;
            fs::rename(u8path(old_dir), u8path(new_dir), ec);
        }
        res = new_dir;
        create_directories(res);
        return res;
    }

    static fs::path u8path(const std::string& path) {
        return fs::u8path(path);
    }

    static bool create_directories(const std::string& path) {
        std::error_code ec;
        return fs::create_directories(u8path(path), ec);
    }

    static bool file_exists(const std::string& path) {
        std::error_code ec;
        return fs::exists(u8path(path), ec);
    }

    static uintmax_t file_size(const std::string& path) {
        std::error_code ec;
        return fs::file_size(u8path(path), ec);
    }

    static std::string read_file_string(const std::string& path) {
        std::ifstream f(u8path(path), std::ios::in | std::ios::binary);
        if (!f.is_open()) return "";
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static bool write_file_string(const std::string& path, const std::string& content) {
        std::error_code ec;
        fs::path p = u8path(path);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path(), ec);
        }
        std::ofstream f(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!f.is_open()) return false;
        f.write(content.data(), content.size());
        return true;
    }

    static bool delete_file_or_dir(const std::string& path) {
        std::error_code ec;
        return fs::remove_all(u8path(path), ec) > 0;
    }

    static void open_in_explorer(const std::string& path) {
        ShellExecuteA(nullptr, "open", path.c_str(), nullptr, nullptr, SW_SHOW);
    }
};

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>
#include "http_client.hpp"
#include "fs_utils.hpp"
#include "settings_manager.hpp"

using json = nlohmann::json;

struct Account {
    std::string id;
    std::string type; // "microsoft" or "offline"
    std::string username;
    std::string uuid;
    std::string access_token;
    std::string refresh_token;
    std::string skin_url;
    uint64_t expires_at = 0;
};

class AuthManager {
public:
    static AuthManager& instance() {
        static AuthManager inst;
        return inst;
    }

    AuthManager() {
        accounts_file_ = FsUtils::get_appdata_dir() + "\\accounts.json";
        load();
    }

    void load() {
        accounts_.clear();
        if (!FsUtils::file_exists(accounts_file_)) {
            // Create default offline account if none
            add_offline_account("Player");
            return;
        }

        try {
            std::string content = FsUtils::read_file_string(accounts_file_);
            json j = json::parse(content);
            if (j.is_array()) {
                for (const auto& item : j) {
                    Account acc;
                    acc.id = item.value("id", "");
                    acc.type = item.value("type", "offline");
                    acc.username = item.value("username", "Player");
                    acc.uuid = item.value("uuid", "");
                    acc.access_token = item.value("access_token", "");
                    acc.refresh_token = item.value("refresh_token", "");
                    acc.skin_url = item.value("skin_url", "");
                    acc.expires_at = item.value("expires_at", 0ULL);
                    if (!acc.id.empty()) {
                        accounts_.push_back(acc);
                    }
                }
            }
        } catch (...) {}

        if (accounts_.empty()) {
            add_offline_account("Player");
        }
    }

    void save() {
        json j = json::array();
        for (const auto& acc : accounts_) {
            json item;
            item["id"] = acc.id;
            item["type"] = acc.type;
            item["username"] = acc.username;
            item["uuid"] = acc.uuid;
            item["access_token"] = acc.access_token;
            item["refresh_token"] = acc.refresh_token;
            item["skin_url"] = acc.skin_url;
            item["expires_at"] = acc.expires_at;
            j.push_back(item);
        }
        FsUtils::write_file_string(accounts_file_, j.dump(4));
    }

    json get_accounts_json() {
        json j = json::array();
        std::string active_id = SettingsManager::instance().get_json().value("active_account_id", "");
        if (active_id.empty() && !accounts_.empty()) {
            active_id = accounts_[0].id;
            SettingsManager::instance().update({{"active_account_id", active_id}});
        }

        for (const auto& acc : accounts_) {
            json item;
            item["id"] = acc.id;
            item["type"] = acc.type;
            item["username"] = acc.username;
            item["uuid"] = acc.uuid;
            item["skin_url"] = acc.skin_url.empty() ? 
                ("https://minotar.net/helm/" + acc.username + "/100.png") : acc.skin_url;
            item["is_active"] = (acc.id == active_id);
            j.push_back(item);
        }
        return j;
    }

    Account get_active_account() {
        std::string active_id = SettingsManager::instance().get_json().value("active_account_id", "");
        for (const auto& acc : accounts_) {
            if (acc.id == active_id) return acc;
        }
        if (!accounts_.empty()) return accounts_[0];

        Account fallback;
        fallback.id = "default";
        fallback.type = "offline";
        fallback.username = "Player";
        fallback.uuid = generate_offline_uuid("Player");
        return fallback;
    }

    Account add_offline_account(const std::string& username) {
        Account acc;
        acc.id = "offline_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        acc.type = "offline";
        acc.username = username.empty() ? "Player" : username;
        acc.uuid = generate_offline_uuid(acc.username);
        acc.skin_url = "https://minotar.net/helm/" + acc.username + "/100.png";
        acc.access_token = "0";

        accounts_.push_back(acc);
        SettingsManager::instance().update({{"active_account_id", acc.id}});
        save();
        return acc;
    }

    bool select_account(const std::string& id) {
        for (const auto& acc : accounts_) {
            if (acc.id == id) {
                SettingsManager::instance().update({{"active_account_id", id}});
                return true;
            }
        }
        return false;
    }

    bool delete_account(const std::string& id) {
        for (auto it = accounts_.begin(); it != accounts_.end(); ++it) {
            if (it->id == id) {
                accounts_.erase(it);
                save();
                if (!accounts_.empty()) {
                    SettingsManager::instance().update({{"active_account_id", accounts_[0].id}});
                } else {
                    add_offline_account("Player");
                }
                return true;
            }
        }
        return false;
    }

    // Microsoft Device Authorization flow
    struct DeviceCodeResponse {
        bool success = false;
        std::string device_code;
        std::string user_code;
        std::string verification_uri;
        int expires_in = 900;
        int interval = 5;
        std::string error;
    };

    DeviceCodeResponse start_microsoft_device_flow() {
        DeviceCodeResponse res;
        std::string url = "https://login.live.com/oauth20_connect.srf";
        std::string body = "client_id=00000000402b5328&scope=service::user.auth.xboxlive.com::MBI_SSL&response_type=device_code";

        auto resp = HttpClient::instance().post(url, body, {
            "Content-Type: application/x-www-form-urlencoded"
        });

        if (!resp.success) {
            res.error = resp.error.empty() ? ("HTTP " + std::to_string(resp.status_code)) : resp.error;
            return res;
        }

        try {
            json j = json::parse(resp.body);
            res.device_code = j.value("device_code", "");
            res.user_code = j.value("user_code", "");
            res.verification_uri = j.value("verification_uri", "https://www.microsoft.com/link");
            res.expires_in = j.value("expires_in", 900);
            res.interval = j.value("interval", 5);
            res.success = !res.device_code.empty();

            current_device_code_ = res.device_code;
        } catch (const std::exception& e) {
            res.error = e.what();
        }

        return res;
    }

    struct PollAuthResult {
        std::string status; // "pending", "success", "error", "expired"
        std::string message;
        Account account;
    };

    PollAuthResult poll_microsoft_auth(const std::string& device_code) {
        PollAuthResult result;
        std::string url = "https://login.live.com/oauth20_token.srf";
        std::string body = "client_id=00000000402b5328"
                           "&grant_type=urn:ietf:params:oauth:grant-type:device_code"
                           "&device_code=" + device_code;

        auto resp = HttpClient::instance().post(url, body, {
            "Content-Type: application/x-www-form-urlencoded"
        });

        if (!resp.success) {
            try {
                json j = json::parse(resp.body);
                std::string err = j.value("error", "");
                if (err == "authorization_pending") {
                    result.status = "pending";
                    result.message = "Ожидание подтверждения входа пользователем...";
                    return result;
                } else if (err == "authorization_declined") {
                    result.status = "error";
                    result.message = "Вход отклонен пользователем.";
                    return result;
                } else if (err == "bad_verification_code" || err == "expired_token") {
                    result.status = "expired";
                    result.message = "Код авторизации истек.";
                    return result;
                }
            } catch (...) {}
            result.status = "error";
            result.message = "Ошибка запроса авторизации: " + resp.body;
            return result;
        }

        return finish_msa_login(resp.body);
    }

    PollAuthResult authenticate_with_oauth_code(const std::string& code) {
        PollAuthResult result;
        std::string url = "https://login.live.com/oauth20_token.srf";
        std::string body = "client_id=00000000402b5328"
                           "&grant_type=authorization_code"
                           "&code=" + code +
                           "&redirect_uri=https://login.live.com/oauth20_desktop.srf"
                           "&scope=service::user.auth.xboxlive.com::MBI_SSL";

        auto resp = HttpClient::instance().post(url, body, {
            "Content-Type: application/x-www-form-urlencoded"
        });

        if (!resp.success) {
            result.status = "error";
            result.message = "Ошибка авторизации Microsoft: " + resp.body;
            return result;
        }

        return finish_msa_login(resp.body);
    }

    PollAuthResult finish_msa_login(const std::string& token_response_body) {
        PollAuthResult result;
        try {
            json j = json::parse(token_response_body);
            std::string msa_token = j.value("access_token", "");
            std::string refresh_token = j.value("refresh_token", "");

            // 1. Authenticate with Xbox Live
            json xbl_req = {
                {"Properties", {
                    {"AuthMethod", "RPS"},
                    {"SiteName", "user.auth.xboxlive.com"},
                    {"RpsTicket", "d=" + msa_token}
                }},
                {"RelyingParty", "http://auth.xboxlive.com"},
                {"TokenType", "JWT"}
            };

            auto xbl_resp = HttpClient::instance().post(
                "https://user.auth.xboxlive.com/user/authenticate",
                xbl_req.dump(),
                {"Content-Type: application/json", "Accept: application/json"}
            );

            if (!xbl_resp.success) {
                result.status = "error";
                result.message = "Не удалось подключиться к Xbox Live (" + (xbl_resp.error.empty() ? ("код " + std::to_string(xbl_resp.status_code)) : xbl_resp.error) + "). Проверьте сеть или используйте Offline аккаунт.";
                return result;
            }

            json xbl_json = json::parse(xbl_resp.body);
            std::string xbl_token = xbl_json.value("Token", "");
            std::string uhs = "";
            if (xbl_json.contains("DisplayClaims") && xbl_json["DisplayClaims"].contains("xui") && !xbl_json["DisplayClaims"]["xui"].empty()) {
                uhs = xbl_json["DisplayClaims"]["xui"][0].value("uhs", "");
            }

            // 2. Authenticate with XSTS
            json xsts_req = {
                {"Properties", {
                    {"SandboxId", "RETAIL"},
                    {"UserTokens", {xbl_token}}
                }},
                {"RelyingParty", "rp://api.minecraftservices.com/"},
                {"TokenType", "JWT"}
            };

            auto xsts_resp = HttpClient::instance().post(
                "https://xsts.auth.xboxlive.com/xsts/authorize",
                xsts_req.dump(),
                {"Content-Type: application/json", "Accept: application/json"}
            );

            if (!xsts_resp.success) {
                result.status = "error";
                result.message = "Ошибка авторизации XSTS (" + (xsts_resp.error.empty() ? ("код " + std::to_string(xsts_resp.status_code)) : xsts_resp.error) + "). Убедитесь, что аккаунт Xbox настроен, или используйте Offline аккаунт.";
                return result;
            }

            json xsts_json = json::parse(xsts_resp.body);
            std::string xsts_token = xsts_json.value("Token", "");

            // 3. Login to Minecraft
            json mc_login_req = {
                {"identityToken", "XBL3.0 x=" + uhs + ";" + xsts_token}
            };

            auto mc_resp = HttpClient::instance().post(
                "https://api.minecraftservices.com/authentication/login_with_xbox",
                mc_login_req.dump(),
                {"Content-Type: application/json", "Accept: application/json"}
            );

            if (!mc_resp.success) {
                result.status = "error";
                result.message = "Ошибка входа в Minecraft Services.";
                return result;
            }

            json mc_json = json::parse(mc_resp.body);
            std::string mc_token = mc_json.value("access_token", "");

            // 4. Fetch profile
            auto prof_resp = HttpClient::instance().get(
                "https://api.minecraftservices.com/minecraft/profile",
                {"Authorization: Bearer " + mc_token}
            );

            std::string player_name = "Player";
            std::string player_uuid = "";
            std::string skin_url = "";

            if (prof_resp.success) {
                json prof_json = json::parse(prof_resp.body);
                player_name = prof_json.value("name", "Player");
                player_uuid = prof_json.value("id", "");
                if (prof_json.contains("skins") && prof_json["skins"].is_array() && !prof_json["skins"].empty()) {
                    skin_url = prof_json["skins"][0].value("url", "");
                }
            }

            if (player_uuid.empty()) {
                player_uuid = generate_offline_uuid(player_name);
            }

            Account new_acc;
            new_acc.id = "ms_" + player_uuid;
            new_acc.type = "microsoft";
            new_acc.username = player_name;
            new_acc.uuid = player_uuid;
            new_acc.access_token = mc_token;
            new_acc.refresh_token = refresh_token;
            new_acc.skin_url = skin_url.empty() ? ("https://minotar.net/helm/" + player_name + "/100.png") : skin_url;
            new_acc.expires_at = (uint64_t)std::chrono::system_clock::now().time_since_epoch().count() + 86400000ULL;

            // Remove if existing account with same uuid
            for (auto it = accounts_.begin(); it != accounts_.end(); ++it) {
                if (it->uuid == player_uuid) {
                    accounts_.erase(it);
                    break;
                }
            }

            accounts_.push_back(new_acc);
            SettingsManager::instance().update({{"active_account_id", new_acc.id}});
            save();

            result.status = "success";
            result.message = "Успешный вход в аккаунт Microsoft: " + player_name;
            result.account = new_acc;
            return result;
        } catch (const std::exception& e) {
            result.status = "error";
            result.message = std::string("Ошибка парсинга ответа: ") + e.what();
            return result;
        }
    }

private:
    std::string accounts_file_;
    std::vector<Account> accounts_;
    std::string current_device_code_;

    static std::string generate_offline_uuid(const std::string& name) {
        // Simple fast MD5-like hex generation for deterministic offline uuid
        uint32_t hash = 2166136261u;
        for (char c : name) {
            hash = (hash ^ (uint8_t)c) * 16777619u;
        }
        char hex[37];
        snprintf(hex, sizeof(hex), "%08x-%04x-3%03x-8%03x-%08x%04x",
                 hash, (hash >> 16) & 0xFFFF, hash & 0xFFF, (hash >> 8) & 0xFFF, hash, (hash ^ 0xDEADBEEF) & 0xFFFF);
        return std::string(hex);
    }
};

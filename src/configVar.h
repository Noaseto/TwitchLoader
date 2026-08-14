/*
   (@__  Quack
\\\_\
<____)
*/

#pragma once

#include <string>
#include "internationalisation.h"

// ----------------------- Register options -----------------------

inline ModResult register_string_option(
    const char* name, const char* default_value, ConfigVarHandle& out_handle, ModError* error) {
    ConfigVarDesc config_var_desc = CONFIG_VAR_DESC_INIT;
    config_var_desc.name = name;
    config_var_desc.type = CONFIG_VAR_STRING;
    config_var_desc.default_string = default_value;
    if (svc_config->register_var(mod_ctx, &config_var_desc, &out_handle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, REGISTER_CONFIG_VAR_FAILED.data());
    }
    return MOD_OK;
}

inline ModResult register_bool_option(
    const char* name, const bool default_value, ConfigVarHandle& out_handle, ModError* error) {
    ConfigVarDesc config_var_desc = CONFIG_VAR_DESC_INIT;
    config_var_desc.name = name;
    config_var_desc.type = CONFIG_VAR_BOOL;
    config_var_desc.default_bool = default_value;
    if (svc_config->register_var(mod_ctx, &config_var_desc, &out_handle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, REGISTER_CONFIG_VAR_FAILED.data());
    }
    return MOD_OK;
}

// ----------------------- Options getters -----------------------

inline std::string get_string_option(const ConfigVarHandle handle, std::string fallback = "") {
    size_t handle_size;
    if (handle == 0 || svc_config->get_string(mod_ctx, handle, NULL, 0, &handle_size) != MOD_OK) {
        return fallback;
    }

    std::string handle_value(handle_size, '\0');
    if (svc_config->get_string(mod_ctx, handle, handle_value.data(), handle_size + 1, NULL) !=
        MOD_OK)
    {
        return fallback;
    }

    return handle_value;
}

inline bool get_bool_option(const ConfigVarHandle handle, const bool fallback) {
    bool value = fallback;
    if (handle == 0 || svc_config->get_bool(mod_ctx, handle, &value) != MOD_OK) {
        return fallback;
    }
    return value;
}

// ----------------------- Register this mod variables -----------------------

inline ConfigVarHandle g_config_var_username = 0;
inline ConfigVarHandle g_config_var_twitch_id = 0;
inline ConfigVarHandle g_config_var_client_id = 0;
inline ConfigVarHandle g_config_var_oauth = 0;
inline ConfigVarHandle g_config_var_auto_start = 0;

// these magic strings are the values stored in config file for instance
// "mod.io_github_noaseto_twitchloader.autoStart": true/false,
inline ModResult register_variables(ModError* error) {
    ModResult result = register_string_option("username", "", g_config_var_username, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchId", "", g_config_var_twitch_id, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchClientId", "", g_config_var_client_id, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchOAuth", "", g_config_var_oauth, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_bool_option("autoStart", false, g_config_var_auto_start, error);
    if (result != MOD_OK) {
        return result;
    }
    return result;
}

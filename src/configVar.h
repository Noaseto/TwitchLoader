#pragma once

inline ConfigVarHandle g_cvarUsername = 0;
inline ConfigVarHandle g_cvarTwitchId = 0;
inline ConfigVarHandle g_cvarClientId = 0;
inline ConfigVarHandle g_cvarOAuth = 0;
inline ConfigVarHandle g_cvarAutoStart = 0;

ModResult register_string_option(
    const char* name, char* defaultValue, ConfigVarHandle& outHandle, ModError* error) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    cvarDesc.type = CONFIG_VAR_STRING;
    cvarDesc.default_string = defaultValue;
    if (svc_config->register_var(mod_ctx, &cvarDesc, &outHandle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, REGISTER_CONFIG_VAR_FAILED.data());
    }
    return MOD_OK;
}

ModResult register_bool_option(
    const char* name, bool defaultValue, ConfigVarHandle& outHandle, ModError* error) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    cvarDesc.type = CONFIG_VAR_BOOL;
    cvarDesc.default_bool = defaultValue;
    if (svc_config->register_var(mod_ctx, &cvarDesc, &outHandle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, REGISTER_CONFIG_VAR_FAILED.data());
    }
    return MOD_OK;
}

inline ModResult registerVariables(ModError* error) {
    ModResult result = register_string_option("username", "twitch username", g_cvarUsername, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchId", "twitch id", g_cvarTwitchId, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchClientId", "clientId", g_cvarClientId, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_string_option("twitchOAuth", "OAuthToken", g_cvarOAuth, error);
    if (result != MOD_OK) {
        return result;
    }
    result = register_bool_option("autoStart", false, g_cvarAutoStart, error);
    if (result != MOD_OK) {
        return result;
    }
    return result;
}

std::string get_string_option(ConfigVarHandle handle) {
    // todo check get_string return (ModResult) to add robustness
    size_t handleSize;
    svc_config->get_string(mod_ctx, handle, NULL, 0, &handleSize);

    std::string handleValue(handleSize, '\0');
    svc_config->get_string(mod_ctx,handle,handleValue.data(), handleSize+1, NULL);

    return handleValue;
}

bool get_bool_option(ConfigVarHandle handle, bool fallback) {
    bool value = fallback;
    if (handle == 0 || svc_config->get_bool(mod_ctx, handle, &value) != MOD_OK) {
        return fallback;
    }
    return value;
}
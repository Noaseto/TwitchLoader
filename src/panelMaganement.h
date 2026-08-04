// once again, i wanna split this file
// also in the mod sdk there are .h, and .hpp files, why is that
// todo Lis noisette
#pragma once

#include "mods/service.hpp"
#include "mods/svc/ui.h"
#include "mods/svc/config.h"
#include "ws_client.hpp"

inline UiWindowHandle g_controlsWindow = 0;

inline ConfigVarHandle g_cvarUsername = 0;
inline ConfigVarHandle g_cvarTwitchId = 0;
inline ConfigVarHandle g_cvarClientId = 0;
inline ConfigVarHandle g_cvarOAuth = 0;
inline ConfigVarHandle g_cvarAutoStart = 0;

IMPORT_SERVICE(UiService, svc_ui);
IMPORT_SERVICE(ConfigService, svc_config);

inline void add_control(UiElementHandle pane, const UiControlDesc& desc) {
    svc_ui->pane_add_control(mod_ctx, pane, &desc, nullptr);
}

void add_toggle(UiElementHandle pane, const char* label, ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    add_control(pane, control);
}

void add_string(UiElementHandle pane, const char* label, ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_STRING;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    add_control(pane, control);
}

ModResult register_string_option(
    const char* name, char* defaultValue, ConfigVarHandle& outHandle, ModError* error) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    cvarDesc.type = CONFIG_VAR_STRING;
    cvarDesc.default_string = defaultValue;
    if (svc_config->register_var(mod_ctx, &cvarDesc, &outHandle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, "failed to register twitch loader options");
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
        return mods::set_error(error, MOD_ERROR, "failed to register twitch loader options");
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
    result = register_bool_option("g_cvarAutoStart", false, g_cvarAutoStart, error);
    if (result != MOD_OK) {
        return result;
    }
    return result;
}

inline ModResult buildTwitchConfigTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, "Twitch config");
    add_string(left, "Username", g_cvarUsername, "Your twitch username");
    add_string(left, "Twitch ID", g_cvarTwitchId, "Twitch id");

    svc_ui->pane_add_section(mod_ctx, left, "Check actions");
    //add_button(left, "Enabled", "Enables dynamic shadows.");
    add_toggle(left, "AutoStart", g_cvarAutoStart, "Should the twitch communication start at dusklight launch");

    return MOD_OK;
}

inline ModResult buildTwitchSecretTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, "Secret");
    add_string(left, "Client ID", g_cvarClientId, "client id twitch app");
    add_string(left, "OAuth token", g_cvarOAuth, "token");

    return MOD_OK;
}

inline void onOpenModConfig(ModContext*, void*) {
    if (g_controlsWindow != 0) {
        return;
    }
    UiTabDesc tabs[2] = {UI_TAB_DESC_INIT, UI_TAB_DESC_INIT};
    tabs[0].title = "Twitch config";
    tabs[0].build = buildTwitchConfigTab;
    tabs[1].title = "Twitch secrets";
    tabs[1].build = buildTwitchSecretTab;
    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 2;
    desc.on_closed = [](ModContext*, UiWindowHandle, void*) {g_controlsWindow = 0;};
    if (svc_ui->window_push(mod_ctx, &desc, &g_controlsWindow) != MOD_OK) {
        svc_log->error(mod_ctx, "failed to open twitch loader secrets window");
    }
}

inline ModResult buildMainPanel(ModContext*, UiElementHandle panel, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.label = "Configure Mod";
    control.kind = UI_CONTROL_BUTTON;
    control.on_pressed = onOpenModConfig;
    add_control(panel, control);
    //
    // control = UI_CONTROL_DESC_INIT;
    // control.kind = UI_CONTROL_BUTTON;
    // control.label = "Test connection";
    // control.on_pressed = [](ModContext*, void*){};
    // add_control(panel, control);
    // // could i add some text to view the return of the onPressed action ?
    //
    // control = UI_CONTROL_DESC_INIT;
    // control.kind = UI_CONTROL_BUTTON;
    // control.label = "Start/Stop Twitch connection";
    // control.on_pressed = [](ModContext*, void*){};
    // add_control(panel, control);

    return MOD_OK;
}

inline ModResult clearPanels() {
    // todo clear what must be
}
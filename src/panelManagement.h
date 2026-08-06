// once again, i wanna split this file
// also in the mod sdk there are .h, and .hpp files, why is that
// todo Lis noisette
#pragma once

#include "mods/service.hpp"
#include "mods/svc/ui.h"
#include "mods/svc/config.h"
#include "internationalisation.h"
#include "configVar.h"
#include "twitchCommunication/ws_client.hpp"

inline UiWindowHandle g_controlsWindow = 0;

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

void onToggleSocket(ModContext*, void*) {
    g_ws.toggleSocket();
}


inline ModResult buildTwitchConfigTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, TWITCH_SECTION_NAME.data());
    add_string(left, TWITCH_USERNAME.data(), g_cvarUsername, TWITCH_USERNAME_DESCRIPTION.data());
    add_string(left, TWITCH_USER_ID.data(), g_cvarTwitchId, TWITCH_USER_ID_DESCRIPTION.data());

    svc_ui->pane_add_section(mod_ctx, left, ACTIONS_SECTION_NAME.data());
    add_toggle(left, ACTIONS_AUTO_START.data(), g_cvarAutoStart, ACTIONS_AUTO_START_DESCRIPTION.data());

    UiControlDesc startWebsocketControl = UI_CONTROL_DESC_INIT;
    startWebsocketControl.kind = UI_CONTROL_BUTTON;
    startWebsocketControl.label = ACTIONS_TOGGLE.data();
    startWebsocketControl.help_rml = ACTIONS_TOGGLE_DESCRIPTION.data();
    startWebsocketControl.on_pressed = onToggleSocket;
    add_control(left, startWebsocketControl);

    return MOD_OK;
}

inline ModResult buildTwitchSecretTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, "Secret");
    add_string(left, SECRETS_CLIENT_ID.data(), g_cvarClientId, SECRETS_CLIENT_ID_DESCRIPTION.data());
    add_string(left, SECRETS_OAUTH_TOKEN.data(), g_cvarOAuth, SECRETS_OAUTH_TOKEN_DESCRIPTION.data());

    return MOD_OK;
}

inline void onOpenModConfig(ModContext*, void*) {
    if (g_controlsWindow != 0) {
        return;
    }
    UiTabDesc tabs[2] = {UI_TAB_DESC_INIT, UI_TAB_DESC_INIT};
    tabs[0].title = TWITCH_CONFIG_TAB.data();
    tabs[0].build = buildTwitchConfigTab;
    tabs[1].title = TWITCH_SECRETS_TAB.data();
    tabs[1].build = buildTwitchSecretTab;
    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 2;
    desc.on_closed = [](ModContext*, UiWindowHandle, void*) {g_controlsWindow = 0;};
    if (svc_ui->window_push(mod_ctx, &desc, &g_controlsWindow) != MOD_OK) {
        svc_log->error(mod_ctx, TWITCH_LOADER_PANE_FAILED.data());
    }
}

inline ModResult buildMainPanel(ModContext*, UiElementHandle panel, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.label = TWITCH_LOADER_OPTIONS_BUTTON.data();
    control.kind = UI_CONTROL_BUTTON;
    control.on_pressed = onOpenModConfig;
    add_control(panel, control);

    return MOD_OK;
}

inline ModResult clearPanels() {
    // todo clear what must be, is there anything to clear ?
}
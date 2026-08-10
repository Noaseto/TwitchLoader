#pragma once

#include "mods/service.hpp"
#include "mods/svc/config.h"
#include "mods/svc/ui.h"

#include "configVar.h"
#include "internationalisation.h"
#include "twitchCommunication/ws_client.hpp"

inline UiWindowHandle g_controlsWindow = 0;

// add specific fields
void add_toggle(UiElementHandle pane, const char* label, ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

void add_string(UiElementHandle pane, const char* label, ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_STRING;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

void add_button(UiElementHandle pane, const char* label, const char* help, const UiPressedFn onPressed) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_BUTTON;
    control.label = label;
    control.help_rml = help;
    control.on_pressed = onPressed;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

// tab management
inline ModResult buildTwitchConfigTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, TWITCH_SECTION_NAME.data());
    add_string(left, TWITCH_USERNAME.data(), g_cvarUsername, TWITCH_USERNAME_DESCRIPTION.data());
    add_string(left, TWITCH_USER_ID.data(), g_cvarTwitchId, TWITCH_USER_ID_DESCRIPTION.data());

    svc_ui->pane_add_section(mod_ctx, left, ACTIONS_SECTION_NAME.data());
    add_toggle(left, ACTIONS_AUTO_START.data(), g_cvarAutoStart, ACTIONS_AUTO_START_DESCRIPTION.data());
    add_button(left, ACTIONS_TOGGLE.data(), ACTIONS_TOGGLE_DESCRIPTION.data(), [](ModContext*, void*){g_ws.toggleSocket();});

    return MOD_OK;
}

inline ModResult buildTwitchSecretTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, SECRETS_SECTION_NAME.data());
    add_string(left, SECRETS_CLIENT_ID.data(), g_cvarClientId, SECRETS_CLIENT_ID_DESCRIPTION.data());
    add_string(left, SECRETS_OAUTH_TOKEN.data(), g_cvarOAuth, SECRETS_OAUTH_TOKEN_DESCRIPTION.data());

    return MOD_OK;
}

// Mod config management
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

inline ModResult buildMainPanel(ModContext*, UiElementHandle pane, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.label = TWITCH_LOADER_OPTIONS_BUTTON.data();
    control.kind = UI_CONTROL_BUTTON;
    control.on_pressed = onOpenModConfig;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);

    return MOD_OK;
}

inline ModResult clearPanels() {
    // todo clear what must be, is there anything to clear ?
}
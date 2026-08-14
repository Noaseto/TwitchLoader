#pragma once

#include "mods/svc/config.h"
#include "mods/svc/ui.h"

#include "configVar.h"
#include "internationalisation.h"
#include "twitchCommunication/ws_client.hpp"

inline UiWindowHandle g_controlsModConfig = 0;

// add specific fields
inline void add_toggle(
    const UiElementHandle pane, const char* label, const ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

inline void add_string(
    const UiElementHandle pane, const char* label, const ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_STRING;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

inline void add_button(
    const UiElementHandle pane, const char* label, const char* help, const UiPressedFn on_pressed) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_BUTTON;
    control.label = label;
    control.help_rml = help;
    control.on_pressed = on_pressed;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
}

// onClick actions
inline void on_toggle_connection(ModContext*, void*) {
    UiDialogDesc desc = UI_DIALOG_DESC_INIT;
    desc.title = ACTIONS_TOGGLE.data();
    desc.body_rml =
        g_ws.is_started() ? TOGGLE_POPUP_TEXT_STOP.data() : TOGGLE_POPUP_TEXT_START.data();
    const std::string toggle_text =
        g_ws.is_started() ? TOGGLE_POPUP_BUTTON_STOP.data() : TOGGLE_POPUP_BUTTON_START.data();
    UiDialogHandle dialog_handle;
    desc.action_count = 2;
    const UiDialogAction cancel_action = {.label = TOGGLE_POPUP_BUTTON_CANCEL.data(),
        .on_pressed = [](ModContext*, UiDialogHandle, void*) {},
        .user_data = NULL,
        .keep_open = false};
    const UiDialogAction toggle_action = {.label = toggle_text.c_str(),
        .on_pressed = [](ModContext*, UiDialogHandle, void*) { g_ws.toggle_socket(); },
        .user_data = NULL,
        .keep_open = false};
    const UiDialogAction actions[] = {cancel_action, toggle_action};

    desc.actions = actions;
    svc_ui->dialog_push(mod_ctx, &desc, &dialog_handle);
}

// tab management
inline ModResult build_twitch_config_tab(ModContext*, UiWindowHandle, const UiElementHandle left,
    const UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, TWITCH_SECTION_NAME.data());
    add_string(
        left, TWITCH_USERNAME.data(), g_config_var_username, TWITCH_USERNAME_DESCRIPTION.data());
    add_string(
        left, TWITCH_USER_ID.data(), g_config_var_twitch_id, TWITCH_USER_ID_DESCRIPTION.data());

    svc_ui->pane_add_section(mod_ctx, left, ACTIONS_SECTION_NAME.data());
    add_toggle(left, ACTIONS_AUTO_START.data(), g_config_var_auto_start,
        ACTIONS_AUTO_START_DESCRIPTION.data());
    add_button(
        left, ACTIONS_TOGGLE.data(), ACTIONS_TOGGLE_DESCRIPTION.data(), on_toggle_connection);

    return MOD_OK;
}

inline ModResult build_twitch_secret_tab(ModContext*, UiWindowHandle, const UiElementHandle left,
    const UiElementHandle right, void*, ModError*) {
    (void)right;
    svc_ui->pane_add_section(mod_ctx, left, SECRETS_SECTION_NAME.data());
    add_string(left, SECRETS_CLIENT_ID.data(), g_config_var_client_id,
        SECRETS_CLIENT_ID_DESCRIPTION.data());
    add_string(left, SECRETS_OAUTH_TOKEN.data(), g_config_var_oauth,
        SECRETS_OAUTH_TOKEN_DESCRIPTION.data());

    return MOD_OK;
}

// Mod config management
inline void on_open_mod_config(ModContext*, void*) {
    if (g_controlsModConfig != 0) {
        return;
    }
    UiTabDesc tabs[2] = {UI_TAB_DESC_INIT, UI_TAB_DESC_INIT};
    tabs[0].title = TWITCH_CONFIG_TAB.data();
    tabs[0].build = build_twitch_config_tab;
    tabs[1].title = TWITCH_SECRETS_TAB.data();
    tabs[1].build = build_twitch_secret_tab;
    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 2;
    desc.on_closed = [](ModContext*, UiWindowHandle, void*) { g_controlsModConfig = 0; };
    if (svc_ui->window_push(mod_ctx, &desc, &g_controlsModConfig) != MOD_OK) {
        svc_log->error(mod_ctx, TWITCH_LOADER_PANE_FAILED.data());
    }
}

inline ModResult build_main_panel(ModContext*, const UiElementHandle pane, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.label = TWITCH_LOADER_OPTIONS_BUTTON.data();
    control.kind = UI_CONTROL_BUTTON;
    control.on_pressed = on_open_mod_config;
    svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);

    return MOD_OK;
}
#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"

#include "config_var.hpp"
#include "i18n.hpp"
#include "panel_management.hpp"
#include "twitch/ws_client.hpp"
#include "twitch_loader_service.h"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(UiService, svc_ui);
IMPORT_SERVICE(ConfigService, svc_config);

WsClient g_ws;
extern "C" {
static std::vector<TwitchEvent> published_events;

MOD_EXPORT ModResult mod_initialize(ModError* error) {
    // set config var
    ModResult result = register_variables(error);
    if (result != MOD_OK) {
        return result;
    }

    // init mod panel menu
    UiModsPanelDesc ui_mods_panel_desc = UI_MODS_PANEL_DESC_INIT;
    ui_mods_panel_desc.build = build_main_panel;
    result = svc_ui->register_mods_panel(mod_ctx, &ui_mods_panel_desc);
    if (result != MOD_OK) {
        return result;
    }

    // at startup, thread is not running, the toggle will launch the start
    if (get_bool_option(g_config_var_auto_start, false)) {
        g_ws.toggle_socket();
    }

    svc_log->info(mod_ctx, LOG_MOD_INIT.data());
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    // Clear the previous frame's events
    published_events.clear();
    published_events.reserve(g_ws.get_messages_length());

    TwitchEvent twitch_event;
    while (g_ws.try_pop_message(twitch_event)) {
        published_events.push_back(twitch_event);
    }
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    g_ws.stop();
    svc_log->info(mod_ctx, LOG_MOD_STOP.data());
    return MOD_OK;
}

// ------------------------- Service Related -------------------------
// Service function(s) implementation, all of them are called by mod consumers
// todo, should these be defined in a dedicated file as the sdk does

static ModResult get_events(
    ModContext*, const TwitchEvent** out_events, uint32_t* out_event_count) {
    if (out_events == nullptr || out_event_count == nullptr) {
        return MOD_INVALID_ARGUMENT;
    }
    *out_events = published_events.empty() ? nullptr : published_events.data();
    *out_event_count = static_cast<uint32_t>(published_events.size());
    return MOD_OK;
}

constexpr TwitchLoaderService g_service{
    .header = SERVICE_HEADER(TwitchLoaderService, MY_MOD_SERVICE_MAJOR, MY_MOD_SERVICE_MINOR),
    .get_events = get_events,
};
EXPORT_SERVICE(g_service);
}

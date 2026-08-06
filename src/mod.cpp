// basic mods include
#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"

// this mod related imports
#include "configVar.h"
#include "internationalisation.h"
#include "panelManagement.h"
#include "twitchCommunication/ws_client.hpp"
#include "twitchData.h"
#include "twitchLoaderService.h"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(UiService, svc_ui);
IMPORT_SERVICE(ConfigService, svc_config);

WsClient g_ws;
extern "C" {
static std::vector<TwitchEvent> publishedEvents;

MOD_EXPORT ModResult mod_initialize(ModError* error) {
    // set config var
    ModResult result = registerVariables(error);
    if (result != MOD_OK) {
        return result;
    }

    // init mod view menu
    UiModsPanelDesc panelDesc = UI_MODS_PANEL_DESC_INIT;
    panelDesc.build = buildMainPanel;
    result = svc_ui->register_mods_panel(mod_ctx, &panelDesc);
    if (result != MOD_OK) {
        return result;
    }

    // at startup, thread is not running, the toggle will launch the start
    if (get_bool_option(g_cvarAutoStart, false)) g_ws.toggleSocket();

    svc_log->info(mod_ctx, LOG_MOD_INIT.data());
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError* error) {
    // Clear the previous frame's events
    publishedEvents.clear();
    publishedEvents.reserve(g_ws.get_messages_length());

    TwitchEvent twitchEvent;
    while (g_ws.try_pop_message(twitchEvent)) {
        publishedEvents.push_back(twitchEvent);
    }
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    // todo properly disconnect from twitch
    // I'm pretty (nice) sure that as of right now, reloading the mod ends up in memory leak
    g_ws.stop();
    svc_log->info(mod_ctx, LOG_MOD_STOP.data());
    return MOD_OK;
}

// ------------------------- Service Related -------------------------
// Service function(s) implementation, all of them are called by mod consumers
// todo, should these be defined in a dedicated file as the sdk does

static ModResult get_events(ModContext*,const TwitchEvent** outEvents, uint32_t* outEventCount) {
    if (outEvents == nullptr || outEventCount == nullptr) {
        return MOD_INVALID_ARGUMENT;
    }
    *outEvents = publishedEvents.empty()
        ? nullptr
        : publishedEvents.data();
    *outEventCount = static_cast<uint32_t>(publishedEvents.size());
    return MOD_OK;
}

constexpr TwitchEventsService g_service{
    .header = SERVICE_HEADER(TwitchEventsService, MY_MOD_SERVICE_MAJOR, MY_MOD_SERVICE_MINOR),
    .get_events = get_events,
};
EXPORT_SERVICE(g_service);
}

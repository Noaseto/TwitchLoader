// todo, this file is poorly organised, it was my mind battlefield, (not pretty)
// it needs to be properly cleared, and split into several files
// also i need to setup CLang to format everything

// basic mods include
#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"

// custom service
#include "twitchStuff/twitchData.h"
#include "twitchLoaderService.h"

// websocket business import
#include "ws_client.hpp"
#include "nlohmann/json.hpp"

// custom definiton of menu
#include "configVar.h"
#include "panelMaganement.h"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);

WsClient g_ws;
extern "C" {

#include <optional>

struct ChatMessage {
    std::string user;
    std::string text;
};

std::optional<ChatMessage> parse_message(const std::string& raw) {
    try {
        json data = json::parse(raw);
        const auto& event = data.at("payload").at("event");
        std::string user = event.at("chatter_user_name").get<std::string>();
        std::string text = event.at("message").at("text").get<std::string>();
        return ChatMessage{user, text};
    } catch (const json::exception&) {
        return std::nullopt;
    }
}


MOD_EXPORT ModResult mod_initialize(ModError* error) {
    // set config var
    ModResult result = registerVariables(error);
    if (result != MOD_OK) {
        return result;
    }

    // init mod view menu
    UiModsPanelDesc panelDesc = UI_MODS_PANEL_DESC_INIT;
    panelDesc.build = buildMainPanel;
    svc_ui->register_mods_panel(mod_ctx, &panelDesc);

    // at startup, thread is not running, the toggle will launch the start
    if (get_bool_option(g_cvarAutoStart, false)) g_ws.toggleSocket();

    svc_log->info(mod_ctx, LOG_INIT_SUCCESS.data());
    return MOD_OK;
}


struct OwnedChatEvent {
    std::string username;
    std::string message;
};

// todo
// this was an exemple provided by encounter, i messed around a bit, i will rework it as i don't want to snd 2 strings
// but the TwitchEvent struct, they use the size of struct as 1st param, should I do the same
// These events hold the `std::string` instances
static std::vector<OwnedChatEvent> ownedEvents;
// These events simply hold `char*` pointers to the `std::string`s
// Which makes it safe to send across the service boundary
static std::vector<TwitchEvent> publishedEvents;

static void add_chat_event(std::string username, std::string message) {
    // Create a new owned event
    ownedEvents.push_back({
        std::move(username),
        std::move(message),
    });
}

static void clear_chat_event() {
    // Clears this one ownedEvents
    ownedEvents.clear();
}

static void publish_events() {
    publishedEvents.reserve(ownedEvents.size());

    // Populate the new publishedEvents
    for (const auto& ownedEvent : ownedEvents) {
        // This is taking a pointer from the `std::string`s
        // The strings must stay alive while mods are reading them!
        // (i.e. until the next frame, ownedEvents must not be modified)

        // todo send only a TwitchEvent and let client parse as needed
        publishedEvents.push_back({TwitchEventType::ChatMessage,
            //ownedEvent.username.c_str()
            ownedEvent.message.c_str()
        });
    }
}

MOD_EXPORT ModResult mod_update(ModError* error) {
    // Clear the previous frame's events
    clear_chat_event();
    publishedEvents.clear();

    TwitchEvent twitchEvent;
    while (g_ws.try_pop_message(twitchEvent)) {
        svc_log->debug(mod_ctx, twitchEvent.data.c_str());

        if (twitchEvent.type == TwitchEventType::ChatMessage){
            auto chat = parse_message(twitchEvent.data);
            add_chat_event(chat->user.c_str(), chat->text.c_str());
        }
        if (twitchEvent.type == TwitchEventType::Follow) {
            // tmp check with friends, that one works, unfollow refollow too
            // TODO do the payload provide refollow ? or should it be linked with api calls, if so, update the Twitch loader too ?
            svc_log->info(mod_ctx, "+1 follow");
        }

        // sorry I ignored subs and follow >':
        // todo : think about subs and follow

    }
    publish_events();

    return MOD_OK; // but am i ? here, have an ascii blåhaj <*)))>{
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    // todo properly disconnect from twitch
    // I'm pretty (nice) sure that as of right now, reloading the mod ends up in memory leak
    g_ws.stop();
    svc_log->info(mod_ctx, "WebSocket client stopped");
    return MOD_OK;
}

// ------------------------- Service Related -------------------------

// Service function implementation
static ModResult get_events(ModContext*,const TwitchEvent** outEvents,uint32_t* outEventCount) {

    if (outEvents == nullptr || outEventCount == nullptr) {
        return MOD_INVALID_ARGUMENT;
    }

    *outEvents = publishedEvents.empty()
        ? nullptr
        : publishedEvents.data();

    *outEventCount =
        static_cast<uint32_t>(publishedEvents.size());

    return MOD_OK;
}

constexpr TwitchEventsService g_service{
    .header = SERVICE_HEADER(TwitchEventsService, MY_MOD_SERVICE_MAJOR, MY_MOD_SERVICE_MINOR),
    .get_events = get_events,
};
EXPORT_SERVICE(g_service);
}

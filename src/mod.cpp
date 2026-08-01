// todo, this file is poorly organised, it was my mind battlefield, (not pretty)
// it needs to be properly cleared, and split into several files
// also i need to setup CLang to format everything

// basic mods include
#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"
#include "mods/svc/resource.h"

// custom service
#include "twitchStuff/twitchData.h"
#include "twitchLoaderService.h"

// websocket business import
#include "ws_client.hpp"
#include "nlohmann/json.hpp"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(ResourceService, svc_resource);

extern "C" {
static WsClient g_ws;

struct Credentials {
    std::string clientId, oauth, username, userId;
};

bool load_credentials(Credentials& out) {
    ResourceBuffer buf = RESOURCE_BUFFER_INIT;
    // todo : do i really need a dedicated config file ? could it not simply exist in the dusklight config file ? and how do i store the authkey securely
    if (svc_resource->load(mod_ctx, "twitch_credentials.config", &buf) != MOD_OK) {
        return false;
    }
    std::string content(reinterpret_cast<const char*>(buf.data), buf.size);
    svc_resource->free(mod_ctx, &buf);

    std::istringstream iss(content);
    std::string line;
    // found this readfile on stackoverflow, i dislike that
    // in my head .config files could be read with loadValue("key", "default") todo find if it exists in cpp
    while (std::getline(iss, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        if (key == "clientId") out.clientId = val;
        else if (key == "oauth") out.oauth = val;
        else if (key == "username") out.username = val;
        else if (key == "userId") out.userId = val;
    }
    return true;
}

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

    Credentials credentials;

    // todo everything here should be define as const at the head of this file (do i need data across several files ? if so, dedicated const files maybe)
    if (!load_credentials(credentials)) {
        return mods::set_error(error, MOD_ERROR, "res/twitch_credentials.config introuvable ou invalide");
    }
    g_ws.start("eventsub.wss.twitch.tv", "443", credentials.clientId, credentials.oauth, credentials.username, credentials.userId);

    svc_log->info(mod_ctx, "twitch loader started");
    return MOD_OK;
}


struct OwnedChatEvent {
    std::string username;
    std::string message;
};

// todo
// this was an exemple provided by encounter, i messed around a bit, i will rework it
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

        // sorry I ignored subs and follow >':
        // todo : think about subs and follow

    }
    publish_events();

    return MOD_OK; // but am i ? here, have an ascii blåhaj <*)))>{
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
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

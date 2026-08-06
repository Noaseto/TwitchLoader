#pragma once
#include <string>


// see https://dev.twitch.tv/docs/eventsub/websocket-reference/
enum class TwitchEventType {
    ChatMessage,
    Follow,
    Subscribe,
    SubGift,
    Cheer,
    SessionWelcome,
    Unknown, // bruh
    TwitchEventError // do not name this only error in capital, windows compiler does not like
    // todo some ideas to add: Raid, prediction, channel point reward
};

typedef struct TwitchEvent {
    uint32_t struct_size;
    TwitchEventType type;
    std::string* data;
} TwitchEvent;

typedef struct TwitchEventsService {
    ServiceHeader header;
    ModResult (*get_events)(ModContext* ctx,
        const TwitchEvent** outEvents,
        uint32_t* outEventCount);
} TwitchEventsService;

// only used by the ws_client
typedef struct TwitchSubscription {
// see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
    TwitchEventType eventType;
    std::string type;
    std::string version;
};
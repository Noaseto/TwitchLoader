#pragma once
#include <string>

// todo, in java there is javadoc, python docstring what is the equivalent in c/cpp
// -> create doc

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

// Todo: Make this is agnostic for the data transfer over service call (or should I ?)
typedef struct TwitchEvent {
    TwitchEventType type;
    std::string data;
} TwitchEvent;

typedef struct TwitchSubscription {
// see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
    TwitchEventType eventType;
    std::string type;
    std::string version;
};

typedef struct TwitchEventsService {
    ServiceHeader header;
    ModResult (*get_events)(ModContext* ctx,
        const TwitchEvent** outEvents,
        uint32_t* outEventCount);
} TwitchEventsService;
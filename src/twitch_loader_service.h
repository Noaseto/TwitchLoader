#pragma once

#include "mods/api.h"

// see https://dev.twitch.tv/docs/eventsub/websocket-reference/
enum class TwitchEventType {
    ChatMessage,
    Follow,
    Subscribe,
    SubGift,
    Cheer,
    SessionWelcome,
    Unknown,  // bruh
    TwitchEventError
    // todo some ideas to add: Raid, prediction, channel point reward
};

typedef struct TwitchEvent {
    uint32_t struct_size;
    TwitchEventType type;
    const char* data;
} TwitchEvent;
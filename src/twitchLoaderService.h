// This file should be the one copied to every consumers mod
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

typedef struct TwitchLoaderService {
    ServiceHeader header;
    ModResult (*get_events)(
        ModContext* ctx, const TwitchEvent** outEvents, uint32_t* outEventCount);
} TwitchLoaderService;

#define MY_MOD_SERVICE_ID "io.github.noaseto.twitchloader"
#define MY_MOD_SERVICE_MAJOR 1u
#define MY_MOD_SERVICE_MINOR 0u

#ifdef __cplusplus
#include "mods/service.hpp"
template <>
struct mods::ServiceTraits<TwitchLoaderService> {
    static constexpr const char* id = MY_MOD_SERVICE_ID;
    static constexpr uint16_t major_version = MY_MOD_SERVICE_MAJOR;
    static constexpr uint16_t minor_version = MY_MOD_SERVICE_MINOR;
};
#endif
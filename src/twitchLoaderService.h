#include "mods/api.h"
#include "twitchStuff/twitchData.h"

#define MY_MOD_SERVICE_ID "io.github.noaseto.twitchloader"
#define MY_MOD_SERVICE_MAJOR 1u
#define MY_MOD_SERVICE_MINOR 0u

#ifdef __cplusplus
#include "mods/service.hpp"
template <>
struct mods::ServiceTraits<TwitchEventsService> {
    static constexpr const char* id = MY_MOD_SERVICE_ID;
    static constexpr uint16_t major_version = MY_MOD_SERVICE_MAJOR;
    static constexpr uint16_t minor_version = MY_MOD_SERVICE_MINOR;
};
#endif
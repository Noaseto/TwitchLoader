// This file is not meant to stay, once dusklight defines the way to translate displayed text, the same mechanic should
// be applied here. To make things easier for later, all displayed texts are aggregated here instead of magic string inside
// the code. For now, they stay in english.

// That's a fish not a poisson.
/*
   O     O           ,
   o o          .:/
     o      ,,///;,   ,;/
       o   o)::::::;;///
          >::::::::;;\\\
            ''\\\\\'" ';\
               ';\
 */

#pragma once
#include <string_view>

// websocket related
inline constexpr std::string_view SESSION_WELCOME_FAILED = "Failed to established connection, we are not welcomed :c : {}";
inline constexpr std::string_view EVENT_SUBSCRIPTION_FAILED = "Failed to subscribe to {} : HTTP {} - {}";
inline constexpr std::string_view EXCEPTION_MESSAGE = "Error: {}";

// config var
inline constexpr std::string_view REGISTER_CONFIG_VAR_FAILED = "failed to register twitch loader options";

// mod pane
inline constexpr std::string_view TWITCH_LOADER_OPTIONS_BUTTON = "Configure Mod";
inline constexpr std::string_view TWITCH_CONFIG_TAB = "Twitch config";
inline constexpr std::string_view TWITCH_SECRETS_TAB = "Twitch secrets";

inline constexpr std::string_view TWITCH_SECTION_NAME = "Configuration";
inline constexpr std::string_view TWITCH_USERNAME = "Username";
inline constexpr std::string_view TWITCH_USERNAME_DESCRIPTION = "Your twitch username";
inline constexpr std::string_view TWITCH_USER_ID = "Twitch ID";
inline constexpr std::string_view TWITCH_USER_ID_DESCRIPTION = "Your twitch id";

inline constexpr std::string_view ACTIONS_SECTION_NAME = "Check actions";
inline constexpr std::string_view ACTIONS_AUTO_START = "Auto start";
inline constexpr std::string_view ACTIONS_AUTO_START_DESCRIPTION = "Should the Twitch-Dusklight connection start at dusklight launch";
inline constexpr std::string_view ACTIONS_TOGGLE= "Start/Stop communication";
inline constexpr std::string_view ACTIONS_TOGGLE_DESCRIPTION = "Starts or stops the Twitch-Dusklight connection";

inline constexpr std::string_view SECRETS_SECTION_NAME = "Secrets";
inline constexpr std::string_view SECRETS_CLIENT_ID= "Client ID";
inline constexpr std::string_view SECRETS_CLIENT_ID_DESCRIPTION = "Client ID to twitch app";
inline constexpr std::string_view SECRETS_OAUTH_TOKEN= "OAuth token";
inline constexpr std::string_view SECRETS_OAUTH_TOKEN_DESCRIPTION = "Your token";

inline constexpr std::string_view TWITCH_LOADER_PANE_FAILED = "failed to open twitch loader secrets window";

// logs
inline constexpr std::string_view LOG_INIT_SUCCESS = "twitch loader started";
inline constexpr std::string_view LOG_START_WEBSOCKET = "Starts websocket connection";
inline constexpr std::string_view LOG_STOP_WEBSOCKET = "Stops websocket connection";

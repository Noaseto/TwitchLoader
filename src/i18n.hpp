/*
This file is not meant to stay, once dusklight defines the way to translate displayed text, the
same mechanic should be applied here. To make things easier for later, all displayed texts are
aggregated here instead of magic string inside the code. For now, they stay in english.

also nobody knows if it is internationalisation or internationalization ... i18n is fine as a
name :>

That's a fish not a poisson.

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
inline constexpr std::string_view LAUNCH_WEBSOCKET_FAILED =
    "Configuration is not fully done, webSocket connection cannot start.";
inline constexpr std::string_view SESSION_WELCOME_FAILED =
    "Failed to established connection, we are not welcomed :c : ";
inline constexpr std::string_view EVENT_SUBSCRIPTION_FAILED = "Failed to subscribe to ";
inline constexpr std::string_view EXCEPTION_MESSAGE = "Error: ";

// config var
inline constexpr std::string_view REGISTER_CONFIG_VAR_FAILED =
    "failed to register twitch loader options";

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
inline constexpr std::string_view ACTIONS_AUTO_START_DESCRIPTION =
    "Should the Twitch-Dusklight connection start at dusklight launch";
inline constexpr std::string_view ACTIONS_TOGGLE = "Start/Stop communication";
inline constexpr std::string_view ACTIONS_TOGGLE_DESCRIPTION =
    "Starts or stops the Twitch-Dusklight connection";

inline constexpr std::string_view TOGGLE_POPUP_TEXT_START = "Start the Twitch connection";
inline constexpr std::string_view TOGGLE_POPUP_TEXT_STOP =
    "Stop the Twitch connection (can take some time)";
inline constexpr std::string_view TOGGLE_POPUP_BUTTON_START = "Start";
inline constexpr std::string_view TOGGLE_POPUP_BUTTON_STOP = "Stop";
inline constexpr std::string_view TOGGLE_POPUP_BUTTON_CANCEL = "Cancel";

inline constexpr std::string_view SECRETS_SECTION_NAME = "Secrets";
inline constexpr std::string_view SECRETS_CLIENT_ID = "Client ID";
inline constexpr std::string_view SECRETS_CLIENT_ID_DESCRIPTION = "Client ID to twitch app";
inline constexpr std::string_view SECRETS_OAUTH_TOKEN = "OAuth token";
inline constexpr std::string_view SECRETS_OAUTH_TOKEN_DESCRIPTION = "Your token";

inline constexpr std::string_view TWITCH_LOADER_PANE_FAILED =
    "failed to open twitch loader secrets window";

// logs, should these really be translated ?
inline constexpr std::string_view LOG_MOD_INIT = "Twitch loader started";
inline constexpr std::string_view LOG_MOD_STOP = "Twitch loader stopped";
inline constexpr std::string_view LOG_START_WEBSOCKET = "Starts websocket connection";
inline constexpr std::string_view LOG_STOP_WEBSOCKET = "Stops websocket connection";
inline constexpr std::string_view LOG_MESSAGE_TYPE_RECEIVED = " received";
inline constexpr std::string_view LOG_WEBSOCKET_CLOSE_FAILED = "Websocket close failed: ";
inline constexpr std::string_view LOG_WEBSOCKET_STREAM_TRUNCATED =
    "Websocket close: peer TLS stream truncated";

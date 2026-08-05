// Mhhh, could this be done with defines ? this would be my java slope then. have a coffee c[_]
#pragma once

#include <string_view>

inline constexpr std::string_view TWITCH_WEBSOCKET_URL = "eventsub.wss.twitch.tv";
inline constexpr std::string_view HTTPS_PORT = "443";
inline constexpr std::string_view HANDSHAKE_ENDPOINT = "/ws";
inline constexpr std::string_view TWITCH_API_URL = "api.twitch.tv";
inline constexpr std::string_view TWITCH_EVENT_SUBSCRIPTION_ENDPOINT = "/helix/eventsub/subscriptions";
inline constexpr int HTTP_VERSION = 11;
inline constexpr std::string_view TWITCH_API_AUTHORIZATION = "Bearer {}";
inline constexpr std::string_view TWITCH_API_CONTENT_TYPE_JSON = "application/json";
inline constexpr std::string_view TWITCH_API_CLIENT_ID = "Client-Id";

// Json Nodes
inline constexpr std::string_view JSON_METADATA = "metadata";
inline constexpr std::string_view JSON_MESSAGE_TYPE = "message_type";
inline constexpr std::string_view JSON_PAYLOAD = "payload";
inline constexpr std::string_view JSON_SESSION = "session";
inline constexpr std::string_view JSON_ID = "id";
inline constexpr std::string_view JSON_BROADCASTER_USER_ID = "broadcaster_user_id";
inline constexpr std::string_view JSON_USER_ID = "user_id";
inline constexpr std::string_view JSON_MODERATOR_USER_ID = "moderator_user_id";
inline constexpr std::string_view JSON_TYPE = "type";
inline constexpr std::string_view JSON_VERSION = "version";
inline constexpr std::string_view JSON_CONDITION = "condition";
inline constexpr std::string_view JSON_TRANSPORT = "transport";
inline constexpr std::string_view JSON_METHOD = "method";
inline constexpr std::string_view JSON_WEBSOCKET = "websocket";
inline constexpr std::string_view JSON_SESSION_ID = "session_id";
inline constexpr std::string_view JSON_SUBSCRIPTION = "subscription";

// MESSAGE_TYPES
inline constexpr std::string_view JSON_MESSAGE_TYPE_SESSION_WELCOME = "session_welcome";
inline constexpr std::string_view JSON_MESSAGE_TYPE_SESSION_KEEPALIVE = "session_keepalive";
inline constexpr std::string_view JSON_MESSAGE_TYPE_SESSION_RECONNECT = "session_reconnect";
inline constexpr std::string_view JSON_MESSAGE_TYPE_NOTIFICATION = "notification";

// Twitch event type subscription
inline constexpr std::string_view SUBSCRIPTION_CHAT_MESSAGE = "channel.chat.message";
inline constexpr std::string_view SUBSCRIPTION_CHAT_MESSAGE_VERSION = "1";
inline constexpr std::string_view SUBSCRIPTION_FOLLOW = "channel.follow";
inline constexpr std::string_view SUBSCRIPTION_FOLLOW_VERSION = "2";
inline constexpr std::string_view SUBSCRIPTION_SUBSCRIBE = "channel.subscribe";
inline constexpr std::string_view SUBSCRIPTION_SUBSCRIBE_VERSION = "1";
inline constexpr std::string_view SUBSCRIPTION_SUB_GIFT = "channel.subscription.gift";
inline constexpr std::string_view SUBSCRIPTION_SUB_GIFT_VERSION = "1";
inline constexpr std::string_view SUBSCRIPTION_CHEER = "channel.cheer";
inline constexpr std::string_view SUBSCRIPTION_CHEER_VERSION = "1";

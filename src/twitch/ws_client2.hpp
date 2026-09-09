#pragma once

#include <string>

#include "../twitch_loader_service.h"
#include "mods/svc/http.h"
#include "mods/svc/websocket.h"

// namespace {
struct TwitchSubscription {
    // see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
    TwitchEventType event_type;
    std::string type;
    std::string version;
};
// }  // namespace

class WsClient2 {
public:
    ModResult toggle_socket();
    bool is_started();

    bool is_subscribed();

    bool is_user_id_requested();
    std::string get_user_id();

    bool is_secret_set();
    ModResult connect_websocket();
    ModResult close_websocket();

    void request_user_id();
    void poll();
    void register_topic(TwitchSubscription topic);

private:
    static void on_user_id_complete(ModContext*, HttpRequestHandle, const HttpResult*, void*);
    static void on_user_register_topic_complete(
        ModContext*, HttpRequestHandle, const HttpResult*, void*);
    void manage_event_message(std::string data);

    bool m_running = false;

    bool m_subscribed = false;
    bool m_subscribeRequested = false;

    std::string m_oauth;
    std::string m_clientId;

    std::string m_userId = "";
    bool m_userIdRequested = false;
    HttpRequestHandle m_userIdHandle = 0;

    std::string m_sessionId;
    WebSocketHandle m_wsHandle = 0;
};

extern WsClient2 g_ws2;
#include "ws_client2.hpp"

#include <nlohmann/json.hpp>
#include <string>

#include "../config_var.hpp"
#include "../i18n.hpp"
#include "mods/svc/log.h"
#include "ws_constant.hpp"

using json = nlohmann::json;

std::string url = std::string("wss://") + TWITCH_WEBSOCKET_URL.data() + HANDSHAKE_ENDPOINT.data();

ModResult WsClient2::toggle_socket() {
    if (m_running) {
        m_running = false;
        m_subscribed = false;
        m_sessionId = "";
        ModResult result = svc_websocket->close(mod_ctx, m_wsHandle, 1000, "why not");
        if (result != MOD_OK) {
            // what should happen then oO
            svc_log->error(mod_ctx, "failed to stop websocket");
        } else
            svc_log->info(mod_ctx, ("Disconnecting from " + url).c_str());
        return result;
    }
    return connect_websocket();
}

bool WsClient2::is_started() {
    return m_running;
}

bool WsClient2::is_subscribed() {
    return m_subscribed;
}

bool WsClient2::is_user_id_requested() {
    return m_userIdRequested;
}

std::string WsClient2::get_user_id() {
    return m_userId;
}

bool WsClient2::is_secret_set() {
    return !m_oauth.empty() && !m_clientId.empty();
}

void WsClient2::poll() {
    WebSocketEvent event = WEBSOCKET_EVENT_INIT;
    if (svc_websocket->poll_event(mod_ctx, &event) != MOD_OK || event.type == WEBSOCKET_EVENT_NONE)
        return;

    switch (event.type) {
    case WEBSOCKET_EVENT_OPEN:
        svc_log->debug(mod_ctx, "web socket connection OPEN");
        break;
    case WEBSOCKET_EVENT_MESSAGE:
        if (event.data != nullptr && event.size > 0) {
            std::string data(static_cast<const char*>(event.data), event.size);
            manage_event_message(data);
            svc_log->debug(mod_ctx, data.c_str());
        }
        break;
    case WEBSOCKET_EVENT_CLOSED:
        svc_log->info(mod_ctx, "Connection closed");
        m_wsHandle = 0;
        m_running = false;
        break;
    }
}

ModResult WsClient2::connect_websocket() {
    m_oauth = get_string_option(g_config_var_oauth);
    m_clientId = get_string_option(g_config_var_client_id);
    if (!is_secret_set()) {
        svc_log->warn(mod_ctx, "Secrets are not set");
        return MOD_OK;
    }
    m_running = true;

    WebSocketConnectDesc ws_desc = WEBSOCKET_CONNECT_DESC_INIT;
    ws_desc.url = url.c_str();

    svc_log->info(mod_ctx, ("Connecting to " + url).c_str());
    ModResult result = svc_websocket->connect(mod_ctx, &ws_desc, &m_wsHandle);
    return result;
}

void WsClient2::request_user_id() {
    if (m_userIdRequested)
        return;
    m_userIdRequested = true;

    std::string url =
        std::string("https://") + TWITCH_API_URL.data() + TWITCH_API_USERS_ENDPOINT.data();
    std::string auth = TWITCH_API_AUTHORIZATION.data() + m_oauth;

    HttpHeader headers[] = {
        {"Authorization", auth.c_str()},
        {TWITCH_API_CLIENT_ID.data(), m_clientId.c_str()},
    };

    HttpRequestDesc desc = HTTP_REQUEST_DESC_INIT;
    desc.method = HTTP_METHOD_GET;
    desc.url = url.c_str();
    desc.headers = headers;
    desc.header_count = 2u;

    svc_http->request(mod_ctx, &desc, &WsClient2::on_user_id_complete, this, &m_userIdHandle);
}

void WsClient2::on_user_id_complete(
    ModContext*, HttpRequestHandle, const HttpResult* result, void* user_data) {
    auto self = static_cast<WsClient2*>(user_data);
    svc_log->info(mod_ctx, ("status=" + std::to_string(result->status_code)).c_str());
    if (result->body && result->body_size > 0) {
        std::string body(static_cast<const char*>(result->body), result->body_size);

        json user_json = json::parse(body);
        self->m_userId =
            user_json.at(JSON_DATA.data()).front().at(JSON_ID.data()).get<std::string>();

        svc_log->debug(mod_ctx, body.c_str());
    }
}

void WsClient2::register_topic(TwitchSubscription topic) {
    if (m_userId.empty()) {
        request_user_id();
        return;
    }

    // wait for the welcome message
    if (m_sessionId.empty())
        return;

    if (m_subscribeRequested)
        return;
    m_subscribeRequested = true;

    json condition;

    condition[JSON_BROADCASTER_USER_ID.data()] = m_userId;
    if (topic.event_type == TwitchEventType::ChatMessage) {
        condition[JSON_USER_ID.data()] = m_userId;
    } else if (topic.event_type == TwitchEventType::Follow) {
        condition[JSON_MODERATOR_USER_ID.data()] = m_userId;
    }

    json body;
    body[JSON_TYPE.data()] = topic.type;
    body[JSON_VERSION.data()] = topic.version;
    body[JSON_CONDITION.data()] = condition;
    body[JSON_TRANSPORT.data()] =
        json{{JSON_METHOD.data(), JSON_WEBSOCKET.data()}, {JSON_SESSION_ID.data(), m_sessionId}};

    std::string body_str = body.dump();
    std::string auth = TWITCH_API_AUTHORIZATION.data() + m_oauth;
    std::string url =
        std::string("https://") + TWITCH_API_URL.data() + TWITCH_EVENT_SUBSCRIPTION_ENDPOINT.data();

    HttpHeader headers[] = {
        {"Authorization", auth.c_str()},
        {TWITCH_API_CLIENT_ID.data(), m_clientId.c_str()},
        {"Content-Type", TWITCH_API_CONTENT_TYPE_JSON.data()},
    };

    HttpRequestDesc desc = HTTP_REQUEST_DESC_INIT;
    desc.method = HTTP_METHOD_POST;
    desc.url = url.c_str();
    desc.headers = headers;
    desc.header_count = 3u;
    desc.body = body_str.data();
    desc.body_size = body_str.size();

    HttpRequestHandle handle = 0;
    svc_http->request(mod_ctx, &desc, &WsClient2::on_user_register_topic_complete, this, &handle);
}

void WsClient2::on_user_register_topic_complete(
    ModContext*, HttpRequestHandle, const HttpResult* result, void* user_data) {
    auto self = static_cast<WsClient2*>(user_data);
    svc_log->info(mod_ctx, ("status=" + std::to_string(result->status_code)).c_str());
    svc_log->info(mod_ctx, static_cast<const char*>(result->body));

    std::string body = result->body && result->body_size > 0 ?
                           std::string(static_cast<const char*>(result->body), result->body_size) :
                           std::string();

    if (result->status_code != 202) {
        svc_log->error(mod_ctx,
            ("subscribe failed status=" + std::to_string(result->status_code) + " - " + body)
                .c_str());
        return;
    }

    svc_log->info(mod_ctx, ("subscribed: " + body).c_str());
    self->m_subscribed = true;
    self->m_subscribeRequested = false;
}

void WsClient2::manage_event_message(std::string data) {
    json data_json = json::parse(data);

    std::string message_type =
        data_json.at(JSON_METADATA.data()).at(JSON_MESSAGE_TYPE.data()).get<std::string>();
    svc_log->info(mod_ctx, (message_type + LOG_MESSAGE_TYPE_RECEIVED.data()).c_str());

    if (message_type == JSON_MESSAGE_TYPE_SESSION_WELCOME.data())
        m_sessionId = data_json.at(JSON_PAYLOAD.data())
                          .at(JSON_SESSION.data())
                          .at(JSON_ID.data())
                          .get<std::string>();
}

WsClient2 g_ws2;
// todo
// nothing but this file should depend on beast, everybody else should be agnostic of the communication protocol

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <string>
#include "twitchStuff/twitchData.h"

#include "constants/internationalisation.h"
#include "configVar.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using json = nlohmann::json;
using tcp = boost::asio::ip::tcp;

class WsClient {
public:
    void toggleSocket() {
        if (!m_running) {
            start("eventsub.wss.twitch.tv", "443",
        get_string_option(g_cvarClientId),
        get_string_option(g_cvarOAuth),
        get_string_option(g_cvarUsername),
        get_string_option(g_cvarTwitchId));
        }else {
            stop();
        }
    }

    void start(std::string host, std::string port, std::string clientId,  std::string oauth, std::string username, std::string userId) {
        m_running = true;
        svc_log->info(mod_ctx, LOG_START_WEBSOCKET.data());
        m_thread = std::thread([this, host, port, clientId, oauth, username, userId] {
            run(host, port, clientId, oauth, username, userId);
        });
    }

    void stop() {
        m_running = false;
        svc_log->info(mod_ctx, LOG_STOP_WEBSOCKET.data());
        if (tcp::socket* socket = m_socket_ptr.load()) {
            beast::error_code ec;
            socket->cancel(ec);
            socket->shutdown(tcp::socket::shutdown_both, ec);
            socket->close(ec);
        }
        if (m_thread.joinable()) m_thread.join();
    }

    bool try_pop_message(TwitchEvent& out) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_messages.empty()) return false;
        out = m_messages.front();
        m_messages.pop();
        return true;
    }

private:
    void push(TwitchEventType type, std::string msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_messages.push(TwitchEvent{type, msg});
    }

    void run(const std::string& host, const std::string& port,
             const std::string& clientId, const std::string& oauth, const std::string& username, const std::string& userId) {
        try {
            net::io_context ioc;
            ssl::context ctx{ssl::context::tlsv12_client};
            ctx.set_default_verify_paths();
            websocket::stream<ssl::stream<tcp::socket>> ws{ioc, ctx};

            // first connection
            tcp::resolver resolver{ioc};
            auto results = resolver.resolve(host, port);
            net::connect(beast::get_lowest_layer(ws), results);

            if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
                throw beast::system_error(
                    beast::error_code(static_cast<int>(::ERR_get_error()),
                                       net::error::get_ssl_category()));
            }
            ws.next_layer().handshake(ssl::stream_base::client);
            ws.handshake(host, "/ws");
            m_socket_ptr = &beast::get_lowest_layer(ws);

            // waiting for welcome message
            beast::flat_buffer buffer;
            ws.read(buffer);
            std::string welcomeData = beast::buffers_to_string(buffer.data());
            json welcomeJson = json::parse(welcomeData);

            std::string message_type = welcomeJson.at("metadata").at("message_type").get<std::string>();
            if (message_type != "session_welcome") {
                throw std::runtime_error(std::format(SESSION_WELCOME_FAILED, welcomeData));
            }
            m_messages.push({TwitchEventType::SessionWelcome, welcomeData});

            // then we have 10s to subscribe to events with the payload id
            // see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
            // todo: allow users to subscribe to whatever they like in config file ?
            std::string session_id = welcomeJson.at("payload").at("session").at("id").get<std::string>();

            std::vector<TwitchSubscription> topics = {
                {TwitchEventType::ChatMessage,"channel.chat.message","1"},
                {TwitchEventType::Follow,"channel.follow","2"},
                {TwitchEventType::Subscribe,"channel.subscribe","1"},
                {TwitchEventType::SubGift,"channel.subscription.gift","1"},
                {TwitchEventType::Cheer,"channel.cheer","1"},
            };

            for (const auto& topic: topics) {
                ssl::stream<tcp::socket> stream{ioc, ctx};
                const std::string api_host = "api.twitch.tv";
                auto results = resolver.resolve(api_host, "443");

                net::connect(beast::get_lowest_layer(stream), results);
                SSL_set_tlsext_host_name(stream.native_handle(), api_host.c_str());
                stream.handshake(ssl::stream_base::client);

                json condition;
                // here I made the choice that the one using the mod wants to interact with their channel as themselves
                condition["broadcaster_user_id"] = userId;
                if (topic.eventType == TwitchEventType::ChatMessage) {
                    condition["user_id"] = userId;
                }else if (topic.eventType == TwitchEventType::Follow) {
                    condition["moderator_user_id"] = userId;
                }

                json body;
                body["type"] = topic.type;
                body["version"] = topic.version;
                body["condition"] = condition;
                body["transport"] = json{
                {"method", "websocket"},
                {"session_id", session_id}
                };

                http::request<http::string_body> request{http::verb::post, "/helix/eventsub/subscriptions", 11};
                request.set(beast::http::field::host, api_host);
                request.set(beast::http::field::authorization, "Bearer " + oauth);
                request.set(http::field::content_type, "application/json");
                request.set("Client-Id", clientId);
                request.body() = body.dump();
                request.prepare_payload();

                http::write(stream, request);

                http::response<http::string_body> response;

                beast::flat_buffer buffer;
                http::read(stream, buffer, response);

                beast::error_code ec;
                stream.shutdown(ec);

                if (response.result_int() != 202) {
                    throw std::runtime_error(std::format(EVENT_SUBSCRIPTION_FAILED, topic.type, std::to_string(response.result_int()), response.body()));
                }
            }

            while (m_running) {
                beast::flat_buffer buffer;
                ws.read(buffer);
                std::string data = beast::buffers_to_string(buffer.data());

                json jsonData = json::parse(data);
                std::string message_type = jsonData.at("metadata").at("message_type").get<std::string>();

                // todo manage the keepalive_timeout_seconds properly
                // see https://dev.twitch.tv/docs/eventsub/handling-websocket-events#welcome-message
                // here we assume connection never breaks, that's optimistic
                if (message_type == "session_keepalive") continue;

                // todo must reconnect:
                // see https://dev.twitch.tv/docs/eventsub/handling-websocket-events#reconnect-message
                if (message_type == "session_reconnect") {
                    push(TwitchEventType::Unknown, data);
                    break;
                }

                // if its a revocation message or close message i just ignore it >:
                // revocation should not occur within a gameplay session (i believe)
                // todo : properly manage close message, or does the socket properly manages it already
                if (message_type != "notification") {
                    push(TwitchEventType::Unknown, data);
                    continue;
                }

                std::string subscription_type =
                    jsonData.at("payload").at("subscription").at("type").get<std::string>();

                TwitchEventType type = TwitchEventType::Unknown;
                if (subscription_type == "channel.chat.message")   type = TwitchEventType::ChatMessage;
                else if (subscription_type == "channel.follow")    type = TwitchEventType::Follow;
                else if (subscription_type == "channel.subscribe") type = TwitchEventType::Subscribe;
                else if (subscription_type == "channel.subscribe.gift") type = TwitchEventType::SubGift;
                else if (subscription_type == "channel.cheer")     type = TwitchEventType::Cheer;

                push(type, data);
            }

        } catch (std::exception const& exception) {
            m_socket_ptr = nullptr;
            if (m_running) {
                push(TwitchEventType::TwitchEventError, std::format(EXCEPTION_MESSAGE, exception.what()));
            }
        }
    }

    std::atomic<bool> m_running{false};
    std::atomic<tcp::socket*> m_socket_ptr{nullptr};
    std::thread m_thread;
    std::mutex m_mutex;
    std::queue<TwitchEvent> m_messages;
};
extern WsClient g_ws;
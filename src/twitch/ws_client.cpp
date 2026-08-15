/* Meow, this async cat is responsible to communicate between twitch and dusklight

            |\__/,|   (`\
            |_ _  |.--.) )
            ( T   )     /
            (((^_(((/(((_/
*/
#include "ws_client.hpp"

#include "mods/svc/log.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <nlohmann/json.hpp>

#include "../config_var.hpp"
#include "../i18n.hpp"
#include "ws_constant.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using json = nlohmann::json;
using tcp = boost::asio::ip::tcp;

namespace {
struct TwitchSubscription {
    // see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
    TwitchEventType event_type;
    std::string type;
    std::string version;
};
}  // namespace

void WsClient::toggle_socket() {
    if (!m_running) {
        // todo username unused, I'd rather have the twitch_id to be unused instead
        const std::string client_id = get_string_option(g_config_var_client_id);
        const std::string oauth = get_string_option(g_config_var_oauth);
        const std::string username = get_string_option(g_config_var_username);
        const std::string twitch_id = get_string_option(g_config_var_twitch_id);
        if (!client_id.empty() && !oauth.empty() && !username.empty() && !twitch_id.empty()) {
            start(TWITCH_WEBSOCKET_URL.data(), HTTPS_PORT.data(), client_id, oauth, username,
                twitch_id);
        } else {
            svc_log->error(mod_ctx, LAUNCH_WEBSOCKET_FAILED.data());
        }
    } else {
        stop();
    }
}

void WsClient::stop() {
    // Stops the thread running loop and puts an error code, so the thread can stops gracefully
    // itself next iteration
    if (m_running) {
        m_running = false;
        svc_log->info(mod_ctx, LOG_STOP_WEBSOCKET.data());
        if (tcp::socket* socket = m_socket_ptr.load()) {
            beast::error_code ec;
            socket->cancel(ec);
        }
        if (m_thread.joinable())
            m_thread.join();
    }
}

bool WsClient::is_started() {
    return m_running;
}

bool WsClient::try_pop_message(TwitchEvent& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_messages.empty())
        return false;
    out = m_messages.front();
    m_messages.pop();
    return true;
}

int WsClient::get_messages_length() const {
    return m_messages.size();
}

void WsClient::start(const std::string& host, const std::string& port, const std::string& client_id,
    const std::string& oauth, const std::string& username, const std::string& user_id) {
    if (!m_running) {
        m_running = true;
        svc_log->info(mod_ctx, LOG_START_WEBSOCKET.data());
        m_thread = std::thread([this, host, port, client_id, oauth, username, user_id] {
            run(host, port, client_id, oauth, username, user_id);
        });
    }
}

void WsClient::push(const TwitchEventType type, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    TwitchEvent twitch_event = {.struct_size = sizeof(TwitchEvent), .type = type, .data = NULL};

    // this is needed for the mod communication to consummers
    char* copy = new char[message.size() + 1];
    std::memcpy(copy, message.c_str(), message.size() + 1);
    twitch_event.data = copy;
    m_messages.push(twitch_event);
}

void WsClient::run(const std::string& host, const std::string& port, const std::string& client_id,
    const std::string& oauth, const std::string& username, const std::string& user_id) {
    // todo split into several method, it feels like it could be, init, running, shutdown
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();
    websocket::stream<ssl::stream<tcp::socket>> ws{ioc, ctx};
    try {
        // first connection
        tcp::resolver resolver{ioc};
        auto results = resolver.resolve(host, port);
        net::connect(beast::get_lowest_layer(ws), results);

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
            throw beast::system_error(beast::error_code(
                static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()));
        }
        ws.next_layer().handshake(ssl::stream_base::client);
        ws.handshake(host, HANDSHAKE_ENDPOINT.data());
        m_socket_ptr = &beast::get_lowest_layer(ws);

        // waiting for welcome message
        beast::flat_buffer buffer;
        ws.read(buffer);
        std::string welcome_data = beast::buffers_to_string(buffer.data());
        json welcome_json = json::parse(welcome_data);

        std::string message_type =
            welcome_json.at(JSON_METADATA.data()).at(JSON_MESSAGE_TYPE.data()).get<std::string>();
        svc_log->info(mod_ctx, (message_type + LOG_MESSAGE_TYPE_RECEIVED.data()).c_str());
        if (message_type != JSON_MESSAGE_TYPE_SESSION_WELCOME.data()) {
            throw std::runtime_error(SESSION_WELCOME_FAILED.data() + welcome_data);
        }
        // TODO make sure the welcome message is valid, and if not, stop the process, and add an
        // error level log

        // then we have 10s to subscribe to events with the payload id
        // see https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/
        // TODO_2: allow users to subscribe to whatever they like in config file ? Extend the
        // service to add a method for consumers to describe what they wish to listen and only
        // subscribe to what is needed
        std::string session_id = welcome_json.at(JSON_PAYLOAD.data())
                                     .at(JSON_SESSION.data())
                                     .at(JSON_ID.data())
                                     .get<std::string>();

        std::vector<TwitchSubscription> topics = {
            {.event_type = TwitchEventType::ChatMessage,
                .type = SUBSCRIPTION_CHAT_MESSAGE.data(),
                .version = SUBSCRIPTION_CHAT_MESSAGE_VERSION.data()},
            {.event_type = TwitchEventType::Follow,
                .type = SUBSCRIPTION_FOLLOW.data(),
                .version = SUBSCRIPTION_FOLLOW_VERSION.data()},
            {.event_type = TwitchEventType::Subscribe,
                .type = SUBSCRIPTION_SUBSCRIBE.data(),
                .version = SUBSCRIPTION_SUBSCRIBE_VERSION.data()},
            {.event_type = TwitchEventType::SubGift,
                .type = SUBSCRIPTION_SUB_GIFT.data(),
                .version = SUBSCRIPTION_SUB_GIFT_VERSION.data()},
            {.event_type = TwitchEventType::Cheer,
                .type = SUBSCRIPTION_CHEER.data(),
                .version = SUBSCRIPTION_CHEER_VERSION.data()},
        };

        results = resolver.resolve(TWITCH_API_URL.data(), HTTPS_PORT.data());
        for (const auto& topic : topics) {
            ssl::stream<tcp::socket> stream{ioc, ctx};

            net::connect(beast::get_lowest_layer(stream), results);
            SSL_set_tlsext_host_name(stream.native_handle(), TWITCH_API_URL.data());
            stream.handshake(ssl::stream_base::client);

            json condition;
            // here I made the choice that the one using the mod wants to interact with their
            // channel as themselves
            condition[JSON_BROADCASTER_USER_ID.data()] = user_id;
            if (topic.event_type == TwitchEventType::ChatMessage) {
                condition[JSON_USER_ID.data()] = user_id;
            } else if (topic.event_type == TwitchEventType::Follow) {
                condition[JSON_MODERATOR_USER_ID.data()] = user_id;
            }

            json body;
            body[JSON_TYPE.data()] = topic.type;
            body[JSON_VERSION.data()] = topic.version;
            body[JSON_CONDITION.data()] = condition;
            body[JSON_TRANSPORT.data()] = json{
                {JSON_METHOD.data(), JSON_WEBSOCKET.data()}, {JSON_SESSION_ID.data(), session_id}};

            http::request<http::string_body> request{
                http::verb::post, TWITCH_EVENT_SUBSCRIPTION_ENDPOINT.data(), HTTP_VERSION};
            request.set(beast::http::field::host, TWITCH_API_URL.data());
            request.set(beast::http::field::authorization, TWITCH_API_AUTHORIZATION.data() + oauth);
            request.set(http::field::content_type, TWITCH_API_CONTENT_TYPE_JSON.data());
            request.set(TWITCH_API_CLIENT_ID.data(), client_id);
            request.body() = body.dump();
            request.prepare_payload();

            http::write(stream, request);

            http::response<http::string_body> response;

            beast::flat_buffer buffer;
            http::read(stream, buffer, response);

            beast::error_code ec;
            stream.shutdown(ec);

            if (response.result() != http::status::accepted) {
                // todo ... that is truly ugly (but cannot use std::format, is there a clean way
                // to format text like this ("fish and {}", "chips"))
                // maybe fmt::format ?
                std::string error_message = EVENT_SUBSCRIPTION_FAILED.data() + topic.type + " " +
                                            std::to_string(response.result_int()) + " - " +
                                            response.body();
                throw std::runtime_error(error_message);
            }
        }

        while (m_running) {
            beast::flat_buffer buffer;
            beast::error_code ec;
            ws.read(buffer, ec);

            if (ec == net::error::operation_aborted) {
                // stop has been called
                break;
            }
            if (ec == websocket::error::closed) {
                // see https://dev.twitch.tv/docs/eventsub/websocket-reference Close message
                // todo: manage some errors there ? problem for later me
                break;
            }
            if (ec) {
                // else could be some network error ? Everything explodes (Me being dramatic)
                throw beast::system_error(ec);
            }

            std::string data = beast::buffers_to_string(buffer.data());
            svc_log->debug(mod_ctx, data.c_str());

            json json_data = json::parse(data);
            std::string message_type =
                json_data.at(JSON_METADATA.data()).at(JSON_MESSAGE_TYPE.data()).get<std::string>();

            // TODO_3 manage the keepalive_timeout_seconds properly
            // see https://dev.twitch.tv/docs/eventsub/handling-websocket-events#welcome-message
            // here we assume connection never breaks, that's optimistic
            if (message_type == JSON_MESSAGE_TYPE_SESSION_KEEPALIVE.data())
                continue;

            // TODO_3 must reconnect:
            // see
            // https://dev.twitch.tv/docs/eventsub/handling-websocket-events#reconnect-message
            if (message_type == JSON_MESSAGE_TYPE_SESSION_RECONNECT.data()) {
                push(TwitchEventType::Unknown, data);
                break;
            }

            // if its a revocation message or close message i just ignore it >:
            // revocation should not occur within a gameplay session (i believe)
            // TODO_3 : properly manage close message, or does the socket properly manages it
            // already
            if (message_type != JSON_MESSAGE_TYPE_NOTIFICATION.data()) {
                push(TwitchEventType::Unknown, data);
                continue;
            }

            std::string subscription_type = json_data.at(JSON_PAYLOAD.data())
                                                .at(JSON_SUBSCRIPTION.data())
                                                .at(JSON_TYPE.data())
                                                .get<std::string>();

            TwitchEventType type = TwitchEventType::Unknown;
            if (subscription_type == SUBSCRIPTION_CHAT_MESSAGE.data())
                type = TwitchEventType::ChatMessage;
            else if (subscription_type == SUBSCRIPTION_FOLLOW.data())
                type = TwitchEventType::Follow;
            else if (subscription_type == SUBSCRIPTION_SUBSCRIBE.data())
                type = TwitchEventType::Subscribe;
            else if (subscription_type == SUBSCRIPTION_SUB_GIFT.data())
                type = TwitchEventType::SubGift;
            else if (subscription_type == SUBSCRIPTION_CHEER.data())
                type = TwitchEventType::Cheer;

            svc_log->info(mod_ctx, (subscription_type + LOG_MESSAGE_TYPE_RECEIVED.data()).c_str());

            push(type, data);
        }
    } catch (std::exception const& exception) {
        m_socket_ptr = nullptr;
        if (m_running) {
            push(TwitchEventType::TwitchEventError,
                EXCEPTION_MESSAGE.data() + std::string(exception.what()));
        }
    }

    // stop has been called, now we can properly close the websocket
    if (ws.is_open()) {
        beast::error_code close_ec;
        ws.close(websocket::close_code::normal, close_ec);
        if (close_ec != ssl::error::stream_truncated) {
            svc_log->warn(mod_ctx,
                (std::string(LOG_WEBSOCKET_CLOSE_FAILED.data()) + close_ec.message()).c_str());
        } else if (close_ec) {
            // twitch does not notify properly of the tls shutdown
            // see
            // https://www.boost.org/doc/libs/develop/libs/beast/example/http/server/async-ssl/http_server_async_ssl.cpp
            // it is safe with websocket though
            svc_log->debug(mod_ctx, LOG_WEBSOCKET_STREAM_TRUNCATED.data());
        }
    }
}

WsClient g_ws;
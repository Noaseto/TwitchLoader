#pragma once

#include <atomic>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "../twitch_loader_service.h"

class WsClient {
public:
    void toggle_socket();
    void stop();
    bool is_started();
    bool try_pop_message(TwitchEvent& out);
    int get_messages_length() const;

private:
    void start(const std::string& host, const std::string& port, const std::string& client_id,
        const std::string& oauth);
    void push(TwitchEventType type, const std::string& message);
    std::string get_user_id(const std::string& client_id, const std::string& oauth,
        boost::asio::io_context& ioc, boost::asio::ssl::context& ctx,
        boost::asio::ip::tcp::resolver& resolver);
    void run(const std::string& host, const std::string& port, const std::string& client_id,
        const std::string& oauth);

    std::atomic<bool> m_running{false};
    std::atomic<boost::asio::ip::tcp::socket*> m_socket_ptr{nullptr};
    std::thread m_thread;
    std::mutex m_mutex;
    std::queue<TwitchEvent> m_messages;
};

extern WsClient g_ws;
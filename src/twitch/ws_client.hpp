#pragma once

#include <atomic>
// todo remove dependency? I want to be agnostic of the way we communicate with twitch Events
// see https://arne-mertz.de/2019/01/the-pimpl-idiom/
#include <boost/asio/ip/tcp.hpp>
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
        const std::string& oauth, const std::string& username, const std::string& user_id);
    void push(TwitchEventType type, const std::string& message);
    void run(const std::string& host, const std::string& port, const std::string& client_id,
        const std::string& oauth, const std::string& username, const std::string& user_id);

    std::atomic<bool> m_running{false};
    std::atomic<boost::asio::ip::tcp::socket*> m_socket_ptr{nullptr};
    std::thread m_thread;
    std::mutex m_mutex;
    std::queue<TwitchEvent> m_messages;
};

extern WsClient g_ws;
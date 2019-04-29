#pragma once

#include "proto/message.pb.h"

#include <uv.h>
#include <string>

class Endpoint;
class UdpServiceImpl;
struct IMessage;

struct UdpOption {
    int af;
    std::string ip;
    uint16_t port;

    UdpOption() : af(AF_INET), ip("0.0.0.0"), port(0) {}
};

class UdpService {
public:
    UdpService(uv_loop_t& loop, const UdpOption opts);
    ~UdpService();

    bool start();
    void stop();

    bool send(const Endpoint& peer, const IMessage& message);

    struct IMessageHandler {
        virtual ~IMessageHandler() { }
        virtual void handleMessage(const Endpoint& peer, 
            const proto::Message& message) = 0;
    };

    void addMessageHandler(IMessageHandler* handler);

private:
    UdpServiceImpl* m_pImpl;
    UdpServiceImpl& m_impl;
};
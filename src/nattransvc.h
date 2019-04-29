#pragma once

#include <uv.h>

class Endpoint;
class UdpService;
class NatTraversalServiceImpl;

class NatTraversalService {
public:
    NatTraversalService(uv_loop_t& loop, UdpService& udpsvc);

    void bootstrap(const Endpoint& peer);

private:
    NatTraversalServiceImpl* m_pImpl;
    NatTraversalServiceImpl& m_impl;
};
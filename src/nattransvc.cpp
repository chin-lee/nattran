#include "nattransvc.h"
#include "udpsvc.h"
#include "endpoint.h"
#include "util.h"
#include "log.h"
#include "message.h"
#include "timer.h"

#include "proto/message.pb.h"

#include <arpa/inet.h>

static const int kGetNodeRetryInternvalMillis = 5000;
static const int kPingPeerIntervalMillis = 5000;

struct Peer {
    Endpoint endpoint;
    Timer* pingTimer;
    uint64_t lastTimeRecvPing;

    explicit Peer(const Endpoint& ep)
        : endpoint(ep), pingTimer(NULL), lastTimeRecvPing(0) {}

    DEFAULT_COPY_MOVE_AND_ASSIGN(Peer);

public:
    bool isAlive() const {
        return ( (util::currentMilliSecs() - lastTimeRecvPing) 
            < (3 * kPingPeerIntervalMillis) );
    }
};

// -----------------------------------------------------------------------------
// Section: NatTraversalServiceImpl
// -----------------------------------------------------------------------------
class NatTraversalServiceImpl : public UdpService::IMessageHandler {
    uv_loop_t& m_loop;
    UdpService& m_udpsvc;
    Timer* m_getNodeTimer;
    
    typedef std::map<Endpoint, Peer*> PeerMap;
    PeerMap m_peerMap;

public:
    NatTraversalServiceImpl(uv_loop_t& loop, UdpService& udpsvc) 
        : m_loop(loop), m_udpsvc(udpsvc) {
        udpsvc.addMessageHandler(this);
    }

    void bootstrap(const Endpoint& peer) {
        getNode(peer);

        int millis = kGetNodeRetryInternvalMillis;
        m_getNodeTimer = Timer::create(m_loop);
        m_getNodeTimer->start(millis, millis, [=]() {
            getNode(peer);
        });

        addPeer(peer);
    }

    virtual void handleMessage(const Endpoint& peer, 
                               const proto::Message& message) {
        switch (message.type()) {
        case proto::Message_Type_PING:
            {
                Message<proto::Ping> msg;
                if (!msg.parseFromProto(message)) {
                    LOGE << "error parsing PING message from "
                         << peer.ip() << ":" << peer.port();
                } else {
                    handlePing(peer, msg.payload);
                }
            }
            break;
        case proto::Message_Type_GET_NODE:
            LOGI << "received GET_NODE message from " << peer.ip() << ":" 
                 << peer.port();
            handleGetNode(peer);    
            break;
        case proto::Message_Type_RETURN_NODE:
            {
                m_getNodeTimer->stop();
                m_getNodeTimer->destroy();
                m_getNodeTimer = NULL;

                Message<proto::ReturnNode> msg;
                if (!msg.parseFromProto(message)) {
                    LOGE << "error parsing RETURN_CODE message from "
                        << peer.ip() << ":" << peer.port();
                } else {
                    handleReturnNode(peer, msg.payload);
                }
            }
            break;
        }
    }

private:
    void ping(const Endpoint& peer) {
        LOGI << "ping " << peer.ip() << ":" << peer.port();
        Message<proto::Ping> msg(proto::Message_Type_PING);
        if (!peer.serializeToProto(*msg.payload.mutable_addr())) {
            LOGE << "failed to serialize peer address";
            return;
        }
        m_udpsvc.send(peer, msg);
    }

    void getNode(const Endpoint& peer) {
        LOGI << "get node from " << peer.ip() << ":" << peer.port();
        Message<proto::GetNode> msg(proto::Message_Type_GET_NODE);
        m_udpsvc.send(peer, msg);
    }

    Peer* addPeer(const Endpoint& ep) {
        auto it = m_peerMap.find(ep);
        if (it != m_peerMap.end()) {
            return it->second;
        }

        LOGI << "new peer found " << ep.ip() << ":" << ep.port();
        Peer* peer = new Peer(ep);
        m_peerMap.emplace(ep, peer);

        ping(ep);
        int millis = kPingPeerIntervalMillis;
        peer->pingTimer = Timer::create(m_loop);
        peer->pingTimer->start(millis, millis, [=]() {
            ping(peer->endpoint);
        });
        return peer;
    }

    void handlePing(const Endpoint& ep, const proto::Ping& msg) {
        Endpoint myAddr(msg.addr());
        LOGI << "received PING from " << ep.ip() << ":" << ep.port() 
             << ", my address is " << myAddr.ip() << ":" << myAddr.port();

        Peer* peer = addPeer(ep);
        peer->lastTimeRecvPing = util::currentMilliSecs();
    }

    void handleGetNode(const Endpoint& peer) {
        addPeer(peer);

        Message<proto::ReturnNode> msg(proto::Message_Type_RETURN_NODE);
        for (auto it = m_peerMap.begin(); it != m_peerMap.end(); ++it) {
            if (it->first == peer) {
                continue;
            }
            Peer* peer = it->second;
            if (!peer->isAlive()) {
                continue;
            }
            proto::Node* node = msg.payload.add_nodes();
            if (!peer->endpoint.serializeToProto(*node->mutable_addr())) {
                LOGE << "failed to serialize peer address to proto";
                msg.payload.mutable_nodes()->RemoveLast();
                continue;
            }
        }
        if (msg.payload.nodes().empty()) {
            LOGD << "no node will be sent to "
                 << peer.ip() << ":" << peer.port();
            return;
        }
        LOGD << "send " << msg.payload.nodes().size() << " nodes to "
                << peer.ip() << ":" << peer.port();
        m_udpsvc.send(peer, msg);
    }

    void handleReturnNode(const Endpoint& peer, const proto::ReturnNode& msg) {
        LOGI << "get " << msg.nodes().size() << " node(s) from " 
             << peer.ip() << ":" << peer.port();
        for (int i = 0; i < msg.nodes().size(); i++) {
            const proto::Node& node = msg.nodes().Get(i);
            Endpoint ep(node.addr());
            addPeer(ep);
        }
    }

};

// -----------------------------------------------------------------------------
// Section: NatTraversalService
// -----------------------------------------------------------------------------
NatTraversalService::NatTraversalService(uv_loop_t& loop, UdpService& udpsvc)
    : m_pImpl(new NatTraversalServiceImpl(loop, udpsvc)), m_impl(*m_pImpl) {
}

void NatTraversalService::bootstrap(const Endpoint& peer) {
    m_impl.bootstrap(peer);
}
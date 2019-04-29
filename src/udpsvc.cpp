#include "udpsvc.h"
#include "endpoint.h"
#include "message.h"
#include "log.h"
#include "util.h"

#include "proto/message.pb.h"

typedef UdpService::IMessageHandler IMessageHandler;

// -----------------------------------------------------------------------------
// Section: SendReq
// -----------------------------------------------------------------------------
struct SendReq {
    uv_udp_send_t handle;
    Endpoint peer;
    int cbData;
    char data[1];

    static SendReq* create(const Endpoint& peer, 
                           const proto::Message& message) {
        int msgSize = message.ByteSize();
        SendReq* req = (SendReq*)malloc(sizeof(SendReq) + msgSize);
        new(&req->peer) Endpoint(0, "", 0);
        req->peer = peer;
        req->cbData = msgSize;
        message.SerializeToArray(req->data, msgSize);
        return req;
    }

    uv_buf_t buf() {
        return uv_buf_t{data, size_t(cbData)};
    }

    void release() {
        free(this);
    }
};

// -----------------------------------------------------------------------------
// Section: UdpServiceImpl
// -----------------------------------------------------------------------------
class UdpServiceImpl {

    uv_loop_t& m_loop;
    Endpoint m_endpoint;

    uv_udp_t m_udphandle;
    uint32_t m_nextMsgId;

    std::vector<IMessageHandler*> m_handlers;

public:
    static void handleAlloc(uv_handle_t* handle, 
                            size_t suggested_size, 
                            uv_buf_t* buf) {
        buf->base = (char*)malloc(suggested_size);
        buf->len = suggested_size;
        (void)handle;
    }

    static void handleRecv(uv_udp_t* handle, ssize_t nread, 
                           const uv_buf_t* buf, 
                           const struct sockaddr* addr, 
                           unsigned flags) {
        UdpServiceImpl* udpsvc = CONTAINER_OF(handle, UdpServiceImpl, 
                                              m_udphandle);
        if (0 == nread && NULL == addr) {
            // LOGT << "there is nothing to read";
            free(buf->base);
            return;
        }
        Endpoint peer(addr);
        // LOGT << "received " << nread << " byte(s) from " << peer.ip() << ":" 
        //      << peer.port();

        if ( (flags & UV_UDP_PARTIAL) != 0 ) {
            LOGE << "partial data received from " << peer.ip() << ":" 
                 << peer.port();
            free(buf->base);
            return;
        }
        if (nread > 0) {
            proto::Message msg;
            if (!msg.ParseFromArray(buf->base, nread)) {
                LOGE << "error parsing message from " << peer.ip() 
                     << ":" << peer.port();
                free(buf->base);
                return;
            }
            LOGD << "received message type " << msg.type() << " from "
                 << peer.ip() << ":" << peer.port();
            udpsvc->handleMessage(peer, msg);
        }
    }

    static void handleSend(uv_udp_send_t* req, int status) {
        SendReq* sr = CONTAINER_OF(req, SendReq, handle);
        if (status) {
            LOGE << "error sending message to " << sr->peer.ip() 
                 << ":" << sr->peer.port();
        } else {
            // LOGT << "message has been sent to "  << sr->peer.ip() 
            //      << ":" << sr->peer.port();
        }
        sr->release();
    }

public:
    UdpServiceImpl(uv_loop_t& mainloop, const UdpOption& opts) 
        : m_loop(mainloop)
        , m_endpoint(opts.af, opts.ip, opts.port)
        , m_nextMsgId(0) {
        m_handlers.reserve(128);
    }

    bool start() {
        int retval = uv_udp_init(&m_loop, &m_udphandle);
        if (retval != 0) {
            LOGE << "uv_udp_init: " << uv_strerror(retval);
            return false;
        }
        retval = uv_udp_bind(&m_udphandle, m_endpoint.addr(), 
                             UV_UDP_REUSEADDR);
        if (retval != 0) {
            LOGE << "uv_udp_bind: " << uv_strerror(retval);
            return false;
        }
        retval = uv_udp_recv_start(&m_udphandle, handleAlloc, handleRecv);
        if (retval != 0) {
            LOGE << "uv_udp_recv_start: " << uv_strerror(retval);
            return false;
        }
        LOGI << "udp service listen on " << m_endpoint.ip() 
             << ":" << m_endpoint.port();
        return true;
    }

    bool send(const Endpoint& peer, const IMessage& message) {
        proto::Message msgOut;
        msgOut.set_id(++m_nextMsgId);
        msgOut.set_type(message.getType());
        msgOut.set_payload(message.getPayload());
        SendReq* req = SendReq::create(peer, msgOut);
        uv_buf_t bufs[1] = { req->buf() };
        int retval = uv_udp_send(&req->handle, &m_udphandle, bufs, 1, 
                                 req->peer.addr(), handleSend);
        if (retval != 0) {
            LOGE << "uv_udp_send: " << uv_strerror(retval);
            return false;
        }
        return true;
    }

    void addMessageHandler(IMessageHandler* handler) {
        if (std::find(m_handlers.begin(), m_handlers.end(), handler) 
                == m_handlers.end()) {
            m_handlers.push_back(handler);
        }
    }

private:
    void handleMessage(const Endpoint& peer, const proto::Message& message) {
        for (auto handler : m_handlers) {
            handler->handleMessage(peer, message);
        }
    }
};

// -----------------------------------------------------------------------------
// Section: UdpService
// -----------------------------------------------------------------------------
UdpService::UdpService(uv_loop_t& loop, const UdpOption opts) 
    : m_pImpl(new UdpServiceImpl(loop, opts)), m_impl(*m_pImpl) {

}

UdpService::~UdpService() {

}

bool UdpService::start() {
    return m_impl.start();
}

void UdpService::stop() {

}

bool UdpService::send(const Endpoint& peer, const IMessage& message) {
    return m_impl.send(peer, message);
}

void UdpService::addMessageHandler(IMessageHandler* handler) {
    m_impl.addMessageHandler(handler);
}
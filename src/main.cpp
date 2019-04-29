#include "log.h"
#include "udpsvc.h"
#include "longopt.h"
#include "util.h"
#include "async.h"
#include "endpoint.h"
#include "message.h"
#include "nattransvc.h"

#include "proto/message.pb.h"

#include <uv.h>
#include <iostream>
#include <thread>

static const option_t kOptions[] = {
    { '-', NULL, 0, NULL, "arguments:" },
    { 'l', "listen", LONGOPT_REQUIRE, NULL, "" },
    { 'b', "bootstrap", LONGOPT_REQUIRE, NULL, "" },
    { 0, NULL, 0, NULL, NULL }
};

static bool parseIpPort(const std::string& src, 
                        std::string& ip, 
                        uint16_t& port) {
    int pos = src.find_first_of(":");
    if (pos > 0) {
        ip = src.substr(0, pos);
        port = uint16_t( atoi(src.substr(pos + 1).c_str()) );
    } else if (pos == 0) {
        ip = "0.0.0.0";
        port = uint16_t( atoi(src.substr(pos + 1).c_str()) );
    } else {
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {

    std::string listenAddr;
    std::string bootstrap;
    int opt;
    LOGI << "parsing args";
    while ( (opt = longopt(argc, argv, kOptions)) != LONGOPT_DONE ) {
        if (LONGOPT_NEED_PARAM == opt) {
            const option_t& ent = kOptions[errindex];
            LOGE << "missing parameter for -" << char(ent.val) 
                 << "--" << ent.name;
            exit(1);
        } else if (opt <= 0 || opt >= (int)ARRAY_SIZE(kOptions)) {
            continue;
        }
        switch (opt) {
        case 1:
            listenAddr = optparam;
            break;
        case 2:
            bootstrap = optparam;
            break;
        }
    }

    if (listenAddr.empty()) {
        const option_t& ent = kOptions[1];
        LOGE << "missing argument -" << char(ent.val) << " --" << ent.name;
        exit(1);
    }

    UdpOption opts;
    opts.af = AF_INET;
    if (!parseIpPort(listenAddr, opts.ip, opts.port)) {
        LOGE << "invalid listen address: " << listenAddr;
        exit(1);
    }

    std::string bootstrapIp;
    uint16_t bootstrapPort;
    if (!bootstrap.empty()) {
        if (!parseIpPort(bootstrap, bootstrapIp, bootstrapPort)) {
            LOGE << "invalid bootstrap address: " << listenAddr;
            exit(1);
        }
    }

    uv_loop_t mainloop;
    uv_loop_init(&mainloop);

    AsyncHandler handler(mainloop);
    UdpService udpsvc(mainloop, opts);
    NatTraversalService nattransvc(mainloop, udpsvc);

    std::thread t([&mainloop]() {
        uv_run(&mainloop, UV_RUN_DEFAULT);
    });

    handler.post([&]() {
        udpsvc.start();
    });

    if (!bootstrapIp.empty()) {
        handler.post([&]() {
            Endpoint peer(AF_INET, bootstrapIp, bootstrapPort);
            nattransvc.bootstrap(peer);
        });
    }

    t.join();
    uv_loop_close(&mainloop);

    return 0;
}
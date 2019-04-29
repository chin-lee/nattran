#pragma once

#include "proto/message.pb.h"

struct IMessage {
    virtual proto::Message::Type getType() const = 0;
    virtual std::string getPayload() const = 0;
};

template <class Payload>
struct Message : public IMessage {
    proto::Message::Type type;
    Payload payload;

    Message() {

    }

    explicit Message(proto::Message::Type t) : type(t) {

    }

    virtual proto::Message::Type getType() const {
        return type;
    }

    virtual std::string getPayload() const {
        return payload.SerializeAsString();
    }

    bool parseFromProto(const proto::Message& msg) {
        type = proto::Message::Type(msg.type());
        return payload.ParseFromString(msg.payload());
    }
};
#pragma once

#include <google/protobuf/message.h>
#include <common/exception.h>

namespace NRpc {

// Error codes specific to protobuf operations
enum class EProtoError {
    DeserializationError,
    SerializationError,
    InvalidMessage
};

// Exception class for protobuf errors
class TProtoException : public NCommon::TException {
public:
    template<typename... Args>
    TProtoException(EProtoError code, const std::string& format, Args&&... args)
        : NCommon::TException(format, std::forward<Args>(args)...), Code_(code)
    {}

    EProtoError ProtoErrorCode() const {
        return Code_;
    }

private:
    EProtoError Code_;
};

} // namespace NRpc

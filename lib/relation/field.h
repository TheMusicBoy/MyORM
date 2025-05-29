#pragma once

#include <relation/base.h>

#include <variant>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

// Structure for boolean field information
struct TBoolFieldInfo {
    bool defaultValue;
};

// Structure for 32-bit integer field information
struct TInt32FieldInfo {
    int32_t defaultValue;
};

// Structure for unsigned 32-bit integer field information
struct TUInt32FieldInfo {
    uint32_t defaultValue;
};

// Structure for 64-bit integer field information
struct TInt64FieldInfo {
    int64_t defaultValue;
};

// Structure for unsigned 64-bit integer field information
struct TUInt64FieldInfo {
    uint64_t defaultValue;
};

// Structure for float field information
struct TFloatFieldInfo {
    float defaultValue;
};

// Structure for double field information
struct TDoubleFieldInfo {
    double defaultValue;
};

// Structure for string field information
struct TStringFieldInfo {
    std::string defaultValue;
};

// Structure for bytes field information
struct TBytesFieldInfo {
    std::vector<unsigned char> defaultValue;
};

// Structure for enumeration field information
struct TEnumFieldInfo {
    int32_t defaultValue;
    google::protobuf::EnumDescriptor* descriptor;
};

using TValueInfo = std::variant<
    std::monostate,
    TBoolFieldInfo,
    TInt32FieldInfo,
    TUInt32FieldInfo,
    TInt64FieldInfo,
    TUInt64FieldInfo,
    TFloatFieldInfo,
    TDoubleFieldInfo,
    TStringFieldInfo,
    TBytesFieldInfo,
    TEnumFieldInfo>;

// Field type information structure
class TPrimitiveFieldInfo : public TFieldBase {
  public:
    // Accessor for DefaultValueString_
    const std::string& GetDefaultValueString() const;

    // Accessor for TypeInfo_
    const TValueInfo& GetTypeInfo() const;

    // Constructor from google::protobuf::FieldDescriptor*
    TPrimitiveFieldInfo(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path);

  private:
    void HandleBoolField(const google::protobuf::FieldDescriptor* field);
    void HandleInt32Field(const google::protobuf::FieldDescriptor* field);
    void HandleUInt32Field(const google::protobuf::FieldDescriptor* field);
    void HandleInt64Field(const google::protobuf::FieldDescriptor* field);
    void HandleUInt64Field(const google::protobuf::FieldDescriptor* field);
    void HandleFloatField(const google::protobuf::FieldDescriptor* field);
    void HandleDoubleField(const google::protobuf::FieldDescriptor* field);
    void HandleStringField(const google::protobuf::FieldDescriptor* field);
    void HandleBytesField(const google::protobuf::FieldDescriptor* field);
    void HandleEnumField(const google::protobuf::FieldDescriptor* field);

    std::string DefaultValueString_;

    // Type-dependent field information using std::variant
    TValueInfo TypeInfo_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

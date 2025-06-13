#pragma once

#include <common/intrusive_ptr.h>

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
    bool increment;
};

// Structure for unsigned 32-bit integer field information
struct TUInt32FieldInfo {
    uint32_t defaultValue;
    bool increment;
};

// Structure for 64-bit integer field information
struct TInt64FieldInfo {
    int64_t defaultValue;
    bool increment;
};

// Structure for unsigned 64-bit integer field information
struct TUInt64FieldInfo {
    uint64_t defaultValue;
    bool increment;
};

// Structure for float field information
struct TFloatFieldInfo {
    float defaultValue;
    bool increment;
};

// Structure for double field information
struct TDoubleFieldInfo {
    double defaultValue;
    bool increment;
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

    std::string GetId() const {
        return Format("f_{}", GetPath().number());
    }

    std::string GetTableId() const {
        return Format("t_{num;name=false;onlydelim;delimiter='_'}", GetPath().parent());
    }

    bool IsMessage() const override {
        return false;
    }

    bool HasDefaultValue() const {
        return HasDefault_;
    }

    bool IsRequired() const {
        return IsRequired_;
    }

    bool IsPrimaryKey() const {
        return IsPrimaryKey_;
    }

    bool AutoIncrement() const {
        return AutoIncrement_;
    }

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

    bool HasDefault_;
    bool Unique_;
    bool IsRequired_;
    bool IsPrimaryKey_;
    bool AutoIncrement_;

    std::string DefaultValueString_;

    // Type-dependent field information using std::variant
    TValueInfo TypeInfo_;
};

using TPrimitiveFieldInfoPtr = std::shared_ptr<TPrimitiveFieldInfo>;

////////////////////////////////////////////////////////////////////////////////

class TPrimitiveFieldIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TPrimitiveFieldInfoPtr;
    using difference_type = std::ptrdiff_t;
    using pointer = TPrimitiveFieldInfoPtr*;
    using reference = TPrimitiveFieldInfoPtr&;

    TPrimitiveFieldIterator(
        std::map<int, TFieldBasePtr>::iterator it,
        std::map<int, TFieldBasePtr>::iterator end
    );

    TPrimitiveFieldIterator& operator++();
    TPrimitiveFieldIterator operator++(int);
    bool operator==(const TPrimitiveFieldIterator& other) const;
    bool operator!=(const TPrimitiveFieldIterator& other) const;
    TPrimitiveFieldInfoPtr operator*() const;
    TPrimitiveFieldInfoPtr operator->() const;

  private:
    void skipMessageFields();

    std::map<int, TFieldBasePtr>::iterator it_;
    std::map<int, TFieldBasePtr>::iterator end_;
};

class TPrimitiveFieldsRange {
  public:
    TPrimitiveFieldsRange(std::map<int, TFieldBasePtr>& fields);

    TPrimitiveFieldIterator begin();
    TPrimitiveFieldIterator end();

  private:
    std::map<int, TFieldBasePtr>& fields_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

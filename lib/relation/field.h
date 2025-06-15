#pragma once

#include <common/intrusive_ptr.h>

#include <relation/base.h>

#include <variant>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

struct TBoolFieldInfo {
    bool defaultValue;
};

struct TInt32FieldInfo {
    int32_t defaultValue;
    bool increment;
};

struct TUInt32FieldInfo {
    uint32_t defaultValue;
    bool increment;
};

struct TInt64FieldInfo {
    int64_t defaultValue;
    bool increment;
};

struct TUInt64FieldInfo {
    uint64_t defaultValue;
    bool increment;
};

struct TFloatFieldInfo {
    float defaultValue;
    bool increment;
};

struct TDoubleFieldInfo {
    double defaultValue;
    bool increment;
};

struct TStringFieldInfo {
    std::string defaultValue;
};

struct TBytesFieldInfo {
    std::vector<unsigned char> defaultValue;
};

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

class TPrimitiveFieldInfo : public TFieldBase {
  public:
    const std::string& GetDefaultValueString() const;

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

#pragma once

#include <relation/config.h>
#include <relation/path.h>

#include <google/protobuf/descriptor.pb.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

enum EFieldType {
    SINGULAR,
    REPEATED,
    MAP,
    OPTIONAL,
    ONEOF,

    FIELD_TYPE_COUNT
};

struct TFieldTypeInfo {
    virtual constexpr EFieldType GetFieldType() const = 0;
};

using TFieldTypeInfoPtr = std::shared_ptr<TFieldTypeInfo>;

struct TSingularFieldInfo : public TFieldTypeInfo {
    constexpr EFieldType GetFieldType() const override {
        return EFieldType::SINGULAR;
    }
};

struct TRepeatedFieldInfo : public TFieldTypeInfo {
    constexpr EFieldType GetFieldType() const override {
        return EFieldType::REPEATED;
    }
};

struct TMapFieldInfo : public TFieldTypeInfo {
    constexpr EFieldType GetFieldType() const override {
        return EFieldType::MAP;
    }

    google::protobuf::FieldDescriptor::Type KeyType;
};

struct TOptionalFieldInfo : public TFieldTypeInfo {
    constexpr EFieldType GetFieldType() const override {
        return EFieldType::OPTIONAL;
    }
};

struct TOneofFieldInfo : public TFieldTypeInfo {
    constexpr EFieldType GetFieldType() const override {
        return EFieldType::ONEOF;
    }

    int OneofIndex;
};

TFieldTypeInfoPtr GetFieldTypeInfo(const google::protobuf::FieldDescriptor* desc);

////////////////////////////////////////////////////////////////////////////////

class TMessageBase {
  public:
    virtual ~TMessageBase() = default;

    virtual const TMessagePath& GetPath() const = 0;
    std::string GetTableName() const;
};

using TMessageBasePtr = std::shared_ptr<TMessageBase>;

////////////////////////////////////////////////////////////////////////////////

/**
 * @class TFieldBase
 * @brief A base class for fields representing a specific data type in a
 * protocol buffer context.
 *
 * This class provides basic functionality to manage field properties such as
 * field number, name, value type, and field type.
 */
class TFieldBase : virtual public TMessageBase {
  public:
    TFieldBase(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path);
    virtual ~TFieldBase() = default;

    int GetFieldNumber() const;
    virtual bool IsMessage() const = 0;
    const std::string& GetName() const;
    google::protobuf::FieldDescriptor::Type GetValueType() const;
    EFieldType GetFieldType() const;
    const TMessagePath& GetPath() const override;
    const google::protobuf::FieldDescriptor*  GetFieldDescriptor() const;

  protected:
    int FieldNumber_;
    std::string Name_;
    google::protobuf::FieldDescriptor::Type ValueType_;
    TFieldTypeInfoPtr FieldTypeInfo_;
    TMessagePath Path_;
    const google::protobuf::FieldDescriptor* FieldDescriptor_;
};

using TFieldBasePtr = std::shared_ptr<TFieldBase>;

////////////////////////////////////////////////////////////////////////////////

class TRootBase : virtual public TMessageBase {
  public:
    TRootBase(TTableConfigPtr config);
    virtual ~TRootBase() = default;

    const TMessagePath& GetPath() const override;

    const google::protobuf::Descriptor* GetDescriptor() const;

    int Number() const;

    const std::string& GetCamelCase() const;

    const std::string& GetSnakeCase() const;

  private:
    int Number_;
    std::string CamelCase_;
    std::string SnakeCase_;

    TMessagePath Path_;
    const google::protobuf::Descriptor* Descriptor_;
};

using TRootBasePtr = std::shared_ptr<TRootBase>;

////////////////////////////////////////////////////////////////////////////////

class TFieldIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TFieldBasePtr;
    using difference_type = std::ptrdiff_t;
    using pointer = TFieldBasePtr*;
    using reference = TFieldBasePtr&;

    TFieldIterator(std::map<int, TFieldBasePtr>::iterator it);

    TFieldIterator& operator++();
    TFieldIterator operator++(int);
    bool operator==(const TFieldIterator& other) const;
    bool operator!=(const TFieldIterator& other) const;
    TFieldBasePtr operator*() const;
    TFieldBasePtr operator->() const;

  private:
    std::map<int, TFieldBasePtr>::iterator it_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldsRange {
public:
    TFieldsRange(std::map<int, TFieldBasePtr>& fields);

    TFieldIterator begin();
    TFieldIterator end();

private:
    std::map<int, TFieldBasePtr>& fields_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
requires requires { google::protobuf::GetEnumDescriptor<T>(); } && std::is_enum_v<T>
inline void FormatHandler(std::ostringstream& out, T value, const FormatOptions& options) {
    auto desc = google::protobuf::GetEnumDescriptor<T>();
    if (auto v = desc->FindValueByNumber(value)) {
        FormatHandler(out, std::string(v->name()), options);
    } else {
        FormatHandler(out, static_cast<int>(value), options);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

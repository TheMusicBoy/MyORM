#pragma once

#include <relation/config.h>
#include <relation/path.h>

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
    virtual const TMessagePath& GetPath() const = 0;
    std::string GetTableName() const;
};

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
    // Constructor
    TFieldBase(
        int fieldNumber,
        const std::string& name,
        const TMessagePath& path,
        google::protobuf::FieldDescriptor::Type type,
        TFieldTypeInfoPtr fieldTypeInfo
    );

    // Constructor from FieldDescriptor
    TFieldBase(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path);

    int GetFieldNumber() const;
    virtual bool IsMessage() const = 0;
    const std::string& GetName() const;
    google::protobuf::FieldDescriptor::Type GetValueType() const;
    EFieldType GetFieldType() const;
    const TMessagePath& GetPath() const override;

  protected:
    int FieldNumber_;
    std::string Name_;
    google::protobuf::FieldDescriptor::Type ValueType_;
    TFieldTypeInfoPtr FieldTypeInfo_;
    TMessagePath Path_;
};

////////////////////////////////////////////////////////////////////////////////

class TRootBase : virtual public TMessageBase {
  public:
    TRootBase(TTableConfigPtr config);

    const TMessagePath& GetPath() const override;

    const google::protobuf::Descriptor* GetDescriptor() const;

  private:
    int Number_;
    std::string CamelCase_;
    std::string SnakeCase_;

    TMessagePath Path_;
    const google::protobuf::Descriptor* Descriptor_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TFieldBase*;
    using difference_type = std::ptrdiff_t;
    using pointer = TFieldBase**;
    using reference = TFieldBase*&;

    TFieldIterator(std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it);

    TFieldIterator& operator++();
    TFieldIterator operator++(int);
    bool operator==(const TFieldIterator& other) const;
    bool operator!=(const TFieldIterator& other) const;
    TFieldBase* operator*() const;
    TFieldBase* operator->() const;

  private:
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldsRange {
public:
    TFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields);

    TFieldIterator begin();
    TFieldIterator end();

private:
    std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

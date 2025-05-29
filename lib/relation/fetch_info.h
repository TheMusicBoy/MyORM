#pragma once

#include <relation/config.h>

#include <common/format.h>

#include <google/protobuf/descriptor.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

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

struct TMessagePathEntry {
    int protonum;
    std::string name;
};

/**
 * @class TMessagePath
 * @brief Represents a path composed of multiple TMessagePathEntry elements.
 *
 * This class provides various constructors to initialize a path, methods
 * to manipulate and retrieve entries, and operators for concatenation with
 * other paths or entries.
 */
class TMessagePath {
  public:
    // Default constructor
    TMessagePath() = default;

    // Constructor that initializes with a single TMessagePathEntry
    TMessagePath(const TMessagePathEntry& entry);

    // Constructor that initializes with a vector of TMessagePathEntry
    TMessagePath(const std::vector<TMessagePathEntry>& entries);

    // Constructor that initializes with a range of elements
    template <typename It>
    TMessagePath(It begin, It end)
        : Path_(begin, end) {}

    // Copy constructor
    TMessagePath(const TMessagePath& other);

    // Move constructor
    TMessagePath(TMessagePath&& other) noexcept;

    // Copy assignment operator
    TMessagePath& operator=(const TMessagePath& other);

    // Move assignment operator
    TMessagePath& operator=(TMessagePath&& other) noexcept;

    // Retrieve the TMessagePathEntry at the specified index
    TMessagePathEntry at(int index) const;

    // Concatenate another TMessagePath to this one
    TMessagePath& operator/=(const TMessagePath& other);

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(const TMessagePathEntry& entry);

    // Append a protobuf FieldDescriptor to this path
    TMessagePath& operator/=(const google::protobuf::FieldDescriptor* desc);

    // Get TMessagePath resulting from the concatenation with another TMessagePath
    TMessagePath operator/(const TMessagePath& other) const;

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(const TMessagePathEntry& entry) const;

    // Get TMessagePath resulting from concatenation with a protobuf
    // FieldDescriptor
    TMessagePath operator/(const google::protobuf::FieldDescriptor* desc) const;

    // Check if the path is empty
    bool empty() const;

    // Get the parent path by removing the last entry
    TMessagePath parent() const;

    // Begin iterator for path entries
    std::vector<TMessagePathEntry>::iterator begin();

    // End iterator for path entries
    std::vector<TMessagePathEntry>::iterator end();

    const std::vector<TMessagePathEntry>& data() const;

  private:
    std::vector<TMessagePathEntry> Path_;
};

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

class TFieldMessage;

class TMessageInfo : virtual public TMessageBase {
public:
    class TPrimitiveFieldIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TPrimitiveFieldInfo*;
        using difference_type = std::ptrdiff_t;
        using pointer = TPrimitiveFieldInfo**;
        using reference = TPrimitiveFieldInfo*&;

        TPrimitiveFieldIterator(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
            const std::unordered_set<int>& subMessages
        );

        TPrimitiveFieldIterator& operator++();
        TPrimitiveFieldIterator operator++(int);
        bool operator==(const TPrimitiveFieldIterator& other) const;
        bool operator!=(const TPrimitiveFieldIterator& other) const;
        TPrimitiveFieldInfo* operator*() const;
        TPrimitiveFieldInfo* operator->() const;

    private:
        void skipMessageFields();

        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end_;
        const std::unordered_set<int>& subMessages_;
    };

    class TMessageFieldIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = TFieldMessage*;
        using difference_type = std::ptrdiff_t;
        using pointer = TFieldMessage**;
        using reference = TFieldMessage*&;

        TMessageFieldIterator(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
            std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
            const std::unordered_set<int>& subMessages
        );

        TMessageFieldIterator& operator++();
        TMessageFieldIterator operator++(int);
        bool operator==(const TMessageFieldIterator& other) const;
        bool operator!=(const TMessageFieldIterator& other) const;
        TFieldMessage* operator*() const;
        TFieldMessage* operator->() const;

    private:
        void skipNonMessageFields();

        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it_;
        std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end_;
        const std::unordered_set<int>& subMessages_;
    };

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

    class TPrimitiveFieldsRange {
    public:
        TPrimitiveFieldsRange(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
            const std::unordered_set<int>& subMessages
        );

        TPrimitiveFieldIterator begin();
        TPrimitiveFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
        const std::unordered_set<int>& subMessages_;
    };

    class TMessageFieldsRange {
    public:
        TMessageFieldsRange(
            std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
            const std::unordered_set<int>& subMessages
        );

        TMessageFieldIterator begin();
        TMessageFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
        const std::unordered_set<int>& subMessages_;
    };

    class TFieldsRange {
    public:
        TFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields);

        TFieldIterator begin();
        TFieldIterator end();

    private:
        std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields_;
    };

    // Метод для получения диапазона всех полей
    TFieldsRange Fields();

    // Метод для получения диапазона только примитивных полей
    TPrimitiveFieldsRange PrimitiveFields();

    // Метод для получения диапазона только полей-сообщений
    TMessageFieldsRange MessageFields();

    TMessageInfo(const google::protobuf::Descriptor* descriptor);

protected:
    void Process();

    std::string MessageName_;
    std::unordered_map<int, std::unique_ptr<TFieldBase>> Fields_;
    std::unordered_set<int> SubMessages_;
    const google::protobuf::Descriptor* Descriptor_;
};

////////////////////////////////////////////////////////////////////////////////

class TFieldMessage : public TFieldBase, public TMessageInfo {
  public:
    TFieldMessage(const google::protobuf::FieldDescriptor* descriptor, const TMessagePath& path)
        : TFieldBase(descriptor, path),
          TMessageInfo(descriptor->message_type()) {}
};

////////////////////////////////////////////////////////////////////////////////

class TRootMessage : public TRootBase, public TMessageInfo {
  public:
    TRootMessage(TTableConfigPtr config)
        : TRootBase(config),
          TMessageInfo(GetDescriptor()) {}
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePathEntry& entry, const FormatOptions& options) {
    bool number = options.GetBool("num", false);
    bool name = options.GetBool("name", true);
    std::string delim = options.GetString("delimiter", ";");

    if (number) {
        out << entry.protonum;
    }

    if (number && name) {
        out << delim;
    }

    if (name) {
        out << entry.name;
    }
}

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePath& container, const FormatOptions& options) {
    FormatOptions defaultOpts;
    defaultOpts.Set("delimiter", "/");
    defaultOpts.Set("prefix", "");
    defaultOpts.Set("suffix", "");
    defaultOpts.Set("limit", -1);

    detail::FormatSequenceContainer(out, container.data(), options.Merge(defaultOpts));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

#include "fetch_info.h"

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

// TMessagePath implementation
TMessagePath::TMessagePath(const TMessagePathEntry& entry) : Path_({ entry }) {}

TMessagePath::TMessagePath(const std::vector<TMessagePathEntry>& entries) : Path_(entries) {}

TMessagePath::TMessagePath(const TMessagePath& other) : Path_(other.Path_) {}

TMessagePath::TMessagePath(TMessagePath&& other) noexcept : Path_(std::move(other.Path_)) {}

TMessagePath& TMessagePath::operator=(const TMessagePath& other) {
    if (this != &other) {
        Path_ = other.Path_;
    }
    return *this;
}

TMessagePath& TMessagePath::operator=(TMessagePath&& other) noexcept {
    if (this != &other) {
        Path_ = std::move(other.Path_);
    }
    return *this;
}

TMessagePathEntry TMessagePath::at(int index) const {
    if (index < 0 || index >= Path_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return Path_[index];
}

TMessagePath& TMessagePath::operator/=(const TMessagePath& other) {
    Path_.insert(Path_.end(), other.Path_.begin(), other.Path_.end());
    return *this;
}

TMessagePath& TMessagePath::operator/=(const TMessagePathEntry& entry) {
    Path_.push_back(entry);
    return *this;
}

TMessagePath& TMessagePath::operator/=(const google::protobuf::FieldDescriptor* desc) {
    Path_.push_back({desc->number(), desc->name()});
    return *this;
}

TMessagePath TMessagePath::operator/(const TMessagePath& other) const {
    TMessagePath temp = *this;
    return temp /= other; 
}

TMessagePath TMessagePath::operator/(const TMessagePathEntry& entry) const {
    TMessagePath temp = *this;
    return temp /= entry; 
}

TMessagePath TMessagePath::operator/(const google::protobuf::FieldDescriptor* desc) const {
    TMessagePath temp = *this;
    return temp /= desc; 
}

bool TMessagePath::empty() const {
    return Path_.empty();
}

TMessagePath TMessagePath::parent() const {
    if (empty()) {
        return TMessagePath{};
    }
    return TMessagePath(Path_.begin(), std::prev(Path_.end()));
}

std::vector<TMessagePathEntry>::iterator TMessagePath::begin() {
    return Path_.begin();
}

std::vector<TMessagePathEntry>::iterator TMessagePath::end() {
    return Path_.end();
}

const std::vector<TMessagePathEntry>& TMessagePath::data() const {
    return Path_;
}

////////////////////////////////////////////////////////////////////////////////

// TMessageBase implementation
TMessageBase::TMessageBase(const TMessagePath& path)
    : Path_(path)
{}

TMessageBase::TMessageBase(const TMessagePath& path, TMessagePathEntry entry)
    : Path_(path / entry)
{}

const TMessagePath& TMessageBase::GetPath() const {
    return Path_;
}

std::string TMessageBase::GetTableName() const {
    return Format("t_{delimiter='_',element={num=true,name=false}}", Path_);
}

////////////////////////////////////////////////////////////////////////////////

// TFieldBase implementation
TFieldBase::TFieldBase(int fieldNumber, const std::string& name, const TMessagePath& path, 
                      google::protobuf::FieldDescriptor::Type type, TFieldTypeInfoPtr fieldTypeInfo)
  : TMessageBase(path, {fieldNumber, name}),
    FieldNumber_(fieldNumber),
    Name_(name), 
    ValueType_(type), 
    FieldTypeInfo_(fieldTypeInfo)
{}

TFieldBase::TFieldBase(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path)
  : TFieldBase(fieldDescriptor->number(), fieldDescriptor->name(), path, fieldDescriptor->type(), 
              ::NOrm::NRelation::GetFieldTypeInfo(fieldDescriptor))
{}

int TFieldBase::GetFieldNumber() const {
    return FieldNumber_;
}

const std::string& TFieldBase::GetName() const {
    return Name_;
}

google::protobuf::FieldDescriptor::Type TFieldBase::GetValueType() const {
    return ValueType_;
}

EFieldType TFieldBase::GetFieldType() const {
    return FieldTypeInfo_->GetFieldType();
}

////////////////////////////////////////////////////////////////////////////////

// TRootBase implementation
TRootBase::TRootBase(TTableConfigPtr config)
  : TMessageBase({}, {config->Number, config->SnakeCase}),
    Number_(config->Number),
    SnakeCase_(config->SnakeCase),
    CamelCase_(config->CamelCase),
    Descriptor_(google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(config->Scheme)),
    CustomTypeHandler_(config->CustomTypeHandler)
{}

////////////////////////////////////////////////////////////////////////////////

// TPrimitiveFieldInfo implementation
const std::string& TPrimitiveFieldInfo::GetDefaultValueString() const {
    return DefaultValueString_;
}

const std::variant<
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
    TEnumFieldInfo
>& TPrimitiveFieldInfo::GetTypeInfo() const {
    return TypeInfo_;
}

TPrimitiveFieldInfo::TPrimitiveFieldInfo(
    const google::protobuf::FieldDescriptor* fieldDescriptor,
    const TMessagePath& path
) : TFieldBase(fieldDescriptor, path)
{
    // Set DefaultValueString_ based on the fieldDescriptor if available
    switch (fieldDescriptor->type()) {
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
            HandleBoolField(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            HandleInt32Field(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            HandleUInt32Field(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            HandleInt64Field(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            HandleUInt64Field(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
            HandleFloatField(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
            HandleDoubleField(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_STRING:
            HandleStringField(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
            HandleBytesField(fieldDescriptor);
            break;
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
            HandleEnumField(fieldDescriptor);
            break;
        default:
            // Handle unknown field types
            break;
    }
}

void TPrimitiveFieldInfo::HandleBoolField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->default_value_bool() ? "true" : "false";
    TypeInfo_ = TBoolFieldInfo{field->default_value_bool()};
}

void TPrimitiveFieldInfo::HandleInt32Field(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = std::to_string(field->default_value_int32());
    TypeInfo_ = TInt32FieldInfo{field->default_value_int32()};
}

void TPrimitiveFieldInfo::HandleUInt32Field(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? std::to_string(field->default_value_uint32()) : "0";
    TypeInfo_ = TUInt32FieldInfo{field->default_value_uint32()};
}

void TPrimitiveFieldInfo::HandleInt64Field(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? std::to_string(field->default_value_int64()) : "0";
    TypeInfo_ = TInt64FieldInfo{field->default_value_int64()};
}

void TPrimitiveFieldInfo::HandleUInt64Field(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? std::to_string(field->default_value_uint64()) : "0";
    TypeInfo_ = TUInt64FieldInfo{field->default_value_uint64()};
}

void TPrimitiveFieldInfo::HandleFloatField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? std::to_string(field->default_value_float()) : "0.0";
    TypeInfo_ = TFloatFieldInfo{field->default_value_float()};
}

void TPrimitiveFieldInfo::HandleDoubleField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? std::to_string(field->default_value_double()) : "0.0";
    TypeInfo_ = TDoubleFieldInfo{field->default_value_double()};
}

void TPrimitiveFieldInfo::HandleStringField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? "\\\"" + field->default_value_string() + "\\\"" : "\\\"\\\"";
    TypeInfo_ = TStringFieldInfo{field->default_value_string()};
}

void TPrimitiveFieldInfo::HandleBytesField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? "<bytes>" : "<empty>";
    TypeInfo_ = TBytesFieldInfo{};
}

void TPrimitiveFieldInfo::HandleEnumField(const google::protobuf::FieldDescriptor* field) {
    DefaultValueString_ = field->has_default_value() ? field->default_value_enum()->name() : "unknown";
    TypeInfo_ = TEnumFieldInfo{field->default_value_enum()->index()};
}

////////////////////////////////////////////////////////////////////////////////

// TMessageInfo implementation
TMessageInfo::TMessageInfo(const google::protobuf::Descriptor* descriptor, const TMessagePath& path, const TMessagePathEntry& entry)
  : TMessageBase(path, entry)
{ }

////////////////////////////////////////////////////////////////////////////////

// GetFieldTypeInfo implementation
TFieldTypeInfoPtr GetFieldTypeInfo(const google::protobuf::FieldDescriptor* desc) {
    // Handle map fields first (maps are a special kind of repeated field)
    if (desc->is_map()) {
        auto info = std::make_shared<TMapFieldInfo>();
        // For map fields, get the key type
        info->KeyType = desc->message_type()->field(0)->type();
        return info;
    }
    
    // Handle regular repeated fields
    if (desc->is_repeated()) {
        return std::make_shared<TRepeatedFieldInfo>();
    }
    
    // Handle real oneof fields (not synthetic ones used for optional fields)
    if (desc->containing_oneof() && !desc->containing_oneof()->is_synthetic()) {
        auto info = std::make_shared<TOneofFieldInfo>();
        info->OneofIndex = desc->containing_oneof()->index();
        return info;
    }
    
    // In proto3, fields with explicit "optional" keyword have presence
    if (desc->has_presence()) {
        return std::make_shared<TOptionalFieldInfo>();
    }
    
    // All other fields are singular
    return std::make_shared<TSingularFieldInfo>();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

#include <relation/field.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

const std::string& TPrimitiveFieldInfo::GetDefaultValueString() const {
    return DefaultValueString_;
}

const TValueInfo& TPrimitiveFieldInfo::GetTypeInfo() const {
    return TypeInfo_;
}

TPrimitiveFieldInfo::TPrimitiveFieldInfo(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path)
    : TFieldBase(fieldDescriptor, path) {
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

// Реализация TPrimitiveFieldIterator
void TPrimitiveFieldIterator::skipMessageFields() {
    while (it_ != end_ && it_->second->IsMessage()) {
        ++it_;
    }
}

TPrimitiveFieldIterator::TPrimitiveFieldIterator(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end)
    : it_(it), end_(end)  {
    skipMessageFields();
}

TPrimitiveFieldIterator& TPrimitiveFieldIterator::operator++() {
    ++it_;
    skipMessageFields();
    return *this;
}

TPrimitiveFieldIterator TPrimitiveFieldIterator::operator++(int) {
    TPrimitiveFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TPrimitiveFieldIterator::operator==(const TPrimitiveFieldIterator& other) const {
    return it_ == other.it_;
}

bool TPrimitiveFieldIterator::operator!=(const TPrimitiveFieldIterator& other) const {
    return !(*this == other);
}

TPrimitiveFieldInfo* TPrimitiveFieldIterator::operator*() const {
    return static_cast<TPrimitiveFieldInfo*>(it_->second.get());
}

TPrimitiveFieldInfo* TPrimitiveFieldIterator::operator->() const {
    return static_cast<TPrimitiveFieldInfo*>(it_->second.get());
}

// Реализация TPrimitiveFieldsRange
TPrimitiveFieldsRange::TPrimitiveFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields)
    : fields_(fields) {}

TPrimitiveFieldIterator TPrimitiveFieldsRange::begin() {
    return TPrimitiveFieldIterator(fields_.begin(), fields_.end());
}

TPrimitiveFieldIterator TPrimitiveFieldsRange::end() {
    return TPrimitiveFieldIterator(fields_.end(), fields_.end());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

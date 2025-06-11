#include <relation/field.h>

#include <lib/relation/proto/orm_core.pb.h>

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
    if (!fieldDescriptor) {
        DefaultValueString_ = "";
        TypeInfo_ = std::monostate{};
        HasDefault_ = false;
        Unique_ = false;
        IsRequired_ = false;
        IsPrimaryKey_ = false;
        return;
    }

    // Уникальное поле
    if (fieldDescriptor->options().HasExtension(orm::unique)) {
        Unique_ = fieldDescriptor->options().GetExtension(orm::unique);
    } else {
        Unique_ = false;
    }
    
    // Обязательное поле
    if (fieldDescriptor->options().HasExtension(orm::required)) {
        IsRequired_ = fieldDescriptor->options().GetExtension(orm::required);
    } else {
        IsRequired_ = false;
    }
    
    // Первичный ключ
    if (fieldDescriptor->options().HasExtension(orm::primary_key)) {
        IsPrimaryKey_ = fieldDescriptor->options().GetExtension(orm::primary_key);
    } else {
        IsPrimaryKey_ = false;
    }
    
    // Auto increment
    if (fieldDescriptor->options().HasExtension(orm::auto_increment)) {
        AutoIncrement_ = fieldDescriptor->options().GetExtension(orm::auto_increment);
    } else {
        AutoIncrement_ = false;
    }
    
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
    HasDefault_ = field->options().HasExtension(orm::default_bool);
    if (HasDefault_) {
        DefaultValueString_ = field->options().GetExtension(orm::default_bool) ? "true" : "false";
        TypeInfo_ = TBoolFieldInfo{field->options().GetExtension(orm::default_bool)};
    } else {
        DefaultValueString_ = "false";
        TypeInfo_ = TBoolFieldInfo{false};
    }
}

void TPrimitiveFieldInfo::HandleInt32Field(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_int32);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_int32));
        TypeInfo_ = TInt32FieldInfo{field->options().GetExtension(orm::default_int32)};
    } else {
        DefaultValueString_ = "0";
        TypeInfo_ = TInt32FieldInfo{0};
    }
}

void TPrimitiveFieldInfo::HandleUInt32Field(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_uint32);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_uint32));
        TypeInfo_ = TUInt32FieldInfo{field->options().GetExtension(orm::default_uint32)};
    } else {
        DefaultValueString_ = "0";
        TypeInfo_ = TUInt32FieldInfo{0};
    }
}

void TPrimitiveFieldInfo::HandleInt64Field(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_int64);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_int64));
        TypeInfo_ = TInt64FieldInfo{field->options().GetExtension(orm::default_int64)};
    } else {
        DefaultValueString_ = "0";
        TypeInfo_ = TInt64FieldInfo{0};
    }
}

void TPrimitiveFieldInfo::HandleUInt64Field(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_uint64);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_uint64));
        TypeInfo_ = TUInt64FieldInfo{field->options().GetExtension(orm::default_uint64)};
    } else {
        DefaultValueString_ = "0";
        TypeInfo_ = TUInt64FieldInfo{0};
    }
}

void TPrimitiveFieldInfo::HandleFloatField(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_float);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_float));
        TypeInfo_ = TFloatFieldInfo{field->options().GetExtension(orm::default_float)};
    } else {
        DefaultValueString_ = "0.0";
        TypeInfo_ = TFloatFieldInfo{0.0f};
    }
}

void TPrimitiveFieldInfo::HandleDoubleField(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_double);
    if (HasDefault_) {
        DefaultValueString_ = std::to_string(field->options().GetExtension(orm::default_double));
        TypeInfo_ = TDoubleFieldInfo{field->options().GetExtension(orm::default_double)};
    } else {
        DefaultValueString_ = "0.0";
        TypeInfo_ = TDoubleFieldInfo{0.0};
    }
}

void TPrimitiveFieldInfo::HandleStringField(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_string);
    if (HasDefault_) {
        DefaultValueString_ = "\\\"" + field->options().GetExtension(orm::default_string) + "\\\"";
        TypeInfo_ = TStringFieldInfo{field->options().GetExtension(orm::default_string)};
    } else {
        DefaultValueString_ = "\\\"\\\"";
        TypeInfo_ = TStringFieldInfo{""};
    }
}

void TPrimitiveFieldInfo::HandleBytesField(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_bytes);
    if (HasDefault_) {
        DefaultValueString_ = "<bytes>";
        TypeInfo_ = TBytesFieldInfo{};
    } else {
        DefaultValueString_ = "<empty>";
        TypeInfo_ = TBytesFieldInfo{};
    }
}

void TPrimitiveFieldInfo::HandleEnumField(const google::protobuf::FieldDescriptor* field) {
    HasDefault_ = field->options().HasExtension(orm::default_enum);
    if (HasDefault_) {
        const google::protobuf::EnumValueDescriptor* enumValue = 
            field->enum_type()->FindValueByName(field->options().GetExtension(orm::default_enum));
        DefaultValueString_ = enumValue ? enumValue->name() : "unknown";
        TypeInfo_ = TEnumFieldInfo{enumValue ? enumValue->index() : 0};
    } else {
        DefaultValueString_ = "unknown";
        TypeInfo_ = TEnumFieldInfo{0};
    }
}

////////////////////////////////////////////////////////////////////////////////

// Implementation of TPrimitiveFieldIterator
void TPrimitiveFieldIterator::skipMessageFields() {
    while (it_ != end_ && it_->second->IsMessage()) {
        ++it_;
    }
}

TPrimitiveFieldIterator::TPrimitiveFieldIterator(
    std::map<int, TFieldBasePtr>::iterator it,
    std::map<int, TFieldBasePtr>::iterator end)
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

TPrimitiveFieldInfoPtr TPrimitiveFieldIterator::operator*() const {
    return std::static_pointer_cast<TPrimitiveFieldInfo>(it_->second);
}

TPrimitiveFieldInfoPtr TPrimitiveFieldIterator::operator->() const {
    return std::static_pointer_cast<TPrimitiveFieldInfo>(it_->second);
}

// Implementation of TPrimitiveFieldsRange
TPrimitiveFieldsRange::TPrimitiveFieldsRange(std::map<int, TFieldBasePtr>& fields)
    : fields_(fields) {}

TPrimitiveFieldIterator TPrimitiveFieldsRange::begin() {
    return TPrimitiveFieldIterator(fields_.begin(), fields_.end());
}

TPrimitiveFieldIterator TPrimitiveFieldsRange::end() {
    return TPrimitiveFieldIterator(fields_.end(), fields_.end());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation


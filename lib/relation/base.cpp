#include <relation/base.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

std::string TMessageBase::GetTableName() const {
    return Format("t_{delimiter='_',element={num=true,name=false}}", this->GetPath());
}

////////////////////////////////////////////////////////////////////////////////

// TFieldBase implementation
TFieldBase::TFieldBase(
    int fieldNumber,
    const std::string& name,
    const TMessagePath& path,
    google::protobuf::FieldDescriptor::Type type,
    TFieldTypeInfoPtr fieldTypeInfo
)
    : FieldNumber_(fieldNumber),
      Name_(name),
      ValueType_(type),
      FieldTypeInfo_(fieldTypeInfo),
      Path_(path / TMessagePathEntry{fieldNumber, name}) {}

TFieldBase::TFieldBase(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path)
    : TFieldBase(fieldDescriptor->number(), fieldDescriptor->name(), path, fieldDescriptor->type(), ::NOrm::NRelation::GetFieldTypeInfo(fieldDescriptor)) {}

const TMessagePath& TFieldBase::GetPath() const {
    return Path_;
}

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
    : Number_(config->Number),
      SnakeCase_(config->SnakeCase),
      CamelCase_(config->CamelCase),
      Descriptor_(google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(config->Scheme)),
      Path_({Number_, SnakeCase_}) {}

const TMessagePath& TRootBase::GetPath() const {
    return Path_;
}

const google::protobuf::Descriptor* TRootBase::GetDescriptor() const {
    return Descriptor_;
}

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

// Реализация TFieldIterator
TFieldIterator::TFieldIterator(std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it)
    : it_(it) {}

TFieldIterator& TFieldIterator::operator++() {
    ++it_;
    return *this;
}

TFieldIterator TFieldIterator::operator++(int) {
    TFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TFieldIterator::operator==(const TFieldIterator& other) const {
    return it_ == other.it_;
}

bool TFieldIterator::operator!=(const TFieldIterator& other) const {
    return !(*this == other);
}

TFieldBase* TFieldIterator::operator*() const {
    return it_->second.get();
}

TFieldBase* TFieldIterator::operator->() const {
    return it_->second.get();
}

////////////////////////////////////////////////////////////////////////////////

TFieldsRange::TFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields)
    : fields_(fields) {}

TFieldIterator TFieldsRange::begin() {
    return TFieldIterator(fields_.begin());
}

TFieldIterator TFieldsRange::end() {
    return TFieldIterator(fields_.end());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

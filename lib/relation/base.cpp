#include <relation/base.h>
#include <relation/relation_manager.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

std::string TMessageBase::GetTableName() const {
    return Format("{table_id}", this->GetPath());
}

////////////////////////////////////////////////////////////////////////////////

TFieldBase::TFieldBase(const google::protobuf::FieldDescriptor* fieldDescriptor, const TMessagePath& path) {
    ASSERT(fieldDescriptor, "Tried to create reflection with no data at {}", path.Number())
    FieldNumber_ = fieldDescriptor->number();
    Name_ = fieldDescriptor->name();
    ValueType_ = fieldDescriptor->type();
    FieldTypeInfo_ = ::NOrm::NRelation::GetFieldTypeInfo(fieldDescriptor);
    Path_ = path / fieldDescriptor;
    FieldDescriptor_ = fieldDescriptor;
}

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

const google::protobuf::FieldDescriptor* TFieldBase::GetFieldDescriptor() const {
    return FieldDescriptor_;
}

EFieldType TFieldBase::GetFieldType() const {
    return FieldTypeInfo_->GetFieldType();
}

////////////////////////////////////////////////////////////////////////////////

TRootBase::TRootBase(TTableConfigPtr config)
    : Number_(config->Number),
      SnakeCase_(config->SnakeCase),
      CamelCase_(config->CamelCase),
      Descriptor_(google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(config->Scheme)),
      Path_(Number_) {}

const TMessagePath& TRootBase::GetPath() const {
    return Path_;
}

int TRootBase::Number() const {
    return Number_;
}

const std::string& TRootBase::GetSnakeCase() const {
    return SnakeCase_;
}

const std::string& TRootBase::GetCamelCase() const {
    return CamelCase_;
}

const google::protobuf::Descriptor* TRootBase::GetDescriptor() const {
    return Descriptor_;
}

////////////////////////////////////////////////////////////////////////////////

TFieldTypeInfoPtr GetFieldTypeInfo(const google::protobuf::FieldDescriptor* desc) {
    if (!desc) {
        return std::make_shared<TSingularFieldInfo>();
    }

    if (desc->is_map()) {
        auto info = std::make_shared<TMapFieldInfo>();
        info->KeyType = desc->message_type()->field(0)->type();
        return info;
    }

    if (desc->is_repeated()) {
        return std::make_shared<TRepeatedFieldInfo>();
    }

    if (desc->containing_oneof() && !desc->containing_oneof()->is_synthetic()) {
        auto info = std::make_shared<TOneofFieldInfo>();
        info->OneofIndex = desc->containing_oneof()->index();
        return info;
    }

    if (desc->has_presence()) {
        return std::make_shared<TOptionalFieldInfo>();
    }

    return std::make_shared<TSingularFieldInfo>();
}

////////////////////////////////////////////////////////////////////////////////

TFieldIterator::TFieldIterator(std::map<int, TFieldBasePtr>::iterator it)
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

TFieldBasePtr TFieldIterator::operator*() const {
    return it_->second;
}

TFieldBasePtr TFieldIterator::operator->() const {
    return it_->second;
}

////////////////////////////////////////////////////////////////////////////////

TFieldsRange::TFieldsRange(std::map<int, TFieldBasePtr>& fields)
    : fields_(fields) {}

TFieldIterator TFieldsRange::begin() {
    return TFieldIterator(fields_.begin());
}

TFieldIterator TFieldsRange::end() {
    return TFieldIterator(fields_.end());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

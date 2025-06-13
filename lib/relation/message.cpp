#include <relation/field.h>
#include <relation/message.h>
#include <relation/relation_manager.h>

namespace NOrm::NRelation {

namespace {

////////////////////////////////////////////////////////////////////////////////

TFieldBasePtr RegisterPrimitiveField(const google::protobuf::FieldDescriptor* desc, TMessagePath path) {
    if (!desc) {
        return nullptr;
    }

    auto field = std::make_shared<TPrimitiveFieldInfo>(desc, path);
    TRelationManager::GetInstance().RegisterField(field);
    return field;
}

TFieldBasePtr RegisterMessageField(const google::protobuf::FieldDescriptor* desc, TMessagePath path) {
    if (!desc) {
        return nullptr;
    }

    auto field = std::make_shared<TFieldMessage>(desc, path);

    auto& relationManager = TRelationManager::GetInstance();
    auto parentMessage = relationManager.GetMessage(path);

    relationManager.RegisterField(field);

    if (parentMessage) {
        relationManager.SetParentMessage(field, parentMessage);
    }

    return field;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////

void RegisterRootMessage(TTableConfigPtr config) {
    if (!config) {
        return;
    }

    auto rootMessage = std::make_shared<TRootMessage>(config);

    TRelationManager::GetInstance().RegisterRoot(rootMessage);
}

////////////////////////////////////////////////////////////////////////////////

// TMessageInfo implementation
TMessageInfo::TMessageInfo(const google::protobuf::Descriptor* descriptor)
    : Descriptor_(descriptor) {}

void TMessageInfo::Process() {
    if (!Descriptor_) {
        return;
    }

    MessageName_ = Descriptor_->name();

    for (int i = 0; i < Descriptor_->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = Descriptor_->field(i);

        if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
            SubMessages_.insert(field->number());
            Fields_[field->number()] = RegisterMessageField(field, this->GetPath());
        } else {
            Fields_[field->number()] = RegisterPrimitiveField(field, this->GetPath());
        }
    }
}

const google::protobuf::Descriptor* TMessageInfo::GetMessageDescriptor() const {
    return Descriptor_;
}

// Implementation of TMessageFieldIterator
void TMessageFieldIterator::skipNonMessageFields() {
    while (it_ != end_ && !it_->second->IsMessage()) {
        ++it_;
    }
}

TMessageFieldIterator::TMessageFieldIterator(std::map<int, TFieldBasePtr>::iterator it, std::map<int, TFieldBasePtr>::iterator end)
    : it_(it),
      end_(end) {
    skipNonMessageFields();
}

TMessageFieldIterator& TMessageFieldIterator::operator++() {
    ++it_;
    skipNonMessageFields();
    return *this;
}

TMessageFieldIterator TMessageFieldIterator::operator++(int) {
    TMessageFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TMessageFieldIterator::operator==(const TMessageFieldIterator& other) const {
    return it_ == other.it_;
}

bool TMessageFieldIterator::operator!=(const TMessageFieldIterator& other) const {
    return !(*this == other);
}

TFieldMessagePtr TMessageFieldIterator::operator*() const {
    return std::static_pointer_cast<TFieldMessage>(it_->second);
}

TFieldMessagePtr TMessageFieldIterator::operator->() const {
    return std::static_pointer_cast<TFieldMessage>(it_->second);
}

// Implementation of TMessageFieldsRange
TMessageFieldsRange::TMessageFieldsRange(std::map<int, TFieldBasePtr>& fields)
    : fields_(fields) {}

TMessageFieldIterator TMessageFieldsRange::begin() {
    return TMessageFieldIterator(fields_.begin(), fields_.end());
}

TMessageFieldIterator TMessageFieldsRange::end() {
    return TMessageFieldIterator(fields_.end(), fields_.end());
}

// Implementation of range access methods
TFieldsRange TMessageInfo::Fields() {
    return TFieldsRange(Fields_);
}

TPrimitiveFieldsRange TMessageInfo::PrimitiveFields() {
    return TPrimitiveFieldsRange(Fields_);
}

TMessageFieldsRange TMessageInfo::MessageFields() {
    return TMessageFieldsRange(Fields_);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation


#include <relation/message.h>
#include <relation/field.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

// TMessageInfo implementation
TMessageInfo::TMessageInfo(const google::protobuf::Descriptor* descriptor)
    : Descriptor_(descriptor) {
    Process();
}

void TMessageInfo::Process() {
    if (!Descriptor_) {
        return;
    }

    MessageName_ = Descriptor_->name();

    for (int i = 0; i < Descriptor_->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = Descriptor_->field(i);

        if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
            SubMessages_.insert(field->number());
            Fields_[field->number()] = std::make_unique<TFieldMessage>(field, this->GetPath());
        } else {
            Fields_[field->number()] = std::make_unique<TPrimitiveFieldInfo>(field, this->GetPath());
        }
    }
}
// Реализация TMessageFieldIterator
void TMessageFieldIterator::skipNonMessageFields() {
    while (it_ != end_ && !it_->second->IsMessage()) {
        ++it_;
    }
}

TMessageFieldIterator::TMessageFieldIterator(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end
)
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

TFieldMessage* TMessageFieldIterator::operator*() const {
    return static_cast<TFieldMessage*>(it_->second.get());
}

TFieldMessage* TMessageFieldIterator::operator->() const {
    return static_cast<TFieldMessage*>(it_->second.get());
}

// Реализация TMessageFieldsRange
TMessageFieldsRange::TMessageFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields)
    : fields_(fields) {}

TMessageFieldIterator TMessageFieldsRange::begin() {
    return TMessageFieldIterator(fields_.begin(), fields_.end());
}

TMessageFieldIterator TMessageFieldsRange::end() {
    return TMessageFieldIterator(fields_.end(), fields_.end());
}

// Реализация методов доступа к диапазонам
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

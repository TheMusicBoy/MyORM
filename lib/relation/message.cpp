#include <relation/message.h>

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
// Реализация TPrimitiveFieldIterator
void TMessageInfo::TPrimitiveFieldIterator::skipMessageFields() {
    while (it_ != end_ && subMessages_.count(it_->first) > 0) {
        ++it_;
    }
}

TMessageInfo::TPrimitiveFieldIterator::TPrimitiveFieldIterator(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
    const std::unordered_set<int>& subMessages)
    : it_(it), end_(end), subMessages_(subMessages) {
    skipMessageFields();
}

TMessageInfo::TPrimitiveFieldIterator& TMessageInfo::TPrimitiveFieldIterator::operator++() {
    ++it_;
    skipMessageFields();
    return *this;
}

TMessageInfo::TPrimitiveFieldIterator TMessageInfo::TPrimitiveFieldIterator::operator++(int) {
    TPrimitiveFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TMessageInfo::TPrimitiveFieldIterator::operator==(const TPrimitiveFieldIterator& other) const {
    return it_ == other.it_;
}

bool TMessageInfo::TPrimitiveFieldIterator::operator!=(const TPrimitiveFieldIterator& other) const {
    return !(*this == other);
}

TPrimitiveFieldInfo* TMessageInfo::TPrimitiveFieldIterator::operator*() const {
    return static_cast<TPrimitiveFieldInfo*>(it_->second.get());
}

TPrimitiveFieldInfo* TMessageInfo::TPrimitiveFieldIterator::operator->() const {
    return static_cast<TPrimitiveFieldInfo*>(it_->second.get());
}

// Реализация TMessageFieldIterator
void TMessageInfo::TMessageFieldIterator::skipNonMessageFields() {
    while (it_ != end_ && subMessages_.count(it_->first) == 0) {
        ++it_;
    }
}

TMessageInfo::TMessageFieldIterator::TMessageFieldIterator(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it,
    std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator end,
    const std::unordered_set<int>& subMessages)
    : it_(it), end_(end), subMessages_(subMessages) {
    skipNonMessageFields();
}

TMessageInfo::TMessageFieldIterator& TMessageInfo::TMessageFieldIterator::operator++() {
    ++it_;
    skipNonMessageFields();
    return *this;
}

TMessageInfo::TMessageFieldIterator TMessageInfo::TMessageFieldIterator::operator++(int) {
    TMessageFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TMessageInfo::TMessageFieldIterator::operator==(const TMessageFieldIterator& other) const {
    return it_ == other.it_;
}

bool TMessageInfo::TMessageFieldIterator::operator!=(const TMessageFieldIterator& other) const {
    return !(*this == other);
}

TFieldMessage* TMessageInfo::TMessageFieldIterator::operator*() const {
    return static_cast<TFieldMessage*>(it_->second.get());
}

TFieldMessage* TMessageInfo::TMessageFieldIterator::operator->() const {
    return static_cast<TFieldMessage*>(it_->second.get());
}

// Реализация TFieldIterator
TMessageInfo::TFieldIterator::TFieldIterator(std::unordered_map<int, std::unique_ptr<TFieldBase>>::iterator it)
    : it_(it) {}

TMessageInfo::TFieldIterator& TMessageInfo::TFieldIterator::operator++() {
    ++it_;
    return *this;
}

TMessageInfo::TFieldIterator TMessageInfo::TFieldIterator::operator++(int) {
    TFieldIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool TMessageInfo::TFieldIterator::operator==(const TFieldIterator& other) const {
    return it_ == other.it_;
}

bool TMessageInfo::TFieldIterator::operator!=(const TFieldIterator& other) const {
    return !(*this == other);
}

TFieldBase* TMessageInfo::TFieldIterator::operator*() const {
    return it_->second.get();
}

TFieldBase* TMessageInfo::TFieldIterator::operator->() const {
    return it_->second.get();
}

// Реализация TPrimitiveFieldsRange
TMessageInfo::TPrimitiveFieldsRange::TPrimitiveFieldsRange(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
    const std::unordered_set<int>& subMessages)
    : fields_(fields), subMessages_(subMessages) {}

TMessageInfo::TPrimitiveFieldIterator TMessageInfo::TPrimitiveFieldsRange::begin() {
    return TPrimitiveFieldIterator(fields_.begin(), fields_.end(), subMessages_);
}

TMessageInfo::TPrimitiveFieldIterator TMessageInfo::TPrimitiveFieldsRange::end() {
    return TPrimitiveFieldIterator(fields_.end(), fields_.end(), subMessages_);
}

// Реализация TMessageFieldsRange
TMessageInfo::TMessageFieldsRange::TMessageFieldsRange(
    std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields,
    const std::unordered_set<int>& subMessages)
    : fields_(fields), subMessages_(subMessages) {}

TMessageInfo::TMessageFieldIterator TMessageInfo::TMessageFieldsRange::begin() {
    return TMessageFieldIterator(fields_.begin(), fields_.end(), subMessages_);
}

TMessageInfo::TMessageFieldIterator TMessageInfo::TMessageFieldsRange::end() {
    return TMessageFieldIterator(fields_.end(), fields_.end(), subMessages_);
}

// Реализация TFieldsRange
TMessageInfo::TFieldsRange::TFieldsRange(std::unordered_map<int, std::unique_ptr<TFieldBase>>& fields)
    : fields_(fields) {}

TMessageInfo::TFieldIterator TMessageInfo::TFieldsRange::begin() {
    return TFieldIterator(fields_.begin());
}

TMessageInfo::TFieldIterator TMessageInfo::TFieldsRange::end() {
    return TFieldIterator(fields_.end());
}

// Реализация методов доступа к диапазонам
TMessageInfo::TFieldsRange TMessageInfo::Fields() {
    return TFieldsRange(Fields_);
}

TMessageInfo::TPrimitiveFieldsRange TMessageInfo::PrimitiveFields() {
    return TPrimitiveFieldsRange(Fields_, SubMessages_);
}

TMessageInfo::TMessageFieldsRange TMessageInfo::MessageFields() {
    return TMessageFieldsRange(Fields_, SubMessages_);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

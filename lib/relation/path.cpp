#include <relation/path.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

TMessagePath::TMessagePath(const TMessagePathEntry& entry)
    : Path_({entry}) {}

TMessagePath::TMessagePath(const std::vector<TMessagePathEntry>& entries)
    : Path_(entries) {}

TMessagePath::TMessagePath(const TMessagePath& other)
    : Path_(other.Path_) {}

TMessagePath::TMessagePath(TMessagePath&& other) noexcept
    : Path_(std::move(other.Path_)) {}

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

} // namespace NRelation

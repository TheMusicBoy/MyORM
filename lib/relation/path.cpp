#include <relation/path.h>

#include <common/exception.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

void TPathManager::RegisterEntry(const TMessagePath& path, const std::string& name) {
    size_t hash = std::hash<TMessagePath>{}(path.parent());
    EntryNameToEntry_[hash][name] = path.number();
    PathToEntryName_[GetHash(hash, path.number())] = name;
}

std::string TPathManager::EntryName(const TMessagePath& path) const {
    return PathToEntryName_.at(std::hash<TMessagePath>{}(path));
}

std::vector<std::string> TPathManager::ToString(const TMessagePath& path) const {
    std::vector<std::string> result;

    size_t hash = 0;
    for (size_t el : path) {
        hash = GetHash(hash, el);
        if (!PathToEntryName_.contains(hash)) { return result; }
        result.emplace_back(PathToEntryName_.at(hash));
    }
    return result;
}

uint32_t TPathManager::GetEntry(const TMessagePath& path, const std::string& entry) {
    return EntryNameToEntry_[std::hash<TMessagePath>{}(path)][entry];
}

bool TPathManager::PathRegistered(const TMessagePath& path) const {
    return PathToEntryName_.contains(std::hash<TMessagePath>{}(path));
}

////////////////////////////////////////////////////////////////////////////////

TMessagePath::TMessagePath(uint32_t entry)
    : Path_({entry}) {}

TMessagePath::TMessagePath(const std::string& entry)
    : Path_({TPathManager::GetInstance().GetEntry({}, entry)}) {}

TMessagePath::TMessagePath(const std::vector<uint32_t>& entries)
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

uint32_t TMessagePath::at(int index) const {
    if (index < 0 || index >= Path_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return Path_[index];
}

TMessagePath& TMessagePath::operator/=(const TMessagePath& other) {
    Path_.insert(Path_.end(), other.Path_.begin(), other.Path_.end());
    return *this;
}

TMessagePath& TMessagePath::operator/=(uint32_t entry) {
    Path_.push_back(entry);
    return *this;
}

TMessagePath& TMessagePath::operator/=(const std::string& entry) {
    Path_.push_back(NOrm::NRelation::TPathManager::GetInstance().GetEntry(Path_, entry));
    return *this;
}

TMessagePath& TMessagePath::operator/=(const google::protobuf::FieldDescriptor* desc) {
    Path_.push_back(desc->number());
    NOrm::NRelation::TPathManager::GetInstance().RegisterEntry(Path_, desc->name());
    return *this;
}

TMessagePath TMessagePath::operator/(const TMessagePath& other) const {
    TMessagePath temp = *this;
    return temp /= other;
}

TMessagePath TMessagePath::operator/(uint32_t entry) const {
    TMessagePath temp = *this;
    return temp /= entry;
}

TMessagePath TMessagePath::operator/(const std::string& entry) const {
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

std::vector<uint32_t>::iterator TMessagePath::begin() {
    return Path_.begin();
}

std::vector<uint32_t>::iterator TMessagePath::end() {
    return Path_.end();
}

std::vector<uint32_t>::const_iterator TMessagePath::begin() const {
    return Path_.begin();
}

std::vector<uint32_t>::const_iterator TMessagePath::end() const {
    return Path_.end();
}

const uint32_t TMessagePath::front() const {
    ASSERT(!Path_.empty(), "Attempt to access element of empty TMessagePath");
    return *Path_.begin();
}

const uint32_t TMessagePath::back() const {
    ASSERT(!Path_.empty(), "Attempt to access element of empty TMessagePath");
    return *std::prev(Path_.end());
}

uint32_t& TMessagePath::front() {
    ASSERT(!Path_.empty(), "Attempt to access element of empty TMessagePath");
    return *Path_.begin();
}

uint32_t& TMessagePath::back() {
    ASSERT(!Path_.empty(), "Attempt to access element of empty TMessagePath");
    return *std::prev(Path_.end());
}

uint32_t TMessagePath::number() const {
    return back();
}

std::string TMessagePath::name() const {
    const auto& manager = TPathManager::GetInstance();
    ASSERT(!manager.PathRegistered(*this), "Attept to access unknown name in TMessagePath");
    return manager.EntryName(*this);
}

const std::vector<uint32_t>& TMessagePath::data() const {
    return Path_;
}

bool TMessagePath::operator==(const TMessagePath& other) const {
    if (Path_.size() != other.Path_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < Path_.size(); ++i) {
        if (Path_[i] != other.Path_[i]) {
            return false;
        }
    }
    
    return true;
}

bool TMessagePath::operator!=(const TMessagePath& other) const {
    return !(*this == other);
}

bool TMessagePath::operator<(const TMessagePath& other) const {
    const size_t minSize = std::min(Path_.size(), other.Path_.size());
    
    for (size_t i = 0; i < minSize; ++i) {
        if (Path_[i] < other.Path_[i]) {
            return true;
        }
        if (Path_[i] > other.Path_[i]) {
            return false;
        }
    }
    
    // If all common elements are equal, shorter path is less
    return Path_.size() < other.Path_.size();
}

bool TMessagePath::operator<=(const TMessagePath& other) const {
    return *this < other || *this == other;
}

bool TMessagePath::operator>(const TMessagePath& other) const {
    return !(*this <= other);
}

bool TMessagePath::operator>=(const TMessagePath& other) const {
    return !(*this < other);
}

bool TMessagePath::isParentOf(const TMessagePath& other) const {
    // A parent path must be exactly one element shorter than the child path
    if (Path_.size() + 1 != other.Path_.size()) {
        return false;
    }
    
    // All elements in this path must match corresponding elements in the other path
    for (size_t i = 0; i < Path_.size(); ++i) {
        if (Path_[i] != other.Path_[i]) {
            return false;
        }
    }
    
    return true;
}

bool TMessagePath::isAncestorOf(const TMessagePath& other) const {
    // An ancestor path must be shorter than the descendant path
    if (Path_.size() >= other.Path_.size()) {
        return false;
    }
    
    // All elements in this path must match corresponding elements in the other path
    for (size_t i = 0; i < Path_.size(); ++i) {
        if (Path_[i] != other.Path_[i]) {
            return false;
        }
    }
    
    return true;
}

bool TMessagePath::isChildOf(const TMessagePath& other) const {
    return other.isParentOf(*this);
}

bool TMessagePath::isDescendantOf(const TMessagePath& other) const {
    return other.isAncestorOf(*this);
}

} // namespace NRelation

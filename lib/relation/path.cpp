#include <relation/path.h>
#include <relation/relation_manager.h>

#include <common/exception.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

size_t GetNextPathEntryHash(size_t parent, size_t entry) {
    return parent ^ (std::hash<size_t>{}(entry) + 0x9e3779b9 + (parent << 6) + (parent >> 2));
}

size_t GetHash(const std::vector<uint32_t>& path) {
    size_t hash_value = 0;
    for (const auto& entry : path) {
        hash_value = GetNextPathEntryHash(hash_value, entry);
    }
    return hash_value;
}

size_t GetHash(const NOrm::NRelation::TMessagePath& path) {
    size_t hash_value = 0;
    for (const auto& entry : path) {
        hash_value = GetNextPathEntryHash(hash_value, entry);
    }
    return hash_value;
}


////////////////////////////////////////////////////////////////////////////////

TMessagePath::TMessagePath(uint32_t entry)
    : Path_({entry}) {}

TMessagePath::TMessagePath(const std::string& entry)
    : Path_({})
{
    for (const auto& entry : NCommon::Split(entry, "/")) { AppendEntry(entry); }
}

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

TMessagePath& TMessagePath::operator/=(uint32_t entry) {
    AppendEntry(entry);
    return *this;
}

TMessagePath& TMessagePath::operator/=(const std::string& entry) {
    for (const auto& entry : NCommon::Split(entry, "/")) { AppendEntry(entry); }
    return *this;
}

void TMessagePath::AppendEntry(const std::string& entry) {
    auto& relationManager = TRelationManager::GetInstance();
    auto pathHash = GetHash(Path_);
    ASSERT(relationManager.EntryNameToEntry_[pathHash].contains(entry), "Entry \"{}/{}\" does not exists", *this, entry);
    Path_.push_back(relationManager.EntryNameToEntry_[pathHash].at(entry));
}

void TMessagePath::AppendEntry(uint32_t entry) {
    Path_.push_back(entry);
}

TMessagePath& TMessagePath::operator/=(const google::protobuf::FieldDescriptor* desc) {
    Path_.push_back(desc->number());
    return *this;
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

std::vector<std::string> TMessagePath::String() const {
    auto& relationManager = TRelationManager::GetInstance();
    std::vector<std::string> result;
    size_t hash = 0;
    for (size_t el : Path_) {
        hash = GetNextPathEntryHash(hash, el);
        if (!relationManager.PathToEntryName_.contains(hash)) { return result; }
        result.emplace_back(relationManager.PathToEntryName_.at(hash));
    }
    return result;
}

const std::vector<uint32_t>& TMessagePath::Number() const {
    return Path_;
}

bool TMessagePath::empty() const {
    return Path_.empty();
}

TMessagePath TMessagePath::parent() const {
    if (empty()) {
        return TMessagePath{};
    }
    TMessagePath result = *this;
    result.PopEntry();
    return result;
}

TMessagePath TMessagePath::parent_() const {
    TMessagePath result = *this;
    result.Path_.pop_back();
    return result;
}

void TMessagePath::PopEntry() {
    Path_.pop_back();
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

size_t TMessagePath::size() const {
    return Path_.size();
}

uint32_t TMessagePath::number() const {
    return back();
}

std::string TMessagePath::name() const {
    const auto& manager = TRelationManager::GetInstance();
    auto pathHash = GetHash(Path_);
    ASSERT(manager.PathToEntryName_.contains(pathHash), "Attept to access unknown name in TMessagePath");
    return manager.PathToEntryName_.at(pathHash);
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
    if (Path_.size() + 1 != other.Path_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < Path_.size(); ++i) {
        if (Path_[i] != other.Path_[i]) {
            return false;
        }
    }
    
    return true;
}

bool TMessagePath::isAncestorOf(const TMessagePath& other) const {
    if (Path_.size() >= other.Path_.size()) {
        return false;
    }
    
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

TMessagePath TMessagePath::GetTablePath() const {
    const auto& table = TRelationManager::GetInstance().GetParentTable(Path_);
    return table->GetPath();
}

std::vector<uint32_t> TMessagePath::GetTable() const {
    return GetTablePath().data();
}

std::vector<uint32_t> TMessagePath::GetField() const {
    return {std::next(Path_.begin(), GetTablePath().size()), Path_.end()};
}

} // namespace NOrm::NRelation

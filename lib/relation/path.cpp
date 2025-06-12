#include <relation/path.h>

#include <common/exception.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

void TPathManager::RegisterField(const TMessagePath& path, const std::string& name) {
    size_t hash = std::hash<TMessagePath>{}(path.parent());
    EntryNameToEntry_[hash][name] = path.number();
    PathToEntryName_[GetHash(hash, path.number())] = name;
}

void TPathManager::RegisterRepeatedField(const TMessagePath& path, const std::string& name) {
    size_t hash = std::hash<TMessagePath>{}(path.parent());
    EntryNameToEntry_[hash][name] = path.number();
    PathToEntryName_[GetHash(hash, path.number())] = name;
    RepeatedFields_.emplace(GetHash(hash, path.number()));
}

void TPathManager::RegisterMapField(
    const TMessagePath& path,
    const std::string& name,
    google::protobuf::FieldDescriptor::Type keyType
) {
    size_t hash = std::hash<TMessagePath>{}(path.parent());
    EntryNameToEntry_[hash][name] = path.number();
    PathToEntryName_[GetHash(hash, path.number())] = name;
    MapFields_[GetHash(hash, path.number())] = keyType;
}

TPathManager::EFieldType TPathManager::FieldType(const TMessagePath& path) const {
    auto hash = std::hash<TMessagePath>{}(path);
    if (RepeatedFields_.contains(hash)) {
        return EFieldType::Repeated;
    }
    if (MapFields_.contains(hash)) {
        return EFieldType::Map;
    }
    return EFieldType::Simple;
}

TPathManager::EFieldType TPathManager::FieldType(const std::vector<uint32_t>& path) const {
    auto hash = GetHash(path);
    if (RepeatedFields_.contains(hash)) {
        return EFieldType::Repeated;
    }
    if (MapFields_.contains(hash)) {
        return EFieldType::Map;
    }
    return EFieldType::Simple;
}

google::protobuf::FieldDescriptor::Type TPathManager::MapType(const TMessagePath& path) const {
    return MapFields_.at(std::hash<TMessagePath>{}(path));
}

google::protobuf::FieldDescriptor::Type TPathManager::MapType(const std::vector<uint32_t>& path) const {
    return MapFields_.at(GetHash(path));
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

size_t TPathManager::GetEntry(const TMessagePath& path, const std::string& entry) const {
    return EntryNameToEntry_.at(std::hash<TMessagePath>{}(path)).at(entry);
}

size_t TPathManager::GetEntry(const std::vector<uint32_t>& path, const std::string& entry) const {
    return EntryNameToEntry_.at(GetHash(path)).at(entry);
}

bool TPathManager::PathRegistered(const TMessagePath& path) const {
    return PathToEntryName_.contains(std::hash<TMessagePath>{}(path));
}

////////////////////////////////////////////////////////////////////////////////

TMessagePath::TMessagePath(uint32_t entry)
    : Path_({entry}), WaitIndex_(false) {}

TMessagePath::TMessagePath(const std::string& entry)
    : Path_({}), WaitIndex_(false)
{
    for (const auto& entry : NCommon::Split(entry, "/")) { AppendEntry(entry); }
}

TMessagePath::TMessagePath(const std::vector<uint32_t>& entries)
    : Path_(entries), WaitIndex_(false) {}

TMessagePath::TMessagePath(const TMessagePath& other)
    : Path_(other.Path_), WaitIndex_(false) {}

TMessagePath::TMessagePath(TMessagePath&& other) noexcept
    : Path_(std::move(other.Path_)), WaitIndex_(false) {}

TMessagePath& TMessagePath::operator=(const TMessagePath& other) {
    if (this != &other) {
        Path_ = other.Path_;
        Indexes_ = other.Indexes_;
        WaitIndex_ = other.WaitIndex_;
        IndexType_ = other.IndexType_;
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
    if (WaitIndex_) {
        if (entry == "*") {
            Indexes_.emplace_back(TAllIndex());
        } else {
            switch (IndexType_) {
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                    Indexes_.push_back(std::stoll(entry));
                    break;
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                    Indexes_.push_back(std::stod(entry));
                    break;
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    Indexes_.push_back(entry);
                    break;
                default:
                    THROW("Uncapable type of index in path");
            }
        }
        WaitIndex_ = false;
        return;
    }
    const auto& pathManger = TPathManager::GetInstance();
    Path_.push_back(pathManger.GetEntry(Path_, entry));
    switch (TPathManager::GetInstance().FieldType(Path_)) {
        case TPathManager::EFieldType::Repeated:
            WaitIndex_ = true;
            IndexType_ = google::protobuf::FieldDescriptor::TYPE_UINT64;
            break;
        case TPathManager::EFieldType::Map:
            WaitIndex_ = true;
            IndexType_ = TPathManager::GetInstance().MapType(Path_);
            break;
        default:
            break;
    }
}

void TMessagePath::AppendEntry(uint32_t entry) {
    if (WaitIndex_) {
        ASSERT(IndexType_ == google::protobuf::FieldDescriptor::TYPE_UINT64, "Uncapable type of index in path");
        Indexes_.push_back(static_cast<long long>(entry));
        WaitIndex_ = false;
        return;
    }
    const auto& pathManger = TPathManager::GetInstance();
    Path_.push_back(entry);
    switch (TPathManager::GetInstance().FieldType(Path_)) {
        case TPathManager::EFieldType::Repeated:
            WaitIndex_ = true;
            IndexType_ = google::protobuf::FieldDescriptor::TYPE_UINT64;
            break;
        case TPathManager::EFieldType::Map:
            WaitIndex_ = true;
            IndexType_ = TPathManager::GetInstance().MapType(Path_);
            break;
        default:
            break;
    }
}

TMessagePath& TMessagePath::operator/=(const google::protobuf::FieldDescriptor* desc) {
    Path_.push_back(desc->number());
    if (desc->is_repeated()) {
        NOrm::NRelation::TPathManager::GetInstance().RegisterRepeatedField(Path_, desc->name());
    } else if (desc->is_map()) {
        NOrm::NRelation::TPathManager::GetInstance().RegisterMapField(Path_, desc->name(), desc->message_type()->field(0)->type());
    } else {
        NOrm::NRelation::TPathManager::GetInstance().RegisterField(Path_, desc->name());
    }
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

bool TMessagePath::empty() const {
    return Path_.empty();
}

TMessagePath TMessagePath::parent() const {
    if (empty()) {
        return TMessagePath{};
    }
    std::vector<uint32_t> newPath(Path_.begin(), std::prev(Path_.end()));
    if (TPathManager::GetInstance().FieldType(newPath) != TPathManager::EFieldType::Simple && !WaitIndex_) {
        return TMessagePath(newPath.begin(), newPath.end(), Indexes_.begin(), std::prev(Indexes_.end()));
    }
    return TMessagePath(Path_.begin(), std::prev(Path_.end()), Indexes_.begin(), Indexes_.end());
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

size_t TMessagePath::GetIndexSize() const {
    return Indexes_.size();
}
TMessagePath::TIndex TMessagePath::GetIndex(size_t idx) const {
    return Indexes_.at(idx);
}

} // namespace NRelation

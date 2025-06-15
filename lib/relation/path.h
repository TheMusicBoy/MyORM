#pragma once

#include <google/protobuf/descriptor.h>

#include <common/format.h>

#include <variant>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

/**
 * @class TMessagePath
 * @brief Represents a path composed of multiple TMessagePathEntry elements.
 *
 * This class provides various constructors to initialize a path, methods
 * to manipulate and retrieve entries, and operators for concatenation with
 * other paths or entries.
 */
class TMessagePath {
  public:
    TMessagePath() = default;

    TMessagePath(uint32_t entry);

    TMessagePath(const std::string& entry);

    TMessagePath(const std::vector<uint32_t>& entries);

    template <typename EntryIt>
    TMessagePath(EntryIt entryBegin, EntryIt entryEnd) : Path_(entryBegin, entryEnd) {}

    TMessagePath(const TMessagePath& other);

    TMessagePath(TMessagePath&& other) noexcept;

    TMessagePath& operator=(const TMessagePath& other);

    TMessagePath& operator=(TMessagePath&& other) noexcept;

    uint32_t at(int index) const;

    TMessagePath& operator/=(uint32_t entry);

    TMessagePath& operator/=(const std::string& entry);

    TMessagePath& operator/=(const google::protobuf::FieldDescriptor* desc);

    TMessagePath operator/(const std::string& entry) const;

    TMessagePath operator/(uint32_t entry) const;

    TMessagePath operator/(const google::protobuf::FieldDescriptor* desc) const;

    std::vector<std::string> String() const;

    const std::vector<uint32_t>& Number() const;

    bool empty() const;

    TMessagePath parent() const;

    TMessagePath parent_() const;

    std::vector<uint32_t>::iterator begin();

    std::vector<uint32_t>::iterator end();

    std::vector<uint32_t>::const_iterator begin() const;

    std::vector<uint32_t>::const_iterator end() const;

    const uint32_t front() const;

    const uint32_t back() const;

    uint32_t& front();

    uint32_t& back();

    std::string name() const;

    uint32_t number() const;

    const std::vector<uint32_t>& data() const;

    size_t size() const;

    bool operator==(const TMessagePath& other) const;
    bool operator!=(const TMessagePath& other) const;
    bool operator<(const TMessagePath& other) const;
    bool operator<=(const TMessagePath& other) const;
    bool operator>(const TMessagePath& other) const;
    bool operator>=(const TMessagePath& other) const;

    bool isParentOf(const TMessagePath& other) const;
    bool isAncestorOf(const TMessagePath& other) const;
    bool isChildOf(const TMessagePath& other) const;
    bool isDescendantOf(const TMessagePath& other) const;

    TMessagePath GetTablePath() const;

    std::vector<uint32_t> GetTable() const;
    std::vector<uint32_t> GetField() const;

  private:
    void AppendEntry(const std::string& entry);
    void AppendEntry(uint32_t entry);
    void PopEntry();

    std::vector<uint32_t> Path_;
};

////////////////////////////////////////////////////////////////////////////////

size_t GetNextPathEntryHash(size_t parent, size_t entry);

size_t GetHash(const std::vector<uint32_t>& path);

size_t GetHash(const NOrm::NRelation::TMessagePath& path);

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

namespace std {
template <>
struct hash<NOrm::NRelation::TMessagePath> {
    size_t operator()(const NOrm::NRelation::TMessagePath& path) const {
        return NOrm::NRelation::GetHash(path);
    }
};
}

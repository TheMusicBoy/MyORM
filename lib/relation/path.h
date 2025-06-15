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
    // Default constructor
    TMessagePath() = default;

    // Constructor that initializes with a single TMessagePathEntry
    TMessagePath(uint32_t entry);

    TMessagePath(const std::string& entry);

    // Constructor that initializes with a vector of TMessagePathEntry
    TMessagePath(const std::vector<uint32_t>& entries);

    // Constructor that initializes with a range of elements
    template <typename EntryIt>
    TMessagePath(EntryIt entryBegin, EntryIt entryEnd) : Path_(entryBegin, entryEnd) {}

    // Copy constructor
    TMessagePath(const TMessagePath& other);

    // Move constructor
    TMessagePath(TMessagePath&& other) noexcept;

    // Copy assignment operator
    TMessagePath& operator=(const TMessagePath& other);

    // Move assignment operator
    TMessagePath& operator=(TMessagePath&& other) noexcept;

    // Retrieve the TMessagePathEntry at the specified index
    uint32_t at(int index) const;

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(uint32_t entry);

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(const std::string& entry);

    // Append a protobuf FieldDescriptor to this path
    TMessagePath& operator/=(const google::protobuf::FieldDescriptor* desc);

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(const std::string& entry) const;

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(uint32_t entry) const;

    // Get TMessagePath resulting from concatenation with a protobuf
    // FieldDescriptor
    TMessagePath operator/(const google::protobuf::FieldDescriptor* desc) const;

    std::vector<std::string> String() const;

    const std::vector<uint32_t>& Number() const;

    // Check if the path is empty
    bool empty() const;

    // Get the parent path by removing the last entry
    TMessagePath parent() const;

    TMessagePath parent_() const;

    // Begin iterator for path entries
    std::vector<uint32_t>::iterator begin();

    // End iterator for path entries
    std::vector<uint32_t>::iterator end();

    // Begin iterator for path entries
    std::vector<uint32_t>::const_iterator begin() const;

    // End iterator for path entries
    std::vector<uint32_t>::const_iterator end() const;

    // Begin iterator for path entries
    const uint32_t front() const;

    // End iterator for path entries
    const uint32_t back() const;

    // Begin iterator for path entries
    uint32_t& front();

    // End iterator for path entries
    uint32_t& back();

    std::string name() const;

    uint32_t number() const;

    const std::vector<uint32_t>& data() const;

    size_t size() const;

    // Comparison operators based on protonum values
    bool operator==(const TMessagePath& other) const;
    bool operator!=(const TMessagePath& other) const;
    bool operator<(const TMessagePath& other) const;
    bool operator<=(const TMessagePath& other) const;
    bool operator>(const TMessagePath& other) const;
    bool operator>=(const TMessagePath& other) const;

    // Path relationship methods
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

// Hash function for TMessagePath
namespace std {
template <>
struct hash<NOrm::NRelation::TMessagePath> {
    size_t operator()(const NOrm::NRelation::TMessagePath& path) const {
        return NOrm::NRelation::GetHash(path);
    }
};
}

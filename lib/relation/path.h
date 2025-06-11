#pragma once

#include <google/protobuf/descriptor.h>

#include <common/format.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

inline size_t GetHash(size_t parent, size_t entry) {
    return parent ^ (std::hash<size_t>{}(entry) + 0x9e3779b9 + (parent << 6) + (parent >> 2));
}

////////////////////////////////////////////////////////////////////////////////

class TMessagePath;

class TPathManager {
  public:
    static TPathManager& GetInstance() {
        static TPathManager instance;
        return instance;
    }

    TPathManager(const TPathManager&) = delete;
    TPathManager& operator=(const TPathManager&) = delete;

    void RegisterEntry(const TMessagePath& path, const std::string& name);

    std::string EntryName(const TMessagePath& path) const;

    std::vector<std::string> ToString(const TMessagePath& path) const;

    uint32_t GetEntry(const TMessagePath& path, const std::string& entry);

    bool PathRegistered(const TMessagePath& path) const;

  private:
    // Приватный конструктор
    TPathManager() = default;
    
    std::unordered_map<size_t, std::string> PathToEntryName_;
    std::unordered_map<size_t, std::unordered_map<std::string, size_t>> EntryNameToEntry_;
};

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
    template <typename It>
    TMessagePath(It begin, It end)
        : Path_(begin, end) {}

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

    // Concatenate another TMessagePath to this one
    TMessagePath& operator/=(const TMessagePath& other);

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(uint32_t entry);

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(const std::string& entry);

    // Append a protobuf FieldDescriptor to this path
    TMessagePath& operator/=(const google::protobuf::FieldDescriptor* desc);

    // Get TMessagePath resulting from the concatenation with another TMessagePath
    TMessagePath operator/(const TMessagePath& other) const;

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(const std::string& entry) const;

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(uint32_t entry) const;

    // Get TMessagePath resulting from concatenation with a protobuf
    // FieldDescriptor
    TMessagePath operator/(const google::protobuf::FieldDescriptor* desc) const;

    // Check if the path is empty
    bool empty() const;

    // Get the parent path by removing the last entry
    TMessagePath parent() const;

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

  private:
    std::vector<uint32_t> Path_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

// Hash function for TMessagePath
namespace std {
template <>
struct hash<NOrm::NRelation::TMessagePath> {
    size_t operator()(const NOrm::NRelation::TMessagePath& path) const {
        size_t hash_value = 0;
        for (const auto& entry : path.data()) {
            hash_value = NOrm::NRelation::GetHash(hash_value, entry);
        }
        return hash_value;
    }
};
}

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePath& container, const FormatOptions& options) {
    if (options.GetBool("table_id", false)) {
        FormatOptions opts;
        opts.Set("delimiter", "_");
        opts.Set("prefix", "t_");
        opts.Set("suffix", "");
        detail::FormatSequenceContainer(out, container.data(), opts);
        return;
    }

    if (options.GetBool("full_field_id", false)) {
        FormatOptions opts;
        opts.Set("delimiter", "_");
        opts.Set("prefix", "t_");
        opts.Set("suffix", "");
        detail::FormatSequenceContainer(out, container.parent().data(), opts);
        out << Format(".f_{}", container.number());
        return;
    }

    if (options.GetBool("field_id", false)) {
        out << Format("f_{}", container.number());
        return;
    }

    FormatOptions defaultOpts;
    defaultOpts.Set("delimiter", "/");
    defaultOpts.Set("prefix", "");
    defaultOpts.Set("suffix", "");
    defaultOpts.Set("limit", -1);

    auto textPath = NOrm::NRelation::TPathManager::GetInstance().ToString(container);

    detail::FormatSequenceContainer(out, textPath, options.Merge(defaultOpts));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

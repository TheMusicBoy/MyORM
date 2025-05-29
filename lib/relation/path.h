#pragma once

#include <google/protobuf/descriptor.h>

#include <common/format.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

struct TMessagePathEntry {
    int protonum;
    std::string name;
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
    TMessagePath(const TMessagePathEntry& entry);

    // Constructor that initializes with a vector of TMessagePathEntry
    TMessagePath(const std::vector<TMessagePathEntry>& entries);

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
    TMessagePathEntry at(int index) const;

    // Concatenate another TMessagePath to this one
    TMessagePath& operator/=(const TMessagePath& other);

    // Append a TMessagePathEntry to this path
    TMessagePath& operator/=(const TMessagePathEntry& entry);

    // Append a protobuf FieldDescriptor to this path
    TMessagePath& operator/=(const google::protobuf::FieldDescriptor* desc);

    // Get TMessagePath resulting from the concatenation with another TMessagePath
    TMessagePath operator/(const TMessagePath& other) const;

    // Get TMessagePath resulting from concatenation with a TMessagePathEntry
    TMessagePath operator/(const TMessagePathEntry& entry) const;

    // Get TMessagePath resulting from concatenation with a protobuf
    // FieldDescriptor
    TMessagePath operator/(const google::protobuf::FieldDescriptor* desc) const;

    // Check if the path is empty
    bool empty() const;

    // Get the parent path by removing the last entry
    TMessagePath parent() const;

    // Begin iterator for path entries
    std::vector<TMessagePathEntry>::iterator begin();

    // End iterator for path entries
    std::vector<TMessagePathEntry>::iterator end();

    const std::vector<TMessagePathEntry>& data() const;

  private:
    std::vector<TMessagePathEntry> Path_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePathEntry& entry, const FormatOptions& options) {
    bool number = options.GetBool("num", false);
    bool name = options.GetBool("name", true);
    std::string delim = options.GetString("delimiter", ";");

    if (number) {
        out << entry.protonum;
    }

    if (number && name) {
        out << delim;
    }

    if (name) {
        out << entry.name;
    }
}

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePath& container, const FormatOptions& options) {
    FormatOptions defaultOpts;
    defaultOpts.Set("delimiter", "/");
    defaultOpts.Set("prefix", "");
    defaultOpts.Set("suffix", "");
    defaultOpts.Set("limit", -1);

    detail::FormatSequenceContainer(out, container.data(), options.Merge(defaultOpts));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

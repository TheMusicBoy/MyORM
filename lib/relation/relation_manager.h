#pragma once

#include <relation/message.h>
#include <relation/field.h>
#include <relation/config.h>

#include <common/intrusive_ptr.h>

#include <memory>
#include <unordered_map>
#include <map>
#include <string>
#include <mutex>
#include <vector>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

class TRelationManager;

class TTableInfo
    : public NRefCounted::TRefCountedBase {
public:
    TTableInfo(const TMessagePath& path, const google::protobuf::Descriptor* desc);

    TTableInfo(const TMessagePath& path, const google::protobuf::FieldDescriptor* desc);

    void AddRelatedMessage(size_t hash);

    void AddRelatedMessage(const TMessagePath& path) { AddRelatedMessage(GetHash(path)); }

    const std::unordered_set<size_t>& GetRelatedMessages() const;

    void AddRelatedField(size_t hash);

    void AddRelatedField(const TMessagePath& path) { AddRelatedField(GetHash(path)); }

    const std::unordered_set<size_t>& GetRelatedFields() const;

    void AddRelatedTable(size_t hash);

    void AddRelatedTable(const TMessagePath& path) { AddRelatedTable(GetHash(path)); }

    const std::unordered_set<size_t>& GetRelatedTables() const;

    const TMessagePath& GetPath() const;

    const std::unordered_set<size_t>& GetPrimaryFields() const;

    const std::vector<google::protobuf::FieldDescriptor::Type>& GetIndexes() const;

private:
    TMessagePath Path_;
    std::unordered_set<size_t> RelatedMessages_;
    std::unordered_set<size_t> RelatedFields_;
    std::unordered_set<size_t> RelatedTables_;

    std::unordered_set<size_t> PrimaryFields_;
    std::vector<google::protobuf::FieldDescriptor::Type> IndexFields_;

    friend TRelationManager;

};

using TTableInfoPtr = NCommon::TIntrusivePtr<TTableInfo>;

enum EObjectType {
    None = 0,
    Message = 1 << 0,
    Root = 1 << 1,
    Field = 1 << 2,

    PrimitiveField = Field,
    FieldMessage = Field | Message,
    RootMessage = Root | Message,
};

/**
 * @class TRelationManager
 * @brief Singleton for managing access to messages and fields.
 *
 * Provides methods for searching and retrieving information about messages
 * and fields. Registration of objects is done externally.
 */
class TRelationManager {
public:
    // Access to the singleton
    static TRelationManager& GetInstance();

    // Prohibit copying and moving
    TRelationManager(const TRelationManager&) = delete;
    TRelationManager& operator=(const TRelationManager&) = delete;
    TRelationManager(TRelationManager&&) = delete;
    TRelationManager& operator=(TRelationManager&&) = delete;

    // Public registration methods
    void RegisterRoot(TRootMessagePtr root);
    void RegisterField(TFieldBasePtr field);

    // Get all messages from the subtree
    std::map<TMessagePath, TMessageInfoPtr> GetMessagesFromSubtree(const TMessagePath& rootPath);

    // Get a single object by path
    TMessageBasePtr GetObject(size_t hash);
    TMessageBasePtr GetObject(const TMessagePath& path);

    // Get a single message by path
    TMessageInfoPtr GetMessage(size_t hash);
    TMessageInfoPtr GetMessage(const TMessagePath& path);

    // Get a root message by path
    TRootMessagePtr GetRootMessage(size_t hash);
    TRootMessagePtr GetRootMessage(const TMessagePath& path);

    // Get a single field by path
    TFieldBasePtr GetField(size_t hash);
    TFieldBasePtr GetField(const TMessagePath& path);

    // Get a primitive field by path
    TPrimitiveFieldInfoPtr GetPrimitiveField(size_t hash);
    TPrimitiveFieldInfoPtr GetPrimitiveField(const TMessagePath& path);

    uint32_t GetObjectType(size_t hash) const;
    uint32_t GetObjectType(const TMessagePath& path) const;

    // Get a message and all its ancestors
    std::map<TMessagePath, TMessageBasePtr> GetObjectWithAncestors(const TMessagePath& path);

    // Get parent message for a field or message
    TMessageInfoPtr GetParentMessage(const TMessageBasePtr entity);

    // Set parent message for a field or message
    void SetParentMessage(const TMessageBasePtr entity, const TMessageInfoPtr parent);

    google::protobuf::FieldDescriptor::Type GetIndexType(const TMessagePath& path);

    // Clear all indexes and data
    void Clear();

    TTableInfo& GetParentTable(const TMessagePath& path);

private:
    // Private constructor (singleton)
    TRelationManager() = default;

    // Indexes for fast access
    std::unordered_map<size_t, TMessageInfoPtr> MessagesByPath_;
    std::unordered_map<size_t, TPrimitiveFieldInfoPtr> PrimitiveFieldsByPath_;

    std::unordered_map<size_t, TRootMessagePtr> RootMessagesByPath_;
    std::unordered_map<size_t, TFieldBasePtr> FieldsByPath_;

    std::unordered_map<size_t, uint32_t> ObjectType_;

    std::unordered_map<size_t, size_t> ParentTable_;
    std::unordered_map<size_t, TTableInfoPtr> TableByPath_;

    // For paths
    std::unordered_map<size_t, std::string> PathToEntryName_;
    std::unordered_map<size_t, std::unordered_map<std::string, size_t>> EntryNameToEntry_;

    std::unordered_map<TMessageBasePtr, TMessageInfoPtr> ParentMap_;
    
    // Caches for expensive operations
    std::unordered_map<TMessagePath, std::map<TMessagePath, TMessageInfoPtr>> MessagesFromSubtreeCache_;
    std::unordered_map<TMessagePath, std::map<TMessagePath, TMessageBasePtr>> ObjectWithAncestorsCache_;

    friend TMessagePath;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

////////////////////////////////////////////////////////////////////////////////

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

inline void FormatHandler(std::ostringstream& out, const ::NOrm::NRelation::TMessagePath& container, const FormatOptions& options) {
    if (options.GetBool("table_id", false)) {
        FormatOptions opts;
        opts.Set("delimiter", "_");
        opts.Set("prefix", "t_");
        opts.Set("suffix", "");
        detail::FormatSequenceContainer(out, container.GetTable(), opts);
        return;
    }

    if (options.GetBool("full_field_id", false)) {
        FormatOptions opts;
        opts.Set("delimiter", "_");
        opts.Set("prefix", "t_");
        opts.Set("suffix", "");
        detail::FormatSequenceContainer(out, container.GetTable(), opts);
        opts.Set("prefix", ".f_");
        auto fieldPath = container.GetField();
        detail::FormatSequenceContainer(out, fieldPath.empty() ? std::vector<uint32_t>({1}) : fieldPath, opts);
        return;
    }

    if (options.GetBool("field_id", false)) {
        FormatOptions opts;
        opts.Set("delimiter", "_");
        opts.Set("prefix", "f_");
        opts.Set("suffix", "");
        auto fieldPath = container.GetField();
        detail::FormatSequenceContainer(out, fieldPath.empty() ? std::vector<uint32_t>({1}) : fieldPath, opts);
        return;
    }

    FormatOptions defaultOpts;
    defaultOpts.Set("delimiter", "/");
    defaultOpts.Set("prefix", "");
    defaultOpts.Set("suffix", "");
    defaultOpts.Set("limit", -1);

    detail::FormatSequenceContainer(out, container.String(), options.Merge(defaultOpts));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

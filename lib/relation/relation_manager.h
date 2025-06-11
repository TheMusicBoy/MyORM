#pragma once

#include <relation/message.h>
#include <relation/field.h>
#include <relation/config.h>

#include <memory>
#include <unordered_map>
#include <map>
#include <string>
#include <mutex>
#include <vector>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

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
    void RegisterMessage(TMessageInfoPtr message);
    void RegisterField(TFieldBasePtr field);

    // Get all messages from the subtree
    std::map<TMessagePath, TMessageInfoPtr> GetMessagesFromSubtree(const TMessagePath& rootPath);

    // Get a single object by path
    TMessageBasePtr GetObject(const TMessagePath& path);

    // Get a single message by path
    TMessageInfoPtr GetMessage(const TMessagePath& path);

    // Get a single field by path
    TPrimitiveFieldInfoPtr GetField(const TMessagePath& path);

    // Get a message and all its ancestors
    std::map<TMessagePath, TMessageBasePtr> GetObjectWithAncestors(const TMessagePath& path);

    // Get parent message for a field or message
    TMessageInfoPtr GetParentMessage(const TMessageBasePtr entity);

    // Set parent message for a field or message
    void SetParentMessage(const TMessageBasePtr entity, const TMessageInfoPtr parent);

    // Clear all indexes and data
    void Clear();

private:
    // Private constructor (singleton)
    TRelationManager() = default;

    // Indexes for fast access
    std::unordered_map<TMessagePath, TMessageInfoPtr> MessagesByPath_;
    std::unordered_map<TMessagePath, TPrimitiveFieldInfoPtr> FieldsByPath_;
    std::unordered_map<TMessageBasePtr, TMessageInfoPtr> ParentMap_;
    
    // Caches for expensive operations
    std::unordered_map<TMessagePath, std::map<TMessagePath, TMessageInfoPtr>> MessagesFromSubtreeCache_;
    std::unordered_map<TMessagePath, std::map<TMessagePath, TMessageBasePtr>> ObjectWithAncestorsCache_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

#include <relation/relation_manager.h>
#include <common/format.h>
#include <stdexcept>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

// Get singleton instance
TRelationManager& TRelationManager::GetInstance() {
    static TRelationManager instance;
    return instance;
}

void TRelationManager::RegisterMessage(TMessageInfoPtr message) {
    if (!message) {
        return;
    }
    
    message->Process();
    // Add message to MessagesByPath_
    MessagesByPath_[message->GetPath()] = message;

    for (const auto& field : message->Fields()) {
        SetParentMessage(field, message);
    }
}

void TRelationManager::RegisterField(TFieldBasePtr field) {
    if (!field || field->IsMessage()) {
        return;
    }
    
    // For primitive fields, add to FieldsByPath_
    auto primitiveField = std::static_pointer_cast<TPrimitiveFieldInfo>(field);
    
    // Ensure we have a valid path
    const TMessagePath& path = field->GetPath();
    if (!path.empty()) {
        FieldsByPath_[path] = primitiveField;
    }
}

std::map<TMessagePath, TMessageInfoPtr> TRelationManager::GetMessagesFromSubtree(const TMessagePath& rootPath) {
    // Check if the result is in the cache
    auto cacheIt = MessagesFromSubtreeCache_.find(rootPath);
    if (cacheIt != MessagesFromSubtreeCache_.end()) {
        return cacheIt->second;
    }
    
    // If rootPath is empty, return empty result for empty paths
    if (rootPath.empty()) {
        std::map<TMessagePath, TMessageInfoPtr> emptyResult;
        MessagesFromSubtreeCache_[rootPath] = emptyResult;
        return emptyResult;
    }
    
    // If not in cache, compute the result
    std::map<TMessagePath, TMessageInfoPtr> result;
    for (const auto& [path, message] : MessagesByPath_) {
        // Check if rootPath is an ancestor of the current path
        if (rootPath.isAncestorOf(path) || rootPath == path) {
            result[path] = message;
        }
    }
    
    // Store the result in the cache
    MessagesFromSubtreeCache_[rootPath] = result;
    
    return result;
}

TMessageInfoPtr TRelationManager::GetMessage(const TMessagePath& path) {
    auto it = MessagesByPath_.find(path);
    if (it != MessagesByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TPrimitiveFieldInfoPtr TRelationManager::GetField(const TMessagePath& path) {
    auto it = FieldsByPath_.find(path);
    if (it != FieldsByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TMessageBasePtr TRelationManager::GetObject(const TMessagePath& path) {
    if (TMessageBasePtr obj = GetMessage(path)) {
        return obj;
    }
    return GetField(path);
}

std::map<TMessagePath, TMessageBasePtr> TRelationManager::GetObjectWithAncestors(const TMessagePath& path) {
    auto cacheIt = ObjectWithAncestorsCache_.find(path);
    if (cacheIt != ObjectWithAncestorsCache_.end()) {
        return cacheIt->second;
    }
    
    // If not in cache, compute the result
    std::map<TMessagePath, TMessageBasePtr> result;
    
    // Get the field
    TMessageBasePtr field = GetObject(path);
    if (!field) {
        ObjectWithAncestorsCache_[path] = result;
        return result;
    }
    
    result[field->GetPath()] = field;
    
    TMessageInfoPtr parent = GetParentMessage(field);
    while (parent) {
        result[parent->GetPath()] = parent;
        parent = GetParentMessage(parent);
    }
    
    // Store the result in the cache
    ObjectWithAncestorsCache_[path] = result;
    
    return result;
}

TMessageInfoPtr TRelationManager::GetParentMessage(const TMessageBasePtr entity) {
    if (!entity) {
        return nullptr;
    }
    
    auto it = ParentMap_.find(entity);
    if (it != ParentMap_.end()) {
        return it->second;
    }
    
    return nullptr;
}

void TRelationManager::SetParentMessage(const TMessageBasePtr entity, const TMessageInfoPtr parent) {
    if (!entity && !parent) {
        return;
    }
    
    // Устанавливаем связь между сущностью и ее родителем
    ParentMap_[entity] = parent;
    
    // Очищаем кеши, зависящие от родительских отношений
    ObjectWithAncestorsCache_.clear();
}

void TRelationManager::Clear() {
    MessagesByPath_.clear();
    FieldsByPath_.clear();
    ParentMap_.clear();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation










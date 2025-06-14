#include <relation/relation_manager.h>
#include <lib/relation/proto/orm_core.pb.h>

#include <common/format.h>

namespace NOrm::NRelation {

namespace {

////////////////////////////////////////////////////////////////////////////////

std::unordered_set<size_t> FindPrimaryFields(const google::protobuf::Descriptor* desc, const TMessagePath& basePath) {
    std::unordered_set<size_t> result;
    if (!desc) {
        return result;
    }

    for (int i = 0; i < desc->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = desc->field(i);
        
        // Пропускаем повторяющиеся поля и поля-карты
        if (field->is_repeated() || field->is_map()) {
            continue;
        }
        
        // Создаем путь к полю
        TMessagePath fieldPath = basePath / field;
        
        // Проверяем, имеет ли поле расширение primary_key
        const google::protobuf::FieldOptions& options = field->options();
        if (options.HasExtension(orm::primary_key) && 
            options.GetExtension(orm::primary_key)) {
            result.emplace(GetHash(fieldPath));
        }
        
        // Если это поле типа сообщения, рекурсивно обрабатываем его
        if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
            result.merge(FindPrimaryFields(field->message_type(), fieldPath));
        }
    }
    
    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////

TTableInfo::TTableInfo(const TMessagePath& path, const google::protobuf::Descriptor* desc)
    : Path_(path), PrimaryFields_(FindPrimaryFields(desc, Path_))
{ }

TTableInfo::TTableInfo(const TMessagePath& path, const google::protobuf::FieldDescriptor* desc)
    : Path_(path)
{
    auto& relationManager = TRelationManager::GetInstance();

    auto parentTable = relationManager.GetParentTable(path.parent_());
    PrimaryFields_ = parentTable->PrimaryFields_;
    IndexFields_ = parentTable->IndexFields_;
    parentTable->AddRelatedTable(GetHash(Path_));

    if (desc->is_repeated()) {
        IndexFields_.emplace_back(google::protobuf::FieldDescriptor::TYPE_UINT64);
    }

    if (desc->is_map()) {
        IndexFields_.emplace_back(desc->message_type()->field(0)->type());
    }
}

void TTableInfo::AddRelatedMessage(size_t hash) { RelatedMessages_.emplace(hash); }

const std::unordered_set<size_t>& TTableInfo::GetRelatedMessages() const { return RelatedMessages_; }

void TTableInfo::AddRelatedField(size_t hash) { RelatedFields_.emplace(hash); }

const std::unordered_set<size_t>& TTableInfo::GetRelatedFields() const { return RelatedFields_; }

void TTableInfo::AddRelatedTable(size_t hash) { RelatedTables_.emplace(hash); }

const std::unordered_set<size_t>& TTableInfo::GetRelatedTables() const { return RelatedTables_; }

const TMessagePath& TTableInfo::GetPath() const { return Path_; }

const std::unordered_set<size_t>& TTableInfo::GetPrimaryFields() const { return PrimaryFields_; }

const std::vector<google::protobuf::FieldDescriptor::Type>& TTableInfo::GetIndexes() const { return IndexFields_; }

////////////////////////////////////////////////////////////////////////////////

// Get singleton instance
TRelationManager& TRelationManager::GetInstance() {
    static TRelationManager instance;
    return instance;
}

void TRelationManager::RegisterRoot(TRootMessagePtr message) {
    auto pathHash = GetHash(message->GetPath());
    auto tableInfo = NCommon::New<TTableInfo>(message->GetPath(), message->GetMessageDescriptor());
    TableByPath_[pathHash] = tableInfo;
    
    tableInfo->AddRelatedMessage(pathHash);
    ParentTable_.emplace(pathHash, pathHash);

    PathToEntryName_[pathHash] = message->GetSnakeCase();
    EntryNameToEntry_[0][message->GetSnakeCase()] = message->Number();

    message->Process();
    // Add message to MessagesByPath_
    MessagesByPath_[pathHash] = message;
    RootMessagesByPath_[pathHash] = message;
    ObjectType_[pathHash] = EObjectType::RootMessage;

    for (const auto& field : message->Fields()) {
        SetParentMessage(field, message);
    }
}

void TRelationManager::RegisterField(TFieldBasePtr field) {
    auto pathHash = GetHash(field->GetPath());
    auto parentHash = GetHash(field->GetPath().parent_());

    PathToEntryName_[pathHash] = field->GetFieldDescriptor()->name();
    EntryNameToEntry_[parentHash][field->GetFieldDescriptor()->name()] = field->GetPath().back();

    auto fieldType = field->GetFieldType();
    if (fieldType == EFieldType::REPEATED || fieldType == EFieldType::MAP) {
        auto tableInfo = NCommon::New<TTableInfo>(field->GetPath(), field->GetFieldDescriptor());
        TableByPath_[pathHash] = tableInfo;
        ParentTable_.emplace(pathHash, pathHash);
    } else {
        const auto path = field->GetPath();
        ParentTable_.emplace(pathHash, ParentTable_.at(parentHash));
    }

    if (field->IsMessage()) {
        auto messageField = std::static_pointer_cast<TFieldMessage>(field);
        TRelationManager::GetInstance().GetParentTable(field->GetPath())->AddRelatedMessage(pathHash);

        MessagesByPath_[pathHash] = messageField;
        FieldsByPath_[pathHash] = messageField;
        ObjectType_[pathHash] = EObjectType::FieldMessage;

        messageField->Process();
    } else {
        auto primitiveField = std::static_pointer_cast<TPrimitiveFieldInfo>(field);
        TRelationManager::GetInstance().GetParentTable(field->GetPath())->AddRelatedField(pathHash);

        FieldsByPath_[pathHash] = primitiveField;
        PrimitiveFieldsByPath_[pathHash] = primitiveField;
        ObjectType_[pathHash] = EObjectType::PrimitiveField;
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
    for (const auto& [_, message] : MessagesByPath_) {
        const auto& path = message->GetPath();
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
    return GetMessage(GetHash(path));
}

TMessageInfoPtr TRelationManager::GetMessage(size_t hash) {
    auto it = MessagesByPath_.find(hash);
    if (it != MessagesByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TRootMessagePtr TRelationManager::GetRootMessage(const TMessagePath& path) {
    return GetRootMessage(GetHash(path));
}

TRootMessagePtr TRelationManager::GetRootMessage(size_t hash) {
    auto it = RootMessagesByPath_.find(hash);
    if (it != RootMessagesByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TPrimitiveFieldInfoPtr TRelationManager::GetPrimitiveField(const TMessagePath& path) {
    return GetPrimitiveField(GetHash(path));
}

TPrimitiveFieldInfoPtr TRelationManager::GetPrimitiveField(size_t hash) {
    auto it = PrimitiveFieldsByPath_.find(hash);
    if (it != PrimitiveFieldsByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TFieldBasePtr TRelationManager::GetField(const TMessagePath& path) {
    return GetField(GetHash(path));
}

TFieldBasePtr TRelationManager::GetField(size_t hash) {
    auto it = FieldsByPath_.find(hash);
    if (it != FieldsByPath_.end()) {
        return it->second;
    }
    
    return nullptr;
}

TMessageBasePtr TRelationManager::GetObject(const TMessagePath& path) {
    return GetObject(GetHash(path));
}

TMessageBasePtr TRelationManager::GetObject(size_t hash) {
    if (TMessageBasePtr obj = GetMessage(hash)) {
        return obj;
    }
    return GetField(hash);
}

uint32_t TRelationManager::GetObjectType(const TMessagePath& path) const {
    return GetObjectType(GetHash(path));
}

uint32_t TRelationManager::GetObjectType(size_t hash) const {
    auto it = ObjectType_.find(hash);
    if (it != ObjectType_.end()) {
        return it->second;
    }
    
    return 0;
}

TTableInfoPtr TRelationManager::GetParentTable(const TMessagePath& path) {
    auto pathHash = GetHash(path);
    auto parentTableIt = ParentTable_.find(pathHash);
    ASSERT(parentTableIt != ParentTable_.end(), "Table for path not found {}", path.Number());

    auto tableHash = parentTableIt->second;
    return TableByPath_.at(tableHash);
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

google::protobuf::FieldDescriptor::Type TRelationManager::GetIndexType(const TMessagePath& path) {
    auto pathHash = GetHash(path);
    if (!TableByPath_.contains(pathHash)) {
        return static_cast<google::protobuf::FieldDescriptor::Type>(0);
    }

    const auto& table = TableByPath_.at(pathHash);
    if (table->IndexFields_.empty()) {
        return static_cast<google::protobuf::FieldDescriptor::Type>(0);
    }

    return table->IndexFields_.back();
}

void TRelationManager::Clear() {
    MessagesByPath_.clear();
    FieldsByPath_.clear();
    ParentMap_.clear();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation










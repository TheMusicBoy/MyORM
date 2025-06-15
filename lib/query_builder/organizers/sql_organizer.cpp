#include <query_builder/organizers/sql_organizer.h>

#include <relation/relation_manager.h>

namespace NOrm::NRelation {

////////////////////////////////////////////////////////////////////////////////

Builder::TClausePtr TSqlQueryOrganizer::TransformClause(TClause clause) const {
    if (!clause) {
        return nullptr;
    }

    switch (clause.Type()) {
        case NOrm::NApi::TClause::ValueCase::kString: {
            TString stringClause = clause;
            auto result = std::make_shared<Builder::TString>();
            result->SetValue(stringClause.GetValue());
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kInteger: {
            TInt intClause = clause;
            auto result = std::make_shared<Builder::TInt>();
            result->SetValue(intClause.GetValue());
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kFloat: {
            TFloat floatClause = clause;
            auto result = std::make_shared<Builder::TFloat>();
            result->SetValue(floatClause.GetValue());
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kBool: {
            TBool boolClause = clause;
            auto result = std::make_shared<Builder::TBool>();
            result->SetValue(boolClause.GetValue());
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kExpression: {
            TExpression exprClause = clause;
            auto result = std::make_shared<Builder::TExpression>();
            
            // Преобразуем все операнды
            std::vector<Builder::TClausePtr> operands;
            for (const auto& operand : exprClause.GetOperands()) {
                operands.push_back(TransformClause(operand));
            }
            
            result->SetOperands(operands);
            result->SetExpressionType(exprClause.GetExpressionType());
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kColumn: {
            TColumn columnClause = clause;
            const auto& path = columnClause.GetPath();
            auto hash = GetHash(path.GetTable());
            auto& relationManager = TRelationManager::GetInstance();
            auto table = relationManager.GetParentTable(path);
            auto field = relationManager.GetPrimitiveField(path);
            
            auto result = std::make_shared<Builder::TColumn>(table->GetPath().data(), field->GetPath().GetField());
            result->SetKeyType(Builder::EKeyType::Simple);
            return result;
        }
        case NOrm::NApi::TClause::ValueCase::kAll: {
            return std::make_shared<Builder::TAll>();
        }
        case NOrm::NApi::TClause::ValueCase::kDefault: {
            return std::make_shared<Builder::TDefault>();
        }
        case NOrm::NApi::TClause::ValueCase::kSelect: {
            return OrganizeSelect(clause);
        }
        default:
            return nullptr;
    }
}

std::vector<Builder::TClausePtr> TSqlQueryOrganizer::ExpandSelector(TClause clause) const {
    auto& relationManager = TRelationManager::GetInstance();

    if (clause.Type() == NOrm::NApi::TClause::ValueCase::kColumn) {
        auto column = TColumn(clause);
        std::vector<Builder::TClausePtr> result;

        if (relationManager.GetObjectType(column.GetPath()) & EObjectType::Message) {
            for (const auto& [_, message] : relationManager.GetMessagesFromSubtree(column.GetPath())) {
                for (const auto& field : message->PrimitiveFields()) {
                    auto builderColumn = std::make_shared<Builder::TColumn>(field->GetPath().GetTable(), field->GetPath().GetField());
                    builderColumn->SetKeyType(Builder::EKeyType::Simple);
                    result.push_back(builderColumn);
                }
            }
        } else {
            auto builderColumn = std::make_shared<Builder::TColumn>(column.GetPath().GetTable(), column.GetPath().GetField());
            builderColumn->SetKeyType(Builder::EKeyType::Simple);
            result.push_back(builderColumn);
        }
        return result;
    } else {
        return {TransformClause(clause)};
    }
}

////////////////////////////////////////////////////////////////////////////////

Builder::TSelectPtr TSqlQueryOrganizer::OrganizeSelect(const TSelect& query) const {
    Builder::TSelectPtr result = std::make_shared<Builder::TSelect>();
    auto& relationManager = TRelationManager::GetInstance();

    // Set selectors
    std::vector<Builder::TClausePtr> selectorArray;
    for (auto id : query.GetSelectors()) {
        auto subResult = ExpandSelector(id);
        selectorArray.insert(subResult.begin(), subResult.end(), selectorArray.end());
    }
    result->SetSelectors(selectorArray);

    // Set from
    result->SetFrom(std::make_shared<Builder::TTable>(GetHash(std::vector<uint32_t>{query.GetTableNum()})));

    result->SetWhere(TransformClause(query.GetWhere()));

    result->SetHaving(TransformClause(query.GetHaving()));

    result->SetGroupBy(TransformClause(query.GetGroupBy()));

    result->SetOrderBy(TransformClause(query.GetOrderBy()));

    result->SetLimit(TransformClause(query.GetLimit()));
    
    return result;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TInsertPtr TSqlQueryOrganizer::OrganizeInsert(const TInsert& query) const {
    auto tableHash = GetHash(std::vector<uint32_t>{query.GetTableNum()});
    
    Builder::TInsertPtr result = std::make_shared<Builder::TInsert>(tableHash);
    
    // Если нет подзапросов, возвращаем пустой результат
    if (query.GetSubrequests().empty()) {
        return result;
    }
    
    // Извлекаем уникальные пути атрибутов и создаем соответствующие селекторы
    std::vector<Builder::TClausePtr> selectors;
    std::map<TMessagePath, size_t> pathToIndex;
    
    for (const auto& attribute : query.GetSubrequests()[0]) {
        auto column = std::make_shared<Builder::TColumn>(attribute.Path.GetTable(), attribute.Path.GetField());
        column->SetKeyType(Builder::EKeyType::Simple);
        selectors.push_back(column);
        pathToIndex[attribute.Path] = selectors.size() - 1;
    }
    
    // Создаем значения для каждого подзапроса
    std::vector<std::vector<Builder::TClausePtr>> values;
    
    for (const auto& subrequest : query.GetSubrequests()) {
        std::vector<Builder::TClausePtr> rowValues(selectors.size(), std::make_shared<Builder::TDefault>());
        
        for (const auto& attribute : subrequest) {
            // Находим индекс селектора для этого пути
            auto it = pathToIndex.find(attribute.Path);
            if (it == pathToIndex.end()) {
                // Если путь не найден, добавляем новый селектор
                auto column = std::make_shared<Builder::TColumn>(attribute.Path.GetTable(), attribute.Path.GetField());
                column->SetKeyType(Builder::EKeyType::Simple);
                selectors.push_back(column);
                
                // Обновляем отображение путь -> индекс
                it = pathToIndex.insert({attribute.Path, selectors.size() - 1}).first;
                
                // Расширяем все существующие строки значений
                for (auto& row : values) {
                    row.push_back(std::make_shared<Builder::TDefault>());
                }
                
                // Расширяем текущую строку
                rowValues.push_back(std::make_shared<Builder::TDefault>());
            }
            
            // Преобразуем значение атрибута в соответствующий тип клаузы
            Builder::TClausePtr value;
            
            if (std::holds_alternative<bool>(attribute.Data)) {
                auto boolValue = std::make_shared<Builder::TBool>();
                boolValue->SetValue(std::get<bool>(attribute.Data));
                value = boolValue;
            } else if (std::holds_alternative<uint32_t>(attribute.Data)) {
                auto intValue = std::make_shared<Builder::TInt>();
                intValue->SetValue(static_cast<int32_t>(std::get<uint32_t>(attribute.Data)));
                value = intValue;
            } else if (std::holds_alternative<int32_t>(attribute.Data)) {
                auto intValue = std::make_shared<Builder::TInt>();
                intValue->SetValue(std::get<int32_t>(attribute.Data));
                value = intValue;
            } else if (std::holds_alternative<uint64_t>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::to_string(std::get<uint64_t>(attribute.Data)));
                value = stringValue;
            } else if (std::holds_alternative<int64_t>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::to_string(std::get<int64_t>(attribute.Data)));
                value = stringValue;
            } else if (std::holds_alternative<float>(attribute.Data)) {
                auto floatValue = std::make_shared<Builder::TFloat>();
                floatValue->SetValue(std::get<float>(attribute.Data));
                value = floatValue;
            } else if (std::holds_alternative<double>(attribute.Data)) {
                auto floatValue = std::make_shared<Builder::TFloat>();
                floatValue->SetValue(std::get<double>(attribute.Data));
                value = floatValue;
            } else if (std::holds_alternative<std::string>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::get<std::string>(attribute.Data));
                value = stringValue;
            } else if (std::holds_alternative<std::shared_ptr<google::protobuf::Message>>(attribute.Data)) {
                // Для сообщений используем DEFAULT
                value = std::make_shared<Builder::TDefault>();
            } else {
                value = std::make_shared<Builder::TDefault>();
            }
            
            // Устанавливаем значение в соответствующую позицию
            rowValues[it->second] = value;
        }
        
        values.push_back(rowValues);
    }
    
    // Устанавливаем селекторы и значения
    result->SetSelectors(selectors);
    result->SetIsValues(true);
    result->SetValues(values);
    
    // Устанавливаем флаг обновления, если необходимо
    if (query.GetUpdateIfExists()) {
        result->SetIsDoUpdate(true);
        
        // Создаем список пар (колонка, excluded.колонка) для ON CONFLICT DO UPDATE
        std::vector<std::pair<Builder::TClausePtr, Builder::TClausePtr>> doUpdate;
        
        for (const auto& selector : selectors) {
            auto column = std::dynamic_pointer_cast<Builder::TColumn>(selector);
            if (column) {
                // Создаем копию колонки с типом EXCLUDED
                auto excludedColumn = std::make_shared<Builder::TColumn>(column->GetTablePath(), column->GetFieldPath());
                excludedColumn->SetKeyType(column->GetKeyType());
                excludedColumn->SetColumnType(NQuery::EColumnType::EExcluded);
                
                doUpdate.emplace_back(column, excludedColumn);
            }
        }
        
        result->SetDoUpdate(doUpdate);
    }
    
    return result;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TQueryPtr TSqlQueryOrganizer::OrganizeUpdate(const TUpdate& query) const {
    // Получаем хеш таблицы
    auto tableHash = GetHash(std::vector<uint32_t>{query.GetTableNum()});
    
    // Подготавливаем список пар (колонка, значение) для обновления
    std::vector<std::pair<Builder::TClausePtr, Builder::TClausePtr>> updates;
    
    // Обрабатываем каждый набор атрибутов
    for (const auto& updateSet : query.GetUpdates()) {
        for (const auto& attribute : updateSet) {
            // Создаем колонку
            auto column = std::make_shared<Builder::TColumn>(attribute.Path.GetTable(), attribute.Path.GetField());
            column->SetKeyType(Builder::EKeyType::Simple);
            
            // Преобразуем значение атрибута в соответствующий тип клаузы
            Builder::TClausePtr value;
            
            if (std::holds_alternative<bool>(attribute.Data)) {
                auto boolValue = std::make_shared<Builder::TBool>();
                boolValue->SetValue(std::get<bool>(attribute.Data));
                value = boolValue;
            } else if (std::holds_alternative<uint32_t>(attribute.Data)) {
                auto intValue = std::make_shared<Builder::TInt>();
                intValue->SetValue(static_cast<int32_t>(std::get<uint32_t>(attribute.Data)));
                value = intValue;
            } else if (std::holds_alternative<int32_t>(attribute.Data)) {
                auto intValue = std::make_shared<Builder::TInt>();
                intValue->SetValue(std::get<int32_t>(attribute.Data));
                value = intValue;
            } else if (std::holds_alternative<uint64_t>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::to_string(std::get<uint64_t>(attribute.Data)));
                value = stringValue;
            } else if (std::holds_alternative<int64_t>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::to_string(std::get<int64_t>(attribute.Data)));
                value = stringValue;
            } else if (std::holds_alternative<float>(attribute.Data)) {
                auto floatValue = std::make_shared<Builder::TFloat>();
                floatValue->SetValue(std::get<float>(attribute.Data));
                value = floatValue;
            } else if (std::holds_alternative<double>(attribute.Data)) {
                auto floatValue = std::make_shared<Builder::TFloat>();
                floatValue->SetValue(std::get<double>(attribute.Data));
                value = floatValue;
            } else if (std::holds_alternative<std::string>(attribute.Data)) {
                auto stringValue = std::make_shared<Builder::TString>();
                stringValue->SetValue(std::get<std::string>(attribute.Data));
                value = stringValue;
            } else if (std::holds_alternative<std::shared_ptr<google::protobuf::Message>>(attribute.Data)) {
                // Для сообщений используем DEFAULT
                value = std::make_shared<Builder::TDefault>();
            } else {
                value = std::make_shared<Builder::TDefault>();
            }
            
            updates.emplace_back(column, value);
        }
    }
    
    // Создаем объект Builder::TUpdate
    auto updatePtr = std::make_shared<Builder::TUpdate>(tableHash, updates);
    
    // Создаем общий объект Builder::TQuery и добавляем в него обновление
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(updatePtr);
    
    return queryPtr;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TQueryPtr TSqlQueryOrganizer::OrganizeDelete(const TDelete& query) const {
    // Получаем хеш таблицы
    auto tableHash = GetHash(std::vector<uint32_t>{query.GetTableNum()});
    
    // Преобразуем условие WHERE, если оно задано
    Builder::TClausePtr whereClause = TransformClause(query.GetWhere());
    
    // Создаем объект Builder::TDelete
    auto deletePtr = std::make_shared<Builder::TDelete>(tableHash, whereClause);
    
    // Создаем общий объект Builder::TQuery и добавляем в него удаление
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(deletePtr);
    
    return queryPtr;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TQueryPtr TSqlQueryOrganizer::CreateTable(const TRootMessage& table) const {
    // Получаем экземпляр менеджера отношений
    auto& relationManager = TRelationManager::GetInstance();
    
    // Получаем информацию о таблице по пути
    auto tableInfo = relationManager.GetParentTable(table.GetPath());
    if (!tableInfo) {
        return nullptr;
    }
    
    // Создаем объект Builder::TCreateTable с информацией о таблице
    auto createTablePtr = std::make_shared<Builder::TCreateTable>(tableInfo);
    
    // Создаем общий объект Builder::TQuery и добавляем в него создание таблицы
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(createTablePtr);
    
    return queryPtr;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TQueryPtr TSqlQueryOrganizer::DeleteTable(const TRootMessage& table) const {
    // Получаем экземпляр менеджера отношений
    auto& relationManager = TRelationManager::GetInstance();
    
    // Получаем информацию о таблице по пути
    auto tableInfo = relationManager.GetParentTable(table.GetPath());
    if (!tableInfo) {
        return nullptr;
    }
    
    // Создаем объект Builder::TDropTable с информацией о таблице
    auto dropTablePtr = std::make_shared<Builder::TDropTable>(tableInfo);
    
    // Создаем общий объект Builder::TQuery и добавляем в него удаление таблицы
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(dropTablePtr);
    
    return queryPtr;
}

////////////////////////////////////////////////////////////////////////////////

Builder::TQueryPtr TSqlQueryOrganizer::StartTransaction(const TMessagePath& table) const {
    // Создаем объект Builder::TStartTransaction (не в режиме только для чтения по умолчанию)
    auto startTransactionPtr = std::make_shared<Builder::TStartTransaction>(false);
    
    // Создаем общий объект Builder::TQuery и добавляем в него начало транзакции
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(startTransactionPtr);
    
    return queryPtr;
}

Builder::TQueryPtr TSqlQueryOrganizer::CommitTransaction(const TMessagePath& table) const {
    // Создаем объект Builder::TCommitTransaction
    auto commitTransactionPtr = std::make_shared<Builder::TCommitTransaction>();
    
    // Создаем общий объект Builder::TQuery и добавляем в него подтверждение транзакции
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(commitTransactionPtr);
    
    return queryPtr;
}

Builder::TQueryPtr TSqlQueryOrganizer::RollbackTransaction(const TMessagePath& table) const {
    // Создаем объект Builder::TRollbackTransaction
    auto rollbackTransactionPtr = std::make_shared<Builder::TRollbackTransaction>();
    
    // Создаем общий объект Builder::TQuery и добавляем в него откат транзакции
    auto queryPtr = std::make_shared<Builder::TQuery>();
    queryPtr->AddClause(rollbackTransactionPtr);
    
    return queryPtr;
}

////////////////////////////////////////////////////////////////////////////////

TSqlQueryOrganizer::TSqlQueryOrganizer() = default;

////////////////////////////////////////////////////////////////////////////////

} // namespace NOrm::NRelation

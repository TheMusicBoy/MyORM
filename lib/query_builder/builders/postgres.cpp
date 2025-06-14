#include <query_builder/builders/postgres.h>
#include <relation/message.h>
#include <relation/field.h>
#include <common/format.h>
#include <sstream>

namespace NOrm::NRelation::Builder {

namespace {

////////////////////////////////////////////////////////////////////////////////

std::string FieldToString(const std::vector<uint32_t>& fieldPath, EKeyType type) {
    switch (type) {
        case EKeyType::Simple:
            return Format("f_{onlydelim,delimiter='_'}", fieldPath);
        case EKeyType::Primary:
            return Format("p_{onlydelim,delimiter='_'}", fieldPath);
        case EKeyType::Index:
            return Format("i_{onlydelim,delimiter='_'}", fieldPath);
        default:
            THROW("Invalid field type");
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Конструктор и деструктор

TPostgresBuilder::TPostgresBuilder() {
}

TPostgresBuilder::~TPostgresBuilder() {
}

////////////////////////////////////////////////////////////////////////////////
// Вспомогательные методы

std::string TPostgresBuilder::EscapeStringLiteral(const std::string& str) {
    // В PostgreSQL строки экранируются одинарными кавычками
    std::string result = "'";
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else if (c == '\\') {
            result += "\\\\";
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (c == '\t') {
            result += "\\t";
        } else {
            result += c;
        }
    }
    result += "'";
    return result;
}

std::string TPostgresBuilder::EscapeIdentifier(const std::string& identifier) {
    // В PostgreSQL идентификаторы экранируются двойными кавычками
    std::string result = "\"";
    for (char c : identifier) {
        if (c == '"') {
            result += "\"\""; // Двойные кавычки экранируются двойными кавычками
        } else {
            result += c;
        }
    }
    result += "\"";
    return result;
}

std::vector<std::string> TPostgresBuilder::BuildVector(const std::vector<TClausePtr>& clauses) {
    std::vector<std::string> result;
    for (const auto& clause : clauses) {
        result.emplace_back(BuildClause(clause));
    }
    return result;
}

std::string TPostgresBuilder::GetPostgresType(const TValueInfo& typeInfo) {
    if (std::holds_alternative<TBoolFieldInfo>(typeInfo)) {
        return "BOOLEAN";
    } else if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
        auto& info = std::get<TInt32FieldInfo>(typeInfo);
        return info.increment ? "SERIAL" : "INTEGER";
    } else if (std::holds_alternative<TUInt32FieldInfo>(typeInfo)) {
        auto& info = std::get<TUInt32FieldInfo>(typeInfo);
        return info.increment ? "SERIAL" : "INTEGER";
    } else if (std::holds_alternative<TInt64FieldInfo>(typeInfo)) {
        auto& info = std::get<TInt64FieldInfo>(typeInfo);
        return info.increment ? "BIGSERIAL" : "BIGINT";
    } else if (std::holds_alternative<TUInt64FieldInfo>(typeInfo)) {
        auto& info = std::get<TUInt64FieldInfo>(typeInfo);
        return info.increment ? "BIGSERIAL" : "BIGINT";
    } else if (std::holds_alternative<TFloatFieldInfo>(typeInfo)) {
        return "REAL";
    } else if (std::holds_alternative<TDoubleFieldInfo>(typeInfo)) {
        return "DOUBLE PRECISION";
    } else if (std::holds_alternative<TStringFieldInfo>(typeInfo)) {
        return "TEXT";
    } else if (std::holds_alternative<TBytesFieldInfo>(typeInfo)) {
        return "BYTEA";
    } else if (std::holds_alternative<TEnumFieldInfo>(typeInfo)) {
        return "INTEGER";
    } else {
        return "TEXT";
    }
}

std::string TPostgresBuilder::GetPostgresDefault(const TValueInfo& typeInfo) {
    if (std::holds_alternative<TBoolFieldInfo>(typeInfo)) {
        return std::get<TBoolFieldInfo>(typeInfo).defaultValue ? "TRUE" : "FALSE";
    } else if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
        auto& info = std::get<TInt32FieldInfo>(typeInfo);
        if (!info.increment) {
            return std::to_string(info.defaultValue);
        }
    } else if (std::holds_alternative<TUInt32FieldInfo>(typeInfo)) {
        auto& info = std::get<TUInt32FieldInfo>(typeInfo);
        if (!info.increment) {
            return std::to_string(info.defaultValue);
        }
    } else if (std::holds_alternative<TInt64FieldInfo>(typeInfo)) {
        auto& info = std::get<TInt64FieldInfo>(typeInfo);
        if (!info.increment) {
            return std::to_string(info.defaultValue);
        }
    } else if (std::holds_alternative<TUInt64FieldInfo>(typeInfo)) {
        auto& info = std::get<TUInt64FieldInfo>(typeInfo);
        if (!info.increment) {
            return std::to_string(info.defaultValue);
        }
    } else if (std::holds_alternative<TFloatFieldInfo>(typeInfo)) {
        return std::to_string(std::get<TFloatFieldInfo>(typeInfo).defaultValue);
    } else if (std::holds_alternative<TDoubleFieldInfo>(typeInfo)) {
        return std::to_string(std::get<TDoubleFieldInfo>(typeInfo).defaultValue);
    } else if (std::holds_alternative<TStringFieldInfo>(typeInfo)) {
        return EscapeStringLiteral(std::get<TStringFieldInfo>(typeInfo).defaultValue);
    } else if (std::holds_alternative<TBytesFieldInfo>(typeInfo)) {
        // Для бинарных данных обычно не указывается значение по умолчанию
        return "''::bytes";
    } else if (std::holds_alternative<TEnumFieldInfo>(typeInfo)) {
        return std::to_string(std::get<TEnumFieldInfo>(typeInfo).defaultValue);
    }
    
    // Для std::monostate или неизвестных типов
    return "NULL";
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов интерфейса TBuilderBase

std::string TPostgresBuilder::BuildString(TStringPtr value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::String);
    const std::string& str = value->GetValue();
    std::string result = "'";
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else if (c == '\\') {
            result += "\\\\";
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (c == '\t') {
            result += "\\t";
        } else {
            result += c;
        }
    }
    result += "'";
    return result;
}

std::string TPostgresBuilder::BuildInt(TIntPtr value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Int);
    return std::to_string(value->GetValue());
}

std::string TPostgresBuilder::BuildFloat(TFloatPtr value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Float);
    return std::to_string(value->GetValue());
}

std::string TPostgresBuilder::BuildBool(TBoolPtr value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Bool);
    return value->GetValue() ? "TRUE" : "FALSE";
}

std::string TPostgresBuilder::BuildExpression(TExpressionPtr expression) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Expression);
    
    const auto& operands = expression->GetOperands();
    NQuery::EExpressionType type = expression->GetExpressionType();
    
    switch (type) {
        // Арифметические выражения
        case NQuery::EExpressionType::add:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} + {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::subtract:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} - {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::multiply:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} * {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::divide:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} / {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::modulo:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} % {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::exponent:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("POWER({}, {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
            
        // Сравнения
        case NQuery::EExpressionType::equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} = {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::not_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} <> {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::greater_than:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} > {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::less_than:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} < {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::greater_than_or_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} >= {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::less_than_or_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} <= {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
            
        // Логические выражения
        case NQuery::EExpressionType::and_:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} AND {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::or_:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} OR {})",
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::not_:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("NOT {}", BuildClause(operands[0]));
            
        // Строковые выражения
        case NQuery::EExpressionType::like:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} LIKE {})", 
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::ilike:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} ILIKE {})", 
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::similar_to:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} SIMILAR TO {})", 
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
        case NQuery::EExpressionType::regexp_match:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} ~ {})", 
                BuildClause(operands[0]),
                BuildClause(operands[1])
            );
            
        // Проверка и типы
        case NQuery::EExpressionType::is_null:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("{} IS NULL", BuildClause(operands[0]));
        case NQuery::EExpressionType::is_not_null:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("{} IS NOT NULL", BuildClause(operands[0]));
        case NQuery::EExpressionType::between:
            ASSERT(operands.size() == 3, "Invalid count of operands for {} operation, must: 3, actual: {}", type, operands.size());
            return Format("({} BETWEEN {} AND {})", 
                BuildClause(operands[0]),
                BuildClause(operands[1]),
                BuildClause(operands[2])
            );
        case NQuery::EExpressionType::in:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            {
                std::string result = Format("{} IN (", BuildClause(operands[0]));
                for (size_t i = 1; i < operands.size(); ++i) {
                    if (i > 1) {
                        result += ", ";
                    }
                    result += BuildClause(operands[i]);
                }
                result += ")";
                return result;
            }
            
        // Агрегатные функции
        case NQuery::EExpressionType::count:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("COUNT({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::sum:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("SUM({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::avg:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("AVG({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::min:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("MIN({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::max:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("MAX({})", BuildClause(operands[0]));
            
        // Строковые функции
        case NQuery::EExpressionType::concat:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            {
                std::string result = "CONCAT(";
                for (size_t i = 0; i < operands.size(); ++i) {
                    if (i > 0) {
                        result += ", ";
                    }
                    result += BuildClause(operands[i]);
                }
                result += ")";
                return result;
            }
        case NQuery::EExpressionType::substring:
            if (operands.size() == 3) {
                return Format("SUBSTRING({} FROM {} FOR {})", 
                    BuildClause(operands[0]),
                    BuildClause(operands[1]),
                    BuildClause(operands[2])
                );
            } else if (operands.size() == 2) {
                return Format("SUBSTRING({} FROM {})", 
                    BuildClause(operands[0]),
                    BuildClause(operands[1])
                );
            }
            THROW("Invalid count of operands for {} operation, must: 2 or 3, actual: {}", type, operands.size());
        case NQuery::EExpressionType::upper:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("UPPER({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::lower:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("LOWER({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::length:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("LENGTH({})", BuildClause(operands[0]));
            
        // Математические функции
        case NQuery::EExpressionType::abs:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("ABS({})", BuildClause(operands[0]));
        case NQuery::EExpressionType::round:
            if (operands.size() == 2) {
                return Format("ROUND({}, {})", 
                    BuildClause(operands[0]),
                    BuildClause(operands[1])
                );
            } else if (operands.size() == 1) {
                return Format("ROUND({})", BuildClause(operands[0]));
            }
            THROW("Invalid count of operands for {} operation, must: 1 or 2, actual: {}", type, operands.size());
            
        // Условные выражения
        case NQuery::EExpressionType::coalesce:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            {
                std::string result = "COALESCE(";
                for (size_t i = 0; i < operands.size(); ++i) {
                    if (i > 0) {
                        result += ", ";
                    }
                    result += BuildClause(operands[i]);
                }
                result += ")";
                return result;
            }
            
        // Подзапросы
        case NQuery::EExpressionType::exists:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("EXISTS ({})", BuildClause(operands[0]));

        default:
            THROW("Unknown expression type: {}", type);
    }
    
    return "";
}

std::string TPostgresBuilder::BuildAll(TAllPtr all) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::All);
    return "*";
}

std::string TPostgresBuilder::BuildColumn(TColumnPtr column) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Column);
    
    switch (column->GetColumnType()) {
        case NQuery::EExcluded:
            return Format("EXCLUDED.{}", FieldToString(column->GetFieldPath(), column->GetKeyType()));
        default:
            if (column->GetTablePath().empty()) {
                return Format("{}", FieldToString(column->GetFieldPath(), column->GetKeyType()));
            } else {
                return Format("t_{onlydelim,delimiter='_'}.{}", column->GetTablePath(), FieldToString(column->GetFieldPath(), column->GetKeyType()));
            }
    }
}

std::string TPostgresBuilder::ColumnDefinition(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    if (!field) {
        return "";
    }
    
    std::ostringstream oss;
    oss << Format("{} {}", FieldToString(field->GetPath().GetField(), EKeyType::Simple), GetPostgresType(field->GetTypeInfo()));
    
    // Добавляем NOT NULL, если поле обязательное
    if (field->IsRequired()) {
        oss << " NOT NULL";
    }
    
    // Добавляем DEFAULT для значения по умолчанию
    if (field->HasDefaultValue()) {
        const auto& typeInfo = field->GetTypeInfo();
        
        if (std::holds_alternative<TBoolFieldInfo>(typeInfo)) {
            bool defaultValue = std::get<TBoolFieldInfo>(typeInfo).defaultValue;
            oss << " DEFAULT " << (defaultValue ? "TRUE" : "FALSE");
        } else if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
            auto& info = std::get<TInt32FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TUInt32FieldInfo>(typeInfo)) {
            auto& info = std::get<TUInt32FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TInt64FieldInfo>(typeInfo)) {
            auto& info = std::get<TInt64FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TUInt64FieldInfo>(typeInfo)) {
            auto& info = std::get<TUInt64FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TFloatFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TFloatFieldInfo>(typeInfo).defaultValue;
        } else if (std::holds_alternative<TDoubleFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TDoubleFieldInfo>(typeInfo).defaultValue;
        } else if (std::holds_alternative<TStringFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << EscapeStringLiteral(std::get<TStringFieldInfo>(typeInfo).defaultValue);
        } else if (std::holds_alternative<TEnumFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TEnumFieldInfo>(typeInfo).defaultValue;
        }
    }
    
    // Добавляем PRIMARY KEY, если это первичный ключ
    if (field->IsPrimaryKey()) {
        oss << " PRIMARY KEY";
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildTable(TTablePtr table) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Table);
    return Format("t_{onlydelim,delimiter='_'}", table->GetPath().GetTable());
}

std::string TPostgresBuilder::BuildDefault(TDefaultPtr defaultVal) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Default);
    return "DEFAULT";
}

std::string TPostgresBuilder::BuildJoin(TJoinPtr join) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Join);
    
    std::ostringstream oss;
    
    switch (join->GetJoinType()) {
        case TJoin::EJoinType::Left:
            oss << "LEFT JOIN ";
            break;
        case TJoin::EJoinType::Inner:
            oss << "INNER JOIN ";
            break;
        case TJoin::EJoinType::ExclusiveLeft:
            oss << "LEFT OUTER JOIN ";
            break;
    }
    
    oss << Format("t_{onlydelim,delimiter='_'} ", join->GetTable().GetTable());
    
    if (join->GetCondition()) {
        oss << "ON " << BuildClause(join->GetCondition());
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildSelect(TSelectPtr select) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Select);
    
    std::ostringstream oss;
    oss << "SELECT ";
    
    // Формируем список столбцов
    const auto& selectors = select->GetSelectors();
    if (selectors.empty()) {
        oss << "*";
    } else {
        for (size_t i = 0; i < selectors.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << BuildClause(selectors[i]);
        }
    }
    
    // FROM
    const auto& from = select->GetFrom();
    if (!from.empty()) {
        oss << " FROM ";
        for (size_t i = 0; i < from.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << BuildClause(from[i]);
        }
    }
    
    // JOIN
    const auto& join = select->GetJoin();
    for (const auto& joinClause : join) {
        oss << " " << BuildClause(joinClause);
    }
    
    // WHERE
    auto where = select->GetWhere();
    if (where) {
        oss << " WHERE " << BuildClause(where);
    }
    
    // GROUP BY
    auto groupBy = select->GetGroupBy();
    if (groupBy) {
        oss << " GROUP BY " << BuildClause(groupBy);
    }
    
    // HAVING
    auto having = select->GetHaving();
    if (having) {
        oss << " HAVING " << BuildClause(having);
    }
    
    // ORDER BY
    auto orderBy = select->GetOrderBy();
    if (orderBy) {
        oss << " ORDER BY " << BuildClause(orderBy);
    }
    
    // LIMIT
    auto limit = select->GetLimit();
    if (limit) {
        oss << " LIMIT " << BuildClause(limit);
    }
    
    static std::unordered_set<NOrm::NRelation::Builder::EClauseType> withBraces = {
        NOrm::NRelation::Builder::EClauseType::Expression,
        NOrm::NRelation::Builder::EClauseType::Select,
        NOrm::NRelation::Builder::EClauseType::Update,
        NOrm::NRelation::Builder::EClauseType::Insert,
        NOrm::NRelation::Builder::EClauseType::Delete
    };
    
    if (Stack_.size() > 1 && withBraces.contains(Stack_.at(1))) {
        return Format("({})", oss.str());
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildInsert(TInsertPtr insert) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Insert);
    
    std::ostringstream oss;
    oss << Format("INSERT INTO t_{onlydelim,delimiter='_'} ", insert->GetTable().data());
    
    // Список колонок
    const auto& selectors = insert->GetSelectors();
    if (!selectors.empty()) {
        oss << "(";
        for (size_t i = 0; i < selectors.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << BuildClause(selectors[i]);
        }
        oss << ") ";
    }
    
    // Значения
    if (insert->GetIsValues()) {
        const auto& values = insert->GetValues();
        if (values.empty()) {
            oss << "DEFAULT VALUES";
        } else {
            oss << "VALUES ";
            for (size_t i = 0; i < values.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "(";
                
                const auto& row = values[i];
                for (size_t j = 0; j < row.size(); ++j) {
                    if (j > 0) oss << ", ";
                    oss << BuildClause(row[j]);
                }
                
                oss << ")";
            }
        }
    }
    
    // ON CONFLICT
    if (insert->GetIsDoUpdate()) {
        const auto& updates = insert->GetDoUpdate();
        oss << " ON CONFLICT DO UPDATE SET ";
        
        for (size_t i = 0; i < updates.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << BuildClause(updates[i].first) << " = " << BuildClause(updates[i].second);
        }
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildUpdate(TUpdatePtr update) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Update);
    
    std::ostringstream oss;
    oss << Format("UPDATE t_{onlydelim,delimiter='_'} SET ", update->GetTable().GetTable());
    
    // Список обновлений
    const auto& updates = update->GetUpdates();
    for (size_t i = 0; i < updates.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << BuildClause(updates[i].first) << " = " << BuildClause(updates[i].second);
    }
    
    // WHERE
    auto where = update->GetWhere();
    if (where) {
        oss << " WHERE " << BuildClause(where);
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildDelete(TDeletePtr deleteClause) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Delete);
    
    std::ostringstream oss;
    oss << Format("DELETE FROM t_{onlydelim,delimiter='_'}", deleteClause->GetTable().GetTable());
    
    // WHERE
    auto where = deleteClause->GetWhere();
    if (where) {
        oss << " WHERE " << BuildClause(where);
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildTruncate(TTruncatePtr truncate) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Truncate);
    return Format("TRUNCATE TABLE t_{onlydelim,delimiter='_'}", truncate->GetPath().GetTable());
}

std::string TPostgresBuilder::BuildStartTransaction(TStartTransactionPtr startTransaction) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::StartTransaction);
    
    std::ostringstream oss;
    oss << "BEGIN";
    
    if (startTransaction->GetReadOnly()) {
        oss << " READ ONLY";
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildCommitTransaction(TCommitTransactionPtr commitTransaction) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CommitTransaction);
    return "COMMIT";
}

std::string TPostgresBuilder::BuildRollbackTransaction(TRollbackTransactionPtr rollbackTransaction) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::RollbackTransaction);
    return "ROLLBACK";
}

std::string TPostgresBuilder::BuildColumnDefinition(TColumnDefinitionPtr columnDefinition) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CreateColumn);
    
    std::ostringstream oss;
    oss << Format("{} {}", FieldToString(columnDefinition->GetFieldPath(), columnDefinition->GetKeyType()), GetPostgresType(columnDefinition->GetTypeInfo()));
    
    // Добавляем NOT NULL, если поле обязательное
    if (columnDefinition->IsRequired()) {
        oss << " NOT NULL";
    }
    
    // Добавляем DEFAULT для значения по умолчанию
    if (columnDefinition->HasDefault()) {
        const auto& typeInfo = columnDefinition->GetTypeInfo();
        
        if (std::holds_alternative<TBoolFieldInfo>(typeInfo)) {
            bool defaultValue = std::get<TBoolFieldInfo>(typeInfo).defaultValue;
            oss << " DEFAULT " << (defaultValue ? "TRUE" : "FALSE");
        } else if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
            auto& info = std::get<TInt32FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TUInt32FieldInfo>(typeInfo)) {
            auto& info = std::get<TUInt32FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TInt64FieldInfo>(typeInfo)) {
            auto& info = std::get<TInt64FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TUInt64FieldInfo>(typeInfo)) {
            auto& info = std::get<TUInt64FieldInfo>(typeInfo);
            if (!info.increment) {
                oss << " DEFAULT " << info.defaultValue;
            }
        } else if (std::holds_alternative<TFloatFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TFloatFieldInfo>(typeInfo).defaultValue;
        } else if (std::holds_alternative<TDoubleFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TDoubleFieldInfo>(typeInfo).defaultValue;
        } else if (std::holds_alternative<TStringFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << EscapeStringLiteral(std::get<TStringFieldInfo>(typeInfo).defaultValue);
        } else if (std::holds_alternative<TEnumFieldInfo>(typeInfo)) {
            oss << " DEFAULT " << std::get<TEnumFieldInfo>(typeInfo).defaultValue;
        }
    }
    
    // Добавляем PRIMARY KEY, если это первичный ключ
    if (columnDefinition->IsPrimaryKey()) {
        oss << " PRIMARY KEY";
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildCreateTable(TCreateTablePtr createTable) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CreateTable);
    
    auto message = createTable->GetMessage();
    if (!message) {
        return "";
    }
    
    auto table = TRelationManager::GetInstance().GetParentTable(message->GetPath());
    
    std::ostringstream oss;
    oss << Format("CREATE TABLE t_{onlydelim,delimiter='_'} (", table->GetPath().GetTable());
    
    // Список колонок
    bool first = true;
    auto& relationManager = TRelationManager::GetInstance();
    
    for (const auto& fieldIdx : table->GetRelatedFields()) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        
        auto field = relationManager.GetPrimitiveField(fieldIdx);
        if (field) {
            oss << ColumnDefinition(field);
        }
    }
    
    oss << ")";
    
    return oss.str();
}

std::string TPostgresBuilder::BuildDropTable(TDropTablePtr dropTable) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DropTable);
    
    auto message = dropTable->GetMessage();
    if (!message) {
        return "";
    }
    
    return Format("DROP TABLE t_{onlydelim,delimiter='_'}", message->GetPath().GetTable());
}

std::string TPostgresBuilder::BuildAlterTable(TAlterTablePtr alterTable) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::AlterTable);
    
    std::ostringstream oss;
    oss << "ALTER TABLE ";
    
    // Предполагаем, что первая операция содержит информацию о таблице
    const auto& operations = alterTable->GetOperations();
    if (!operations.empty()) {
        auto firstOp = operations[0];
        if (auto addOp = std::dynamic_pointer_cast<TAddColumn>(firstOp)) {
            oss << Format("t_{onlydelim,delimiter='_'}", addOp->GetField()->GetPath().GetTable());
        } else if (auto dropOp = std::dynamic_pointer_cast<TDropColumn>(firstOp)) {
            oss << Format("t_{onlydelim,delimiter='_'}", dropOp->GetField()->GetPath().GetTable());
        } else if (auto alterOp = std::dynamic_pointer_cast<TAlterColumn>(firstOp)) {
            oss << Format("t_{onlydelim,delimiter='_'}", alterOp->GetColumn()->GetTablePath());
        }
    }
    
    for (size_t i = 0; i < operations.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << BuildClause(operations[i]);
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildAddColumn(TAddColumnPtr addColumn) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CreateColumn);
    auto field = addColumn->GetField();
    if (!field) {
        return "";
    }
    
    return "ADD COLUMN " + ColumnDefinition(field);
}

std::string TPostgresBuilder::BuildDropColumn(TDropColumnPtr dropColumn) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DropColumn);
    auto field = dropColumn->GetField();
    if (!field) {
        return "";
    }
    
    return Format("DROP COLUMN {}", FieldToString(field->GetPath().GetField(), EKeyType::Simple));
}

std::string TPostgresBuilder::BuildAlterColumn(TAlterColumnPtr alterColumn) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::AlterColumn);
    
    // Изменение типа
    switch (alterColumn->GetAlterType()) {
        case TAlterColumn::kSetType:
            return Format("ALTER COLUMN {} TYPE {}",
                FieldToString(alterColumn->GetColumn()->GetFieldPath(), EKeyType::Simple),
                GetPostgresType(*alterColumn->GetValueType()));
        case TAlterColumn::kSetDefault:
            return Format("ALTER COLUMN {} SET DEFAULT {}",
                FieldToString(alterColumn->GetColumn()->GetFieldPath(), EKeyType::Simple),
                GetPostgresDefault(*alterColumn->GetValueType()));
        case TAlterColumn::kDropDefault:
            return Format("ALTER COLUMN {} DROP NOT NULL", FieldToString(alterColumn->GetColumn()->GetFieldPath(), EKeyType::Simple));
        case TAlterColumn::kSetRequired:
            return Format("ALTER COLUMN {} SET NOT NULL", FieldToString(alterColumn->GetColumn()->GetFieldPath(), EKeyType::Simple));
        case TAlterColumn::kDropRequired:
            return Format("ALTER COLUMN {} DROP NOT NULL", FieldToString(alterColumn->GetColumn()->GetFieldPath(), EKeyType::Simple));
        default:
            THROW("Uknonw type of alteration");
    }
}

std::string TPostgresBuilder::JoinQueries(const std::vector<std::string>& queries) {
    std::ostringstream oss;
    
    for (size_t i = 0; i < queries.size(); ++i) {
        if (!queries[i].empty()) {
            if (i > 0) oss << "; ";
            oss << queries[i];
        }
    }
    
    return oss.str();
}

} // namespace NOrm::NRelation::Builder

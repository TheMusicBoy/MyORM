#include <relation/builders/postgres.h>
#include <relation/message.h>
#include <relation/field.h>
#include <common/format.h>
#include <sstream>

namespace NOrm::NRelation::Builder {

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

std::vector<std::string> TPostgresBuilder::BuildVector(const std::vector<TClausePtr>& clauses) {
    std::vector<std::string> result;
    for (const auto& claus : clauses) {
        result.emplace_back(claus->Build(shared_from_this()));
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

std::string TPostgresBuilder::ColumnDefinition(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    std::ostringstream oss;
    oss << field->GetId() << " " << GetPostgresType(field->GetTypeInfo());
    
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

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для базовых типов данных

std::string TPostgresBuilder::BuildString(const std::string& value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::String);
    return EscapeStringLiteral(value);
}

std::string TPostgresBuilder::BuildInt(int32_t value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Int);
    return std::to_string(value);
}

std::string TPostgresBuilder::BuildFloat(double value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Float);
    return std::to_string(value);
}

std::string TPostgresBuilder::BuildBool(bool value) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Bool);
    return value ? "TRUE" : "FALSE";
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для выражений и колонок

std::string TPostgresBuilder::BuildExpression(
    NQuery::EExpressionType type,
    const std::vector<TClausePtr>& operands) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Expression);
    
    switch (type) {
        // Арифметические выражения
        case NQuery::EExpressionType::add:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} + {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::subtract:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} - {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::multiply:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} * {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::divide:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} / {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::modulo:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} % {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::exponent:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("POWER({}, {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
            
        // Сравнения
        case NQuery::EExpressionType::equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} = {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::not_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} <> {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::greater_than:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} > {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::less_than:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} < {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::greater_than_or_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} >= {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::less_than_or_equals:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} <= {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
            
        // Логические выражения
        case NQuery::EExpressionType::and_:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} AND {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::or_:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} OR {})",
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::not_:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("NOT {}", operands[0]->Build(shared_from_this()));
            
        // Строковые выражения
        case NQuery::EExpressionType::like:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} LIKE {}}", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::ilike:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} ILIKE {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::similar_to:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} SIMILAR TO {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::regexp_match:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("({} ~ {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
            
        // Проверка и типы
        case NQuery::EExpressionType::is_null:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("{} IS NULL", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::is_not_null:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("{} IS NOT NULL", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::between:
            ASSERT(operands.size() == 3, "Invalid count of operands for {} operation, must: 3, actual: {}", type, operands.size());
            return Format("({} BETWEEN {} AND {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this()),
                operands[2]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::in:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            {
                std::string result = Format("{} IN (", operands[0]->Build(shared_from_this()));
                for (size_t i = 1; i < operands.size(); ++i) {
                    if (i > 1) {
                        result += ", ";
                    }
                    result += operands[i]->Build(shared_from_this());
                }
                result += ")";
                return result;
            }
        case NQuery::EExpressionType::not_in:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            {
                std::string result = Format("{} NOT IN (", operands[0]->Build(shared_from_this()));
                for (size_t i = 1; i < operands.size(); ++i) {
                    if (i > 1) {
                        result += ", ";
                    }
                    result += operands[i]->Build(shared_from_this());
                }
                result += ")";
                return result;
            }
        case NQuery::EExpressionType::exists:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("EXISTS ({})", operands[0]->Build(shared_from_this()));
            
        // Агрегатные функции
        case NQuery::EExpressionType::count:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            if (operands.size() > 1 && operands[1]->Type() == EClauseType::All) {
                return "COUNT(*)";
            } else {
                return Format("COUNT({})", operands[0]->Build(shared_from_this()));
            }
        case NQuery::EExpressionType::sum:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("SUM({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::avg:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("AVG({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::min:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("MIN({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::max:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("MAX({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::array_agg:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("ARRAY_AGG({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::string_agg:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            if (operands.size() > 1) {
                return Format("STRING_AGG({}, {})", 
                    operands[0]->Build(shared_from_this()),
                    operands[1]->Build(shared_from_this())
                );
            } else {
                return Format("STRING_AGG({}, ',')", operands[0]->Build(shared_from_this()));
            }

        // Числовые функции
        case NQuery::EExpressionType::abs:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("ABS({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::round:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            if (operands.size() > 1) {
                return Format("ROUND({}, {})", 
                    operands[0]->Build(shared_from_this()),
                    operands[1]->Build(shared_from_this())
                );
            } else {
                return Format("ROUND({})", operands[0]->Build(shared_from_this()));
            }
        case NQuery::EExpressionType::ceil:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("CEILING({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::floor:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("FLOOR({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::sqrt:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("SQRT({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::log:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            if (operands.size() > 1) {
                return Format("LOG({}, {})", 
                    operands[1]->Build(shared_from_this()),
                    operands[0]->Build(shared_from_this())
                );
            } else {
                return Format("LOG({})", operands[0]->Build(shared_from_this()));
            }
        case NQuery::EExpressionType::random:
            return "RANDOM()";
        case NQuery::EExpressionType::sin:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("SIN({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::cos:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("COS({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::tan:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("TAN({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::power:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("POWER({}, {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
            
        // Строковые функции
        case NQuery::EExpressionType::concat:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            return Format("{onlydelim,delimiter=' || '}", BuildVector(operands));
        case NQuery::EExpressionType::lower:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("LOWER({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::upper:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("UPPER({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::substring:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            if (operands.size() > 2) {
                return Format("SUBSTRING({} FROM {} FOR {})", 
                    operands[0]->Build(shared_from_this()),
                    operands[1]->Build(shared_from_this()),
                    operands[2]->Build(shared_from_this())
                );
            } else {
                return Format("SUBSTRING({} FROM {})", 
                    operands[0]->Build(shared_from_this()),
                    operands[1]->Build(shared_from_this())
                );
            }
        case NQuery::EExpressionType::length:
            ASSERT(operands.size() == 1, "Invalid count of operands for {} operation, must: 1, actual: {}", type, operands.size());
            return Format("LENGTH({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::replace:
            ASSERT(operands.size() == 3, "Invalid count of operands for {} operation, must: 3, actual: {}", type, operands.size());
            return Format("REPLACE({}, {}, {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this()),
                operands[2]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::trim:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            return Format("TRIM({})", operands[0]->Build(shared_from_this()));
        case NQuery::EExpressionType::left:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("LEFT({}, {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::right:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("RIGHT({}, {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::position:
            ASSERT(operands.size() == 2, "Invalid count of operands for {} operation, must: 2, actual: {}", type, operands.size());
            return Format("POSITION({} IN {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this())
            );
        case NQuery::EExpressionType::split_part:
            ASSERT(operands.size() == 3, "Invalid count of operands for {} operation, must: 3, actual: {}", type, operands.size());
            return Format("SPLIT_PART({}, {}, {})", 
                operands[0]->Build(shared_from_this()),
                operands[1]->Build(shared_from_this()),
                operands[2]->Build(shared_from_this())
            );
            
        // Условные функции
        case NQuery::EExpressionType::case_:
            ASSERT(operands.size() >= 2, "Invalid count of operands for {} operation, must be >= 2, actual: {}", type, operands.size());
            {
                std::string result = "CASE ";
                
                // Если первый операнд - выражение для сравнения
                if (operands.size() % 2 == 1) {
                    result += operands[0]->Build(shared_from_this()) + " ";
                    
                    // WHEN/THEN пары
                    for (size_t i = 1; i < operands.size() - 1; i += 2) {
                        result += "WHEN " + operands[i]->Build(shared_from_this()) + 
                                " THEN " + operands[i+1]->Build(shared_from_this()) + " ";
                    }
                    
                    // ELSE (если есть)
                    if (operands.size() % 2 == 0) {
                        result += "ELSE " + operands[operands.size() - 1]->Build(shared_from_this()) + " ";
                    }
                } else {
                    // WHEN/THEN пары без выражения для сравнения
                    for (size_t i = 0; i < operands.size() - 1; i += 2) {
                        result += "WHEN " + operands[i]->Build(shared_from_this()) + 
                                " THEN " + operands[i+1]->Build(shared_from_this()) + " ";
                    }
                    
                    // ELSE (если есть)
                    if (operands.size() % 2 == 1) {
                        result += "ELSE " + operands[operands.size() - 1]->Build(shared_from_this()) + " ";
                    }
                }
                
                result += "END";
                return result;
            }
        case NQuery::EExpressionType::coalesce:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            return Format("COALESCE({onlydelim})", BuildVector(operands));
        case NQuery::EExpressionType::greatest:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            return Format("GREATEST({onlydelim})", BuildVector(operands));
        case NQuery::EExpressionType::least:
            ASSERT(operands.size() >= 1, "Invalid count of operands for {} operation, must be >= 1, actual: {}", type, operands.size());
            return Format("LEAST({onlydelim})", BuildVector(operands));
            
        default:
            THROW("Uknown expression type: {}", type);
            break;
    }
    
    return "";
}

std::string TPostgresBuilder::BuildAll() {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::All);
    return "*";
}

std::string TPostgresBuilder::BuildColumn(
    const TMessagePath& path,
    NQuery::EColumnType type) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Column);
    
    std::ostringstream oss;

    switch (type) {
        case NQuery::EExcluded:
            return Format("EXCLUDED.{full_field_id}", path);
        default:
            return Format("{full_field_id}", path);
    }
}

std::string TPostgresBuilder::BuildTable(
    const TMessagePath& path) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Table);
    return Format("{table_id}", path);
}

std::string TPostgresBuilder::BuildDefault() {
    return "DEFAULT";
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для запросов SELECT

std::string TPostgresBuilder::BuildJoin(TMessagePath table, TClausePtr condition, TJoin::EJoinType type) {
    std::ostringstream oss;

    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Join);
    
    switch (type) {
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
    
    oss << Format("{table_id} ", table);
    
    if (condition) {
        oss << "ON " << condition->Build(shared_from_this());
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildSelect(
    const std::vector<TClausePtr>& selectors, 
    const std::vector<TClausePtr>& from,
    const std::vector<TClausePtr>& join,
    TClausePtr where, 
    TClausePtr groupBy, 
    TClausePtr having, 
    TClausePtr orderBy, 
    TClausePtr limit) {

    static std::unordered_set<NOrm::NRelation::Builder::EClauseType> withBraces = {
        NOrm::NRelation::Builder::EClauseType::Expression,
        NOrm::NRelation::Builder::EClauseType::DoUpdate,
        NOrm::NRelation::Builder::EClauseType::Select,
        NOrm::NRelation::Builder::EClauseType::Update,
        NOrm::NRelation::Builder::EClauseType::Insert,
        NOrm::NRelation::Builder::EClauseType::Delete
    };

    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Select);

    std::ostringstream oss;
    oss << "SELECT ";
    
    // Формируем список столбцов
    if (selectors.empty()) {
        oss << "*";
    } else {
        for (size_t i = 0; i < selectors.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << selectors[i]->Build(shared_from_this());
        }
    }
    
    // FROM
    if (!from.empty()) {
        oss << " FROM ";
        for (size_t i = 0; i < from.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << from[i]->Build(shared_from_this());
        }
    }
    
    // JOIN
    for (const auto& joinClause : join) {
        oss << " " << joinClause->Build(shared_from_this());
    }
    
    // WHERE
    if (where) {
        oss << " WHERE " << where->Build(shared_from_this());
    }
    
    // GROUP BY
    if (groupBy) {
        oss << " GROUP BY " << groupBy->Build(shared_from_this());
    }
    
    // HAVING
    if (having) {
        oss << " HAVING " << having->Build(shared_from_this());
    }
    
    // ORDER BY
    if (orderBy) {
        oss << " ORDER BY " << orderBy->Build(shared_from_this());
    }
    
    // LIMIT
    if (limit) {
        oss << " LIMIT " << limit->Build(shared_from_this());
    }
    
    if (Stack_.size() > 1 && withBraces.contains(Stack_.at(1))) {
        return Format("({})", oss.str());
    }
    
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для запросов INSERT

std::string TPostgresBuilder::BuildDefaultValueList() {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DefaultValueList);
    return "DEFAULT VALUES";
}

std::string TPostgresBuilder::BuildRowValues(const std::vector<std::vector<TClausePtr>>& rows) {
    std::ostringstream oss;
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::ValueRows);
    oss << "VALUES ";
    
    for (size_t i = 0; i < rows.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << "(";
        
        const auto& row = rows[i];
        for (size_t j = 0; j < row.size(); ++j) {
            if (j > 0) oss << ", ";
            oss << row[j]->Build(shared_from_this());
        }
        
        oss << ")";
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildDoNothing() {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DoNothing);
    return "ON CONFLICT DO NOTHING";
}

std::string TPostgresBuilder::BuildDoUpdate(const std::vector<std::pair<TClausePtr, TClausePtr>>& updates) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DoUpdate);
    std::ostringstream oss;
    oss << "ON CONFLICT DO UPDATE SET ";
    
    for (size_t i = 0; i < updates.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << updates[i].first->Build(shared_from_this()) << " = " << updates[i].second->Build(shared_from_this());
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildInsert(
    const TMessagePath& table,
    const std::vector<TClausePtr>& selectors,
    bool isValues,
    const std::vector<std::vector<TClausePtr>>& values,
    bool isDoUpdate,
    const std::vector<std::pair<TClausePtr, TClausePtr>>& doUpdate) {
    
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Insert);
    std::ostringstream oss;

    oss << Format("INSERT INTO {table_id} ", table);
    
    // Список колонок
    if (!selectors.empty()) {
        oss << Format("({onlydelim}) ", BuildVector(selectors));
    }
    
    // Значения
    if (isValues) {
        if (values.empty()) {
            oss << BuildDefaultValueList();
        } else {
            oss << BuildRowValues(values);
        }
    }
    
    // ON CONFLICT
    if (isDoUpdate) {
        oss << " " << BuildDoUpdate(doUpdate);
    }
    
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для запросов UPDATE

std::string TPostgresBuilder::BuildUpdate(
    const TMessagePath& table,
    const std::vector<std::pair<TClausePtr, TClausePtr>>& updates, 
    TClausePtr where) {

    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Update);
    
    std::ostringstream oss;
    oss << Format("UPDATE {table_id} SET ", table);
    
    // Список обновлений
    for (size_t i = 0; i < updates.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << updates[i].first->Build(shared_from_this()) << " = " << updates[i].second->Build(shared_from_this());
    }
    
    // WHERE
    if (where) {
        oss << " WHERE " << where->Build(shared_from_this());
    }
    
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для запросов DELETE

std::string TPostgresBuilder::BuildDelete(const TMessagePath& table, TClausePtr where) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::Delete);
    std::ostringstream oss;
    oss << Format("DELETE FROM {table_id}", table);
    
    // WHERE
    if (where) {
        oss << " WHERE " << where->Build(shared_from_this());
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildTruncate(TMessagePath path) {
    return Format("TRUNCATE TABLE t_{}", path.number());
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для транзакций

std::string TPostgresBuilder::BuildStartTransaction(bool readOnly) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::StartTransaction);
    std::ostringstream oss;
    oss << "BEGIN";
    
    if (readOnly) {
        oss << " READ ONLY";
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildCommitTransaction() {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CommitTransaction);
    return "COMMIT";
}

std::string TPostgresBuilder::BuildRollbackTransaction() {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::RollbackTransaction);
    return "ROLLBACK";
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для операций с таблицами

std::string TPostgresBuilder::BuildCreateTable(NOrm::NRelation::TMessageInfoPtr message) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CreateTable);
    std::ostringstream oss;
    oss << "CREATE TABLE " << message->GetId() << " (";
    
    // Список колонок
    bool first = true;
    
    for (const auto& field : message->PrimitiveFields()) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        
        oss << ColumnDefinition(field);
    }
    
    oss << ")";
    
    return oss.str();
}

std::string TPostgresBuilder::BuildDropTable(NOrm::NRelation::TMessageInfoPtr message) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DropTable);
    return "DROP TABLE " + message->GetId();
}

////////////////////////////////////////////////////////////////////////////////
// Реализация методов для операций с колонками

std::string TPostgresBuilder::BuildAddColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::CreateColumn);
    return "ADD COLUMN " + field->GetId();
}

std::string TPostgresBuilder::BuildDropColumn(NOrm::NRelation::TPrimitiveFieldInfoPtr field) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::DropColumn);
    return "DROP COLUMN " + field->GetId();
}

std::string TPostgresBuilder::BuildAlterColumn(
    NOrm::NRelation::TPrimitiveFieldInfoPtr newField,
    NOrm::NRelation::TPrimitiveFieldInfoPtr oldField) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::AlterColumn);
    
    std::ostringstream oss;
    
    // Изменение типа
    if (GetPostgresType(newField->GetTypeInfo()) != GetPostgresType(oldField->GetTypeInfo())) {
        oss << "ALTER COLUMN " << newField->GetId() 
            << " TYPE " << GetPostgresType(newField->GetTypeInfo());
    }
    
    // Изменение NULL/NOT NULL
    if (newField->IsRequired() != oldField->IsRequired()) {
        if (!oss.str().empty()) {
            oss << ", ";
        }
        if (newField->IsRequired()) {
            oss << "ALTER COLUMN " << newField->GetId() << " SET NOT NULL";
        } else {
            oss << "ALTER COLUMN " << newField->GetId() << " DROP NOT NULL";
        }
    }
    
    // Изменение DEFAULT
    if (newField->HasDefaultValue() != oldField->HasDefaultValue() || 
        (newField->HasDefaultValue() && newField->GetDefaultValueString() != oldField->GetDefaultValueString())) {
        if (!oss.str().empty()) {
            oss << ", ";
        }
        
        if (newField->HasDefaultValue()) {
            oss << "ALTER COLUMN " << newField->GetId() 
                << " SET DEFAULT " << newField->GetDefaultValueString();
        } else {
            oss << "ALTER COLUMN " << newField->GetId() << " DROP DEFAULT";
        }
    }
    
    return oss.str();
}

std::string TPostgresBuilder::BuildAlterTable(const std::vector<TClausePtr>& operations) {
    auto guard = Stack_.push(NOrm::NRelation::Builder::EClauseType::AlterTable);
    std::ostringstream oss;
    oss << "ALTER TABLE ";
    
    // Таблица определяется контекстом
    
    for (size_t i = 0; i < operations.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << operations[i]->Build(shared_from_this());
    }
    
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
// Объединение запросов

std::string TPostgresBuilder::JoinQueries(const std::vector<std::string>& queries) {
    std::ostringstream oss;
    
    for (size_t i = 0; i < queries.size(); ++i) {
        oss << queries[i] << "; ";
    }
    
    return oss.str();
}

} // namespace NOrm::NRelation::Builder

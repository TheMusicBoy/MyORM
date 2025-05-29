#include "postgresql.h"
#include <common/format.h>
#include <master/proto/object_base.pb.h>
#include <sstream>

namespace NQuery {

TPostgreSQLFormatter::TPostgreSQLFormatter() {}

int TPostgreSQLFormatter::GetTypeValue(const google::protobuf::Descriptor* descriptor) {
    const google::protobuf::MessageOptions& options = descriptor->options();
    if (options.HasExtension(orm::object_type)) {
        return options.GetExtension(orm::object_type).type_value();
    }
    return 0;
}

bool TPostgreSQLFormatter::IsInPlace(const google::protobuf::Descriptor* descriptor) {
    const google::protobuf::MessageOptions& options = descriptor->options();
    return options.HasExtension(orm::in_place) && options.GetExtension(orm::in_place);
}

bool TPostgreSQLFormatter::HasCustomTypeHandler(const google::protobuf::Descriptor* descriptor) {
    const google::protobuf::MessageOptions& options = descriptor->options();
    if (options.HasExtension(orm::object_type)) {
        return options.GetExtension(orm::object_type).custom_type_handler();
    }
    return false;
}

std::string TPostgreSQLFormatter::GetTableName(const google::protobuf::Descriptor* descriptor) {
    // Check cache first
    auto it = tableNameCache_.find(descriptor);
    if (it != tableNameCache_.end()) {
        return it->second;
    }

    int typeValue = GetTypeValue(descriptor);
    if (typeValue == 0) {
        LOG_ERROR("Type value not found for descriptor: {}", descriptor->full_name());
        return "unknown_table";
    }
    
    std::string tableName = Format("t_{}", typeValue);
    tableNameCache_[descriptor] = tableName;
    return tableName;
}

std::string TPostgreSQLFormatter::GetNestedTableName(int parentType, int fieldNumber, int nestedFieldNumber) {
    if (nestedFieldNumber >= 0) {
        return Format("t_{}_{}_{}",  parentType, fieldNumber, nestedFieldNumber);
    }
    return Format("t_{}_{}",  parentType, fieldNumber);
}

std::string TPostgreSQLFormatter::GetSQLType(const google::protobuf::FieldDescriptor* field) {
    using google::protobuf::FieldDescriptor;
    
    switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
            return "DOUBLE PRECISION";
        case FieldDescriptor::TYPE_FLOAT:
            return "REAL";
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SFIXED64:
            return "BIGINT";
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_SFIXED32:
        case FieldDescriptor::TYPE_ENUM:
            return "INTEGER";
        case FieldDescriptor::TYPE_BOOL:
            return "BOOLEAN";
        case FieldDescriptor::TYPE_STRING:
            return "TEXT";
        case FieldDescriptor::TYPE_BYTES:
            return "BYTEA";
        case FieldDescriptor::TYPE_MESSAGE:
            if (IsInPlace(field->message_type())) {
                return "JSONB";
            } else {
                return "UUID"; // Foreign key
            }
        default:
            return "TEXT";
    }
}

std::string TPostgreSQLFormatter::EscapeSQL(const std::string& value) {
    std::string result;
    result.reserve(value.size() * 2);
    
    for (char c : value) {
        if (c == '\'') {
            result.push_back('\'');
        }
        result.push_back(c);
    }
    
    return result;
}

std::string TPostgreSQLFormatter::GetFieldValue(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field) {
    using google::protobuf::FieldDescriptor;
    const google::protobuf::Reflection* refl = message.GetReflection();
    
    if (!field->is_repeated() && !refl->HasField(message, field)) {
        return "DEFAULT";
    }
    
    switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE:
        case FieldDescriptor::TYPE_FLOAT:
            return Format("{}", refl->GetDouble(message, field));
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SFIXED32:
        case FieldDescriptor::TYPE_SFIXED64:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SINT64:
            return Format("{}", refl->GetInt64(message, field));
        case FieldDescriptor::TYPE_ENUM:
            return Format("{}", refl->GetEnum(message, field)->number());
        case FieldDescriptor::TYPE_BOOL:
            return refl->GetBool(message, field) ? "TRUE" : "FALSE";
        case FieldDescriptor::TYPE_STRING:
            return Format("'{}'", EscapeSQL(refl->GetString(message, field)));
        case FieldDescriptor::TYPE_MESSAGE:
            if (IsInPlace(field->message_type())) {
                const google::protobuf::Message& subMsg = refl->GetMessage(message, field);
                // In a real implementation, this would properly serialize to JSON
                return Format("'{}'", EscapeSQL(subMsg.DebugString()));
            } else {
                // Handle foreign key relationship
                const google::protobuf::Message& subMsg = refl->GetMessage(message, field);
                auto metaField = subMsg.GetDescriptor()->FindFieldByName("meta");
                if (metaField) {
                    const google::protobuf::Message& meta = subMsg.GetReflection()->GetMessage(subMsg, metaField);
                    auto idField = meta.GetDescriptor()->FindFieldByName("id");
                    if (idField) {
                        return Format("'{}'", EscapeSQL(meta.GetReflection()->GetString(meta, idField)));
                    }
                }
                return "NULL";
            }
        default:
            return "NULL";
    }
}

// Implementation of the SQL generation methods would follow similar patterns to what's shown in the tests

// For example, GenerateCreateTable would create the appropriate table structure
std::string TPostgreSQLFormatter::GenerateCreateTable(const google::protobuf::Descriptor* descriptor) {
    std::string tableName = GetTableName(descriptor);
    std::ostringstream sql;
    
    sql << "CREATE TABLE IF NOT EXISTS " << tableName << " (\n";
    
    // Fields would be added here with proper types
    // Primary keys, foreign keys, etc. would be defined
    
    sql << "\n);";
    
    return sql.str();
}

// Other methods would be implemented similarly
// ...

} // namespace NQuery

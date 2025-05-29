#pragma once

#include <master/service/query/postgresql.h>
#include <memory>

namespace NORM {

class TObjectInfo {
    
};

// template <typename T>
// class TObjectORM {
// public:
//     TObjectORM() : formatter_(std::make_unique<NQuery::TPostgreSQLFormatter>()) {}
//     
//     // Generate schema creation SQL
//     std::string CreateTableSQL() {
//         return formatter_->GenerateCreateTable(T::descriptor());
//     }
//     
//     // Get a query to fetch objects
//     std::string SelectSQL(
//         const std::vector<std::string>& fields = {},
//         const std::string& whereClause = "",
//         const std::string& orderBy = "",
//         int limit = 0,
//         int offset = 0
//     ) {
//         return formatter_->GenerateSelect(T::descriptor(), fields, whereClause, orderBy, limit, offset);
//     }
//     
//     // Get a query to insert an object
//     std::string InsertSQL(const T& object) {
//         return formatter_->GenerateInsert(object);
//     }
//     
//     // Get a query to update an object
//     std::string UpdateSQL(const T& object, const std::string& whereClause = "") {
//         return formatter_->GenerateUpdate(object, whereClause);
//     }
//     
//     // Get a query to delete objects
//     std::string DeleteSQL(const std::string& whereClause) {
//         return formatter_->GenerateDelete(T::descriptor(), whereClause);
//     }
//     
// private:
//     std::unique_ptr<NQuery::TPostgreSQLFormatter> formatter_;
// };

} // namespace NORM

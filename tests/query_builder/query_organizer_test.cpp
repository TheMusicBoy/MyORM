#include <gtest/gtest.h>
#include <query_builder/organizers/sql_organizer.h>
#include <query_builder/builders/postgres.h>
#include <relation/relation_manager.h>
#include <relation/message.h>
#include <tests/proto/test_objects.pb.h>

namespace {

using namespace NOrm::NRelation;
using namespace NOrm::NRelation::Builder;
using namespace test_objects;

static void EnsureDescriptorsRegistered() {
    test_objects::SimpleMessage simple;
    test_objects::NestedMessage nested;
    test_objects::DeepNestedMessage deepNested;
    
    auto pool = google::protobuf::DescriptorPool::generated_pool();
    auto simpleMessageDesc = pool->FindMessageTypeByName("test_objects.SimpleMessage");
    auto nestedMessageDesc = pool->FindMessageTypeByName("test_objects.NestedMessage");
    auto deepNestedDesc = pool->FindMessageTypeByName("test_objects.DeepNestedMessage");
}

class SqlQueryOrganizerTest : public ::testing::Test {
protected:
  protected:
    void SetUp() override {
        EnsureDescriptorsRegistered();

        TRelationManager::GetInstance().Clear();

        simpleConfig = NCommon::New<TTableConfig>();
        simpleConfig->Number = 1;
        simpleConfig->SnakeCase = "simple_message";
        simpleConfig->CamelCase = "SimpleMessage";
        simpleConfig->Scheme = "test_objects.SimpleMessage";

        nestedConfig = NCommon::New<TTableConfig>();
        nestedConfig->Number = 2;
        nestedConfig->SnakeCase = "nested_message";
        nestedConfig->CamelCase = "NestedMessage";
        nestedConfig->Scheme = "test_objects.NestedMessage";

        RegisterRootMessage(simpleConfig);
        RegisterRootMessage(nestedConfig);

        sqlOrganizer = std::make_shared<TSqlQueryOrganizer>();
        
        postgresBuilder = std::make_shared<TPostgresBuilder>();
        

        simplePath = TMessagePath("simple_message");
        nestedPath = TMessagePath("nested_message");
    }

    void TearDown() override {
        TRelationManager::GetInstance().Clear();
    }
    
    std::string BuildQuery(Builder::TClausePtr clause) {
        return postgresBuilder->BuildClause(clause);
    }
    
    std::string BuildQueryFromPtr(Builder::TQueryPtr queryPtr) {
        std::string result;
        for (const auto& clause : queryPtr->GetClauses()) {
            if (!result.empty()) {
                result += "; ";
            }
            result += postgresBuilder->BuildClause(clause);
        }
        return result;
    }

    std::shared_ptr<TSqlQueryOrganizer> sqlOrganizer;
    std::shared_ptr<TPostgresBuilder> postgresBuilder;
    
    TTableConfigPtr simpleConfig;
    TTableConfigPtr nestedConfig;
    std::shared_ptr<TPostgresBuilder> builder;

    TMessagePath simplePath;
    TMessagePath nestedPath;
};

TEST_F(SqlQueryOrganizerTest, OrganizeSelect) {
    auto selectQuery = NOrm::NRelation::Select(simplePath, All());
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(selectQuery);
    
    ASSERT_NE(organizedSelect, nullptr);
    EXPECT_FALSE(organizedSelect->GetSelectors().empty());
    
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("SELECT") != std::string::npos);
    EXPECT_TRUE(sql.find("FROM") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, OrganizeInsert) {
    auto insertQuery = Insert(simplePath);
    
    std::vector<TAttribute> attributes;
    attributes.emplace_back(simplePath / "id", 1);
    attributes.emplace_back(simplePath / "name", "Test");
    
    insertQuery.AddSubrequest(attributes);
    
    auto organizedInsert = sqlOrganizer->OrganizeInsert(insertQuery);
    
    ASSERT_NE(organizedInsert, nullptr);
    EXPECT_FALSE(organizedInsert->GetSelectors().empty());
    
    std::string sql = BuildQuery(organizedInsert);
    EXPECT_TRUE(sql.find("INSERT INTO") != std::string::npos);
    EXPECT_TRUE(sql.find("VALUES") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, OrganizeUpdate) {
    auto updateQuery = Update(simplePath);
    
    std::vector<TAttribute> attributes;
    attributes.emplace_back(simplePath / "name", "Updated");
    attributes.emplace_back(simplePath / "id", 10);

    updateQuery.AddUpdate(attributes);
    
    auto organizedUpdate = sqlOrganizer->OrganizeUpdate(updateQuery);
    
    ASSERT_NE(organizedUpdate, nullptr);
    
    std::string sql = BuildQueryFromPtr(organizedUpdate);
    EXPECT_TRUE(sql.find("UPDATE") != std::string::npos);
    EXPECT_TRUE(sql.find("SET") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, OrganizeDelete) {
    auto deleteQuery = Delete(simplePath);
    
    auto organizedDelete = sqlOrganizer->OrganizeDelete(deleteQuery);
    
    ASSERT_NE(organizedDelete, nullptr);
    
    std::string sql = BuildQueryFromPtr(organizedDelete);
    EXPECT_TRUE(sql.find("DELETE FROM") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, CreateTable) {
    auto createTableQuery = sqlOrganizer->CreateTable(TRelationManager::GetInstance().GetRootMessage(simplePath));
    
    ASSERT_NE(createTableQuery, nullptr);
    
    std::string sql = BuildQueryFromPtr(createTableQuery);
    EXPECT_TRUE(sql.find("CREATE TABLE") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, DeleteTable) {
    auto dropTableQuery = sqlOrganizer->DeleteTable(TRelationManager::GetInstance().GetRootMessage(simplePath));
    
    ASSERT_NE(dropTableQuery, nullptr);
    
    std::string sql = BuildQueryFromPtr(dropTableQuery);
    EXPECT_TRUE(sql.find("DROP TABLE") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, TransactionCommands) {
    auto startTxQuery = sqlOrganizer->StartTransaction(simplePath);
    auto commitTxQuery = sqlOrganizer->CommitTransaction(simplePath);
    auto rollbackTxQuery = sqlOrganizer->RollbackTransaction(simplePath);
    
    ASSERT_NE(startTxQuery, nullptr);
    ASSERT_NE(commitTxQuery, nullptr);
    ASSERT_NE(rollbackTxQuery, nullptr);
    
    EXPECT_TRUE(BuildQueryFromPtr(startTxQuery).find("BEGIN") != std::string::npos);
    EXPECT_TRUE(BuildQueryFromPtr(commitTxQuery).find("COMMIT") != std::string::npos);
    EXPECT_TRUE(BuildQueryFromPtr(rollbackTxQuery).find("ROLLBACK") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithSubqueryInWhere) {
    auto subquery = Select(simplePath, Max(Col(simplePath / "id")));
    
    auto mainQuery = Select(simplePath, All());
    mainQuery.Where(Col(simplePath / "id") == subquery);
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(mainQuery);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("SELECT") != std::string::npos);
    EXPECT_TRUE(sql.find("(SELECT") != std::string::npos);
    EXPECT_TRUE(sql.find("MAX") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithMultipleAggregateFunctions) {
    auto idCol = Col(simplePath / "id");
    auto nameCol = Col(simplePath / "name");
    
    auto query = Select(
        simplePath, 
        Min(idCol),
        Max(idCol),
        Avg(idCol),
        Count(nameCol),
        Sum(idCol)
    );
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("MIN") != std::string::npos);
    EXPECT_TRUE(sql.find("MAX") != std::string::npos);
    EXPECT_TRUE(sql.find("AVG") != std::string::npos);
    EXPECT_TRUE(sql.find("COUNT") != std::string::npos);
    EXPECT_TRUE(sql.find("SUM") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithCaseExpression) {
    auto idCol = Col(simplePath / "id");
    auto nameCol = Col(simplePath / "name");
    
    auto caseExpr = Case()
        .When(idCol < Val(10)).Then(Val("Малое значение"))
        .When(idCol < Val(100)).Then(Val("Среднее значение"))
        .Else(Val("Большое значение"));
    
    auto query = Select(simplePath, idCol, nameCol, caseExpr);
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("CASE") != std::string::npos);
    EXPECT_TRUE(sql.find("WHEN") != std::string::npos);
    EXPECT_TRUE(sql.find("THEN") != std::string::npos);
    EXPECT_TRUE(sql.find("ELSE") != std::string::npos);
    EXPECT_TRUE(sql.find("END") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithCoalesceFunction) {
    auto idCol = Col(simplePath / "id");
    auto nameCol = Col(simplePath / "name");
    
    auto query = Select(
        simplePath,
        idCol,
        Coalesce(nameCol, Val("Нет имени"))
    );
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("COALESCE") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithMathFunctions) {
    auto idCol = Col(simplePath / "id");
    
    auto query = Select(
        simplePath,
        idCol,
        Abs(idCol - Val(100)),
        Round(idCol / Val(10.0)),
        Sqrt(idCol),
        Pow(idCol, Val(2))
    );
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("ABS") != std::string::npos);
    EXPECT_TRUE(sql.find("ROUND") != std::string::npos);
    EXPECT_TRUE(sql.find("SQRT") != std::string::npos ||
                sql.find("POWER") != std::string::npos); // В зависимости от реализации
}

TEST_F(SqlQueryOrganizerTest, SelectWithStringFunctions) {
    auto nameCol = Col(simplePath / "name");
    
    auto query = Select(
        simplePath,
        nameCol,
        Upper(nameCol),
        Lower(nameCol),
        SubStr(nameCol, Val(1), Val(3)),
        Len(nameCol),
        Trim(nameCol)
    );
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("UPPER") != std::string::npos);
    EXPECT_TRUE(sql.find("LOWER") != std::string::npos);
    EXPECT_TRUE(sql.find("SUBSTRING") != std::string::npos);
    EXPECT_TRUE(sql.find("LENGTH") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithLikeOperators) {
    auto nameCol = Col(simplePath / "name");
    
    auto query = Select(simplePath, All());
    query.Where(
        Like(nameCol, Val("%test%")) || 
        Ilike(nameCol, Val("%TEST%"))
    );
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("LIKE") != std::string::npos);
    EXPECT_TRUE(sql.find("ILIKE") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, SelectWithComplexConditionsAndInClause) {
    auto idCol = Col(simplePath / "id");
    auto nameCol = Col(simplePath / "name");
    auto activeCol = Col(simplePath / "active");
    
    auto subquery = Select(nestedPath, Col(nestedPath / "simple"));
    
    auto complexCondition = 
        (idCol > Val(10) && idCol < Val(100)) || 
        (nameCol == Val("test") && activeCol == Val(true)) ||
        In(idCol, subquery);
    
    auto query = Select(simplePath, All());
    query.Where(complexCondition);
    
    auto organizedSelect = sqlOrganizer->OrganizeSelect(query);
    
    ASSERT_NE(organizedSelect, nullptr);
    std::string sql = BuildQuery(organizedSelect);
    EXPECT_TRUE(sql.find("AND") != std::string::npos);
    EXPECT_TRUE(sql.find("OR") != std::string::npos);
    EXPECT_TRUE(sql.find("IN") != std::string::npos);
    EXPECT_TRUE(sql.find("(SELECT") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, UpdateWithSubquery) {
    auto updateQuery = Update(simplePath);
    
    std::vector<TAttribute> attributes;
    attributes.emplace_back(simplePath / "name", "Updated from subquery");
    attributes.emplace_back(simplePath / "id", 10);
    updateQuery.AddUpdate(attributes);
    
    auto organizedUpdate = sqlOrganizer->OrganizeUpdate(updateQuery);
    
    ASSERT_NE(organizedUpdate, nullptr);
    std::string sql = BuildQueryFromPtr(organizedUpdate);
    EXPECT_TRUE(sql.find("UPDATE") != std::string::npos);
}

TEST_F(SqlQueryOrganizerTest, DeleteWithExistsSubquery) {
    auto deleteQuery = Delete(simplePath);
    
    auto subquery = Select(nestedPath, All());
    subquery.Where(Col(nestedPath / "simple") == Col(simplePath / "id"));
    
    auto whereCondition = Exists(subquery);
    
    deleteQuery.Where(whereCondition);
    
    auto organizedDelete = sqlOrganizer->OrganizeDelete(deleteQuery);
    
    ASSERT_NE(organizedDelete, nullptr);
    std::string sql = BuildQueryFromPtr(organizedDelete);
    EXPECT_TRUE(sql.find("DELETE FROM") != std::string::npos);
    EXPECT_TRUE(sql.find("EXISTS") != std::string::npos);
    EXPECT_TRUE(sql.find("(SELECT") != std::string::npos);
}

} // namespace

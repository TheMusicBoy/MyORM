#include <google/protobuf/descriptor.h>
#include <gtest/gtest.h>
#include <query_builder/builders/postgres.h>
#include <relation/field.h>
#include <relation/message.h>
#include <relation/relation_manager.h>
#include <tests/proto/test_objects.pb.h>

namespace {

using namespace NOrm::NRelation;
using namespace NOrm::NRelation::Builder;
using namespace test_objects;

// Вспомогательная функция для регистрации дескрипторов
static void EnsureDescriptorsRegistered() {
    // Создаем временные объекты для уверенности, что дескрипторы зарегистрированы
    test_objects::SimpleMessage simple;
    test_objects::NestedMessage nested;
    test_objects::DeepNestedMessage deepNested;

    // Проверка доступности дескрипторов в пуле
    auto pool = google::protobuf::DescriptorPool::generated_pool();
    auto simpleDesc = pool->FindMessageTypeByName("test_objects.SimpleMessage");
    auto nestedDesc = pool->FindMessageTypeByName("test_objects.NestedMessage");
    auto deepNestedDesc = pool->FindMessageTypeByName("test_objects.DeepNestedMessage");
}

class PostgresQueryBuilderTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Регистрация дескрипторов в пуле
        EnsureDescriptorsRegistered();

        // Очистка менеджера отношений перед каждым тестом
        TRelationManager::GetInstance().Clear();

        // Создание конфигурации для SimpleMessage
        simpleConfig = NCommon::New<TTableConfig>();
        simpleConfig->Number = 1;
        simpleConfig->SnakeCase = "simple_message";
        simpleConfig->CamelCase = "SimpleMessage";
        simpleConfig->Scheme = "test_objects.SimpleMessage";

        // Создание конфигурации для NestedMessage
        nestedConfig = NCommon::New<TTableConfig>();
        nestedConfig->Number = 2;
        nestedConfig->SnakeCase = "nested_message";
        nestedConfig->CamelCase = "NestedMessage";
        nestedConfig->Scheme = "test_objects.NestedMessage";

        // Регистрация тестовых сообщений
        RegisterRootMessage(simpleConfig);
        RegisterRootMessage(nestedConfig);

        // Создание PostgreSQL билдера
        builder = std::make_shared<TPostgresBuilder>();

        // Общие пути для тестов
        simplePath = TMessagePath("simple_message");
        nestedPath = TMessagePath("nested_message");
    }

    void TearDown() override {
        // Очистка после каждого теста
        TRelationManager::GetInstance().Clear();
    }

    // Вспомогательный метод для получения поля по имени
    TPrimitiveFieldInfoPtr GetFieldByName(TMessageInfoPtr message, const std::string& name) {
        for (auto field : message->PrimitiveFields()) {
            if (field->GetName() == name) {
                return field;
            }
        }
        return nullptr;
    }

    // Вспомогательные методы для тестирования
    std::string BuildString(const std::string& val) {
        auto strPtr = std::make_shared<TString>(val);
        return builder->BuildClause(strPtr);
    }

    std::string BuildInt(int val) {
        auto intPtr = std::make_shared<TInt>(val);
        return builder->BuildClause(intPtr);
    }

    std::string BuildFloat(double val) {
        auto floatPtr = std::make_shared<TFloat>(val);
        return builder->BuildClause(floatPtr);
    }

    std::string BuildBool(bool val) {
        auto boolPtr = std::make_shared<TBool>(val);
        return builder->BuildClause(boolPtr);
    }

    std::string BuildCreateTable(NOrm::NRelation::TTableInfoPtr tableInfo) {
        auto createTable = std::make_shared<TCreateTable>();
        auto message = TRelationManager::GetInstance().GetMessage(tableInfo->GetPath());
        createTable->SetMessage(message);
        return builder->BuildClause(createTable);
    }

    std::string BuildDropTable(NOrm::NRelation::TTableInfoPtr tableInfo) {
        auto dropTable = std::make_shared<TDropTable>();
        auto message = TRelationManager::GetInstance().GetMessage(tableInfo->GetPath());
        dropTable->SetMessage(message);
        return builder->BuildClause(dropTable);
    }

    std::string BuildStartTransaction(bool readOnly) {
        auto startTx = std::make_shared<TStartTransaction>(readOnly);
        return builder->BuildClause(startTx);
    }

    std::string BuildCommitTransaction() {
        auto commitTx = std::make_shared<TCommitTransaction>();
        return builder->BuildClause(commitTx);
    }

    std::string BuildRollbackTransaction() {
        auto rollbackTx = std::make_shared<TRollbackTransaction>();
        return builder->BuildClause(rollbackTx);
    }

    std::string BuildAddColumn(TPrimitiveFieldInfoPtr field) {
        auto addColumn = std::make_shared<TAddColumn>(field);
        return builder->BuildClause(addColumn);
    }

    std::string BuildDropColumn(TPrimitiveFieldInfoPtr field) {
        auto dropColumn = std::make_shared<TDropColumn>(field);
        return builder->BuildClause(dropColumn);
    }

    std::string BuildAlterColumn(TPrimitiveFieldInfoPtr oldField, TPrimitiveFieldInfoPtr newField) {
        // Создаем колонку
        auto column = std::make_shared<TColumn>(oldField->GetPath().GetTable(), oldField->GetPath().GetField());
        column->SetKeyType(EKeyType::Simple);
        
        // Создаем AlterColumn для изменения типа
        auto typeAlterColumn = std::make_shared<TAlterColumn>(column);
        typeAlterColumn->SetType(&newField->GetTypeInfo());
        
        // Создаем AlterColumn для установки NOT NULL
        auto requiredAlterColumn = std::make_shared<TAlterColumn>(column);
        requiredAlterColumn->SetRequired();
        
        // Объединяем два запроса
        return builder->BuildClause(typeAlterColumn) + ", " + builder->BuildClause(requiredAlterColumn);
    }

    std::string JoinQueries(const std::vector<std::string>& queries) {
        std::ostringstream oss;
        
        for (size_t i = 0; i < queries.size(); ++i) {
            if (!queries[i].empty()) {
                if (i > 0) oss << "; ";
                oss << queries[i];
            }
        }
        
        if (!queries.empty()) {
            oss << "; ";
        }
        
        return oss.str();
    }

    std::string BuildTruncate(const TMessagePath& path) {
        auto truncate = std::make_shared<TTruncate>(path);
        return builder->BuildClause(truncate);
    }

    std::string BuildDefault() {
        auto defaultVal = std::make_shared<TDefault>();
        return builder->BuildClause(defaultVal);
    }

    std::string BuildAll() {
        auto all = std::make_shared<TAll>();
        return builder->BuildClause(all);
    }

    TTableConfigPtr simpleConfig;
    TTableConfigPtr nestedConfig;
    std::shared_ptr<TPostgresBuilder> builder;

    // Общие пути для тестов
    TMessagePath simplePath;
    TMessagePath nestedPath;
};

// Тест базовых типов данных
TEST_F(PostgresQueryBuilderTest, BasicDataTypes) {
    EXPECT_EQ(BuildString("test"), "'test'");
    EXPECT_EQ(BuildString("test's value"), "'test''s value'");
    EXPECT_EQ(BuildInt(123), "123");
    EXPECT_EQ(BuildFloat(123.45), "123.450000");
    EXPECT_EQ(BuildBool(true), "TRUE");
    EXPECT_EQ(BuildBool(false), "FALSE");
}

// Тест формирования запроса CREATE TABLE
TEST_F(PostgresQueryBuilderTest, CreateTableQuery) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Тестируем запрос CREATE TABLE
    auto tableInfo = TRelationManager::GetInstance().GetParentTable(message->GetPath());
    std::string createTableSQL = BuildCreateTable(tableInfo);
    EXPECT_TRUE(createTableSQL.find("CREATE TABLE t_1 (") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_1 INTEGER PRIMARY KEY") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_2 TEXT NOT NULL") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_3 BOOLEAN DEFAULT TRUE") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find(")") != std::string::npos);
}

TEST_F(PostgresQueryBuilderTest, CreateNestedTableQuery) {
    // Получаем NestedMessage
    auto message = TRelationManager::GetInstance().GetMessage(nestedPath);
    ASSERT_NE(message, nullptr);

    // Тестируем запрос CREATE TABLE
    auto tableInfo = TRelationManager::GetInstance().GetParentTable(message->GetPath());
    std::string createTableSQL = BuildCreateTable(tableInfo);
    EXPECT_TRUE(createTableSQL.find("CREATE TABLE t_2 (") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_1 INTEGER PRIMARY KEY") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_2_1 INTEGER PRIMARY KEY") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_2_2 TEXT NOT NULL") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find("f_2_3 BOOLEAN DEFAULT TRUE") != std::string::npos);
    EXPECT_TRUE(createTableSQL.find(")") != std::string::npos);
}

// Тест формирования запроса DROP TABLE
TEST_F(PostgresQueryBuilderTest, DropTableQuery) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Тестируем запрос DROP TABLE
    auto tableInfo = TRelationManager::GetInstance().GetParentTable(message->GetPath());
    std::string dropTableSQL = BuildDropTable(tableInfo);
    EXPECT_EQ(dropTableSQL, "DROP TABLE t_1");
}

// Тест команд транзакций
TEST_F(PostgresQueryBuilderTest, TransactionCommands) {
    // Тестируем команды транзакций
    std::string startTxSQL = BuildStartTransaction(false);
    EXPECT_EQ(startTxSQL, "BEGIN");

    std::string startReadOnlyTxSQL = BuildStartTransaction(true);
    EXPECT_EQ(startReadOnlyTxSQL, "BEGIN READ ONLY");

    std::string commitTxSQL = BuildCommitTransaction();
    EXPECT_EQ(commitTxSQL, "COMMIT");

    std::string rollbackTxSQL = BuildRollbackTransaction();
    EXPECT_EQ(rollbackTxSQL, "ROLLBACK");
}

// Тест операций с колонками
TEST_F(PostgresQueryBuilderTest, ColumnOperations) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Находим поле id
    auto idField = GetFieldByName(message, "id");
    ASSERT_NE(idField, nullptr);

    // Тестируем ADD COLUMN
    std::string addColumnSQL = BuildAddColumn(idField);
    EXPECT_EQ(addColumnSQL, "ADD COLUMN f_1 INTEGER PRIMARY KEY");

    // Тестируем DROP COLUMN
    std::string dropColumnSQL = BuildDropColumn(idField);
    EXPECT_EQ(dropColumnSQL, "DROP COLUMN f_1");
}

// Тест ALTER COLUMN
TEST_F(PostgresQueryBuilderTest, AlterColumnQuery) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Находим поля
    auto idField = GetFieldByName(message, "id");
    auto nameField = GetFieldByName(message, "name");
    ASSERT_NE(idField, nullptr);
    ASSERT_NE(nameField, nullptr);

    // Тестируем ALTER COLUMN
    std::string alterColumnSQL = BuildAlterColumn(idField, nameField);
    EXPECT_EQ(alterColumnSQL, "ALTER COLUMN f_1 TYPE TEXT, ALTER COLUMN f_1 SET NOT NULL");
}

// Тест объединения запросов
TEST_F(PostgresQueryBuilderTest, JoinQueries) {
    std::vector<std::string> queries = {"SELECT * FROM table1", "INSERT INTO table2 VALUES (1, 'test')", "UPDATE table3 SET column = 'value'"};

    std::string joinedSQL = JoinQueries(queries);
    EXPECT_EQ(joinedSQL, "SELECT * FROM table1; INSERT INTO table2 VALUES (1, 'test'); UPDATE table3 SET column = 'value'; ");
}

// Тест формирования запроса TRUNCATE
TEST_F(PostgresQueryBuilderTest, TruncateQuery) {
    // Тестируем запрос TRUNCATE
    std::string truncateSQL = BuildTruncate(simplePath);
    EXPECT_EQ(truncateSQL, "TRUNCATE TABLE t_1");
}

// Тест работы с DEFAULT
TEST_F(PostgresQueryBuilderTest, DefaultValue) {
    std::string defaultSQL = BuildDefault();
    EXPECT_EQ(defaultSQL, "DEFAULT");
}

// Тест для BuildAll
TEST_F(PostgresQueryBuilderTest, AllValues) {
    std::string allSQL = BuildAll();
    EXPECT_EQ(allSQL, "*");
}

// Тест для арифметических выражений
TEST_F(PostgresQueryBuilderTest, ArithmeticExpressions) {
    // Создаем операнды
    auto int1 = std::make_shared<TInt>(10);
    auto int2 = std::make_shared<TInt>(20);

    // Тестируем сложение
    auto addExpr = std::make_shared<TExpression>();
    addExpr->SetExpressionType(NOrm::NQuery::EExpressionType::add);
    addExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(addExpr), "(10 + 20)");

    // Тестируем вычитание
    auto subtractExpr = std::make_shared<TExpression>();
    subtractExpr->SetExpressionType(NOrm::NQuery::EExpressionType::subtract);
    subtractExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(subtractExpr), "(10 - 20)");

    // Тестируем умножение
    auto multiplyExpr = std::make_shared<TExpression>();
    multiplyExpr->SetExpressionType(NOrm::NQuery::EExpressionType::multiply);
    multiplyExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(multiplyExpr), "(10 * 20)");

    // Тестируем деление
    auto divideExpr = std::make_shared<TExpression>();
    divideExpr->SetExpressionType(NOrm::NQuery::EExpressionType::divide);
    divideExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(divideExpr), "(10 / 20)");
}

// Тест для операций сравнения
TEST_F(PostgresQueryBuilderTest, ComparisonExpressions) {
    // Создаем операнды
    auto int1 = std::make_shared<TInt>(10);
    auto int2 = std::make_shared<TInt>(20);

    // Тестируем равенство
    auto equalsExpr = std::make_shared<TExpression>();
    equalsExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    equalsExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(equalsExpr), "(10 = 20)");

    // Тестируем неравенство
    auto notEqualsExpr = std::make_shared<TExpression>();
    notEqualsExpr->SetExpressionType(NOrm::NQuery::EExpressionType::not_equals);
    notEqualsExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(notEqualsExpr), "(10 <> 20)");

    // Тестируем больше
    auto greaterThanExpr = std::make_shared<TExpression>();
    greaterThanExpr->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    greaterThanExpr->SetOperands({int1, int2});
    EXPECT_EQ(builder->BuildClause(greaterThanExpr), "(10 > 20)");
}

// Тест для логических выражений
TEST_F(PostgresQueryBuilderTest, LogicalExpressions) {
    // Создаем операнды
    auto bool1 = std::make_shared<TBool>(true);
    auto bool2 = std::make_shared<TBool>(false);

    // Тестируем AND
    auto andExpr = std::make_shared<TExpression>();
    andExpr->SetExpressionType(NOrm::NQuery::EExpressionType::and_);
    andExpr->SetOperands({bool1, bool2});
    EXPECT_EQ(builder->BuildClause(andExpr), "(TRUE AND FALSE)");

    // Тестируем OR
    auto orExpr = std::make_shared<TExpression>();
    orExpr->SetExpressionType(NOrm::NQuery::EExpressionType::or_);
    orExpr->SetOperands({bool1, bool2});
    EXPECT_EQ(builder->BuildClause(orExpr), "(TRUE OR FALSE)");

    // Тестируем NOT
    auto notExpr = std::make_shared<TExpression>();
    notExpr->SetExpressionType(NOrm::NQuery::EExpressionType::not_);
    notExpr->SetOperands({bool1});
    EXPECT_EQ(builder->BuildClause(notExpr), "NOT TRUE");
}

// Тест для проверок на NULL
TEST_F(PostgresQueryBuilderTest, NullCheckExpressions) {
    // Создаем операнд
    auto col = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    col->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    col->SetKeyType(EKeyType::Simple);

    // Тестируем IS NULL
    auto isNullExpr = std::make_shared<TExpression>();
    isNullExpr->SetExpressionType(NOrm::NQuery::EExpressionType::is_null);
    isNullExpr->SetOperands({col});
    EXPECT_EQ(builder->BuildClause(isNullExpr), "t_1.f_1 IS NULL");

    // Тестируем IS NOT NULL
    auto isNotNullExpr = std::make_shared<TExpression>();
    isNotNullExpr->SetExpressionType(NOrm::NQuery::EExpressionType::is_not_null);
    isNotNullExpr->SetOperands({col});
    EXPECT_EQ(builder->BuildClause(isNotNullExpr), "t_1.f_1 IS NOT NULL");
}

// Тест для сложного запроса SELECT с условиями и таблицей
TEST_F(PostgresQueryBuilderTest, ComplexSelectQuery) {
    // Создаем колонки для SELECT
    auto idCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    idCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    idCol->SetKeyType(EKeyType::Simple);

    auto nameCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{2});
    nameCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    nameCol->SetKeyType(EKeyType::Simple);

    // Создаем условие WHERE id > 10
    auto idValue = std::make_shared<TInt>(10);

    auto whereExpr = std::make_shared<TExpression>();
    whereExpr->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    whereExpr->SetOperands({idCol, idValue});

    // Создаем запрос SELECT
    auto selectQuery = std::make_shared<TSelect>();
    selectQuery->SetSelectors({idCol, nameCol});
    selectQuery->SetFrom({std::make_shared<TTable>(simplePath)});
    selectQuery->SetWhere(whereExpr);

    // Тестируем запрос SELECT
    std::string selectSQL = builder->BuildClause(selectQuery);
    EXPECT_EQ(selectSQL, "SELECT t_1.f_1, t_1.f_2 FROM t_1 WHERE (t_1.f_1 > 10)");
}

// Тест для Join операций
TEST_F(PostgresQueryBuilderTest, JoinOperations) {
    // Создаем колонки для условия соединения
    auto simpleIdCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    simpleIdCol->SetKeyType(EKeyType::Simple);

    auto nestedRefCol = std::make_shared<TColumn>(nestedPath.GetTable(), std::vector<uint32_t>{3});
    nestedRefCol->SetKeyType(EKeyType::Simple);

    // Создаем условие JOIN ON simple.id = nested.simple_ref
    auto joinCondition = std::make_shared<TExpression>();
    joinCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    joinCondition->SetOperands({simpleIdCol, nestedRefCol});

    // Тестируем различные типы JOIN
    auto leftJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Left);
    EXPECT_EQ(builder->BuildClause(leftJoin), "LEFT JOIN t_2 ON (t_1.f_1 = t_2.f_3)");

    auto innerJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Inner);
    EXPECT_EQ(builder->BuildClause(innerJoin), "INNER JOIN t_2 ON (t_1.f_1 = t_2.f_3)");

    auto exclusiveLeftJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::ExclusiveLeft);
    EXPECT_EQ(builder->BuildClause(exclusiveLeftJoin), "LEFT OUTER JOIN t_2 ON (t_1.f_1 = t_2.f_3)");
}

// Тест для сложного запроса SELECT с FROM и JOIN
TEST_F(PostgresQueryBuilderTest, SelectWithFromAndJoin) {
    // Создаем колонки для SELECT
    auto simpleIdCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    simpleIdCol->SetKeyType(EKeyType::Simple);

    auto simpleNameCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{2});
    simpleNameCol->SetKeyType(EKeyType::Simple);

    auto nestedValueCol = std::make_shared<TColumn>(nestedPath.GetTable(), std::vector<uint32_t>{3});
    nestedValueCol->SetKeyType(EKeyType::Simple);

    auto nestedRefCol = std::make_shared<TColumn>(nestedPath.GetTable(), std::vector<uint32_t>{3});
    nestedRefCol->SetKeyType(EKeyType::Simple);

    auto joinCondition = std::make_shared<TExpression>();
    joinCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    joinCondition->SetOperands({simpleIdCol, nestedRefCol});

    auto innerJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Inner);

    // Создаем FROM для основной таблицы
    auto simpleTable = std::make_shared<TTable>(simplePath);

    // Создаем WHERE условие
    auto whereCondition = std::make_shared<TExpression>();
    whereCondition->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    whereCondition->SetOperands({simpleIdCol, std::make_shared<TInt>(10)});

    // Создаем запрос SELECT
    auto selectQuery = std::make_shared<TSelect>();
    selectQuery->SetSelectors({simpleIdCol, simpleNameCol, nestedValueCol});
    selectQuery->SetFrom({simpleTable});
    selectQuery->SetJoin({innerJoin});
    selectQuery->SetWhere(whereCondition);

    // Тестируем запрос SELECT с FROM и JOIN
    std::string selectSQL = builder->BuildClause(selectQuery);
    EXPECT_EQ(selectSQL, "SELECT t_1.f_1, t_1.f_2, t_2.f_3 FROM t_1 INNER JOIN t_2 ON (t_1.f_1 = t_2.f_3) WHERE (t_1.f_1 > 10)");
}

// Тест для сложного запроса INSERT
TEST_F(PostgresQueryBuilderTest, ComplexInsertQuery) {
    // Создаем колонки для INSERT
    auto idCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    idCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    idCol->SetKeyType(EKeyType::Simple);

    auto nameCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{2});
    nameCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    nameCol->SetKeyType(EKeyType::Simple);

    // Создаем значения для INSERT
    auto idValue = std::make_shared<TInt>(1);
    auto nameValue = std::make_shared<TString>("Test");

    std::vector<std::vector<TClausePtr>> values = {{idValue, nameValue}};

    // Создаем запрос INSERT
    auto insertQuery = std::make_shared<TInsert>(simplePath);
    insertQuery->SetSelectors({idCol, nameCol});
    insertQuery->SetIsValues(true);
    insertQuery->SetValues(values);

    // Тестируем запрос INSERT
    std::string insertSQL = builder->BuildClause(insertQuery);
    EXPECT_EQ(insertSQL, "INSERT INTO t_1 (t_1.f_1, t_1.f_2) VALUES (1, 'Test')");
}

// Тест для агрегатных функций
TEST_F(PostgresQueryBuilderTest, AggregateFunctionExpressions) {
    // Создаем операнд
    auto col = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    col->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    col->SetKeyType(EKeyType::Simple);

    // Тестируем COUNT
    auto countExpr = std::make_shared<TExpression>();
    countExpr->SetExpressionType(NOrm::NQuery::EExpressionType::count);
    countExpr->SetOperands({col});
    EXPECT_EQ(builder->BuildClause(countExpr), "COUNT(t_1.f_1)");

    // Тестируем SUM
    auto sumExpr = std::make_shared<TExpression>();
    sumExpr->SetExpressionType(NOrm::NQuery::EExpressionType::sum);
    sumExpr->SetOperands({col});
    EXPECT_EQ(builder->BuildClause(sumExpr), "SUM(t_1.f_1)");

    // Тестируем AVG
    auto avgExpr = std::make_shared<TExpression>();
    avgExpr->SetExpressionType(NOrm::NQuery::EExpressionType::avg);
    avgExpr->SetOperands({col});
    EXPECT_EQ(builder->BuildClause(avgExpr), "AVG(t_1.f_1)");
}

// Тест для сложного запроса UPDATE, заменяющего максимальные значения на минимальные
TEST_F(PostgresQueryBuilderTest, UpdateMaxToMinValue) {
    // Создаем колонку для ID
    auto idCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    idCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    idCol->SetKeyType(EKeyType::Simple);

    // Создаем подзапрос MIN
    auto minExpr = std::make_shared<TExpression>();
    minExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minExpr->SetOperands({idCol});

    auto minSelect = std::make_shared<TSelect>();
    minSelect->SetSelectors({minExpr});

    // Создаем подзапрос MAX
    auto maxExpr = std::make_shared<TExpression>();
    maxExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    maxExpr->SetOperands({idCol});

    auto maxSelect = std::make_shared<TSelect>();
    maxSelect->SetSelectors({maxExpr});

    // Создаем условие WHERE id = (SELECT MAX(id))
    auto whereExpr = std::make_shared<TExpression>();
    whereExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereExpr->SetOperands({idCol, maxSelect});

    // Создаем запрос UPDATE
    auto updateQuery = std::make_shared<TUpdate>(simplePath);
    updateQuery->SetUpdates({{idCol, minSelect}});
    updateQuery->SetWhere(whereExpr);

    // Тестируем запрос UPDATE
    std::string updateSQL = builder->BuildClause(updateQuery);
    EXPECT_EQ(updateSQL, "UPDATE t_1 SET t_1.f_1 = (SELECT MIN(t_1.f_1)) WHERE (t_1.f_1 = (SELECT MAX(t_1.f_1)))");
}

// Тест для DELETE с таблицей
TEST_F(PostgresQueryBuilderTest, DeleteWithTableName) {
    // Создаем колонки для условия WHERE
    auto idCol = std::make_shared<TColumn>(simplePath.GetTable(), std::vector<uint32_t>{1});
    idCol->SetColumnType(NOrm::NQuery::EColumnType::ESingular);
    idCol->SetKeyType(EKeyType::Simple);

    // Создаем значение для условия
    auto idValue = std::make_shared<TInt>(1);

    // Создаем условие WHERE
    auto whereCondition = std::make_shared<TExpression>();
    whereCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereCondition->SetOperands({idCol, idValue});

    // Создаем запрос DELETE
    auto deleteQuery = std::make_shared<TDelete>(simplePath, whereCondition);

    // Тестируем запрос DELETE
    std::string deleteSQL = builder->BuildClause(deleteQuery);
    EXPECT_EQ(deleteSQL, "DELETE FROM t_1 WHERE (t_1.f_1 = 1)");
}

} // namespace

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

    TTableConfigPtr simpleConfig;
    TTableConfigPtr nestedConfig;
    std::shared_ptr<TPostgresBuilder> builder;

    // Общие пути для тестов
    TMessagePath simplePath;
    TMessagePath nestedPath;
};

// Тест базовых типов данных
TEST_F(PostgresQueryBuilderTest, BasicDataTypes) {
    EXPECT_EQ(builder->BuildString("test"), "'test'");
    EXPECT_EQ(builder->BuildString("test's value"), "'test''s value'");
    EXPECT_EQ(builder->BuildInt(123), "123");
    EXPECT_EQ(builder->BuildFloat(123.45), "123.450000");
    EXPECT_EQ(builder->BuildBool(true), "TRUE");
    EXPECT_EQ(builder->BuildBool(false), "FALSE");
}

// Тест формирования запроса CREATE TABLE
TEST_F(PostgresQueryBuilderTest, CreateTableQuery) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Тестируем запрос CREATE TABLE
    std::string createTableSQL = builder->BuildCreateTable(message);
    EXPECT_EQ("CREATE TABLE t_1 (f_1 INTEGER PRIMARY KEY, f_2 TEXT NOT NULL, f_3 BOOLEAN DEFAULT TRUE)", createTableSQL);
}

// Тест формирования запроса DROP TABLE
TEST_F(PostgresQueryBuilderTest, DropTableQuery) {
    // Получаем SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(simplePath);
    ASSERT_NE(message, nullptr);

    // Тестируем запрос DROP TABLE
    std::string dropTableSQL = builder->BuildDropTable(message);
    EXPECT_EQ("DROP TABLE t_1", dropTableSQL);
}

// Тест команд транзакций
TEST_F(PostgresQueryBuilderTest, TransactionCommands) {
    // Тестируем команды транзакций
    std::string startTxSQL = builder->BuildStartTransaction(false);
    EXPECT_EQ(startTxSQL, "BEGIN");

    std::string startReadOnlyTxSQL = builder->BuildStartTransaction(true);
    EXPECT_EQ(startReadOnlyTxSQL, "BEGIN READ ONLY");

    std::string commitTxSQL = builder->BuildCommitTransaction();
    EXPECT_EQ(commitTxSQL, "COMMIT");

    std::string rollbackTxSQL = builder->BuildRollbackTransaction();
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
    std::string addColumnSQL = builder->BuildAddColumn(idField);
    EXPECT_EQ("ADD COLUMN f_1", addColumnSQL);

    // Тестируем DROP COLUMN
    std::string dropColumnSQL = builder->BuildDropColumn(idField);
    EXPECT_EQ("DROP COLUMN f_1", dropColumnSQL);
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
    std::string alterColumnSQL = builder->BuildAlterColumn(idField, nameField);
    EXPECT_EQ("ALTER COLUMN f_1 TYPE INTEGER, ALTER COLUMN f_1 DROP NOT NULL", alterColumnSQL);
}

// Тест объединения запросов
TEST_F(PostgresQueryBuilderTest, JoinQueries) {
    std::vector<std::string> queries = {"SELECT * FROM table1", "INSERT INTO table2 VALUES (1, 'test')", "UPDATE table3 SET column = 'value'"};

    std::string joinedSQL = builder->JoinQueries(queries);
    EXPECT_EQ("SELECT * FROM table1; INSERT INTO table2 VALUES (1, 'test'); UPDATE table3 SET column = 'value'; ", joinedSQL);
}

// Тест формирования запроса TRUNCATE
TEST_F(PostgresQueryBuilderTest, TruncateQuery) {
    // Тестируем запрос TRUNCATE
    std::string truncateSQL = builder->BuildTruncate(simplePath);
    EXPECT_EQ("TRUNCATE TABLE t_1", truncateSQL);
}

// Тест работы с DEFAULT
TEST_F(PostgresQueryBuilderTest, DefaultValue) {
    std::string defaultSQL = builder->BuildDefault();
    EXPECT_EQ(defaultSQL, "DEFAULT");
}

// Тест для BuildAll
TEST_F(PostgresQueryBuilderTest, AllValues) {
    std::string allSQL = builder->BuildAll();
    EXPECT_EQ(allSQL, "*");
}

// Тест для арифметических выражений
TEST_F(PostgresQueryBuilderTest, ArithmeticExpressions) {
    // Создаем операнды
    auto int1 = std::make_shared<TInt>();
    int1->SetValue(10);

    auto int2 = std::make_shared<TInt>();
    int2->SetValue(20);

    // Тестируем сложение
    auto addExpr = std::make_shared<TExpression>();
    addExpr->SetExpressionType(NOrm::NQuery::EExpressionType::add);
    addExpr->SetOperands({int1, int2});
    EXPECT_EQ(addExpr->Build(builder), "(10 + 20)");

    // Тестируем вычитание
    auto subtractExpr = std::make_shared<TExpression>();
    subtractExpr->SetExpressionType(NOrm::NQuery::EExpressionType::subtract);
    subtractExpr->SetOperands({int1, int2});
    EXPECT_EQ(subtractExpr->Build(builder), "(10 - 20)");

    // Тестируем умножение
    auto multiplyExpr = std::make_shared<TExpression>();
    multiplyExpr->SetExpressionType(NOrm::NQuery::EExpressionType::multiply);
    multiplyExpr->SetOperands({int1, int2});
    EXPECT_EQ(multiplyExpr->Build(builder), "(10 * 20)");

    // Тестируем деление
    auto divideExpr = std::make_shared<TExpression>();
    divideExpr->SetExpressionType(NOrm::NQuery::EExpressionType::divide);
    divideExpr->SetOperands({int1, int2});
    EXPECT_EQ(divideExpr->Build(builder), "(10 / 20)");
}

// Тест для операций сравнения
TEST_F(PostgresQueryBuilderTest, ComparisonExpressions) {
    // Создаем операнды
    auto int1 = std::make_shared<TInt>();
    int1->SetValue(10);

    auto int2 = std::make_shared<TInt>();
    int2->SetValue(20);

    // Тестируем равенство
    auto equalsExpr = std::make_shared<TExpression>();
    equalsExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    equalsExpr->SetOperands({int1, int2});
    EXPECT_EQ(equalsExpr->Build(builder), "(10 = 20)");

    // Тестируем неравенство
    auto notEqualsExpr = std::make_shared<TExpression>();
    notEqualsExpr->SetExpressionType(NOrm::NQuery::EExpressionType::not_equals);
    notEqualsExpr->SetOperands({int1, int2});
    EXPECT_EQ(notEqualsExpr->Build(builder), "(10 <> 20)");

    // Тестируем больше
    auto greaterThanExpr = std::make_shared<TExpression>();
    greaterThanExpr->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    greaterThanExpr->SetOperands({int1, int2});
    EXPECT_EQ(greaterThanExpr->Build(builder), "(10 > 20)");
}

// Тест для логических выражений
TEST_F(PostgresQueryBuilderTest, LogicalExpressions) {
    // Создаем операнды
    auto bool1 = std::make_shared<TBool>();
    bool1->SetValue(true);

    auto bool2 = std::make_shared<TBool>();
    bool2->SetValue(false);

    // Тестируем AND
    auto andExpr = std::make_shared<TExpression>();
    andExpr->SetExpressionType(NOrm::NQuery::EExpressionType::and_);
    andExpr->SetOperands({bool1, bool2});
    EXPECT_EQ(andExpr->Build(builder), "(TRUE AND FALSE)");

    // Тестируем OR
    auto orExpr = std::make_shared<TExpression>();
    orExpr->SetExpressionType(NOrm::NQuery::EExpressionType::or_);
    orExpr->SetOperands({bool1, bool2});
    EXPECT_EQ(orExpr->Build(builder), "(TRUE OR FALSE)");

    // Тестируем NOT
    auto notExpr = std::make_shared<TExpression>();
    notExpr->SetExpressionType(NOrm::NQuery::EExpressionType::not_);
    notExpr->SetOperands({bool1});
    EXPECT_EQ(notExpr->Build(builder), "NOT TRUE");
}

// Тест для проверок на NULL
TEST_F(PostgresQueryBuilderTest, NullCheckExpressions) {
    // Создаем операнд
    auto col = std::make_shared<TColumn>();
    col->SetPath("simple_message/id");
    col->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Тестируем IS NULL
    auto isNullExpr = std::make_shared<TExpression>();
    isNullExpr->SetExpressionType(NOrm::NQuery::EExpressionType::is_null);
    isNullExpr->SetOperands({col});
    EXPECT_EQ(isNullExpr->Build(builder), "t_1.f_1 IS NULL");

    // Тестируем IS NOT NULL
    auto isNotNullExpr = std::make_shared<TExpression>();
    isNotNullExpr->SetExpressionType(NOrm::NQuery::EExpressionType::is_not_null);
    isNotNullExpr->SetOperands({col});
    EXPECT_EQ(isNotNullExpr->Build(builder), "t_1.f_1 IS NOT NULL");
}

// Тест для сложного запроса SELECT с условиями и таблицей
TEST_F(PostgresQueryBuilderTest, ComplexSelectQuery) {
    // Создаем колонки для SELECT
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    auto nameCol = std::make_shared<TColumn>();
    nameCol->SetPath("simple_message/name");
    nameCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем условие WHERE id > 10
    auto idValue = std::make_shared<TInt>();
    idValue->SetValue(10);

    auto whereExpr = std::make_shared<TExpression>();
    whereExpr->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    whereExpr->SetOperands({idCol, idValue});

    // Создаем запрос SELECT
    auto selectQuery = std::make_shared<TSelect>();
    selectQuery->SetSelectors({idCol, nameCol});
    selectQuery->SetFrom({std::make_shared<TTable>(simplePath)});
    selectQuery->SetWhere(whereExpr);

    // Тестируем запрос SELECT
    std::string selectSQL = selectQuery->Build(builder);
    EXPECT_EQ(selectSQL, "SELECT t_1.f_1, t_1.f_2 FROM t_1 WHERE (t_1.f_1 > 10)");
}

// Тест для Join операций
TEST_F(PostgresQueryBuilderTest, JoinOperations) {
    // Создаем колонки для условия соединения
    auto simpleIdCol = std::make_shared<TColumn>();
    simpleIdCol->SetPath("simple_message/id");

    auto nestedRefCol = std::make_shared<TColumn>();
    nestedRefCol->SetPath("nested_message/tags");

    // Создаем условие JOIN ON simple.id = nested.simple_ref
    auto joinCondition = std::make_shared<TExpression>();
    joinCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    joinCondition->SetOperands({simpleIdCol, nestedRefCol});

    // Тестируем различные типы JOIN
    auto leftJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Left);
    EXPECT_EQ(leftJoin->Build(builder), "LEFT JOIN t_2 ON (t_1.f_1 = t_2_3.f_1)");

    auto innerJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Inner);
    EXPECT_EQ(innerJoin->Build(builder), "INNER JOIN t_2 ON (t_1.f_1 = t_2_3.f_1)");

    auto exclusiveLeftJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::ExclusiveLeft);
    EXPECT_EQ(exclusiveLeftJoin->Build(builder), "LEFT OUTER JOIN t_2 ON (t_1.f_1 = t_2_3.f_1)");
}

// Тест для сложного запроса SELECT с FROM и JOIN
TEST_F(PostgresQueryBuilderTest, SelectWithFromAndJoin) {
    // Создаем колонки для SELECT
    auto simpleIdCol = std::make_shared<TColumn>();
    simpleIdCol->SetPath("simple_message/id");

    auto simpleNameCol = std::make_shared<TColumn>();
    simpleNameCol->SetPath("simple_message/name");

    auto nestedValueCol = std::make_shared<TColumn>();
    nestedValueCol->SetPath("nested_message/tags");

    // Создаем условие JOIN
    auto nestedRefCol = std::make_shared<TColumn>();
    nestedRefCol->SetPath("nested_message/tags");

    auto joinCondition = std::make_shared<TExpression>();
    joinCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    joinCondition->SetOperands({simpleIdCol, nestedRefCol});

    auto innerJoin = std::make_shared<TJoin>(nestedPath, joinCondition, TJoin::EJoinType::Inner);

    // Создаем FROM для основной таблицы
    auto simpleTable = std::make_shared<TString>();
    simpleTable->SetValue(Format("{table_id}", simplePath));

    // Создаем WHERE условие
    auto whereCondition = std::make_shared<TExpression>();
    whereCondition->SetExpressionType(NOrm::NQuery::EExpressionType::greater_than);
    auto valueConst = std::make_shared<TInt>();
    valueConst->SetValue(10);
    whereCondition->SetOperands({simpleIdCol, valueConst});

    // Создаем запрос SELECT
    auto selectQuery = std::make_shared<TSelect>();
    selectQuery->SetSelectors({simpleIdCol, simpleNameCol, nestedValueCol});
    selectQuery->SetFrom({std::make_shared<TTable>(simplePath)});
    selectQuery->SetJoin({innerJoin});
    selectQuery->SetWhere(whereCondition);

    // Тестируем запрос SELECT с FROM и JOIN
    std::string selectSQL = selectQuery->Build(builder);
    EXPECT_EQ(selectSQL, "SELECT t_1.f_1, t_1.f_2, t_2_3.f_1 FROM t_1 INNER JOIN t_2 ON (t_1.f_1 = t_2_3.f_1) WHERE (t_1.f_1 > 10)");
}

// Тест для сложного запроса INSERT
TEST_F(PostgresQueryBuilderTest, ComplexInsertQuery) {
    // Создаем колонки для INSERT
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    auto nameCol = std::make_shared<TColumn>();
    nameCol->SetPath("simple_message/name");
    nameCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем значения для INSERT
    auto idValue = std::make_shared<TInt>();
    idValue->SetValue(1);

    auto nameValue = std::make_shared<TString>();
    nameValue->SetValue("Test");

    std::vector<std::vector<TClausePtr>> values = {{idValue, nameValue}};

    // Создаем запрос INSERT
    auto insertQuery = std::make_shared<TInsert>(simplePath);
    insertQuery->SetSelectors({idCol, nameCol});
    insertQuery->SetIsValues(true);
    insertQuery->SetValues(values);

    // Тестируем запрос INSERT
    std::string insertSQL = insertQuery->Build(builder);
    EXPECT_EQ(insertSQL, "INSERT INTO t_1 (t_1.f_1, t_1.f_2) VALUES (1, 'Test')");
}

// Тест для агрегатных функций
TEST_F(PostgresQueryBuilderTest, AggregateFunctionExpressions) {
    // Создаем операнд
    auto col = std::make_shared<TColumn>();
    col->SetPath("simple_message/id");
    col->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Тестируем COUNT
    auto countExpr = std::make_shared<TExpression>();
    countExpr->SetExpressionType(NOrm::NQuery::EExpressionType::count);
    countExpr->SetOperands({col});
    EXPECT_EQ(countExpr->Build(builder), "COUNT(t_1.f_1)");

    // Тестируем SUM
    auto sumExpr = std::make_shared<TExpression>();
    sumExpr->SetExpressionType(NOrm::NQuery::EExpressionType::sum);
    sumExpr->SetOperands({col});
    EXPECT_EQ(sumExpr->Build(builder), "SUM(t_1.f_1)");

    // Тестируем AVG
    auto avgExpr = std::make_shared<TExpression>();
    avgExpr->SetExpressionType(NOrm::NQuery::EExpressionType::avg);
    avgExpr->SetOperands({col});
    EXPECT_EQ(avgExpr->Build(builder), "AVG(t_1.f_1)");
}

// Тест для сложного запроса UPDATE, заменяющего максимальные значения на минимальные
TEST_F(PostgresQueryBuilderTest, UpdateMaxToMinValue) {
    // Создаем колонку для ID
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем подзапрос MIN
    auto minExpr = std::make_shared<TExpression>();
    minExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minExpr->SetOperands({idCol});

    // Создаем подзапрос с выборкой минимального значения
    auto minSelect = std::make_shared<TSelect>();
    minSelect->SetSelectors({minExpr});

    // Создаем подзапрос MAX
    auto maxExpr = std::make_shared<TExpression>();
    maxExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    maxExpr->SetOperands({idCol});

    // Создаем подзапрос с выборкой максимального значения
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
    std::string updateSQL = updateQuery->Build(builder);
    EXPECT_EQ(updateSQL, "UPDATE t_1 SET t_1.f_1 = (SELECT MIN(t_1.f_1)) WHERE (t_1.f_1 = (SELECT MAX(t_1.f_1)))");
}

// Тест для сложного запроса UPDATE с несколькими полями
TEST_F(PostgresQueryBuilderTest, UpdateMultipleMaxToMinValues) {
    // Создаем колонки
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    auto nameCol = std::make_shared<TColumn>();
    nameCol->SetPath("simple_message/name");
    nameCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем подзапросы MIN для обоих колонок
    auto minIdExpr = std::make_shared<TExpression>();
    minIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minIdExpr->SetOperands({idCol});

    auto minNameExpr = std::make_shared<TExpression>();
    minNameExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minNameExpr->SetOperands({nameCol});

    auto minIdSelect = std::make_shared<TSelect>();
    minIdSelect->SetSelectors({minIdExpr});

    auto minNameSelect = std::make_shared<TSelect>();
    minNameSelect->SetSelectors({minNameExpr});

    // Создаем подзапросы MAX для обоих колонок
    auto maxIdExpr = std::make_shared<TExpression>();
    maxIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    maxIdExpr->SetOperands({idCol});

    auto maxNameExpr = std::make_shared<TExpression>();
    maxNameExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    maxNameExpr->SetOperands({nameCol});

    auto maxIdSelect = std::make_shared<TSelect>();
    maxIdSelect->SetSelectors({maxIdExpr});

    auto maxNameSelect = std::make_shared<TSelect>();
    maxNameSelect->SetSelectors({maxNameExpr});

    // Создаем условие WHERE id = (SELECT MAX(id)) OR name = (SELECT MAX(name))
    auto whereIdExpr = std::make_shared<TExpression>();
    whereIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereIdExpr->SetOperands({idCol, maxIdSelect});

    auto whereNameExpr = std::make_shared<TExpression>();
    whereNameExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereNameExpr->SetOperands({nameCol, maxNameSelect});

    auto whereOrExpr = std::make_shared<TExpression>();
    whereOrExpr->SetExpressionType(NOrm::NQuery::EExpressionType::or_);
    whereOrExpr->SetOperands({whereIdExpr, whereNameExpr});

    // Создаем запрос UPDATE
    auto updateQuery = std::make_shared<TUpdate>(simplePath);
    updateQuery->SetUpdates({{idCol, minIdSelect}, {nameCol, minNameSelect}});
    updateQuery->SetWhere(whereOrExpr);

    // Тестируем запрос UPDATE
    std::string updateSQL = updateQuery->Build(builder);
    EXPECT_EQ(
        updateSQL,
        "UPDATE t_1 SET t_1.f_1 = (SELECT MIN(t_1.f_1)), t_1.f_2 = (SELECT MIN(t_1.f_2)) WHERE ((t_1.f_1 = (SELECT MAX(t_1.f_1))) OR (t_1.f_2 = (SELECT "
        "MAX(t_1.f_2))))"
    );
}

// Тест для запроса с обновлением одних полей на основе других
TEST_F(PostgresQueryBuilderTest, SwapMaxAndMinValues) {
    // Создаем колонку для ID
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем временную переменную для хранения макс. значения
    auto tempMaxExpr = std::make_shared<TExpression>();
    tempMaxExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    tempMaxExpr->SetOperands({idCol});

    auto tempMaxSelect = std::make_shared<TSelect>();
    tempMaxSelect->SetSelectors({tempMaxExpr});

    // Создаем подзапрос MIN
    auto minExpr = std::make_shared<TExpression>();
    minExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minExpr->SetOperands({idCol});

    auto minSelect = std::make_shared<TSelect>();
    minSelect->SetSelectors({minExpr});

    // Создаем условие WHERE id = (SELECT MAX(id))
    auto whereMaxExpr = std::make_shared<TExpression>();
    whereMaxExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereMaxExpr->SetOperands({idCol, tempMaxSelect});

    // Создаем запрос UPDATE для макс. значений
    auto updateMaxQuery = std::make_shared<TUpdate>(simplePath);
    updateMaxQuery->SetUpdates({{idCol, minSelect}});
    updateMaxQuery->SetWhere(whereMaxExpr);

    // Создаем условие WHERE id = (SELECT MIN(id))
    auto whereMinExpr = std::make_shared<TExpression>();
    whereMinExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereMinExpr->SetOperands({idCol, minSelect});

    // Создаем запрос UPDATE для мин. значений
    auto updateMinQuery = std::make_shared<TUpdate>(simplePath);
    updateMinQuery->SetUpdates({{idCol, tempMaxSelect}});
    updateMinQuery->SetWhere(whereMinExpr);

    // Создаем транзакцию
    auto startTx = std::make_shared<TStartTransaction>();
    auto commitTx = std::make_shared<TCommitTransaction>();

    // Объединяем запросы в транзакцию
    TQuery query({startTx, updateMaxQuery, updateMinQuery, commitTx});

    // Тестируем объединенный запрос
    std::string querySQL = query.Build(builder);
    EXPECT_EQ(
        querySQL,
        "BEGIN; UPDATE t_1 SET t_1.f_1 = (SELECT MIN(t_1.f_1)) WHERE (t_1.f_1 = (SELECT MAX(t_1.f_1))); UPDATE t_1 SET t_1.f_1 = (SELECT MAX(t_1.f_1)) WHERE "
        "(t_1.f_1 = (SELECT MIN(t_1.f_1))); COMMIT; "
    );
}

// Тест для условного обновления с использованием CTE (Common Table Expressions)
TEST_F(PostgresQueryBuilderTest, UpdateWithComplexConditions) {
    // Создаем колонки
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");
    idCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    auto activeCol = std::make_shared<TColumn>();
    activeCol->SetPath("simple_message/active");
    activeCol->SetType(NOrm::NQuery::EColumnType::ESingular);

    // Создаем выражение MAX(id)
    auto maxIdExpr = std::make_shared<TExpression>();
    maxIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::max);
    maxIdExpr->SetOperands({idCol});

    auto maxIdSelect = std::make_shared<TSelect>();
    maxIdSelect->SetSelectors({maxIdExpr});

    // Создаем выражение MIN(id)
    auto minIdExpr = std::make_shared<TExpression>();
    minIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::min);
    minIdExpr->SetOperands({idCol});

    auto minIdSelect = std::make_shared<TSelect>();
    minIdSelect->SetSelectors({minIdExpr});

    // Создаем условие WHERE id = (SELECT MAX(id)) AND flag = TRUE
    auto whereIdExpr = std::make_shared<TExpression>();
    whereIdExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereIdExpr->SetOperands({idCol, maxIdSelect});

    auto flagValue = std::make_shared<TBool>();
    flagValue->SetValue(true);

    auto whereFlagExpr = std::make_shared<TExpression>();
    whereFlagExpr->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereFlagExpr->SetOperands({activeCol, flagValue});

    auto whereAndExpr = std::make_shared<TExpression>();
    whereAndExpr->SetExpressionType(NOrm::NQuery::EExpressionType::and_);
    whereAndExpr->SetOperands({whereIdExpr, whereFlagExpr});

    // Создаем запрос UPDATE
    auto updateQuery = std::make_shared<TUpdate>(simplePath);
    updateQuery->SetUpdates({{idCol, minIdSelect}});
    updateQuery->SetWhere(whereAndExpr);

    // Тестируем запрос UPDATE
    std::string updateSQL = updateQuery->Build(builder);
    EXPECT_EQ(updateSQL, "UPDATE t_1 SET t_1.f_1 = (SELECT MIN(t_1.f_1)) WHERE ((t_1.f_1 = (SELECT MAX(t_1.f_1))) AND (t_1.f_3 = TRUE))");
}

// Тест для DELETE с таблицей
TEST_F(PostgresQueryBuilderTest, DeleteWithTableName) {
    // Создаем колонки для условия WHERE
    auto idCol = std::make_shared<TColumn>();
    idCol->SetPath("simple_message/id");

    // Создаем значение для условия
    auto idValue = std::make_shared<TInt>();
    idValue->SetValue(1);

    // Создаем условие WHERE
    auto whereCondition = std::make_shared<TExpression>();
    whereCondition->SetExpressionType(NOrm::NQuery::EExpressionType::equals);
    whereCondition->SetOperands({idCol, idValue});

    // Создаем запрос DELETE
    auto deleteQuery = std::make_shared<TDelete>(simplePath, whereCondition);

    // Тестируем запрос DELETE
    std::string deleteSQL = deleteQuery->Build(builder);
    EXPECT_EQ(deleteSQL, "DELETE FROM t_1 WHERE (t_1.f_1 = 1)");
}

} // namespace

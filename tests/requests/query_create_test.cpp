#include <gtest/gtest.h>
#include <memory>
#include <relation/relation_manager.h>
#include <requests/query.h>
#include <lib/relation/proto/query.pb.h>
#include <tests/proto/test_objects.pb.h>

namespace {

using namespace NOrm::NRelation;
using namespace NOrm::NQuery;
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

class QueryBuilderTest : public ::testing::Test {
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

    // Общие пути для тестов
    TMessagePath simplePath;
    TMessagePath nestedPath;
};

// Тесты для базовых типов данных
TEST_F(QueryBuilderTest, BasicTypesTest) {
    // Тест TString
    auto stringVal = Val("test string");
    ASSERT_EQ(stringVal.GetValue(), "test string");
    
    // Тест TInt
    auto intVal = Val(42);
    ASSERT_EQ(intVal.GetValue(), 42);
    
    // Тест TFloat
    auto floatVal = Val(3.14);
    ASSERT_DOUBLE_EQ(floatVal.GetValue(), 3.14);
    
    // Тест TBool
    auto boolVal = Val(true);
    ASSERT_EQ(boolVal.GetValue(), true);
    
    // Проверка сериализации и десериализации
    NOrm::NApi::TQuery proto;
    stringVal.ToProto(&proto);
    
    auto newStringVal = std::make_shared<TString>();
    newStringVal->FromProto(proto, 0);
    ASSERT_EQ(newStringVal->GetValue(), "test string");
}

// Тесты для выражений и операторов
TEST_F(QueryBuilderTest, ExpressionTest) {
    auto a = Val(5);
    auto b = Val(3);
    
    // Арифметические операции
    auto addExpr = a + b;
    auto addExprCast = TExpression(addExpr);
    ASSERT_EQ(addExprCast.GetExpressionType(), EExpressionType::add);
    ASSERT_EQ(addExprCast.GetOperands().size(), 2);
    
    // Проверка других операторов
    auto subExpr = a - b;
    auto subExprCast = TExpression(subExpr);
    ASSERT_EQ(subExprCast.GetExpressionType(), EExpressionType::subtract);
    
    auto mulExpr = a * b;
    auto mulExprCast = TExpression(mulExpr);
    ASSERT_EQ(mulExprCast.GetExpressionType(), EExpressionType::multiply);
    
    auto divExpr = a / b;
    auto divExprCast = TExpression(divExpr);
    ASSERT_EQ(divExprCast.GetExpressionType(), EExpressionType::divide);
    
    // Сравнения
    auto eqExpr = a == b;
    auto eqExprCast = TExpression(eqExpr);
    ASSERT_EQ(eqExprCast.GetExpressionType(), EExpressionType::equals);
    
    auto neqExpr = a != b;
    auto neqExprCast = TExpression(neqExpr);
    ASSERT_EQ(neqExprCast.GetExpressionType(), EExpressionType::not_equals);
    
    // Логические операторы
    auto andExpr = (a > b) && (a != b);
    auto andExprCast = TExpression(andExpr);
    ASSERT_EQ(andExprCast.GetExpressionType(), EExpressionType::and_);
    
    auto orExpr = (a < b) || (a == b);
    auto orExprCast = TExpression(orExpr);
    ASSERT_EQ(orExprCast.GetExpressionType(), EExpressionType::or_);
    
    auto notExpr = !a;
    auto notExprCast = TExpression(notExpr);
    ASSERT_EQ(notExprCast.GetExpressionType(), EExpressionType::not_);
}

// Тесты для SELECT запросов
TEST_F(QueryBuilderTest, SelectQueryTest) {
    // Создаем колонки
    TMessagePath path1 = nestedPath / "simple/name";
    TMessagePath path2 = nestedPath / "simple/active";
    auto col1 = Col(path1);
    auto col2 = Col(path2);
    
    // Базовый SELECT
    auto select = Select("nested_message");
    select.Selectors(col1, col2);
    
    // Проверяем сериализацию и десериализацию
    NOrm::NApi::TQuery proto;
    select.ToProto(&proto);
    
    auto newSelect = Select("nested_message");
    newSelect.FromProto(proto, 0);
    
    // Проверяем WHERE условие
    auto whereCondition = col1 == Val(10);
    select.Where(whereCondition);
    
    proto.Clear();
    select.ToProto(&proto);
    
    newSelect = Select("nested_message");
    newSelect.FromProto(proto, 0);
    
    // Полный SELECT с GROUP BY, HAVING, ORDER BY, LIMIT
    auto fullSelect =
        Select("nested_message", col1, col2)
        .Where(col1 > 5)
        .GroupBy(col1)
        .Having(Count(col1) > 2)
        .OrderBy(col2)
        .Limit(10);
    
    proto.Clear();
    fullSelect.ToProto(&proto);
    
    auto newFullSelect = TSelect();
    newFullSelect.FromProto(proto, 0);
}

// Тесты для INSERT запросов
TEST_F(QueryBuilderTest, InsertQueryTest) {
    // Создаем атрибуты
    TMessagePath path1 = nestedPath / "simple/name";
    TMessagePath path2 = nestedPath / "simple/active";
    
    std::vector<TAttribute> attributes;
    
    TAttribute attr1;
    attr1.Path = path1;
    attr1.SetString("Test Name");
    
    TAttribute attr2;
    attr2.Path = path2;
    attr2.SetBool(true);
    
    std::vector<TAttribute> attributeSet = {attr1, attr2};
    
    // Базовый INSERT
    auto insert = Insert("nested_message");
    insert.AddSubrequest(attributeSet);
    
    // Проверяем сериализацию и десериализацию
    NOrm::NApi::TQuery proto;
    insert.ToProto(&proto);
    
    auto newInsert = TInsert();
    newInsert.FromProto(proto, 0);
    
    // Проверка свойств
    ASSERT_EQ(newInsert.GetTableNum(), 2);
    ASSERT_EQ(newInsert.GetSubrequests().size(), 1);
    
    // INSERT с UpdateIfExists
    auto insertWithUpdate = Insert("nested_message");
    insertWithUpdate.AddSubrequest(attributeSet);
    insertWithUpdate.UpdateIfExists();
    
    proto.Clear();
    insertWithUpdate.ToProto(&proto);
    
    auto newInsertWithUpdate = TInsert();
    newInsertWithUpdate.FromProto(proto, 0);
    
    // Проверка флага UpdateIfExists
    ASSERT_TRUE(newInsertWithUpdate.GetUpdateIfExists());
}

// Тесты для UPDATE запросов
TEST_F(QueryBuilderTest, UpdateQueryTest) {
    // Создаем атрибуты для обновления
    TMessagePath path1 = nestedPath / "simple/name";
    TMessagePath path2 = nestedPath / "simple/active";
    
    TAttribute attr1;
    attr1.Path = path1;
    attr1.SetString("Updated Name");
    
    TAttribute attr2;
    attr2.Path = path2;
    attr2.SetBool(false);
    
    std::vector<TAttribute> updateAttributes1 = {attr1};
    std::vector<TAttribute> updateAttributes2 = {attr2};
    
    // Базовый UPDATE
    auto update = Update("nested_message");
    update.AddUpdate(updateAttributes1);
    update.AddUpdate(updateAttributes2);
    
    // Проверяем сериализацию и десериализацию
    NOrm::NApi::TQuery proto;
    update.ToProto(&proto);
    
    auto newUpdate = TUpdate();
    newUpdate.FromProto(proto, 0);
    
    // Проверяем количество обновлений
    ASSERT_EQ(newUpdate.GetUpdates().size(), 2);
}

// Тесты для DELETE запросов
TEST_F(QueryBuilderTest, DeleteQueryTest) {
    // Создаем колонку
    TMessagePath path = nestedPath / "simple/name";
    auto col = Col(path);
    
    // Базовый DELETE
    auto deleteQuery = Delete("nested_message");
    
    // Проверяем сериализацию и десериализацию
    NOrm::NApi::TQuery proto;
    deleteQuery.ToProto(&proto);
    
    auto newDelete = TDelete();
    newDelete.FromProto(proto, proto.clauses_size() - 1);
    
    // DELETE с WHERE условием
    auto deleteWithWhere = Delete("nested_message");
    deleteWithWhere.Where(col == Val(10));
    
    proto.Clear();
    deleteWithWhere.ToProto(&proto);
    
    auto newDeleteWithWhere = TDelete();
    newDeleteWithWhere.FromProto(proto, proto.clauses_size() - 1);
    
    // Проверяем наличие WHERE условия
    ASSERT_TRUE(newDeleteWithWhere.GetWhere());
}

// Тесты для транзакций
TEST_F(QueryBuilderTest, TransactionTest) {
    // START TRANSACTION
    auto startTrans = TStartTransaction();
    
    NOrm::NApi::TQuery proto;
    startTrans.ToProto(&proto);
    
    auto newStartTrans = TStartTransaction();
    newStartTrans.FromProto(proto, 0);
    
    // COMMIT
    auto commit = TCommitTransaction();
    
    proto.Clear();
    commit.ToProto(&proto);
    
    auto newCommit = TCommitTransaction();
    newCommit.FromProto(proto, 0);
    
    // ROLLBACK
    auto rollback = TRollbackTransaction();
    
    proto.Clear();
    rollback.ToProto(&proto);
    
    auto newRollback = TRollbackTransaction();
    newRollback.FromProto(proto, 0);
}

// Тесты для корневого TQuery
TEST_F(QueryBuilderTest, QueryTest) {
    // Создаем различные типы запросов
    TMessagePath path = nestedPath / "simple/name";
    auto col = Col(path);
    
    auto select = Select("nested_message", col)
                  .Where(col > 5);
    
    // Создаем атрибуты для INSERT
    TAttribute attr;
    attr.Path = path;
    attr.SetString("Test Value");
    std::vector<TAttribute> attributes = {attr};
    
    auto insert = Insert("nested_message");
    insert.AddSubrequest(attributes);
    
    // Создаем корневой объект запроса
    auto query = CreateQuery();
    query.AddClause(select)
         .AddClause(insert);
    
    // Проверяем сериализацию и десериализацию
    NOrm::NApi::TQuery proto;
    query.ToProto(&proto);
    
    // Проверяем количество стартовых точек
    ASSERT_EQ(proto.start_points_size(), 2);
    
    auto newQuery = TQuery();
    newQuery.FromProto(proto);
    
    // Проверяем количество клаузов
    ASSERT_EQ(newQuery.GetClauses().size(), 2);
}

// Тесты для строковых функций
TEST_F(QueryBuilderTest, StringFunctionsTest) {
    auto str = "Hello World";
    
    // Проверка функций над строками
    auto lowerExpr = Lower(Val(str));
    TExpression lowerExprCast = lowerExpr;
    ASSERT_EQ(lowerExprCast.GetExpressionType(), EExpressionType::lower);
    
    auto upperExpr = Upper(Val(str));
    auto upperExprCast = TExpression(upperExpr);
    ASSERT_EQ(upperExprCast.GetExpressionType(), EExpressionType::upper);
    
    auto substrExpr = SubStr(Val(str), Val(0), Val(5));
    auto substrExprCast = TExpression(substrExpr);
    ASSERT_EQ(substrExprCast.GetExpressionType(), EExpressionType::substring);
    
    // Проверка реальных функций
    std::string testStr = "Hello World";
    ASSERT_EQ(Lower(testStr), "hello world");
    ASSERT_EQ(Upper(testStr), "HELLO WORLD");
    ASSERT_EQ(SubStr(testStr, 0, 5), "Hello");
}

}

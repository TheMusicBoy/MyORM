#include <gtest/gtest.h>
#include <relation/path.h>
#include <relation/relation_manager.h>
#include <tests/proto/test_objects.pb.h>

#include <memory>
#include <string>
#include <vector>
#include <common/format.h>

using namespace NOrm::NRelation;
using namespace test_objects;

// Фикстура для тестирования path
class PathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Регистрация протобаф-объектов
        SimpleMessage simple;
        NestedMessage nested;
        DeepNestedMessage deepNested;
        
        // Очистка состояния менеджера отношений
        TRelationManager::GetInstance().Clear();
        
        // Регистрация корневых сообщений для тестирования
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
        
        deepNestedConfig = NCommon::New<TTableConfig>();
        deepNestedConfig->Number = 3;
        deepNestedConfig->SnakeCase = "deep_nested_message";
        deepNestedConfig->CamelCase = "DeepNestedMessage";
        deepNestedConfig->Scheme = "test_objects.DeepNestedMessage";
        
        RegisterRootMessage(simpleConfig);
        RegisterRootMessage(nestedConfig);
        RegisterRootMessage(deepNestedConfig);
    }
    
    void TearDown() override {
        TRelationManager::GetInstance().Clear();
    }
    
    TTableConfigPtr simpleConfig;
    TTableConfigPtr nestedConfig;
    TTableConfigPtr deepNestedConfig;
};

// Тесты для вложенных сообщений
TEST_F(PathTest, NestedMessages) {
    // Создаем путь к вложенному сообщению
    TMessagePath nestedPath(2); // NestedMessage
    
    // Получаем сообщение
    auto nestedMessage = TRelationManager::GetInstance().GetMessage(nestedPath);
    ASSERT_NE(nestedMessage, nullptr);
    
    // Ищем поле simple в NestedMessage
    TMessagePath simplePath;
    for (auto field : nestedMessage->MessageFields()) {
        if (field->GetName() == "simple") {
            simplePath = field->GetPath();
            break;
        }
    }
    ASSERT_FALSE(simplePath.empty());
    
    // Проверяем путь к вложенному сообщению
    EXPECT_EQ(simplePath.size(), 2);
    EXPECT_EQ(simplePath.at(0), 2); // NestedMessage
    
    // Проверяем parent
    TMessagePath parentPath = simplePath.parent();
    EXPECT_EQ(parentPath, nestedPath);
    
    // Создаем путь к deepNestedMessage
    TMessagePath deepPath(3);
    auto deepMessage = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(deepMessage, nullptr);
    
    // Ищем поле nested в DeepNestedMessage
    TMessagePath nestedInDeepPath;
    for (auto field : deepMessage->MessageFields()) {
        if (field->GetName() == "nested") {
            nestedInDeepPath = field->GetPath();
            break;
        }
    }
    ASSERT_FALSE(nestedInDeepPath.empty());
    
    // Проверяем, что неверный путь к полю обрабатывается корректно
    EXPECT_THROW({
        auto invalidPath = nestedPath / "nonexistent_field";
    }, std::exception);
}

// Тесты для работы с глубоко вложенными сообщениями
TEST_F(PathTest, DeepNestedMessagePaths) {
    // Создаем путь к DeepNestedMessage
    TMessagePath deepPath(3);
    
    // Получаем сообщение
    auto deepMessage = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(deepMessage, nullptr);
    
    // Последовательно находим путь к: nested -> simple -> name
    TMessagePath nestedPath;
    TMessagePath simplePath;
    TMessagePath namePath;
    
    // Находим nested
    for (auto field : deepMessage->MessageFields()) {
        if (field->GetName() == "nested") {
            nestedPath = field->GetPath();
            
            // Получаем объект nested
            auto nestedObj = TRelationManager::GetInstance().GetMessage(nestedPath);
            ASSERT_NE(nestedObj, nullptr);
            
            // Находим simple в nested
            for (auto innerField : nestedObj->MessageFields()) {
                if (innerField->GetName() == "simple") {
                    simplePath = innerField->GetPath();
                    
                    // Получаем объект simple
                    auto simpleObj = TRelationManager::GetInstance().GetMessage(simplePath);
                    ASSERT_NE(simpleObj, nullptr);
                    
                    // Находим name в simple
                    for (auto primitiveField : simpleObj->PrimitiveFields()) {
                        if (primitiveField->GetName() == "name") {
                            namePath = primitiveField->GetPath();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    
    // Проверяем что все пути найдены
    ASSERT_FALSE(nestedPath.empty());
    ASSERT_FALSE(simplePath.empty());
    ASSERT_FALSE(namePath.empty());
    
    // Проверяем размеры путей (должны увеличиваться при углублении)
    EXPECT_EQ(nestedPath.size(), 2);  // [3, nested_field_num]
    EXPECT_EQ(simplePath.size(), 3);  // [3, nested_field_num, simple_field_num]
    EXPECT_EQ(namePath.size(), 4);    // [3, nested_field_num, simple_field_num, name_field_num]
    
    // Проверяем отношения между путями
    EXPECT_TRUE(deepPath.isAncestorOf(nestedPath));
    EXPECT_TRUE(deepPath.isAncestorOf(simplePath));
    EXPECT_TRUE(deepPath.isAncestorOf(namePath));
    
    EXPECT_TRUE(nestedPath.isAncestorOf(simplePath));
    EXPECT_TRUE(nestedPath.isAncestorOf(namePath));
    EXPECT_FALSE(nestedPath.isAncestorOf(deepPath));
    
    EXPECT_TRUE(simplePath.isAncestorOf(namePath));
    EXPECT_FALSE(simplePath.isAncestorOf(nestedPath));
    EXPECT_FALSE(simplePath.isAncestorOf(deepPath));
    
    // Проверяем parent paths
    EXPECT_EQ(namePath.parent(), simplePath);
    EXPECT_EQ(simplePath.parent(), nestedPath);
    EXPECT_EQ(nestedPath.parent(), deepPath);
}

// Тесты для форматирования путей
TEST_F(PathTest, PathFormatting) {
    // Создаем путь к вложенному полю
    TMessagePath deepPath(3);
    auto deepMessage = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(deepMessage, nullptr);
    
    // Создаем сложный путь (DeepNestedMessage -> nested -> simple -> name)
    TMessagePath complexPath;
    for (auto field : deepMessage->MessageFields()) {
        if (field->GetName() == "nested") {
            auto nestedPath = field->GetPath();
            auto nestedObj = TRelationManager::GetInstance().GetMessage(nestedPath);
            
            for (auto innerField : nestedObj->MessageFields()) {
                if (innerField->GetName() == "simple") {
                    auto simplePath = innerField->GetPath();
                    auto simpleObj = TRelationManager::GetInstance().GetMessage(simplePath);
                    
                    for (auto primitiveField : simpleObj->PrimitiveFields()) {
                        if (primitiveField->GetName() == "name") {
                            complexPath = primitiveField->GetPath();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    ASSERT_FALSE(complexPath.empty());
    
    // Проверяем String() представление
    auto pathString = complexPath.String();
    ASSERT_EQ(pathString.size(), 4);
    EXPECT_EQ(pathString[0], "deep_nested_message");
    EXPECT_EQ(pathString[1], "nested");
    EXPECT_EQ(pathString[2], "simple");
    EXPECT_EQ(pathString[3], "name");
    
    // Проверка строкового представления через NCommon::Format
    std::string formatted = Format("{}", complexPath);
    EXPECT_EQ(formatted, "deep_nested_message/nested/simple/name");
    
    // Проверка форматирования с опциями
    formatted = Format("{table_id}", complexPath);
    EXPECT_TRUE(!formatted.empty());
    
    // Проверка форматирования с опцией field_id
    formatted = Format("{field_id}", complexPath);
    EXPECT_TRUE(!formatted.empty());
    
    // Проверка форматирования с опцией full_field_id
    formatted = Format("{full_field_id}", complexPath);
    EXPECT_TRUE(!formatted.empty());
}

// Тесты для GetTablePath, GetTable, GetField с вложенными полями
TEST_F(PathTest, NestedTableAndFieldDecomposition) {
    // Создаем путь к DeepNestedMessage
    TMessagePath deepPath(3);
    
    // Получаем сообщение
    auto deepMessage = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(deepMessage, nullptr);
    
    // Находим вложенный путь nested -> simple -> name
    TMessagePath namePath;
    for (auto field : deepMessage->MessageFields()) {
        if (field->GetName() == "nested") {
            auto nestedObj = TRelationManager::GetInstance().GetMessage(field->GetPath());
            for (auto innerField : nestedObj->MessageFields()) {
                if (innerField->GetName() == "simple") {
                    auto simpleObj = TRelationManager::GetInstance().GetMessage(innerField->GetPath());
                    for (auto primitiveField : simpleObj->PrimitiveFields()) {
                        if (primitiveField->GetName() == "name") {
                            namePath = primitiveField->GetPath();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    ASSERT_FALSE(namePath.empty());
    
    // Проверяем GetTablePath для глубоко вложенного поля
    TMessagePath tablePath = namePath.GetTablePath();
    EXPECT_FALSE(tablePath.empty());
    
    // Проверяем GetTable
    auto tableVector = namePath.GetTable();
    EXPECT_FALSE(tableVector.empty());
    
    // Проверяем GetField
    auto fieldVector = namePath.GetField();
    EXPECT_FALSE(fieldVector.empty());
}

// Тесты для обработки ошибок и крайних случаев
TEST_F(PathTest, ErrorHandlingAndEdgeCases) {
    // Проверка доступа к пустому пути
    TMessagePath emptyPath;
    EXPECT_THROW(emptyPath.front(), std::exception);
    EXPECT_THROW(emptyPath.back(), std::exception);
    
    // Проверка сравнения с пустым путем
    TMessagePath nonEmptyPath(1);
    EXPECT_FALSE(nonEmptyPath == emptyPath);
    EXPECT_TRUE(nonEmptyPath != emptyPath);
    EXPECT_TRUE(nonEmptyPath > emptyPath);
    EXPECT_TRUE(nonEmptyPath >= emptyPath);
    EXPECT_FALSE(nonEmptyPath < emptyPath);
    EXPECT_FALSE(nonEmptyPath <= emptyPath);
    
    // Проверка модификации пути через front() и back()
    std::vector<uint32_t> modPathVec = {10, 20};
    TMessagePath modPath(modPathVec);
    modPath.front() = 15;
    modPath.back() = 25;
    EXPECT_EQ(modPath.at(0), 15);
    EXPECT_EQ(modPath.at(1), 25);
    
    // Проверка поведения при доступе к несуществующему индексу
    EXPECT_THROW(modPath.at(2), std::out_of_range);
    
    // Проверка создания путей с очень большим количеством элементов
    std::vector<uint32_t> largeVector(1000, 42);
    TMessagePath largePath(largeVector);
    EXPECT_EQ(largePath.size(), 1000);
    EXPECT_EQ(largePath.at(999), 42);
}

// Тест parent метода (вместо тестирования приватного PopEntry)
TEST_F(PathTest, ParentMethod) {
    std::vector<uint32_t> pathVec = {10, 20, 30};
    TMessagePath path(pathVec);
    
    // Проверяем метод parent() (использует приватный PopEntry)
    TMessagePath parentPath = path.parent();
    EXPECT_EQ(parentPath.size(), 2);
    EXPECT_EQ(parentPath.at(0), 10);
    EXPECT_EQ(parentPath.at(1), 20);
    
    // Повторное получение parent
    TMessagePath grandparentPath = parentPath.parent();
    EXPECT_EQ(grandparentPath.size(), 1);
    EXPECT_EQ(grandparentPath.at(0), 10);
    
    // Получение parent от пути с одним элементом
    TMessagePath rootParentPath = grandparentPath.parent();
    EXPECT_TRUE(rootParentPath.empty());
}

// Тесты операторов конкатенации
TEST_F(PathTest, ConcatenationOperators) {
    TMessagePath basePath(10);
    
    // Оператор /= с uint32_t
    basePath /= 20;
    EXPECT_EQ(basePath.size(), 2);
    EXPECT_EQ(basePath.at(0), 10);
    EXPECT_EQ(basePath.at(1), 20);
    
    // Оператор / с uint32_t
    TMessagePath newPath = basePath / 30;
    EXPECT_EQ(newPath.size(), 3);
    EXPECT_EQ(newPath.at(0), 10);
    EXPECT_EQ(newPath.at(1), 20);
    EXPECT_EQ(newPath.at(2), 30);
    
    // Оператор /= со строкой (требуется корректная регистрация в TRelationManager)
    TMessagePath rootPath(1); // SimpleMessage
    
    // Проверяем поля в SimpleMessage
    auto message = TRelationManager::GetInstance().GetMessage(rootPath);
    if (message) {
        for (auto field : message->PrimitiveFields()) {
            TMessagePath fieldPath = rootPath / field->GetName();
            EXPECT_GT(fieldPath.size(), rootPath.size());
        }
    }
}

// Тесты для компонентов с векторами
TEST_F(PathTest, VectorComponents) {
    // Создаем пути с векторами
    std::vector<uint32_t> path1Vec = {10, 20};
    std::vector<uint32_t> path2Vec = {10, 20};
    std::vector<uint32_t> path3Vec = {10, 30};
    std::vector<uint32_t> path4Vec = {10, 20, 30};
    
    TMessagePath path1(path1Vec);
    TMessagePath path2(path2Vec);
    TMessagePath path3(path3Vec);
    TMessagePath path4(path4Vec);
    
    // Равенство
    EXPECT_TRUE(path1 == path2);
    EXPECT_FALSE(path1 == path3);
    EXPECT_FALSE(path1 == path4);
    
    // Неравенство
    EXPECT_FALSE(path1 != path2);
    EXPECT_TRUE(path1 != path3);
    EXPECT_TRUE(path1 != path4);
    
    // Меньше
    EXPECT_FALSE(path1 < path2);
    EXPECT_TRUE(path1 < path3);
    EXPECT_TRUE(path1 < path4);
    EXPECT_FALSE(path3 < path1);
    
    // Меньше или равно
    EXPECT_TRUE(path1 <= path2);
    EXPECT_TRUE(path1 <= path3);
    EXPECT_TRUE(path1 <= path4);
    EXPECT_FALSE(path3 <= path1);
    
    // Больше
    EXPECT_FALSE(path1 > path2);
    EXPECT_FALSE(path1 > path3);
    EXPECT_FALSE(path1 > path4);
    EXPECT_TRUE(path3 > path1);
    
    // Больше или равно
    EXPECT_TRUE(path1 >= path2);
    EXPECT_FALSE(path1 >= path3);
    EXPECT_FALSE(path1 >= path4);
    EXPECT_TRUE(path3 >= path1);
}

// Тесты методов отношений между путями
TEST_F(PathTest, PathRelationshipMethods) {
    std::vector<uint32_t> parentVec = {10, 20};
    std::vector<uint32_t> childVec = {10, 20, 30};
    std::vector<uint32_t> otherChildVec = {10, 20, 40};
    std::vector<uint32_t> grandchildVec = {10, 20, 30, 40};
    std::vector<uint32_t> unrelatedVec = {10, 30};
    
    TMessagePath parent(parentVec);
    TMessagePath child(childVec);
    TMessagePath otherChild(otherChildVec);
    TMessagePath grandchild(grandchildVec);
    TMessagePath unrelated(unrelatedVec);
    
    // isParentOf
    EXPECT_TRUE(parent.isParentOf(child));
    EXPECT_TRUE(parent.isParentOf(otherChild));
    EXPECT_FALSE(parent.isParentOf(grandchild));
    EXPECT_FALSE(parent.isParentOf(unrelated));
    EXPECT_FALSE(child.isParentOf(parent));
    
    // isAncestorOf
    EXPECT_TRUE(parent.isAncestorOf(child));
    EXPECT_TRUE(parent.isAncestorOf(grandchild));
    EXPECT_FALSE(parent.isAncestorOf(unrelated));
    EXPECT_FALSE(child.isAncestorOf(parent));
    
    // isChildOf
    EXPECT_TRUE(child.isChildOf(parent));
    EXPECT_FALSE(grandchild.isChildOf(parent));
    EXPECT_FALSE(unrelated.isChildOf(parent));
    EXPECT_FALSE(parent.isChildOf(child));
    
    // isDescendantOf
    EXPECT_TRUE(child.isDescendantOf(parent));
    EXPECT_TRUE(grandchild.isDescendantOf(parent));
    EXPECT_FALSE(unrelated.isDescendantOf(parent));
    EXPECT_FALSE(parent.isDescendantOf(child));
}

// Тесты хеш-функций
TEST_F(PathTest, HashFunctions) {
    std::vector<uint32_t> path1Vec = {10, 20, 30};
    std::vector<uint32_t> path2Vec = {10, 20, 30};
    std::vector<uint32_t> path3Vec = {10, 20, 40};
    
    TMessagePath path1(path1Vec);
    TMessagePath path2(path2Vec);
    TMessagePath path3(path3Vec);
    
    // Хеш одинаковых путей должен быть одинаковым
    EXPECT_EQ(GetHash(path1), GetHash(path2));
    
    // Хеш разных путей должен быть разным
    EXPECT_NE(GetHash(path1), GetHash(path3));
    
    // Хеш вектора должен совпадать с хешем пути
    EXPECT_EQ(GetHash(path1), GetHash(path1.data()));
    
    // Проверка GetNextPathEntryHash
    size_t hash1 = 0;
    size_t hash2 = 0;
    
    for (const auto& entry : path1) {
        hash1 = GetNextPathEntryHash(hash1, entry);
    }
    
    for (const auto& entry : path2) {
        hash2 = GetNextPathEntryHash(hash2, entry);
    }
    
    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(hash1, GetHash(path1));
}

// Тесты представления пути
TEST_F(PathTest, PathRepresentations) {
    // Создаем путь для SimpleMessage
    TMessagePath rootPath(1);
    
    // Проверяем String() представление
    auto stringRep = rootPath.String();
    EXPECT_FALSE(stringRep.empty());
    EXPECT_EQ(stringRep[0], "simple_message");
    
    // Проверяем Number() представление
    auto numberRep = rootPath.Number();
    EXPECT_EQ(numberRep.size(), 1);
    EXPECT_EQ(numberRep[0], 1);
    
    // Проверяем data()
    auto dataRep = rootPath.data();
    EXPECT_EQ(dataRep.size(), 1);
    EXPECT_EQ(dataRep[0], 1);
    
    // Проверяем name() и number()
    EXPECT_EQ(rootPath.name(), "simple_message");
    EXPECT_EQ(rootPath.number(), 1);
}

// Тест для методов GetTable и GetField
TEST_F(PathTest, TableAndFieldMethods) {
    // Создаем путь для SimpleMessage
    TMessagePath rootPath(1);
    
    // Получаем поле через RelationManager
    auto message = TRelationManager::GetInstance().GetMessage(rootPath);
    if (message) {
        for (auto field : message->PrimitiveFields()) {
            TMessagePath fieldPath = field->GetPath();
            
            // GetTable должен вернуть путь к таблице
            auto table = fieldPath.GetTable();
            EXPECT_EQ(table.size(), 1);
            EXPECT_EQ(table[0], 1);
            
            // GetField должен вернуть только номер поля
            auto fieldNum = fieldPath.GetField();
            EXPECT_EQ(fieldNum.size(), 1);
            EXPECT_EQ(fieldNum[0], field->GetFieldNumber());
            
            // GetTablePath должен вернуть путь к таблице
            auto tablePath = fieldPath.GetTablePath();
            EXPECT_EQ(tablePath.size(), 1);
            EXPECT_EQ(tablePath.at(0), 1);
        }
    }
}

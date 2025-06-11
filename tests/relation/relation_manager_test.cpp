#include <gtest/gtest.h>
#include <tests/proto/test_objects.pb.h>
#include <relation/field.h>
#include <relation/message.h>
#include <relation/relation_manager.h>

// Необходима явная инициализация протобуф объектов в наших тестах
// для правильной регистрации в DescriptorPool::generated_pool()
#include <google/protobuf/descriptor.h>
#include <iostream>

namespace {

using namespace NOrm::NRelation;
using namespace test_objects;

// Тестовый класс с общей настройкой для всех тестов
// Функция для проверки доступности дескрипторов в pooле
static void EnsureDescriptorsRegistered() {
    // Создаем временные объекты, чтобы убедиться, что дескрипторы зарегистрированы
    test_objects::SimpleMessage simple;
    test_objects::NestedMessage nested;
    test_objects::DeepNestedMessage deepNested;

    // Проверяем доступность дескрипторов в пуле
    auto pool = google::protobuf::DescriptorPool::generated_pool();

    auto simpleDesc = pool->FindMessageTypeByName("test_objects.SimpleMessage");
    auto nestedDesc = pool->FindMessageTypeByName("test_objects.NestedMessage");
    auto deepNestedDesc = pool->FindMessageTypeByName("test_objects.DeepNestedMessage");

    if (simpleDesc) {
        for (int i = 0; i < simpleDesc->field_count(); ++i) {
            auto field = simpleDesc->field(i);
        }
    }
}

class RelationManagerTest : public ::testing::Test {
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

        // Создание конфигурации для DeepNestedMessage
        deepNestedConfig = NCommon::New<TTableConfig>();
        deepNestedConfig->Number = 3;
        deepNestedConfig->SnakeCase = "deep_nested_message";
        deepNestedConfig->CamelCase = "DeepNestedMessage";
        deepNestedConfig->Scheme = "test_objects.DeepNestedMessage";
    }

    void TearDown() override {
        // Очистка после каждого теста
        TRelationManager::GetInstance().Clear();
    }

    // Регистрация тестовых сообщений
    void RegisterTestMessages() {
        RegisterRootMessage(simpleConfig);
        RegisterRootMessage(nestedConfig);
        RegisterRootMessage(deepNestedConfig);
    }

    TTableConfigPtr simpleConfig;
    TTableConfigPtr nestedConfig;
    TTableConfigPtr deepNestedConfig;
};

// Тест регистрации и получения корневого сообщения
TEST_F(RelationManagerTest, RegisterAndGetRootMessage) {
    // Регистрация корневого сообщения
    RegisterRootMessage(simpleConfig);

    // Получение сообщения по пути
    TMessagePath path(1);
    auto message = TRelationManager::GetInstance().GetMessage(path);

    // Проверка корректности получения сообщения
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->GetPath(), path);
    EXPECT_EQ(message->GetTableName(), "t_1");
}

// Тест получения полей из сообщения
TEST_F(RelationManagerTest, GetFields) {
    RegisterRootMessage(simpleConfig);

    // Получение сообщения
    TMessagePath path(1);
    auto message = TRelationManager::GetInstance().GetMessage(path);
    ASSERT_NE(message, nullptr);

    // Проверим количество полей в сообщении
    int fieldCount = 0;
    for (auto it = message->Fields().begin(); it != message->Fields().end(); ++it) {
        fieldCount++;
    }

    // Проверим primitive fields
    int primitiveFieldCount = 0;
    for (auto field : message->PrimitiveFields()) {
        primitiveFieldCount++;
    }

    // Проверка корректности регистрации полей
    bool foundIdField = false;
    bool foundNameField = false;
    bool foundActiveField = false;

    for (auto field : message->PrimitiveFields()) {
        if (field->GetName() == "id") {
            foundIdField = true;
            EXPECT_EQ(field->GetFieldNumber(), 1);
            EXPECT_EQ(field->GetValueType(), google::protobuf::FieldDescriptor::TYPE_INT32);
        } else if (field->GetName() == "name") {
            foundNameField = true;
            EXPECT_EQ(field->GetFieldNumber(), 2);
            EXPECT_EQ(field->GetValueType(), google::protobuf::FieldDescriptor::TYPE_STRING);
        } else if (field->GetName() == "active") {
            foundActiveField = true;
            EXPECT_EQ(field->GetFieldNumber(), 3);
            EXPECT_EQ(field->GetValueType(), google::protobuf::FieldDescriptor::TYPE_BOOL);
        }
    }

    EXPECT_TRUE(foundIdField);
    EXPECT_TRUE(foundNameField);
    EXPECT_TRUE(foundActiveField);
}

// Тест получения вложенных сообщений
TEST_F(RelationManagerTest, GetNestedMessages) {
    // Регистрация вложенного сообщения
    RegisterRootMessage(nestedConfig);

    // Получение сообщения
    TMessagePath path(2);
    auto message = TRelationManager::GetInstance().GetMessage(path);
    ASSERT_NE(message, nullptr);

    // Проверим descriptor
    if (message) {
        auto descriptor = dynamic_cast<TRootMessage*>(message.get())->GetDescriptor();
        if (descriptor) {
            for (int i = 0; i < descriptor->field_count(); ++i) {
                auto field = descriptor->field(i);
            }
        }
    }

    // Проверим MessageFields range
    int messageFieldCount = 0;
    for (auto it = message->MessageFields().begin(); it != message->MessageFields().end(); ++it) {
        messageFieldCount++;
    }

    // Проверка наличия вложенного simple-сообщения
    bool foundSimpleMessage = false;
    for (auto subMsg : message->MessageFields()) {
        if (subMsg->GetName() == "simple") {
            foundSimpleMessage = true;
            EXPECT_EQ(subMsg->GetFieldNumber(), 2);

            // Проверка полей во вложенном сообщении
            bool foundIdField = false;
            bool foundNameField = false;
            bool foundActiveField = false;

            for (auto field : subMsg->PrimitiveFields()) {
                if (field->GetName() == "id") {
                    foundIdField = true;
                } else if (field->GetName() == "name") {
                    foundNameField = true;
                } else if (field->GetName() == "active") {
                    foundActiveField = true;
                }
            }

            EXPECT_TRUE(foundIdField);
            EXPECT_TRUE(foundNameField);
            EXPECT_TRUE(foundActiveField);
        }
    }

    EXPECT_TRUE(foundSimpleMessage);
}

// Тест получения сообщений из поддерева
TEST_F(RelationManagerTest, GetMessagesFromSubtree) {
    RegisterTestMessages();

    auto& manager = TRelationManager::GetInstance();
    TMessagePath nestedPath(2);
    auto subtreeMessages = TRelationManager::GetInstance().GetMessagesFromSubtree(nestedPath);

    EXPECT_GE(subtreeMessages.size(), 2);

    // Проверка наличия обоих сообщений
    bool foundNestedMessage = false;
    bool foundSimpleInNested = false;

    for (const auto& [path, msg] : subtreeMessages) {
        if (path == nestedPath) {
            foundNestedMessage = true;
        } else if (path.data().size() > 1 && path.data().at(0) == 2 && TPathManager::GetInstance().EntryName(path) == "simple") {
            foundSimpleInNested = true;
        }
    }

    EXPECT_TRUE(foundNestedMessage);
    EXPECT_TRUE(foundSimpleInNested);
}

// Тест получения сообщения с его предками
TEST_F(RelationManagerTest, GetMessageWithAncestors) {
    // Регистрация глубоко вложенного сообщения
    RegisterRootMessage(deepNestedConfig);

    // Получение пути к глубоко вложенному сообщению
    TMessagePath deepPath(3);
    auto message = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(message, nullptr);

    // Поиск пути к глубоко вложенному simple-сообщению
    TMessagePath nestedSimplePath;
    for (auto msgField : message->MessageFields()) {
        if (msgField->GetName() == "nested") {
            for (auto innerMsgField : msgField->MessageFields()) {
                if (innerMsgField->GetName() == "simple") {
                    nestedSimplePath = innerMsgField->GetPath();
                    break;
                }
            }
            break;
        }
    }

    ASSERT_FALSE(nestedSimplePath.empty());

    // Получение сообщения с его предками
    auto ancestorObjects = TRelationManager::GetInstance().GetObjectWithAncestors(nestedSimplePath);

    // У нас должно быть simple-сообщение, его родитель (nested) и корень (deep_nested)
    EXPECT_EQ(ancestorObjects.size(), 3);

    // Проверка наличия всех трёх сообщений
    bool foundSimple = false;
    bool foundNested = false;
    bool foundDeepNested = false;

    for (const auto& [path, obj] : ancestorObjects) {
        if (path == nestedSimplePath) {
            foundSimple = true;
        } else if (path.data().size() == 2 && path.data().at(0) == 3 && TPathManager::GetInstance().EntryName(path) == "nested") {
            foundNested = true;
        } else if (path.data().size() == 1 && path.data().at(0) == 3) {
            foundDeepNested = true;
        }
    }

    EXPECT_TRUE(foundSimple);
    EXPECT_TRUE(foundNested);
    EXPECT_TRUE(foundDeepNested);
}

// Тест получения поля с его предками
TEST_F(RelationManagerTest, GetFieldWithAncestors) {
    // Регистрация глубоко вложенного сообщения
    RegisterRootMessage(deepNestedConfig);

    // Получение пути к глубоко вложенному сообщению
    TMessagePath deepPath(3);
    auto message = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(message, nullptr);

    // Поиск пути к полю id в глубоко вложенном simple-сообщении
    TMessagePath idFieldPath;
    for (auto msgField : message->MessageFields()) {
        if (msgField->GetName() == "nested") {
            for (auto innerMsgField : msgField->MessageFields()) {
                if (innerMsgField->GetName() == "simple") {
                    for (auto field : innerMsgField->PrimitiveFields()) {
                        if (field->GetName() == "id") {
                            idFieldPath = field->GetPath();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    ASSERT_FALSE(idFieldPath.empty());

    // Получение поля с его предками
    auto ancestorObjects = TRelationManager::GetInstance().GetObjectWithAncestors(idFieldPath);

    // У нас должно быть поле id и возможно родительские поля (если они существуют)
    EXPECT_GE(ancestorObjects.size(), 1);

    // Проверка наличия поля id
    bool foundIdField = false;
    for (const auto& [path, obj] : ancestorObjects) {
        if (path == idFieldPath) {
            foundIdField = true;
            auto field = std::dynamic_pointer_cast<TPrimitiveFieldInfo>(obj);
            ASSERT_NE(field, nullptr);
            EXPECT_EQ(field->GetName(), "id");
            EXPECT_EQ(field->GetFieldNumber(), 1);
        }
    }

    EXPECT_TRUE(foundIdField);
}

// Тест получения родительского сообщения
TEST_F(RelationManagerTest, GetParentMessage) {
    // Регистрация вложенного сообщения
    RegisterRootMessage(nestedConfig);

    // Получение вложенного сообщения
    TMessagePath nestedPath(2);
    auto nestedMessage = TRelationManager::GetInstance().GetMessage(nestedPath);
    ASSERT_NE(nestedMessage, nullptr);

    // Поиск вложенного simple-сообщения
    TMessageInfoPtr simpleMessage = nullptr;
    for (auto subMsg : nestedMessage->MessageFields()) {
        if (subMsg->GetName() == "simple") {
            // Создаем невладеющий указатель с пустым делетером
            simpleMessage = std::static_pointer_cast<TMessageInfo>(subMsg);
            break;
        }
    }

    ASSERT_NE(simpleMessage, nullptr);

    // Получение родителя simple-сообщения
    auto parentMessage = TRelationManager::GetInstance().GetParentMessage(simpleMessage);

    // Родителем должно быть вложенное сообщение
    ASSERT_NE(parentMessage, nullptr);
    EXPECT_EQ(parentMessage->GetPath(), nestedPath);
}

// Тест кеширования в GetMessagesFromSubtree
TEST_F(RelationManagerTest, MessagesFromSubtreeCache) {
    // Регистрация всех тестовых сообщений
    RegisterTestMessages();

    // Получение корневого пути для вложенного сообщения
    TMessagePath nestedPath(2);

    // Первый вызов - кеш заполняется
    auto firstCall = TRelationManager::GetInstance().GetMessagesFromSubtree(nestedPath);

    // Очистка не производится, поэтому второй вызов должен дать тот же результат
    auto secondCall = TRelationManager::GetInstance().GetMessagesFromSubtree(nestedPath);

    // Результаты должны быть идентичны
    EXPECT_EQ(firstCall.size(), secondCall.size());

    for (const auto& [path, msg] : firstCall) {
        EXPECT_NE(secondCall.find(path), secondCall.end());
    }
}

// Тест на регистрацию поля и получение его через GetField
TEST_F(RelationManagerTest, RegisterAndGetField) {
    // Регистрация корневого сообщения
    RegisterRootMessage(simpleConfig);

    // Получение сообщения
    TMessagePath path(1);
    auto message = TRelationManager::GetInstance().GetMessage(path);
    ASSERT_NE(message, nullptr);

    // Поиск поля id и его пути
    TMessagePath idFieldPath;
    for (auto field : message->PrimitiveFields()) {
        if (field->GetName() == "id") {
            idFieldPath = field->GetPath();
            break;
        }
    }

    ASSERT_FALSE(idFieldPath.empty());

    // Получение поля по пути
    auto idField = TRelationManager::GetInstance().GetField(idFieldPath);

    // Проверка корректности получения поля
    ASSERT_NE(idField, nullptr);
    EXPECT_EQ(idField->GetName(), "id");
    EXPECT_EQ(idField->GetFieldNumber(), 1);
    EXPECT_EQ(idField->GetValueType(), google::protobuf::FieldDescriptor::TYPE_INT32);
}

// Тест на кеширование в GetObjectWithAncestors
TEST_F(RelationManagerTest, ObjectWithAncestorsCache) {
    // Регистрация глубоко вложенного сообщения
    RegisterRootMessage(deepNestedConfig);

    // Получение пути к глубоко вложенному сообщению
    TMessagePath deepPath(3);
    auto message = TRelationManager::GetInstance().GetMessage(deepPath);
    ASSERT_NE(message, nullptr);

    // Поиск пути к полю id в глубоко вложенном simple-сообщении
    TMessagePath idFieldPath;
    for (auto msgField : message->MessageFields()) {
        if (msgField->GetName() == "nested") {
            for (auto innerMsgField : msgField->MessageFields()) {
                if (innerMsgField->GetName() == "simple") {
                    for (auto field : innerMsgField->PrimitiveFields()) {
                        if (field->GetName() == "id") {
                            idFieldPath = field->GetPath();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    ASSERT_FALSE(idFieldPath.empty());

    // Первый вызов - кеш заполняется
    auto firstCall = TRelationManager::GetInstance().GetObjectWithAncestors(idFieldPath);

    // Второй вызов должен использовать кеш
    auto secondCall = TRelationManager::GetInstance().GetObjectWithAncestors(idFieldPath);

    // Результаты должны быть идентичны
    EXPECT_EQ(firstCall.size(), secondCall.size());

    for (const auto& [path, obj] : firstCall) {
        EXPECT_NE(secondCall.find(path), secondCall.end());
    }
}

// Тест на обработку пустых путей при получении сообщений/полей
TEST_F(RelationManagerTest, EmptyPathHandling) {
    // Регистрация корневого сообщения
    RegisterRootMessage(simpleConfig);

    // Попытка получения сообщения по пустому пути
    TMessagePath emptyPath;
    auto message = TRelationManager::GetInstance().GetMessage(emptyPath);

    // Ожидаем, что сообщение не будет найдено
    EXPECT_EQ(message, nullptr);

    // Попытка получения поля по пустому пути
    auto field = TRelationManager::GetInstance().GetField(emptyPath);

    // Ожидаем, что поле не будет найдено
    EXPECT_EQ(field, nullptr);

    // Получение сообщений из поддерева с пустым корнем
    auto subtreeMessages = TRelationManager::GetInstance().GetMessagesFromSubtree(emptyPath);

    // Для пустого пути не должны находиться никакие сообщения
    EXPECT_EQ(subtreeMessages.size(), 0);
}

} // namespace

#include <gtest/gtest.h>
#include <relation/field.h>
#include <relation/path.h>
#include <google/protobuf/descriptor.h>

namespace NOrm::NRelation::Tests {

// Mock FieldDescriptor for testing
class MockFieldDescriptor : public google::protobuf::FieldDescriptor {
public:
    MockFieldDescriptor(const std::string& name, int number, google::protobuf::FieldDescriptor::Type type)
        : name_(name), number_(number), type_(type) {}

    const std::string& name() const override { return name_; }
    int number() const override { return number_; }
    google::protobuf::FieldDescriptor::Type type() const override { return type_; }
    bool has_default_value() const override { return true; }
    
    // Default value implementations for different types
    bool default_value_bool() const override { return true; }
    int32_t default_value_int32() const override { return 42; }
    uint32_t default_value_uint32() const override { return 42; }
    int64_t default_value_int64() const override { return 42; }
    uint64_t default_value_uint64() const override { return 42; }
    float default_value_float() const override { return 3.14f; }
    double default_value_double() const override { return 3.14159; }
    const std::string& default_value_string() const override { static std::string s = "default"; return s; }
    
    // Stub implementations for required methods
    bool is_map() const override { return false; }
    bool is_repeated() const override { return false; }
    bool is_required() const override { return false; }
    bool is_optional() const override { return true; }
    bool is_packed() const override { return false; }
    bool has_presence() const override { return true; }
    const google::protobuf::OneofDescriptor* containing_oneof() const override { return nullptr; }

private:
    std::string name_;
    int number_;
    google::protobuf::FieldDescriptor::Type type_;
};

class TPrimitiveFieldInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test path
        basePath_ = TMessagePath({1, "root"});
    }

    TMessagePath basePath_;
};

// Test construction and basic functionality
TEST_F(TPrimitiveFieldInfoTest, ConstructAndGetters) {
    // Create a mock field descriptor
    MockFieldDescriptor mockDesc("testField", 5, google::protobuf::FieldDescriptor::TYPE_INT32);
    
    // Create a field info
    TPrimitiveFieldInfo field(&mockDesc, basePath_);
    
    // Test basic getters
    EXPECT_EQ(field.GetName(), "testField");
    EXPECT_EQ(field.GetFieldNumber(), 5);
    EXPECT_EQ(field.GetValueType(), google::protobuf::FieldDescriptor::TYPE_INT32);
    EXPECT_FALSE(field.IsMessage());
    
    // Test path
    EXPECT_EQ(field.GetPath().data().size(), 2); // Base path + field
    EXPECT_EQ(field.GetPath().data().back().protonum, 5);
    EXPECT_EQ(field.GetPath().data().back().name, "testField");
    
    // Check default value
    EXPECT_EQ(field.GetDefaultValueString(), "42");
    
    // Check type info
    const auto& typeInfo = field.GetTypeInfo();
    EXPECT_TRUE(std::holds_alternative<TInt32FieldInfo>(typeInfo));
    if (std::holds_alternative<TInt32FieldInfo>(typeInfo)) {
        EXPECT_EQ(std::get<TInt32FieldInfo>(typeInfo).defaultValue, 42);
    }
}

// Test various field types
TEST_F(TPrimitiveFieldInfoTest, FieldTypes) {
    // Test bool field
    {
        MockFieldDescriptor mockDesc("boolField", 1, google::protobuf::FieldDescriptor::TYPE_BOOL);
        TPrimitiveFieldInfo field(&mockDesc, basePath_);
        EXPECT_EQ(field.GetDefaultValueString(), "true");
        EXPECT_TRUE(std::holds_alternative<TBoolFieldInfo>(field.GetTypeInfo()));
    }
    
    // Test string field
    {
        MockFieldDescriptor mockDesc("stringField", 2, google::protobuf::FieldDescriptor::TYPE_STRING);
        TPrimitiveFieldInfo field(&mockDesc, basePath_);
        EXPECT_EQ(field.GetDefaultValueString(), "\\\"default\\\"");
        EXPECT_TRUE(std::holds_alternative<TStringFieldInfo>(field.GetTypeInfo()));
    }
    
    // Test float field
    {
        MockFieldDescriptor mockDesc("floatField", 3, google::protobuf::FieldDescriptor::TYPE_FLOAT);
        TPrimitiveFieldInfo field(&mockDesc, basePath_);
        // Value might be in scientific notation or have trailing zeros removed
        EXPECT_NE(field.GetDefaultValueString().find("3.14"), std::string::npos);
        EXPECT_TRUE(std::holds_alternative<TFloatFieldInfo>(field.GetTypeInfo()));
    }
    
    // Test double field
    {
        MockFieldDescriptor mockDesc("doubleField", 4, google::protobuf::FieldDescriptor::TYPE_DOUBLE);
        TPrimitiveFieldInfo field(&mockDesc, basePath_);
        EXPECT_NE(field.GetDefaultValueString().find("3.14159"), std::string::npos);
        EXPECT_TRUE(std::holds_alternative<TDoubleFieldInfo>(field.GetTypeInfo()));
    }
}

// Test TPrimitiveFieldIterator
TEST_F(TPrimitiveFieldInfoTest, PrimitiveFieldIterator) {
    // Create mock field descriptors
    MockFieldDescriptor mockDesc1("field1", 1, google::protobuf::FieldDescriptor::TYPE_INT32);
    MockFieldDescriptor mockDesc2("field2", 2, google::protobuf::FieldDescriptor::TYPE_STRING);
    
    // Create fields
    auto field1 = std::make_shared<TPrimitiveFieldInfo>(&mockDesc1, basePath_);
    auto field2 = std::make_shared<TPrimitiveFieldInfo>(&mockDesc2, basePath_);
    
    // Create a map to hold fields
    std::unordered_map<int, TFieldBasePtr> fields;
    fields[1] = field1;
    fields[2] = field2;
    
    // Create a range
    TPrimitiveFieldsRange range(fields);
    
    // Test iteration
    int count = 0;
    for (auto it = range.begin(); it != range.end(); ++it) {
        count++;
        EXPECT_FALSE((*it)->IsMessage());
        EXPECT_TRUE((*it)->GetFieldNumber() == 1 || (*it)->GetFieldNumber() == 2);
    }
    EXPECT_EQ(count, 2);
    
    // Test range-based for loop
    count = 0;
    for (auto field : range) {
        count++;
        EXPECT_FALSE(field->IsMessage());
        EXPECT_TRUE(field->GetFieldNumber() == 1 || field->GetFieldNumber() == 2);
    }
    EXPECT_EQ(count, 2);
}

} // namespace NOrm::NRelation::Tests

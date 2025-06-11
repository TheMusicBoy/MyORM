#include <gtest/gtest.h>
#include <relation/message.h>
#include <relation/path.h>
#include <google/protobuf/descriptor.h>

namespace NOrm::NRelation::Tests {

// Mock Descriptor classes for testing
class MockFieldDescriptor : public google::protobuf::FieldDescriptor {
public:
    MockFieldDescriptor(const std::string& name, int number, google::protobuf::FieldDescriptor::Type type,
                         bool isMessage = false, const google::protobuf::Descriptor* messageType = nullptr)
        : name_(name), number_(number), type_(type), isMessage_(isMessage), messageType_(messageType) {}

    const std::string& name() const override { return name_; }
    int number() const override { return number_; }
    google::protobuf::FieldDescriptor::Type type() const override { return type_; }
    
    // For message fields
    const google::protobuf::Descriptor* message_type() const override { return messageType_; }
    
    // Required virtual method implementations
    bool is_map() const override { return false; }
    bool is_repeated() const override { return false; }
    bool is_required() const override { return false; }
    bool is_optional() const override { return true; }
    bool is_packed() const override { return false; }
    bool has_presence() const override { return true; }
    bool has_default_value() const override { return false; }
    const google::protobuf::OneofDescriptor* containing_oneof() const override { return nullptr; }

private:
    std::string name_;
    int number_;
    google::protobuf::FieldDescriptor::Type type_;
    bool isMessage_;
    const google::protobuf::Descriptor* messageType_;
};

class MockDescriptor : public google::protobuf::Descriptor {
public:
    MockDescriptor(const std::string& name, const std::vector<const google::protobuf::FieldDescriptor*>& fields)
        : name_(name), fields_(fields) {}

    const std::string& name() const override { return name_; }
    int field_count() const override { return static_cast<int>(fields_.size()); }
    const google::protobuf::FieldDescriptor* field(int index) const override { return fields_[index]; }
    
    // Required virtual method implementations
    const google::protobuf::Descriptor* containing_type() const override { return nullptr; }
    const google::protobuf::FileDescriptor* file() const override { return nullptr; }
    int index() const override { return 0; }

private:
    std::string name_;
    std::vector<const google::protobuf::FieldDescriptor*> fields_;
};

class TMessageInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test path
        basePath_ = TMessagePath({1, "root"});
        
        // Create nested message descriptor
        nestedDesc_ = new MockDescriptor("NestedMessage", {});
        
        // Create field descriptors for the main message
        fields_.push_back(new MockFieldDescriptor("intField", 1, google::protobuf::FieldDescriptor::TYPE_INT32));
        fields_.push_back(new MockFieldDescriptor("stringField", 2, google::protobuf::FieldDescriptor::TYPE_STRING));
        fields_.push_back(new MockFieldDescriptor("messageField", 3, google::protobuf::FieldDescriptor::TYPE_MESSAGE, true, nestedDesc_));
        
        // Create main message descriptor
        messageDesc_ = new MockDescriptor("TestMessage", fields_);
    }

    void TearDown() override {
        // Clean up descriptors
        delete messageDesc_;
        delete nestedDesc_;
        for (auto field : fields_) {
            delete field;
        }
        fields_.clear();
    }

    TMessagePath basePath_;
    MockDescriptor* nestedDesc_;
    std::vector<const google::protobuf::FieldDescriptor*> fields_;
    MockDescriptor* messageDesc_;
};

// Test basic message functionality
TEST_F(TMessageInfoTest, BasicFunctionality) {
    // Create a field message
    TFieldMessage message(fields_[2], basePath_);
    
    // Test basic properties
    EXPECT_TRUE(message.IsMessage());
    EXPECT_EQ(message.GetName(), "messageField");
    EXPECT_EQ(message.GetFieldNumber(), 3);
    EXPECT_EQ(message.GetPath().data().size(), 2); // Base path + field
    EXPECT_EQ(message.GetPath().data().back().name, "messageField");
}

// Test field iterators
TEST_F(TMessageInfoTest, FieldIterators) {
    // Create a message info
    TMessageInfo messageInfo(messageDesc_);
    
    // Test Fields() iterator (all fields)
    int fieldCount = 0;
    for (auto field : messageInfo.Fields()) {
        fieldCount++;
        EXPECT_TRUE(field->GetFieldNumber() >= 1 && field->GetFieldNumber() <= 3);
    }
    EXPECT_EQ(fieldCount, 3);
    
    // Test PrimitiveFields() iterator (non-message fields)
    int primitiveCount = 0;
    for (auto field : messageInfo.PrimitiveFields()) {
        primitiveCount++;
        EXPECT_FALSE(field->IsMessage());
        EXPECT_TRUE(field->GetFieldNumber() == 1 || field->GetFieldNumber() == 2);
    }
    EXPECT_EQ(primitiveCount, 2);
    
    // Test MessageFields() iterator (message fields only)
    int messageCount = 0;
    for (auto field : messageInfo.MessageFields()) {
        messageCount++;
        EXPECT_TRUE(field->IsMessage());
        EXPECT_EQ(field->GetFieldNumber(), 3);
    }
    EXPECT_EQ(messageCount, 1);
}

// Test TFieldMessage specific functionality
TEST_F(TMessageInfoTest, FieldMessage) {
    // Create a field message
    TFieldMessage message(fields_[2], basePath_);
    
    // Check that it extends both TFieldBase and TMessageInfo
    EXPECT_TRUE(message.IsMessage());
    EXPECT_EQ(message.GetName(), "messageField");
    
    // Check that message fields are processed
    // The nested message has no fields, so both iterators should return empty ranges
    int fieldCount = 0;
    for (auto field : message.Fields()) {
        fieldCount++;
    }
    EXPECT_EQ(fieldCount, 0);
}

} // namespace NOrm::NRelation::Tests

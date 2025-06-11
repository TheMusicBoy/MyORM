#include <gtest/gtest.h>
#include <relation/message.h>
#include <relation/config.h>
#include <relation/path.h>

namespace NOrm::NRelation::Tests {

// Mock for Descriptor Pool
class MockDescriptorPool : public google::protobuf::DescriptorPool {
public:
    MockDescriptorPool() {}
    
    void AddMessageType(const std::string& name, const google::protobuf::Descriptor* desc) {
        messageTypes_[name] = desc;
    }
    
    const google::protobuf::Descriptor* FindMessageTypeByName(const std::string& name) const override {
        auto it = messageTypes_.find(name);
        if (it != messageTypes_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
private:
    std::map<std::string, const google::protobuf::Descriptor*> messageTypes_;
};

// Mock Descriptor classes for testing
class MockDescriptor : public google::protobuf::Descriptor {
public:
    MockDescriptor(const std::string& name)
        : name_(name) {}

    const std::string& name() const override { return name_; }
    int field_count() const override { return 0; }
    const google::protobuf::FieldDescriptor* field(int index) const override { return nullptr; }
    
    // Required virtual method implementations
    const google::protobuf::Descriptor* containing_type() const override { return nullptr; }
    const google::protobuf::FileDescriptor* file() const override { return nullptr; }
    int index() const override { return 0; }

private:
    std::string name_;
};

class TRootMessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup mock descriptor pool
        mockPool_ = new MockDescriptorPool();
        mockDescriptor_ = new MockDescriptor("TestMessage");
        mockPool_->AddMessageType("test.TestMessage", mockDescriptor_);
        
        // Replace the global descriptor pool with our mock
        originalPool_ = google::protobuf::DescriptorPool::generated_pool();
        google::protobuf::DescriptorPool::InternalSetGeneratedPool(mockPool_);
        
        // Create a table config
        config_ = NCommon::New<TTableConfig>();
        config_->Number = 100;
        config_->SnakeCase = "test_table";
        config_->CamelCase = "TestTable";
        config_->Scheme = "test.TestMessage";
    }

    void TearDown() override {
        // Restore the original descriptor pool
        google::protobuf::DescriptorPool::InternalSetGeneratedPool(originalPool_);
        
        // Clean up
        delete mockDescriptor_;
        delete mockPool_;
    }

    TTableConfigPtr config_;
    MockDescriptorPool* mockPool_;
    MockDescriptor* mockDescriptor_;
    const google::protobuf::DescriptorPool* originalPool_;
};

// Test TRootBase functionality
TEST_F(TRootMessageTest, RootBase) {
    TRootBase rootBase(config_);
    
    // Check path construction
    EXPECT_FALSE(rootBase.GetPath().empty());
    EXPECT_EQ(rootBase.GetPath().data().size(), 1);
    EXPECT_EQ(rootBase.GetPath().data()[0].protonum, 100);
    EXPECT_EQ(rootBase.GetPath().data()[0].name, "test_table");
    
    // Check descriptor access
    EXPECT_EQ(rootBase.GetDescriptor(), mockDescriptor_);
}

// Test TRootMessage functionality
TEST_F(TRootMessageTest, RootMessage) {
    TRootMessage rootMessage(config_);
    
    // Check path construction
    EXPECT_FALSE(rootMessage.GetPath().empty());
    EXPECT_EQ(rootMessage.GetPath().data().size(), 1);
    EXPECT_EQ(rootMessage.GetPath().data()[0].protonum, 100);
    EXPECT_EQ(rootMessage.GetPath().data()[0].name, "test_table");
    
    // Check table name generation
    EXPECT_EQ(rootMessage.GetTableName(), "t_100");
}

// Test RegisterRootMessage functionality
TEST_F(TRootMessageTest, RegisterRootMessage) {
    // Clear existing registration
    TRelationManager::GetInstance().Clear();
    
    // Register a root message
    RegisterRootMessage(config_);
    
    // Check if it's properly registered
    auto rootPath = TMessagePath({100, "test_table"});
    auto message = TRelationManager::GetInstance().GetMessage(rootPath);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->GetPath(), rootPath);
    
    // Check message by table number
    auto byNumber = TRelationManager::GetInstance().GetMessageByTableNumber(100);
    ASSERT_NE(byNumber, nullptr);
    EXPECT_EQ(byNumber->GetPath(), rootPath);
    
    // Check message by table name
    auto byName = TRelationManager::GetInstance().GetMessageByTableName("t_100");
    ASSERT_NE(byName, nullptr);
    EXPECT_EQ(byName->GetPath(), rootPath);
}

} // namespace NOrm::NRelation::Tests

#include <gtest/gtest.h>
#include <master/service/query/postgresql.h>
#include <master/proto/test_object.pb.h>
#include <master/proto/object_base.pb.h>

class PostgreSQLFormatterTest : public ::testing::Test {
protected:
    void SetUp() override {
        formatter = std::make_unique<NQuery::TPostgreSQLFormatter>();
        
        // Set up test objects
        testObject = std::make_unique<TestObject>();
        
        // Set up test data
        auto* meta = testObject->mutable_meta();
        meta->set_id("test-id-123");
        meta->set_parent_id("parent-id-456");
        meta->set_parent_type("ParentType");
        meta->set_created_at(1623456789);
        meta->set_updated_at(1623456790);
        meta->set_created_by("user1");
        meta->set_updated_by("user2");
        meta->set_version("1.0.0");
        
        auto* spec = testObject->mutable_spec();
        spec->set_name("Test Object");
        spec->set_description("This is a test object");
        spec->set_priority(5);
        spec->add_tags("tag1");
        spec->add_tags("tag2");
        
        auto* config = spec->mutable_config();
        config->set_timeout_seconds(30);
        config->set_enable_notifications(true);
        
        auto* status = testObject->mutable_status();
        status->set_state(TestObjectStatus::ACTIVE);
        status->set_message("Running");
        status->set_last_updated(1623456789);
        status->set_progress_percentage(75);
    }

    std::unique_ptr<NQuery::TPostgreSQLFormatter> formatter;
    std::unique_ptr<TestObject> testObject;
};

// Test table creation SQL generation
TEST_F(PostgreSQLFormatterTest, GenerateCreateTable) {
    auto* descriptor = TestObject::descriptor();
    std::string sql = formatter->GenerateCreateTable(descriptor);
    
    std::cout << sql << std::endl;
    // Check that the SQL contains key elements
    EXPECT_TRUE(sql.find("CREATE TABLE") != std::string::npos);
    EXPECT_TRUE(sql.find("id UUID PRIMARY KEY") != std::string::npos);
    
    // Field types should be present
    EXPECT_TRUE(sql.find("INTEGER") != std::string::npos);
    EXPECT_TRUE(sql.find("TEXT") != std::string::npos);
}

// Test SELECT query generation with various options
TEST_F(PostgreSQLFormatterTest, GenerateSelect) {
    auto* descriptor = TestObject::descriptor();
    std::string sql = formatter->GenerateSelect(descriptor);
    
    // Basic structure
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("SELECT") != std::string::npos);
    EXPECT_TRUE(sql.find("FROM") != std::string::npos);
    
    // With specific fields
    std::vector<std::string> fields = {"id", "name"};
    sql = formatter->GenerateSelect(descriptor, fields);
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("SELECT id, name FROM") != std::string::npos);
    
    // With WHERE clause
    sql = formatter->GenerateSelect(descriptor, {}, "id = 'test-id'");
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("WHERE id = 'test-id'") != std::string::npos);
    
    // With ORDER BY
    sql = formatter->GenerateSelect(descriptor, {}, "", "name ASC");
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("ORDER BY name ASC") != std::string::npos);
    
    // With LIMIT and OFFSET
    sql = formatter->GenerateSelect(descriptor, {}, "", "", 10, 5);
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("LIMIT 10") != std::string::npos);
    EXPECT_TRUE(sql.find("OFFSET 5") != std::string::npos);
}

// Test INSERT query generation
TEST_F(PostgreSQLFormatterTest, GenerateInsert) {
    std::string sql = formatter->GenerateInsert(*testObject);
    
    std::cout << sql << std::endl;
    // Check key elements
    EXPECT_TRUE(sql.find("INSERT INTO") != std::string::npos);
    EXPECT_TRUE(sql.find("VALUES") != std::string::npos);
    EXPECT_TRUE(sql.find("uuid_generate_v4()") != std::string::npos);
    
    // Check test object values
    EXPECT_TRUE(sql.find("'Test Object'") != std::string::npos);
    EXPECT_TRUE(sql.find("'This is a test object'") != std::string::npos);
    EXPECT_TRUE(sql.find("5") != std::string::npos); // priority
}

// Test UPDATE query generation
TEST_F(PostgreSQLFormatterTest, GenerateUpdate) {
    std::string sql = formatter->GenerateUpdate(*testObject, "id = 'test-id'");
    
    std::cout << sql << std::endl;
    // Check key elements
    EXPECT_TRUE(sql.find("UPDATE") != std::string::npos);
    EXPECT_TRUE(sql.find("SET") != std::string::npos);
    EXPECT_TRUE(sql.find("WHERE id = 'test-id'") != std::string::npos);
    
    // Check test object values
    EXPECT_TRUE(sql.find("'Test Object'") != std::string::npos);
    EXPECT_TRUE(sql.find("'This is a test object'") != std::string::npos);
    EXPECT_TRUE(sql.find("5") != std::string::npos); // priority
}

// Test DELETE query generation
TEST_F(PostgreSQLFormatterTest, GenerateDelete) {
    auto* descriptor = TestObject::descriptor();
    std::string sql = formatter->GenerateDelete(descriptor, "id = 'test-id'");
    
    std::cout << sql << std::endl;
    // Check key elements
    EXPECT_TRUE(sql.find("DELETE FROM") != std::string::npos);
    EXPECT_TRUE(sql.find("WHERE id = 'test-id'") != std::string::npos);
}

// Test handling of nested objects
TEST_F(PostgreSQLFormatterTest, NestedObjectHandling) {
    auto* descriptor = TestObject::descriptor();
    std::string createSql = formatter->GenerateCreateTable(descriptor);
    
    std::cout << createSql << std::endl;
    // Check for foreign key relationship
    EXPECT_TRUE(createSql.find("parent_id UUID REFERENCES") != std::string::npos);
    
    // Check nested objects in INSERT
    std::string insertSql = formatter->GenerateInsert(*testObject);
    std::cout << insertSql << std::endl;
    EXPECT_TRUE(insertSql.find("nested_insert_") != std::string::npos);
    EXPECT_TRUE(insertSql.find("parent_id") != std::string::npos);
    
    // Check JOIN in SELECT
    std::string selectSql = formatter->GenerateSelect(descriptor);
    std::cout << selectSql << std::endl;
    EXPECT_TRUE(selectSql.find("LEFT JOIN") != std::string::npos);
    
    // Check nested updates
    std::string updateSql = formatter->GenerateUpdate(*testObject);
    std::cout << updateSql << std::endl;
    EXPECT_TRUE(updateSql.find("Nested updates") != std::string::npos);
}

// Test handling of empty objects
TEST_F(PostgreSQLFormatterTest, EmptyObject) {
    auto emptyObj = std::make_unique<TestObject>();
    std::string sql = formatter->GenerateInsert(*emptyObj);
    
    std::cout << sql << std::endl;
    // Should generate valid SQL with default values
    EXPECT_TRUE(sql.find("INSERT INTO") != std::string::npos);
    EXPECT_TRUE(sql.find("uuid_generate_v4()") != std::string::npos);
    
    // Shouldn't include values from our populated test object
    EXPECT_FALSE(sql.find("'Test Object'") != std::string::npos);
}

// Test SQL injection prevention
TEST_F(PostgreSQLFormatterTest, EscapingSQLInjection) {
    auto* spec = testObject->mutable_spec();
    spec->set_name("Test'; DROP TABLE users; --");
    
    std::string sql = formatter->GenerateInsert(*testObject);
    
    std::cout << sql << std::endl;
    // Check that single quotes are properly escaped
    EXPECT_TRUE(sql.find("'Test''; DROP TABLE users; --'") != std::string::npos);
}

// Test complex query criteria
TEST_F(PostgreSQLFormatterTest, ComplexQueryCriteria) {
    auto* descriptor = TestObject::descriptor();
    
    // Test complex WHERE clause
    std::string sql = formatter->GenerateSelect(
        descriptor, 
        {}, 
        "priority > 3 AND status = 'ACTIVE' OR name LIKE 'Test%'"
    );
    
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("WHERE priority > 3 AND status = 'ACTIVE' OR name LIKE 'Test%'") != std::string::npos);
    
    // Test complex ORDER BY
    sql = formatter->GenerateSelect(
        descriptor, 
        {}, 
        "", 
        "priority DESC, name ASC"
    );
    
    std::cout << sql << std::endl;
    EXPECT_TRUE(sql.find("ORDER BY priority DESC, name ASC") != std::string::npos);
}

// Test table and column name hashing consistency
TEST_F(PostgreSQLFormatterTest, TableAndColumnNaming) {
    auto* descriptor = TestObject::descriptor();
    
    // Ensure consistent hashing
    std::string sql1 = formatter->GenerateCreateTable(descriptor);
    std::string sql2 = formatter->GenerateCreateTable(descriptor);
    std::cout << sql1 << std::endl;
    std::cout << sql2 << std::endl;
    EXPECT_EQ(sql1, sql2);
    
    // Different descriptors should produce different table names
    auto* specDescriptor = TestObjectSpec::descriptor();
    std::string specSql = formatter->GenerateCreateTable(specDescriptor);
    
    std::cout << specSql << std::endl;
    // Extract table names (simplified extraction)
    std::string tableName1 = sql1.substr(sql1.find("CREATE TABLE IF NOT EXISTS ") + 27, 
                                      sql1.find(" (") - sql1.find("CREATE TABLE IF NOT EXISTS ") - 27);
    std::string tableName2 = specSql.substr(specSql.find("CREATE TABLE IF NOT EXISTS ") + 27, 
                                         specSql.find(" (") - specSql.find("CREATE TABLE IF NOT EXISTS ") - 27);
    
    std::cout << tableName1 << std::endl;
    std::cout << tableName2 << std::endl;
    EXPECT_NE(tableName1, tableName2);
}

// Test protocol buffer type mapping to SQL types
TEST_F(PostgreSQLFormatterTest, ProtobufTypeMappingToSQL) {
    auto* descriptor = TestObject::descriptor();
    std::string sql = formatter->GenerateCreateTable(descriptor);
    
    std::cout << sql << std::endl;
    // Check SQL types for different protobuf types
    EXPECT_TRUE(sql.find("INTEGER") != std::string::npos);  // For int32
    EXPECT_TRUE(sql.find("TEXT") != std::string::npos);     // For string
    EXPECT_TRUE(sql.find("BIGINT") != std::string::npos);   // For int64
    EXPECT_TRUE(sql.find("BOOLEAN") != std::string::npos);  // For bool
    
    // Nested objects should be handled either as JSONB or separate tables
    bool jsonbOrSeparateTable = 
        (sql.find("JSONB") != std::string::npos) || 
        (sql.find("parent_id UUID REFERENCES") != std::string::npos);
    EXPECT_TRUE(jsonbOrSeparateTable);
}

// Test handling of repeated fields
TEST_F(PostgreSQLFormatterTest, RepeatedFieldHandling) {
    // The 'tags' field in TestObjectSpec is repeated
    std::string sql = formatter->GenerateInsert(*testObject);
    
    std::cout << sql << std::endl;
    // Check both tags are handled
    EXPECT_TRUE(sql.find("tag1") != std::string::npos);
    EXPECT_TRUE(sql.find("tag2") != std::string::npos);
}

// Test handling of enum fields
TEST_F(PostgreSQLFormatterTest, EnumHandling) {
    // The 'state' field in TestObjectStatus is an enum
    std::string sql = formatter->GenerateInsert(*testObject);
    
    std::cout << sql << std::endl;
    // Enum value ACTIVE (2) should be present
    EXPECT_TRUE(sql.find("2") != std::string::npos);
}

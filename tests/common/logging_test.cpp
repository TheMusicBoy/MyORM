#include <gtest/gtest.h>
#include <common/logging.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace NLogging;

class TestHandler : public THandler {
public:
    void Handle(const TLogEntry& entry) override {
        if (ShouldLog(entry.level)) {
            lastEntry = entry;
            entries.push_back(entry);
            handleCalled = true;
        }
    }
    
    void Reset() {
        entries.clear();
        handleCalled = false;
    }
    
    TLogEntry lastEntry;
    std::vector<TLogEntry> entries;
    bool handleCalled = false;
};

class LoggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        testHandler = std::make_shared<TestHandler>();
        GetLogManager().AddHandler(testHandler);
    }
    
    void TearDown() override {
        GetLogManager().RemoveHandler(testHandler);
        GetLogManager().Flush();
    }
    
    std::string GetTempFileName() {
        return std::filesystem::temp_directory_path() / 
               ("test_log_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log");
    }
    
    std::shared_ptr<TestHandler> testHandler;
};

TEST_F(LoggingTest, LevelToString) {
    EXPECT_EQ("DEBUG", LevelToString(ELevel::Debug));
    EXPECT_EQ("INFO", LevelToString(ELevel::Info));
    EXPECT_EQ("WARNING", LevelToString(ELevel::Warning));
    EXPECT_EQ("ERROR", LevelToString(ELevel::Error));
    EXPECT_EQ("FATAL", LevelToString(ELevel::Fatal));
    
    ELevel unknownLevel = static_cast<ELevel>(999);
    EXPECT_EQ("UNKNOWN", LevelToString(unknownLevel));
}

TEST_F(LoggingTest, HandlerLevelFiltering) {
    auto handler = std::make_shared<TestHandler>();
    
    EXPECT_TRUE(handler->ShouldLog(ELevel::Fatal));
    EXPECT_TRUE(handler->ShouldLog(ELevel::Error));
    EXPECT_TRUE(handler->ShouldLog(ELevel::Warning));
    EXPECT_TRUE(handler->ShouldLog(ELevel::Info));
    EXPECT_FALSE(handler->ShouldLog(ELevel::Debug));
    
    handler->SetLevel(ELevel::Warning);
    EXPECT_TRUE(handler->ShouldLog(ELevel::Fatal));
    EXPECT_TRUE(handler->ShouldLog(ELevel::Error));
    EXPECT_TRUE(handler->ShouldLog(ELevel::Warning));
    EXPECT_FALSE(handler->ShouldLog(ELevel::Info));
    EXPECT_FALSE(handler->ShouldLog(ELevel::Debug));
    
    handler->SetLevel(ELevel::Debug);
    EXPECT_TRUE(handler->ShouldLog(ELevel::Debug));
}

TEST_F(LoggingTest, StreamHandler) {
    std::ostringstream stream;
    auto handler = std::make_shared<TStreamHandler>(stream);
    
    TLogEntry entry;
    entry.level = ELevel::Info;
    entry.source = "TestSource";
    entry.message = "Test message";
    entry.timestamp = std::chrono::system_clock::now();
    
    handler->Handle(entry);
    
    std::string output = stream.str();
    EXPECT_TRUE(output.find("INFO") != std::string::npos);
    EXPECT_TRUE(output.find("TestSource") != std::string::npos);
    EXPECT_TRUE(output.find("Test message") != std::string::npos);
    EXPECT_TRUE(output.find("thread:") != std::string::npos);
    
    stream.str("");
    handler->SetLevel(ELevel::Error);
    
    TLogEntry infoEntry;
    infoEntry.level = ELevel::Info;
    infoEntry.message = "This should be filtered";
    
    handler->Handle(infoEntry);
    EXPECT_TRUE(stream.str().empty());
}

TEST_F(LoggingTest, FileHandler) {
    std::string testFile = GetTempFileName();
    
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    auto handler = std::make_shared<TFileHandler>(testFile);
    
    TLogEntry entry;
    entry.level = ELevel::Info;
    entry.source = "TestSource";
    entry.message = "Test file message";
    entry.timestamp = std::chrono::system_clock::now();
    
    handler->Handle(entry);
    
    EXPECT_TRUE(std::filesystem::exists(testFile));
    
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)), 
                       std::istreambuf_iterator<char>());
    
    EXPECT_TRUE(content.find("INFO") != std::string::npos);
    EXPECT_TRUE(content.find("TestSource") != std::string::npos);
    EXPECT_TRUE(content.find("Test file message") != std::string::npos);
    
    file.close();
    std::filesystem::remove(testFile);
}

TEST_F(LoggingTest, FileHandlerRotation) {
    std::string testFile = GetTempFileName();
    
    for (int i = 0; i <= 3; i++) {
        std::string fileName = testFile + (i == 0 ? "" : "." + std::to_string(i));
        if (std::filesystem::exists(fileName)) {
            std::filesystem::remove(fileName);
        }
    }
    
    auto handler = std::make_shared<TFileHandler>(testFile);
    
    handler->SetMaxFileSize(100);
    handler->SetMaxBackupCount(3);
    
    for (int i = 0; i < 10; i++) {
        TLogEntry entry;
        entry.level = ELevel::Info;
        entry.source = "RotationTest";
        entry.message = "This is a long message to trigger file rotation quickly: " + std::to_string(i);
        entry.timestamp = std::chrono::system_clock::now();
        
        handler->Handle(entry);
    }
    
    EXPECT_TRUE(std::filesystem::exists(testFile));
    EXPECT_TRUE(std::filesystem::exists(testFile + ".1"));
    
    for (int i = 0; i <= 3; i++) {
        std::string fileName = testFile + (i == 0 ? "" : "." + std::to_string(i));
        if (std::filesystem::exists(fileName)) {
            std::filesystem::remove(fileName);
        }
    }
}

TEST_F(LoggingTest, LogManagerSingleton) {
    TLogManager& manager1 = TLogManager::GetInstance();
    TLogManager& manager2 = TLogManager::GetInstance();
    
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(LoggingTest, LogManagerFormatting) {
    testHandler->Reset();
    
    GetLogManager().Info("FormatTest", "Value: {}, String: {}, Bool: {}", 42, "test", true);
    GetLogManager().Flush();
    
    EXPECT_TRUE(testHandler->handleCalled);
    EXPECT_EQ("Value: 42, String: test, Bool: true", testHandler->lastEntry.message);
}

TEST_F(LoggingTest, LogManagerLevels) {
    testHandler->Reset();
    testHandler->SetLevel(ELevel::Debug);
    
    auto& manager = GetLogManager();
    manager.Debug("Source", "Debug message");
    manager.Info("Source", "Info message");
    manager.Warning("Source", "Warning message");
    manager.Error("Source", "Error message");
    manager.Fatal("Source", "Fatal message");
    
    manager.Flush();
    
    ASSERT_EQ(5, testHandler->entries.size());
    EXPECT_EQ(ELevel::Debug, testHandler->entries[0].level);
    EXPECT_EQ(ELevel::Info, testHandler->entries[1].level);
    EXPECT_EQ(ELevel::Warning, testHandler->entries[2].level);
    EXPECT_EQ(ELevel::Error, testHandler->entries[3].level);
    EXPECT_EQ(ELevel::Fatal, testHandler->entries[4].level);
}

TEST_F(LoggingTest, HelperFunctions) {
    auto stdoutHandler = CreateStdoutHandler();
    EXPECT_NE(nullptr, stdoutHandler);
    
    auto stderrHandler = CreateStderrHandler();
    EXPECT_NE(nullptr, stderrHandler);
    
    std::string testFile = GetTempFileName();
    auto fileHandler = CreateFileHandler(testFile);
    EXPECT_NE(nullptr, fileHandler);
    
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
}

TEST_F(LoggingTest, LoggingMacros) {
    testHandler->Reset();
    testHandler->SetLevel(ELevel::Debug);
    
    #undef LoggingSource
    #define LoggingSource "MacroTest"
    
    LOG_DEBUG("Debug from macro");
    LOG_INFO("Info from macro");
    LOG_WARNING("Warning from macro");
    LOG_ERROR("Error from macro");
    LOG_FATAL("Fatal from macro");
    
    GetLogManager().Flush();
    
    ASSERT_EQ(5, testHandler->entries.size());
    EXPECT_EQ(ELevel::Debug, testHandler->entries[0].level);
    EXPECT_EQ("MacroTest", testHandler->entries[0].source);
}

TEST_F(LoggingTest, AddRemoveHandlers) {
    auto customHandler = std::make_shared<TestHandler>();
    GetLogManager().AddHandler(customHandler);
    
    GetLogManager().Info("HandlerTest", "Test message");
    GetLogManager().Flush();
    
    EXPECT_TRUE(testHandler->handleCalled);
    EXPECT_TRUE(customHandler->handleCalled);
    
    GetLogManager().RemoveHandler(customHandler);
    testHandler->Reset();
    customHandler->Reset();
    
    GetLogManager().Info("HandlerTest", "After removal");
    GetLogManager().Flush();
    
    EXPECT_TRUE(testHandler->handleCalled);
    EXPECT_FALSE(customHandler->handleCalled);
}

TEST_F(LoggingTest, LogBuffering) {
    testHandler->Reset();
    
    GetLogManager().Info("BufferTest", "This message should be buffered");
    
    EXPECT_FALSE(testHandler->handleCalled);
    
    GetLogManager().Flush();
    
    EXPECT_TRUE(testHandler->handleCalled);
    EXPECT_EQ("This message should be buffered", testHandler->lastEntry.message);
}

TEST_F(LoggingTest, AutoFlushOnBufferFull) {
    testHandler->Reset();
    
    GetLogManager().SetMaxBufferSize(1);
    
    GetLogManager().Info("AutoFlushTest", "This message should trigger auto-flush");
    
    EXPECT_TRUE(testHandler->handleCalled);
    EXPECT_EQ("This message should trigger auto-flush", testHandler->lastEntry.message);
}

TEST_F(LoggingTest, MaxBufferSizeSetting) {
    testHandler->Reset();
    
    GetLogManager().SetMaxBufferSize(3);
    
    GetLogManager().Info("BufferSizeTest", "Message 1");
    GetLogManager().Info("BufferSizeTest", "Message 2");
    
    EXPECT_FALSE(testHandler->handleCalled);
    
    GetLogManager().Info("BufferSizeTest", "Message 3");
    
    EXPECT_TRUE(testHandler->handleCalled);
    ASSERT_EQ(3, testHandler->entries.size());
    EXPECT_EQ("Message 1", testHandler->entries[0].message);
    EXPECT_EQ("Message 2", testHandler->entries[1].message);
    EXPECT_EQ("Message 3", testHandler->entries[2].message);
}

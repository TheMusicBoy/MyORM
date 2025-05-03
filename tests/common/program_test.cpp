#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/program.h>
#include <common/config.h>
#include <fstream>
#include <thread>
#include <chrono>

// Test config that extends the base program config
class TTestConfig : public NCommon::TProgramConfigBase {
public:
    void Load(const nlohmann::json& data) override {
        NCommon::TProgramConfigBase::Load(data);
        TestValue = NCommon::TConfigBase::Load<std::string>(data, "test_value", "default");
        TestFlag = NCommon::TConfigBase::Load<bool>(data, "test_flag", false);
    }
    
    std::string TestValue;
    bool TestFlag = false;
};

// Concrete implementation of TProgram for testing
class TTestProgram : public NCommon::TProgram<TTestConfig> {
public:
    TTestProgram() : InitializeReturnValue(true), RunCalled(false), InitializeCalled(false) {}

    int Run() override {
        RunCalled = true;
        return ExitCode;
    }

    bool Initialize() override {
        InitializeCalled = true;
        return InitializeReturnValue;
    }

    // Make protected methods public for testing
    using NCommon::TProgram<TTestConfig>::Running_;
    using NCommon::TProgram<TTestConfig>::Options_;
    using NCommon::TProgram<TTestConfig>::Config_;
    
    // Test control variables
    bool InitializeReturnValue;
    bool RunCalled;
    bool InitializeCalled;
    int ExitCode = 0;
};

// Test fixture for TProgram tests
class TProgramTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        TempDir = std::filesystem::temp_directory_path() / "program_test";
        std::filesystem::create_directories(TempDir);
        
        // Create a test config file
        ConfigPath = TempDir / "config.json";
        std::ofstream configFile(ConfigPath);
        configFile << R"({
            "logging": {
                "verbose": true,
                "handlers": [
                    {
                        "type": "console",
                        "level": "debug",
                        "stderr": true
                    }
                ]
            },
            "test_value": "test_setting",
            "test_flag": true
        })";
        configFile.close();

        // Set up log file path
        LogPath = TempDir / "test.log";
    }

    void TearDown() override {
        // Clean up temporary files
        std::filesystem::remove_all(TempDir);
    }

    // Helper to prepare program with arguments
    std::vector<char*> PrepareArgs(const std::vector<std::string>& args) {
        ArgStrings.clear();
        ArgStorage.clear();
        
        // First arg should be program name
        ArgStrings.push_back("test_program");
        
        // Add remaining args
        for (const auto& arg : args) {
            ArgStrings.push_back(arg);
        }
        
        // Convert to char* array (safely)
        for (auto& arg : ArgStrings) {
            ArgStorage.push_back(const_cast<char*>(arg.c_str()));
        }
        
        return ArgStorage;
    }

    std::filesystem::path TempDir;
    std::filesystem::path ConfigPath;
    std::filesystem::path LogPath;
    std::vector<std::string> ArgStrings; // Keep strings alive for the duration of the test
    std::vector<char*> ArgStorage;
};

// Basic execution tests
TEST_F(TProgramTest, BasicExecution) {
    TTestProgram program;
    
    auto args = PrepareArgs({});
    int result = program.Execute(args.size(), args.data());
    
    EXPECT_TRUE(program.InitializeCalled);
    EXPECT_TRUE(program.RunCalled);
    EXPECT_EQ(0, result);
}

TEST_F(TProgramTest, FailedInitialization) {
    TTestProgram program;
    program.InitializeReturnValue = false;
    
    auto args = PrepareArgs({});
    int result = program.Execute(args.size(), args.data());
    
    EXPECT_TRUE(program.InitializeCalled);
    EXPECT_FALSE(program.RunCalled);
    EXPECT_EQ(1, result); // Should return error code
}

TEST_F(TProgramTest, CustomExitCode) {
    TTestProgram program;
    program.ExitCode = 42;
    
    auto args = PrepareArgs({});
    int result = program.Execute(args.size(), args.data());
    
    EXPECT_TRUE(program.RunCalled);
    EXPECT_EQ(42, result);
}

// Command line option tests
TEST_F(TProgramTest, HelpOption) {
    TTestProgram program;
    
    auto args = PrepareArgs({"-h"});
    int result = program.Execute(args.size(), args.data());
    
    // Help should exit early with 0
    EXPECT_FALSE(program.InitializeCalled);
    EXPECT_FALSE(program.RunCalled);
    EXPECT_EQ(0, result);
}

TEST_F(TProgramTest, ConfigFileOption) {
    TTestProgram program;
    
    auto args = PrepareArgs({"-c", ConfigPath.string()});
    program.Execute(args.size(), args.data());
    
    EXPECT_EQ("test_setting", program.GetConfig()->TestValue);
    EXPECT_TRUE(program.GetConfig()->TestFlag);
}

TEST_F(TProgramTest, LogFileOption) {
    TTestProgram program;
    
    auto args = PrepareArgs({"-l", LogPath.string()});
    program.Execute(args.size(), args.data());
    
    // Need to write to log to ensure file creation
    LOG_INFO("Test log message");
    NLogging::GetLogManager().Flush();
    
    EXPECT_TRUE(std::filesystem::exists(LogPath));
}

// Config loading tests
TEST_F(TProgramTest, DefaultConfig) {
    TTestProgram program;
    
    auto args = PrepareArgs({});
    program.Execute(args.size(), args.data());
    
    EXPECT_EQ("default", program.GetConfig()->TestValue);
    EXPECT_FALSE(program.GetConfig()->TestFlag);
}

TEST_F(TProgramTest, NonexistentConfig) {
    TTestProgram program;
    
    auto args = PrepareArgs({"-c", "nonexistent.json"});
    program.Execute(args.size(), args.data());
    
    // Should fall back to default
    EXPECT_EQ("default", program.GetConfig()->TestValue);
}

TEST_F(TProgramTest, InvalidConfigFormat) {
    // Create invalid JSON
    std::ofstream badConfig(ConfigPath);
    badConfig << "{ This is not valid JSON }";
    badConfig.close();
    
    TTestProgram program;
    auto args = PrepareArgs({"-c", ConfigPath.string()});
    program.Execute(args.size(), args.data());
    
    // Should fall back to default
    EXPECT_EQ("default", program.GetConfig()->TestValue);
}

// Path tests
TEST_F(TProgramTest, GetConfigPath) {
    TTestProgram program;
    
    auto args = PrepareArgs({"-c", ConfigPath.string()});
    program.Execute(args.size(), args.data());
    
    EXPECT_EQ(ConfigPath.string(), program.GetConfigPath());
}

TEST_F(TProgramTest, GetWorkingDirMatches) {
    TTestProgram program;
    auto workingDir = program.GetWorkingDir();
    auto currentDir = std::filesystem::current_path();
    
    EXPECT_EQ(currentDir, workingDir);
}

// Logging configuration tests
TEST_F(TProgramTest, ConfigureLoggingFromConfig) {
    // Create config with file logger
    std::ofstream configFile(ConfigPath);
    configFile << R"({
        "logging": {
            "verbose": true,
            "handlers": [
                {
                    "type": "file",
                    "level": "debug",
                    "file": ")" + LogPath.string() + R"("
                }
            ]
        }
    })";
    configFile.close();
    
    TTestProgram program;
    auto args = PrepareArgs({"-c", ConfigPath.string()});
    program.Execute(args.size(), args.data());
    
    // Write to log to ensure file creation
    LOG_INFO("Test message for config-based logging");
    NLogging::GetLogManager().Flush();
    
    EXPECT_TRUE(std::filesystem::exists(LogPath));
}

// Signal handling test (simulate interruption)
TEST_F(TProgramTest, OnInterrupt) {
    TTestProgram program;
    auto args = PrepareArgs({});
    
    // Start the program in a thread
    std::thread programThread([&]() {
        program.Execute(args.size(), args.data());
    });
    
    // Give it time to start up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Trigger interrupt
    program.OnInterrupt();
    
    // Wait for completion
    programThread.join();
    
    // Check running flag was set to false
    EXPECT_FALSE(program.Running_);
}

// Custom command line option test
TEST_F(TProgramTest, CustomOption) {
    TTestProgram program;
    program.AddOption('t', "test", "Test option", true);
    
    auto args = PrepareArgs({"-t", "testvalue"});
    program.Execute(args.size(), args.data());
    
    EXPECT_TRUE(program.Options_.Has('t'));
    EXPECT_EQ("testvalue", program.Options_.Get('t'));
}

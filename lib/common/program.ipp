#pragma once

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

template <typename TConfig>
TProgram<TConfig>::TProgram() 
    : Running_(false)
{
    // Create thread pool and invoker
    ThreadPool_ = New<TThreadPool>(std::thread::hardware_concurrency());
    Invoker_ = New<TInvoker>(ThreadPool_);
    
    // Add common command line options
    AddOption('c', "config", "Path to config file", true);
    AddOption('l', "log", "Log file path", true);
    AddOption('v', "verbose", "Enable verbose logging");
    AddOption('h', "help", "Show this help message");
}

template <typename TConfig>
TProgram<TConfig>::~TProgram() {
    StopPeriodicFlush();
    OnShutdown();
}

template <typename TConfig>
int TProgram<TConfig>::Execute(int argc, char** argv) {
    try {
        // Parse command line options
        Options_.Parse(argc, const_cast<const char* const*>(argv));
        
        if (Options_.Has('h')) {
            std::cout << Options_.Help() << std::endl;
            return 0;
        }
        
        // Setup logging
        SetupDefaultLogging();
        if (!InitLogging()) {
            std::cerr << "Failed to initialize logging" << std::endl;
            return 1;
        }
        
        // Load configuration
        if (!LoadConfig()) {
            LOG_ERROR("Failed to load configuration");
            return 1;
        }
        
        // Configure logging from config
        ConfigureLoggingFromConfig();
        
        // Initialize the program
        if (!Initialize()) {
            LOG_ERROR("Program initialization failed");
            return 1;
        }
        
        // Setup signal handlers
        SetupSignalHandlers();
        
        // Start periodic log flushing
        StartPeriodicFlush();
        
        // Set running flag
        Running_.store(true);
        
        // Run the program
        LOG_INFO("Program starting");
        return Run();
    } catch (const std::exception& ex) {
        OnFailure(ex);
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
        return 1;
    }
}

template <typename TConfig>
bool TProgram<TConfig>::Initialize() {
    // Base implementation - can be overridden by derived classes
    return true;
}

template <typename TConfig>
void TProgram<TConfig>::OnFailure(const std::exception& ex) {
    LOG_ERROR("Program failure: {}", ex.what());
}

template <typename TConfig>
void TProgram<TConfig>::OnInterrupt() {
    LOG_INFO("Program interrupted");
    Running_.store(false);
}

template <typename TConfig>
void TProgram<TConfig>::OnShutdown() {
    LOG_INFO("Program shutting down");
    NLogging::GetLogManager().Flush();
}

template <typename TConfig>
void TProgram<TConfig>::SetupSignalHandlers() {
    TProgramSignalHandlerBase::SetupSignalHandlers();
}

template <typename TConfig>
void TProgram<TConfig>::AddOption(char shortName, const std::string& longName, 
                                const std::string& description, bool requiresArgument) {
    Options_.AddOption(shortName, longName, description, requiresArgument);
}

template <typename TConfig>
bool TProgram<TConfig>::InitLogging() {
    // Basic logging initialization from command line
    if (Options_.Has('l')) {
        AddLogFile(Options_.Get('l'));
    }
    
    if (Options_.Has('v')) {
        auto handler = NLogging::CreateStderrHandler();
        handler->SetLevel(NLogging::ELevel::Debug);
        NLogging::GetLogManager().AddHandler(handler);
    }
    
    return true;
}

template <typename TConfig>
void TProgram<TConfig>::ConfigureLoggingFromConfig() {
    if (!Config_ || !Config_->Logging) {
        return;
    }
    
    auto& logging = *Config_->Logging;
    
    if (logging.Verbose) {
        auto handler = NLogging::CreateStderrHandler();
        handler->SetLevel(NLogging::ELevel::Debug);
        NLogging::GetLogManager().AddHandler(handler);
    }
    
    for (const auto& handlerConfig : logging.Handlers) {
        if (handlerConfig->Type == NLogging::EHandlerType::File) {
            auto fileConfig = NLogging::IntrusivePtrCast<NLogging::TFileHandlerConfig>(handlerConfig);
            auto fileHandler = NLogging::CreateFileHandler(fileConfig->FilePath);
            fileHandler->SetLevel(fileConfig->Level);
            
            auto typedFileHandler = std::static_pointer_cast<NLogging::TFileHandler>(fileHandler);
            typedFileHandler->SetMaxFileSize(fileConfig->MaxFileSize);
            typedFileHandler->SetMaxBackupCount(fileConfig->MaxBackupCount);
            
            NLogging::GetLogManager().AddHandler(fileHandler);
            
            LOG_INFO("Added log file handler: {}", fileConfig->FilePath);
        } else {
            auto consoleConfig = NLogging::IntrusivePtrCast<NLogging::TConsoleHandlerConfig>(handlerConfig);
            std::shared_ptr<NLogging::THandler> consoleHandler;
            
            if (consoleConfig->UseStderr) {
                consoleHandler = NLogging::CreateStderrHandler();
            } else {
                consoleHandler = NLogging::CreateStdoutHandler();
            }
            
            consoleHandler->SetLevel(consoleConfig->Level);
            NLogging::GetLogManager().AddHandler(consoleHandler);
            
            LOG_INFO("Added console log handler ({})",
                    consoleConfig->UseStderr ? "stderr" : "stdout");
        }
    }
}

template <typename TConfig>
bool TProgram<TConfig>::LoadConfig() {
    std::string configPath;
    
    if (Options_.Has('c')) {
        configPath = Options_.Get('c');
    } else {
        // Try default locations
        std::vector<std::string> defaultPaths = {
            "config.json",
            GetWorkingDir().string() + "/config.json",
            GetExecutablePath().parent_path().string() + "/config.json"
        };
        
        for (const auto& path : defaultPaths) {
            if (std::filesystem::exists(path)) {
                configPath = path;
                break;
            }
        }
    }
    
    if (!configPath.empty()) {
        try {
            Config_ = New<TConfig>();
            Config_->LoadFromFile(configPath);
            LOG_INFO("Loaded configuration from {}", configPath);
            return true;
        } catch (const std::exception& ex) {
            LOG_ERROR("Failed to load config from {}: {}", configPath, ex.what());
            // Create a default config when loading fails
            Config_ = New<TConfig>();
            Config_->Load(nlohmann::json::object());
            LOG_WARNING("Using default configuration due to load error");
            return true; // Changed to true to continue with defaults
        }
    }
    
    // If no config file is found, create an empty one
    Config_ = New<TConfig>();
    // Initialize with empty JSON object to trigger default values
    Config_->Load(nlohmann::json::object());
    LOG_WARNING("No configuration file found, using defaults");
    return true;
}

template <typename TConfig>
GetOpts& TProgram<TConfig>::GetOptions() {
    return Options_;
}

template <typename TConfig>
TIntrusivePtr<TConfig> TProgram<TConfig>::GetConfig() const {
    return Config_;
}

template <typename TConfig>
std::string TProgram<TConfig>::GetConfigPath() const {
    if (Options_.Has('c')) {
        return Options_.Get('c');
    }
    return "";
}

template <typename TConfig>
std::filesystem::path TProgram<TConfig>::GetExecutablePath() const {
    // Implementation depends on the platform
    #ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::filesystem::path(path);
    #else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::filesystem::path(std::string(result, (count > 0) ? count : 0));
    #endif
}

template <typename TConfig>
std::filesystem::path TProgram<TConfig>::GetWorkingDir() const {
    return std::filesystem::current_path();
}

template <typename TConfig>
void TProgram<TConfig>::StartPeriodicFlush(std::chrono::milliseconds interval) {
    if (FlushExecutor_) {
        StopPeriodicFlush();
    }
    
    auto flushFunction = []() {
        NLogging::GetLogManager().Flush();
        return false; // Continue running
    };
    
    FlushExecutor_ = New<TPeriodicExecutor>(flushFunction, Invoker_, interval);
    FlushExecutor_->Start();
    
    LOG_DEBUG("Started periodic log flushing with interval {} ms", interval.count());
}

template <typename TConfig>
void TProgram<TConfig>::StopPeriodicFlush() {
    if (FlushExecutor_) {
        FlushExecutor_->Stop();
        FlushExecutor_.reset();
        
        // Final flush
        NLogging::GetLogManager().Flush();
        LOG_DEBUG("Stopped periodic log flushing");
    }
}

template <typename TConfig>
void TProgram<TConfig>::SetupDefaultLogging() {
    auto& logManager = NLogging::GetLogManager();
    
    // Clear any existing handlers and add default ones
    // Note: This is simplified since there's no clear method in the provided API
    auto stderrHandler = NLogging::CreateStderrHandler();
    logManager.AddHandler(stderrHandler);
}

template <typename TConfig>
void TProgram<TConfig>::AddLogFile(const std::string& filename, NLogging::ELevel level) {
    auto fileHandler = NLogging::CreateFileHandler(filename);
    fileHandler->SetLevel(level);
    NLogging::GetLogManager().AddHandler(fileHandler);
    LOG_INFO("Added log file: {}", filename);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

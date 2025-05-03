#pragma once

#include <common/config.h>
#include <string>
#include <vector>

namespace NLogging {

enum class EHandlerType {
    Console,
    File
};

enum class ELevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

std::string LevelToString(ELevel level);

class TLogHandlerConfig : public NCommon::TConfigBase {
public:
    void Load(const nlohmann::json& data) override {
        std::string levelStr = NCommon::TConfigBase::Load<std::string>(data, "level", "info");
        
        // Convert string to ELevel
        if (levelStr == "debug") Level = ELevel::Debug;
        else if (levelStr == "info") Level = ELevel::Info;
        else if (levelStr == "warning") Level = ELevel::Warning;
        else if (levelStr == "error") Level = ELevel::Error;
        else if (levelStr == "fatal") Level = ELevel::Fatal;
        else Level = ELevel::Info;
        
        // Load handler type
        std::string typeStr = NCommon::TConfigBase::Load<std::string>(data, "type", "console");
        if (typeStr == "file") Type = EHandlerType::File;
        else Type = EHandlerType::Console;
    }

    ELevel Level = ELevel::Info;
    EHandlerType Type = EHandlerType::Console;
};

class TConsoleHandlerConfig : public TLogHandlerConfig {
public:
    void Load(const nlohmann::json& data) override {
        TLogHandlerConfig::Load(data);
        
        UseStderr = NCommon::TConfigBase::Load<bool>(data, "stderr", false);
    }

    bool UseStderr = false;
};

class TFileHandlerConfig : public TLogHandlerConfig {
public:
    void Load(const nlohmann::json& data) override {
        TLogHandlerConfig::Load(data);
        
        FilePath = NCommon::TConfigBase::LoadRequired<std::string>(data, "file");
        MaxFileSize = NCommon::TConfigBase::Load<size_t>(data, "max_size", 10 * 1024 * 1024);
        MaxBackupCount = NCommon::TConfigBase::Load<size_t>(data, "max_backups", 5);
    }

    std::string FilePath;
    size_t MaxFileSize = 10 * 1024 * 1024;
    size_t MaxBackupCount = 5;
};

// Helper function to cast between TIntrusivePtr types
template <typename TBase, typename TDerived>
NCommon::TIntrusivePtr<TBase> IntrusivePtrCast(const NCommon::TIntrusivePtr<TDerived>& ptr) {
    // This creates a new TIntrusivePtr<TBase> pointing to the same object
    // The static_cast is safe because TDerived inherits from TBase
    return NCommon::TIntrusivePtr<TBase>(static_cast<TBase*>(ptr.operator->()));
}

class TLoggingConfig : public NCommon::TConfigBase {
public:
    void Load(const nlohmann::json& data) override {
        Verbose = NCommon::TConfigBase::Load<bool>(data, "verbose", false);
        
        if (data.contains("handlers") && data["handlers"].is_array()) {
            for (const auto& handlerData : data["handlers"]) {
                std::string typeStr = handlerData.value("type", "console");
                
                if (typeStr == "file") {
                    auto fileHandler = NCommon::New<TFileHandlerConfig>();
                    fileHandler->Load(handlerData);
                    Handlers.push_back(IntrusivePtrCast<TLogHandlerConfig>(fileHandler));
                } else {
                    auto consoleHandler = NCommon::New<TConsoleHandlerConfig>();
                    consoleHandler->Load(handlerData);
                    Handlers.push_back(IntrusivePtrCast<TLogHandlerConfig>(consoleHandler));
                }
            }
        } else {
            auto defaultConsole = NCommon::New<TConsoleHandlerConfig>();
            Handlers.push_back(IntrusivePtrCast<TLogHandlerConfig>(defaultConsole));
            
            if (data.contains("file") && !data["file"].get<std::string>().empty()) {
                auto fileHandler = NCommon::New<TFileHandlerConfig>();
                fileHandler->FilePath = data["file"].get<std::string>();
                fileHandler->Type = EHandlerType::File;
                if (data.contains("max_file_size")) {
                    fileHandler->MaxFileSize = data["max_file_size"].get<size_t>();
                }
                if (data.contains("max_backup_count")) {
                    fileHandler->MaxBackupCount = data["max_backup_count"].get<size_t>();
                }
                Handlers.push_back(IntrusivePtrCast<TLogHandlerConfig>(fileHandler));
            }
        }
    }

    bool Verbose = false;
    std::vector<NCommon::TIntrusivePtr<TLogHandlerConfig>> Handlers;
};

} // namespace NLogging

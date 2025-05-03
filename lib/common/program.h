#pragma once

#include <common/logging.h>
#include <common/getopts.h>
#include <common/config.h>
#include <common/periodic_executor.h>
#include <common/threadpool.h>
#include <common/intrusive_ptr.h>
#include <common/refcounted.h>

#include <string>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>
#include <filesystem>
#include <signal.h>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TProgramConfigBase
    : public TConfigBase {
public:
    void Load(const nlohmann::json& data) override {
        Logging = NCommon::TConfigBase::Load<NLogging::TLoggingConfig>(data, "logging");
    }

    TIntrusivePtr<NLogging::TLoggingConfig> Logging;
};

class TProgramSignalHandlerBase {
public:
    TProgramSignalHandlerBase();
    virtual ~TProgramSignalHandlerBase();

    static void SetupSignalHandlers();
    static void SignalHandler(int signal);
    
    virtual void OnInterrupt() = 0;

protected:
    static TProgramSignalHandlerBase* Instance_;
};

template <typename TConfig = TProgramConfigBase>
class TProgram 
    : public NRefCounted::TRefCountedBase
    , public TProgramSignalHandlerBase
{
public:
    TProgram();
    virtual ~TProgram();

    int Execute(int argc, char** argv);

    virtual bool Initialize();
    virtual int Run() = 0;
    virtual void OnFailure(const std::exception& ex);
    void OnInterrupt() override;
    virtual void OnShutdown();

    static void SetupSignalHandlers();

    void AddOption(char shortName, const std::string& longName, 
                  const std::string& description, bool requiresArgument = false);
    bool InitLogging();
    bool LoadConfig();
    
    GetOpts& GetOptions();
    TIntrusivePtr<TConfig> GetConfig() const;
    std::string GetConfigPath() const;
    std::filesystem::path GetExecutablePath() const;
    std::filesystem::path GetWorkingDir() const;

protected:
    void StartPeriodicFlush(std::chrono::milliseconds interval = std::chrono::seconds(5));
    void StopPeriodicFlush();

    void SetupDefaultLogging();
    void AddLogFile(const std::string& filename, NLogging::ELevel level = NLogging::ELevel::Info);
    void ConfigureLoggingFromConfig();
    
    std::atomic<bool> Running_;
    
    GetOpts Options_;
    TIntrusivePtr<TConfig> Config_;
    TIntrusivePtr<TThreadPool> ThreadPool_;
    TIntrusivePtr<TInvoker> Invoker_;
    TIntrusivePtr<TPeriodicExecutor> FlushExecutor_;
};

#define DECLARE_PROGRAM_PTR(config_type) \
    using config_type ## ProgramPtr = ::NCommon::TIntrusivePtr<TProgram<config_type>>;

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

#include "program.ipp"

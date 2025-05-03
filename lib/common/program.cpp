#include <common/logging_config.h>
#include <common/logging.h>
#include <common/program.h>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

// Initialize static member
TProgramSignalHandlerBase* TProgramSignalHandlerBase::Instance_ = nullptr;

TProgramSignalHandlerBase::TProgramSignalHandlerBase() {
    Instance_ = this;
}

TProgramSignalHandlerBase::~TProgramSignalHandlerBase() {
    if (Instance_ == this) {
        Instance_ = nullptr;
    }
}

void TProgramSignalHandlerBase::SetupSignalHandlers() {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
}

void TProgramSignalHandlerBase::SignalHandler(int signal) {
    if (Instance_) {
        if (signal == SIGINT || signal == SIGTERM) {
            Instance_->OnInterrupt();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

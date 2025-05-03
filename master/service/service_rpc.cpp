#include <ui/service_rpc.h>

////////////////////////////////////////////////////////////////////////////////

void TRpcServer::Setup(NService::TServicePtr service) {
    RegisterHandler("GET", "/list/raw", NRpc::MakeHandler(&NService::TService::HandleRawReadings, MakeWeak(&*service)));
    RegisterHandler("GET", "/list/hour", NRpc::MakeHandler(&NService::TService::HandleHourlyAverages, MakeWeak(&*service)));
    RegisterHandler("GET", "/list/day", NRpc::MakeHandler(&NService::TService::HandleDailyAverages, MakeWeak(&*service)));
}

////////////////////////////////////////////////////////////////////////////////

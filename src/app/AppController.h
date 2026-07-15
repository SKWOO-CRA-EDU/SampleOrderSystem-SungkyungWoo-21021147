// Controller — Model 조작과 View(IOutputPort) 호출을 조율만 한다. iostream을 직접 호출하지 않는다(CLAUDE.md §2).
#pragma once

#include <cstdint>
#include <string>

#include "../model/OrderService.h"
#include "../model/SampleService.h"
#include "../ports/IOutputPort.h"

namespace app {

class AppController {
public:
    AppController(model::SampleService& sampleService, model::OrderService& orderService,
                   ports::IOutputPort& output)
        : sampleService_(sampleService), orderService_(orderService), output_(output) {}

    void RegisterSample(const std::string& sampleId, const std::string& name, double avgProductionTime,
                         int yieldNumerator);
    void ListSamples();
    void SearchSamples(const std::string& query);
    void DeleteSample(const std::string& sampleId);

    void ReserveOrder(const std::string& sampleId, const std::string& customerName, int64_t orderQuantity);
    void PendingOrderList();
    void ApproveOrder(int64_t orderId);
    void RejectOrder(int64_t orderId);
    void CancelConfirmedOrder(int64_t orderId);
    void ReleaseOrder(int64_t orderId);
    void CompleteProduction();
    void ProductionStatusDisplay();
    void CheckOrderVolume();
    void CheckStockLevel();

private:
    model::SampleService& sampleService_;
    model::OrderService& orderService_;
    ports::IOutputPort& output_;
};

}  // namespace app

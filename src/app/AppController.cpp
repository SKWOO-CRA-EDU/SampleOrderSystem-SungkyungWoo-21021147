#include "AppController.h"

namespace app {

SampleSummary AppController::GetSampleSummary() const {
    auto samples = sampleService_.ListSamples();
    int64_t totalStockQuantity = 0;
    for (const auto& sample : samples) {
        totalStockQuantity += sample.stockQuantity;
    }
    return SampleSummary{samples.size(), totalStockQuantity};
}

void AppController::RegisterSample(const std::string& sampleId, const std::string& name, double avgProductionTime,
                                    int yieldNumerator) {
    auto result = sampleService_.RegisterSample(sampleId, name, avgProductionTime, yieldNumerator);
    if (!result.ok) {
        output_.ShowError(result.code, sampleId);
        return;
    }
    output_.ShowSampleRegistered(result.sample);
}

void AppController::ListSamples() { output_.ShowSampleList(sampleService_.ListSamples()); }

void AppController::SearchSamples(const std::string& query) { output_.ShowSampleList(sampleService_.SearchSamples(query)); }

void AppController::DeleteSample(const std::string& sampleId) {
    auto result = sampleService_.DeleteSample(sampleId);
    if (!result.ok) {
        output_.ShowError(result.code, sampleId);
        return;
    }
    output_.ShowSampleDeleted(sampleId);
}

void AppController::ReserveOrder(const std::string& sampleId, const std::string& customerName,
                                  int64_t orderQuantity) {
    auto result = orderService_.ReserveOrder(sampleId, customerName, orderQuantity);
    if (!result.ok) {
        output_.ShowError(result.code, sampleId);
        return;
    }
    output_.ShowOrderReserved(result.order);
}

void AppController::PendingOrderList() { output_.ShowPendingOrders(orderService_.PendingOrderList()); }

void AppController::ApproveOrder(int64_t orderId) {
    auto result = orderService_.ApproveOrder(orderId);
    if (!result.ok) {
        output_.ShowError(result.code, std::to_string(orderId));
        return;
    }
    output_.ShowOrderTransitioned(result.order);
}

void AppController::RejectOrder(int64_t orderId) {
    auto result = orderService_.RejectOrder(orderId);
    if (!result.ok) {
        output_.ShowError(result.code, std::to_string(orderId));
        return;
    }
    output_.ShowOrderTransitioned(result.order);
}

void AppController::CancelConfirmedOrder(int64_t orderId) {
    auto result = orderService_.CancelConfirmedOrder(orderId);
    if (!result.ok) {
        output_.ShowError(result.code, std::to_string(orderId));
        return;
    }
    output_.ShowOrderTransitioned(result.order);
}

void AppController::ReleaseOrder(int64_t orderId) {
    auto result = orderService_.ReleaseOrder(orderId);
    if (!result.ok) {
        output_.ShowError(result.code, std::to_string(orderId));
        return;
    }
    output_.ShowOrderTransitioned(result.order);
}

void AppController::CompleteProduction() {
    auto result = orderService_.CompleteProduction();
    if (!result.ok) {
        output_.ShowError(result.code, "");
        return;
    }
    output_.ShowProductionCompleted(result.order, result.actualProductionQuantity, result.stockReflected);
}

void AppController::ProductionStatusDisplay() { output_.ShowProductionQueue(orderService_.ProductionStatusDisplay()); }

void AppController::CheckOrderVolume() {
    for (const auto& [status, orders] : orderService_.CheckOrderVolume()) {
        output_.ShowOrdersByStatus(status, orders);
    }
}

void AppController::CheckStockLevel() { output_.ShowStockLevels(orderService_.CheckStockLevel()); }

}  // namespace app

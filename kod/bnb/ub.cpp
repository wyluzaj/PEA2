#include "ub.h"

#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/utils.h"
#include "../common/config.h"

UpperBoundResult computeInitialUpperBoundNN(const TSPInstance& instance) {
    if (!instance.isValid()) {
        throw std::runtime_error("Niepoprawna instancja w computeInitialUpperBoundNN.");
    }

    UpperBoundResult result;

    auto dummyStart = std::chrono::high_resolution_clock::now();

    std::vector<int> nnPath = nearestNeighborPath(
            instance,
            Config::START_VERTEX,
            dummyStart,
            std::numeric_limits<double>::max(),
            &result.finished,
            &result.timeoutReached
    );

    result.path = nnPath;

    if (nnPath.size() == static_cast<size_t>(instance.dimension)) {
        result.cost = calculatePathCost(instance, nnPath);
    } else {
        result.cost = std::numeric_limits<int>::max() / 4;
    }

    return result;
}
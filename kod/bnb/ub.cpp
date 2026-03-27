#include "ub.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/utils.h"

namespace {
    std::vector<int> rotateCycleToStartVertex(
            const std::vector<int>& path
    ) {
        if (path.empty()) {
            return path;
        }

        auto it = std::find(path.begin(), path.end(), Config::START_VERTEX);
        if (it == path.end()) {
            return path;
        }

        std::vector<int> rotated;
        rotated.reserve(path.size());

        rotated.insert(rotated.end(), it, path.end());
        rotated.insert(rotated.end(), path.begin(), it);

        return rotated;
    }
}

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

UpperBoundResult computeInitialUpperBoundRNN(const TSPInstance& instance) {
    if (!instance.isValid()) {
        throw std::runtime_error("Niepoprawna instancja w computeInitialUpperBoundRNN.");
    }

    UpperBoundResult bestResult;
    bestResult.cost = std::numeric_limits<int>::max() / 4;
    bestResult.finished = true;
    bestResult.timeoutReached = false;

    auto dummyStart = std::chrono::high_resolution_clock::now();

    for (int startVertex = 0; startVertex < instance.dimension; ++startVertex) {
        bool finished = false;
        bool timeoutReached = false;

        std::vector<int> path = nearestNeighborPath(
                instance,
                startVertex,
                dummyStart,
                std::numeric_limits<double>::max(),
                &finished,
                &timeoutReached
        );

        if (timeoutReached || !finished) {
            continue;
        }

        if (path.size() != static_cast<size_t>(instance.dimension)) {
            continue;
        }

        const int cost = calculatePathCost(instance, path);

        if (cost < bestResult.cost) {
            bestResult.cost = cost;
            bestResult.path = rotateCycleToStartVertex(path);
            bestResult.finished = true;
            bestResult.timeoutReached = false;
        }
    }

    if (bestResult.path.empty()) {
        bestResult.finished = false;
        bestResult.timeoutReached = false;
        bestResult.cost = std::numeric_limits<int>::max() / 4;
    }

    return bestResult;
}
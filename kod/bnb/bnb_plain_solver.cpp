#include "bnb_plain_solver.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/utils.h"
#include "bnb_node.h"
#include "lb.h"   // tylko po to, zeby domknac cykl computeCompletionCost(...)
#include "ub.h"

namespace {
    std::vector<int> materializePath(const std::vector<BnBNode>& nodePool, int nodeId) {
        std::vector<int> reversedPath;

        while (nodeId != -1) {
            reversedPath.push_back(nodePool[nodeId].current_vertex);
            nodeId = nodePool[nodeId].parent_index;
        }

        std::reverse(reversedPath.begin(), reversedPath.end());
        return reversedPath;
    }

    BnBNode createRootNode(const TSPInstance& instance) {
        BnBNode root;
        root.visited.assign(instance.dimension, false);
        root.visited[Config::START_VERTEX] = true;
        root.parent_index = -1;
        root.current_vertex = Config::START_VERTEX;
        root.level = 1;
        root.partial_cost = 0;
        root.lower_bound = 0; // tutaj nie uzywamy LB local
        return root;
    }

    BnBNode createChildNode(
            const TSPInstance& instance,
            const BnBNode& parent,
            int parentIndex,
            int nextVertex
    ) {
        BnBNode child = parent;
        child.parent_index = parentIndex;
        child.visited[nextVertex] = true;
        child.partial_cost += instance.distanceMatrix[parent.current_vertex][nextVertex];
        child.current_vertex = nextVertex;
        child.level = parent.level + 1;
        child.lower_bound = child.partial_cost; // tylko jako priorytet dla BestFS
        return child;
    }

    bool timeLimitReached(
            const std::chrono::high_resolution_clock::time_point& globalStart,
            double maxTimeSeconds
    ) {
        using clock = std::chrono::high_resolution_clock;
        const double elapsedSeconds =
                std::chrono::duration<double>(clock::now() - globalStart).count();

        return elapsedSeconds >= maxTimeSeconds;
    }

    void initializeResultBase(
            TSPResult& result,
            const TSPInstance& instance,
            const std::string& algorithmName
    ) {
        result.algorithm_name = algorithmName;
        result.instance_name = instance.name.empty() ? "unknown_instance" : instance.name;
        result.instance_type = instance.type.empty()
                               ? (instance.symmetric ? "TSP" : "ATSP")
                               : instance.type;
        result.vertex_count = instance.dimension;
        result.best_cost = std::numeric_limits<int>::max();
        result.best_path.clear();
        result.best_path_text.clear();
        result.ub_from_nn = -1;
        result.visited_nodes = 0;
        result.pruned_nodes = 0;
        result.completed_naturally = true;
        result.stop_reason = "Zakonczono naturalnie";
    }

    UpperBoundResult initializeUpperBound(
            const TSPInstance& instance,
            InitialUBMode ubMode
    ) {
        if (ubMode == InitialUBMode::INF) {
            UpperBoundResult result;
            result.cost = std::numeric_limits<int>::max();
            result.path.clear();
            result.finished = true;
            result.timeoutReached = false;
            return result;
        }

        return computeInitialUpperBoundRNN(instance);
    }
}

TSPResult solveBranchAndBoundPlain(
        const TSPInstance& instance,
        ISearchFrontier& frontier,
        const std::string& algorithmName,
        InitialUBMode ubMode,
        double maxTimeSeconds
) {
    using clock = std::chrono::high_resolution_clock;

    if (!instance.isValid()) {
        throw std::runtime_error("Instancja jest niepoprawna lub niepelna.");
    }

    if (instance.dimension < 2) {
        throw std::runtime_error("Instancja musi miec co najmniej 2 miasta.");
    }

    TSPResult result;
    initializeResultBase(result, instance, algorithmName);

    UpperBoundResult ubResult = initializeUpperBound(instance, ubMode);

    if (ubMode == InitialUBMode::RNN) {
        result.ub_from_nn = ubResult.cost;
    } else {
        result.ub_from_nn = -1; // INF
    }

    result.best_cost = ubResult.cost;

    if (ubResult.path.size() == static_cast<size_t>(instance.dimension)) {
        result.best_path = ubResult.path;
    }

    const auto bnbStart = clock::now();

    std::vector<BnBNode> nodePool;
    nodePool.reserve(100000);

    BnBNode root = createRootNode(instance);

    nodePool.push_back(std::move(root));
    frontier.push(0, 0);

    while (!frontier.empty()) {
        if (timeLimitReached(bnbStart, maxTimeSeconds)) {
            result.completed_naturally = false;
            result.stop_reason = "Przekroczono limit czasu";
            break;
        }

        const NodeId nodeId = frontier.pop();
        const BnBNode node = nodePool[nodeId];
        result.visited_nodes++;

        // W tym wariancie odcinamy po koszcie aktualnej sciezki, bez LB local
        if (node.partial_cost >= result.best_cost) {
            result.pruned_nodes++;
            continue;
        }

        if (node.level == instance.dimension) {
            const int fullCost = computeCompletionCost(instance, node);

            if (fullCost < result.best_cost) {
                result.best_cost = fullCost;
                result.best_path = materializePath(nodePool, nodeId);
            }

            continue;
        }

        for (int next = 0; next < instance.dimension; ++next) {
            if (node.visited[next]) {
                continue;
            }

            BnBNode child = createChildNode(instance, node, nodeId, next);

            if (child.partial_cost < result.best_cost) {
                nodePool.push_back(std::move(child));
                const auto childId = static_cast<NodeId>(nodePool.size() - 1);

                // dla DFS/BFS ten parametr jest ignorowany,
                // dla BestFS robi za priority = aktualny koszt sciezki
                frontier.push(childId, nodePool[childId].partial_cost);
            } else {
                result.pruned_nodes++;
            }
        }
    }

    const auto bnbEnd = clock::now();
    result.total_time_ms = elapsedMilliseconds(bnbStart, bnbEnd);

    if (!result.best_path.empty() &&
        result.best_path.size() == static_cast<size_t>(instance.dimension)) {
        result.best_path_text = pathToString(result.best_path, instance);
    } else {
        result.best_cost = -1;
        result.best_path_text = "Brak pelnej sciezki";
    }

    return result;
}
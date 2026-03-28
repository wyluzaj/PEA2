#include "bnb_plain_solver.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>

#include "bnb_node.h"
#include "ub.h"
#include "../common/utils.h"

namespace {
    constexpr int INF = std::numeric_limits<int>::max() / 4;

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
        root.lower_bound = 0;
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
        child.lower_bound = child.partial_cost;
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

    int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
        return node.partial_cost +
               instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
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
        result.best_cost = INF;
        result.best_path.clear();
        result.best_path_text.clear();
        result.ub_from_nn = -1;

        result.visited_nodes = 0;
        result.pruned_nodes = 0;
        result.generated_nodes = 0;
        result.stored_nodes = 0;
        result.max_frontier_size = 0;
        result.max_node_pool_size = 0;

        result.memory_exhausted = false;
        result.completed_naturally = true;
        result.stop_reason = "Zakonczono naturalnie";
        result.summary_file_name.clear();
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

    if (ubMode == InitialUBMode::RNN) {
        UpperBoundResult ubResult = computeInitialUpperBoundRNN(instance);
        result.ub_from_nn = ubResult.cost;

        if (ubResult.path.size() == static_cast<size_t>(instance.dimension)) {
            result.best_cost = ubResult.cost;
            result.best_path = ubResult.path;
        } else {
            result.best_cost = ubResult.cost;
            result.best_path.clear();
        }
    }

    const auto bnbStart = clock::now();

    try {
        std::vector<BnBNode> nodePool;
        nodePool.reserve(100000);

        BnBNode root = createRootNode(instance);
        nodePool.push_back(std::move(root));
        result.stored_nodes = 1;
        result.max_node_pool_size = 1;

        frontier.push(0, nodePool[0].partial_cost);
        result.max_frontier_size = std::max<long long>(
                result.max_frontier_size,
                static_cast<long long>(frontier.size())
        );

        while (!frontier.empty()) {
            if (timeLimitReached(bnbStart, maxTimeSeconds)) {
                result.completed_naturally = false;
                result.stop_reason = "Przekroczono limit czasu";
                break;
            }

            result.max_frontier_size = std::max<long long>(
                    result.max_frontier_size,
                    static_cast<long long>(frontier.size())
            );
            result.max_node_pool_size = std::max<long long>(
                    result.max_node_pool_size,
                    static_cast<long long>(nodePool.size())
            );

            const NodeId nodeId = frontier.pop();
            const BnBNode node = nodePool[nodeId];
            result.visited_nodes++;

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

                result.generated_nodes++;

                BnBNode child = createChildNode(instance, node, nodeId, next);

                if (child.partial_cost < result.best_cost) {
                    nodePool.push_back(std::move(child));
                    result.stored_nodes++;
                    result.max_node_pool_size = std::max<long long>(
                            result.max_node_pool_size,
                            static_cast<long long>(nodePool.size())
                    );

                    const auto childId = static_cast<NodeId>(nodePool.size() - 1);
                    frontier.push(childId, nodePool[childId].partial_cost);
                    result.max_frontier_size = std::max<long long>(
                            result.max_frontier_size,
                            static_cast<long long>(frontier.size())
                    );
                } else {
                    result.pruned_nodes++;
                }
            }
        }
    }
    catch (const std::bad_alloc&) {
        result.completed_naturally = false;
        result.memory_exhausted = true;
        result.stop_reason = "Brak pamieci (std::bad_alloc)";
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
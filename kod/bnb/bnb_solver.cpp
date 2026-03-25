//#include "bnb_solver.h"
//
//#include <chrono>
//#include <limits>
//#include <stdexcept>
//#include <vector>
//
//#include "../common/utils.h"
//#include "bnb_node.h"
//#include "lb.h"
//#include "ub.h"
//#include "../common/config.h"
//
//namespace {
//    BnBNode createRootNode(const TSPInstance& instance) {
//        BnBNode root;
//        root.path.push_back(Config::START_VERTEX);
//        root.visited.assign(instance.dimension, false);
//        root.visited[Config::START_VERTEX] = true;
//        root.current_vertex = Config::START_VERTEX;
//        root.level = 1;
//        root.partial_cost = 0;
//        root.lower_bound = 0;
//        return root;
//    }
//
//    BnBNode createChildNode(
//            const TSPInstance& instance,
//            const BnBNode& parent,
//            int nextVertex
//    ) {
//        BnBNode child = parent;
//        child.path.push_back(nextVertex);
//        child.visited[nextVertex] = true;
//        child.partial_cost += instance.distanceMatrix[parent.current_vertex][nextVertex];
//        child.current_vertex = nextVertex;
//        child.level = parent.level + 1;
//        return child;
//    }
//
//    bool timeLimitReached(
//            const std::chrono::high_resolution_clock::time_point& globalStart,
//            double maxTimeSeconds
//    ) {
//        using clock = std::chrono::high_resolution_clock;
//        const double elapsedSeconds =
//                std::chrono::duration<double>(clock::now() - globalStart).count();
//
//        return elapsedSeconds >= maxTimeSeconds;
//    }
//
//    void initializeResultBase(
//            TSPResult& result,
//            const TSPInstance& instance,
//            const std::string& algorithmName
//    ) {
//        result.algorithm_name = algorithmName;
//        result.instance_name = instance.name.empty() ? "unknown_instance" : instance.name;
//        result.instance_type = instance.type.empty()
//                               ? (instance.symmetric ? "TSP" : "ATSP")
//                               : instance.type;
//        result.vertex_count = instance.dimension;
//        result.best_cost = std::numeric_limits<int>::max();
//        result.best_path.clear();
//        result.best_path_text.clear();
//        result.ub_from_nn = -1;
//        result.visited_nodes = 0;
//        result.pruned_nodes = 0;
//        result.completed_naturally = true;
//        result.stop_reason = "Zakonczono naturalnie";
//    }
//}
//
//TSPResult solveBranchAndBound(
//        const TSPInstance& instance,
//        ISearchFrontier& frontier,
//        const std::string& algorithmName,
//        double maxTimeSeconds
//) {
//    using clock = std::chrono::high_resolution_clock;
//
//    if (!instance.isValid()) {
//        throw std::runtime_error("Instancja jest niepoprawna lub niepelna.");
//    }
//
//    if (instance.dimension < 2) {
//        throw std::runtime_error("Instancja musi miec co najmniej 2 miasta.");
//    }
//
//    TSPResult result;
//    initializeResultBase(result, instance, algorithmName);
//
//    // 1. Najpierw NN - poza pomiarem czasu BnB
//    UpperBoundResult ubResult = computeInitialUpperBoundNN(instance);
//
//    result.ub_from_nn = ubResult.cost;
//
//    if (ubResult.path.size() == static_cast<size_t>(instance.dimension)) {
//        result.best_cost = ubResult.cost;
//        result.best_path = ubResult.path;
//    } else {
//        result.best_cost = ubResult.cost;
//        result.best_path.clear();
//    }
//
//    // 2. Dopiero teraz startuje czas BnB
//    const auto bnbStart = clock::now();
//
//    BnBNode root = createRootNode(instance);
//    root.lower_bound = computeLowerBound(instance, root);
//    frontier.push(root);
//
//    while (!frontier.empty()) {
//        if (timeLimitReached(bnbStart, maxTimeSeconds)) {
//            result.completed_naturally = false;
//            result.stop_reason = "Przekroczono limit czasu";
//            break;
//        }
//
//        BnBNode node = frontier.pop();
//        result.visited_nodes++;
//
//        if (node.lower_bound >= result.best_cost) {
//            result.pruned_nodes++;
//            continue;
//        }
//
//        if (node.level == instance.dimension) {
//            const int fullCost = computeCompletionCost(instance, node);
//
//            if (fullCost < result.best_cost) {
//                result.best_cost = fullCost;
//                result.best_path = node.path;
//            }
//
//            continue;
//        }
//
//        for (int next = 0; next < instance.dimension; ++next) {
//            if (node.visited[next]) {
//                continue;
//            }
//
//            BnBNode child = createChildNode(instance, node, next);
//            child.lower_bound = computeLowerBound(instance, child);
//
//            if (child.lower_bound < result.best_cost) {
//                frontier.push(child);
//            } else {
//                result.pruned_nodes++;
//            }
//        }
//    }
//
//    const auto bnbEnd = clock::now();
//    result.total_time_ms = elapsedMilliseconds(bnbStart, bnbEnd);
//
//    if (!result.best_path.empty() &&
//        result.best_path.size() == static_cast<size_t>(instance.dimension)) {
//        result.best_path_text = pathToString(result.best_path, instance);
//    } else {
//        result.best_cost = -1;
//        result.best_path_text = "Brak pelnej sciezki";
//    }
//
//    return result;
//}

#include "bnb_solver.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/utils.h"
#include "bnb_node.h"
#include "lb.h"
#include "ub.h"
#include "../common/config.h"

namespace {
    BnBNode createRootNode(const TSPInstance& instance) {
        BnBNode root;
        root.path.push_back(Config::START_VERTEX);
        root.visited.assign(instance.dimension, false);
        root.visited[Config::START_VERTEX] = true;
        root.current_vertex = Config::START_VERTEX;
        root.level = 1;
        root.partial_cost = 0;
        root.lower_bound = 0;
        return root;
    }

    BnBNode createChildNode(
            const TSPInstance& instance,
            const BnBNode& parent,
            int nextVertex
    ) {
        BnBNode child = parent;
        child.path.push_back(nextVertex);
        child.visited[nextVertex] = true;
        child.partial_cost += instance.distanceMatrix[parent.current_vertex][nextVertex];
        child.current_vertex = nextVertex;
        child.level = parent.level + 1;
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

    bool completeGreedyFromNode(
            const TSPInstance& instance,
            const BnBNode& node,
            std::vector<int>& completedPath,
            int& completedCost
    ) {
        const int n = instance.dimension;

        completedPath = node.path;
        std::vector<bool> visited = node.visited;
        int current = node.current_vertex;
        int cost = node.partial_cost;

        while (static_cast<int>(completedPath.size()) < n) {
            int bestNext = -1;
            int bestDist = std::numeric_limits<int>::max();

            for (int v = 0; v < n; ++v) {
                if (visited[v]) {
                    continue;
                }

                const int d = instance.distanceMatrix[current][v];
                if (d < bestDist) {
                    bestDist = d;
                    bestNext = v;
                }
            }

            if (bestNext == -1) {
                return false;
            }

            completedPath.push_back(bestNext);
            visited[bestNext] = true;
            cost += instance.distanceMatrix[current][bestNext];
            current = bestNext;
        }

        cost += instance.distanceMatrix[current][Config::START_VERTEX];
        completedCost = cost;
        return true;
    }

    void tryUpdateIncumbentFromNode(
            const TSPInstance& instance,
            const BnBNode& node,
            TSPResult& result
    ) {
        std::vector<int> candidatePath;
        int candidateCost = -1;

        if (!completeGreedyFromNode(instance, node, candidatePath, candidateCost)) {
            return;
        }

        if (candidateCost < result.best_cost) {
            result.best_cost = candidateCost;
            result.best_path = candidatePath;
        }
    }
}

TSPResult solveBranchAndBound(
        const TSPInstance& instance,
        ISearchFrontier& frontier,
        const std::string& algorithmName,
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

    // 1. Najpierw NN - poza pomiarem czasu BnB
    UpperBoundResult ubResult = computeInitialUpperBoundNN(instance);

    result.ub_from_nn = ubResult.cost;

    if (ubResult.path.size() == static_cast<size_t>(instance.dimension)) {
        result.best_cost = ubResult.cost;
        result.best_path = ubResult.path;
    } else {
        result.best_cost = ubResult.cost;
        result.best_path.clear();
    }

    // 2. Dopiero teraz startuje czas BnB
    const auto bnbStart = clock::now();

    BnBNode root = createRootNode(instance);

    // opcjonalnie: także root można spróbować szybko domknąć,
    // ale tu zwykle to samo dał już computeInitialUpperBoundNN(instance)
    root.lower_bound = computeLowerBound(instance, root);
    frontier.push(root);

    while (!frontier.empty()) {
        if (timeLimitReached(bnbStart, maxTimeSeconds)) {
            result.completed_naturally = false;
            result.stop_reason = "Przekroczono limit czasu";
            break;
        }

        BnBNode node = frontier.pop();
        result.visited_nodes++;

        if (node.lower_bound >= result.best_cost) {
            result.pruned_nodes++;
            continue;
        }

        if (node.level == instance.dimension) {
            const int fullCost = computeCompletionCost(instance, node);

            if (fullCost < result.best_cost) {
                result.best_cost = fullCost;
                result.best_path = node.path;
            }

            continue;
        }

        for (int next = 0; next < instance.dimension; ++next) {
            if (node.visited[next]) {
                continue;
            }

            BnBNode child = createChildNode(instance, node, next);

            // NOWE:
            // szybkie heurystyczne domkniecie dziecka -> poprawa globalnego UB
            tryUpdateIncumbentFromNode(instance, child, result);

            // dopiero potem liczymy LB, już z potencjalnie lepszym best_cost
            child.lower_bound = computeLowerBound(instance, child);

            if (child.lower_bound < result.best_cost) {
                frontier.push(child);
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
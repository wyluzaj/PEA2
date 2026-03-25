#include "lb.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/config.h"

namespace {
    constexpr int INF = std::numeric_limits<int>::max() / 8;

    bool isInternalVisitedVertex(const BnBNode& node, int v) {
        if (!(node.visited_mask & (1ULL << v))) {
            return false;
        }

        if (v == Config::START_VERTEX) {
            return false;
        }

        if (v == node.current_vertex) {
            return false;
        }

        return true;
    }

    bool isResidualEdgeAllowed(
            const BnBNode& node,
            int u,
            int v,
            const std::vector<int>& remainingDegree,
            int n
    ) {
        if (u == v) {
            return false;
        }

        if (remainingDegree[u] <= 0 || remainingDegree[v] <= 0) {
            return false;
        }

        // Wierzchołki wewnętrzne ścieżki są już "zamknięte"
        if (isInternalVisitedVertex(node, u) || isInternalVisitedVertex(node, v)) {
            return false;
        }

        // Nie wolno domykać cyklu za wcześnie krawędzią start-current
        if (node.level < n) {
            const int s = Config::START_VERTEX;
            const int c = node.current_vertex;

            if ((u == s && v == c) || (u == c && v == s)) {
                return false;
            }
        }

        return true;
    }

    int sumKCheapestResidualEdges(
            const TSPInstance& instance,
            const BnBNode& node,
            int v,
            int k,
            const std::vector<int>& remainingDegree
    ) {
        if (k == 0) {
            return 0;
        }

        const int n = instance.dimension;
        long long sum = 0;
        int found = 0;

        // BŁYSKAWICZNE SZUKANIE: Iterujemy po gotowej, posortowanej liście sąsiadów
        for (int u : instance.sortedNeighbors[v]) {
            if (!isResidualEdgeAllowed(node, v, u, remainingDegree, n)) {
                continue;
            }

            sum += instance.distanceMatrix[v][u];
            found++;

            // Jeśli znaleźliśmy wymagane 'k' najtańszych krawędzi, od razu kończymy szukanie!
            if (found == k) {
                break;
            }
        }

        if (found < k || sum >= INF) {
            return INF;
        }

        return static_cast<int>(sum);
    }

    int computeDegreeBoundTSP(const TSPInstance& instance, const BnBNode& node) {
        const int n = instance.dimension;

        std::vector<int> remainingDegree(n, 0);

        // Nieodwiedzone wierzchołki muszą mieć finalnie stopień 2
        for (int v = 0; v < n; ++v) {
            if (!(node.visited_mask & (1ULL << v))) {
                remainingDegree[v] = 2;
            }
        }

        // Dla częściowej ścieżki:
        // - start potrzebuje jeszcze 1 krawędzi,
        // - current potrzebuje jeszcze 1 krawędzi,
        // - wyjątek: root (level == 1), wtedy start == current i potrzeba jeszcze 2.
        if (node.level == 1) {
            remainingDegree[Config::START_VERTEX] = 2;
        } else {
            remainingDegree[Config::START_VERTEX] += 1;
            remainingDegree[node.current_vertex] += 1;
        }

        long long residualSum = 0;

        for (int v = 0; v < n; ++v) {
            const int req = remainingDegree[v];
            if (req == 0) {
                continue;
            }

            const int add =
                    sumKCheapestResidualEdges(instance, node, v, req, remainingDegree);

            if (add >= INF) {
                return INF;
            }

            residualSum += add;
        }

        // Każda przyszła krawędź została policzona na obu końcach
        const long long total =
                static_cast<long long>(node.partial_cost) + (residualSum + 1) / 2;

        if (total >= INF) {
            return INF;
        }

        return static_cast<int>(total);
    }

    // -------------------- ATSP helpers --------------------

    int minOutgoingToUnvisitedOrStart(
            const TSPInstance& instance,
            int from,
            uint64_t visited_mask
    ) {
        for (int to : instance.sortedNeighbors[from]) {
            if (!(visited_mask & (1ULL << to)) || to == Config::START_VERTEX) {
                return instance.distanceMatrix[from][to];
            }
        }
        return INF;
    }

    int minIncomingFromUnvisitedOrCurrent(
            const TSPInstance& instance,
            int to,
            uint64_t visited_mask,
            int currentVertex
    ) {
        int best = INF;
        const int n = instance.dimension;

        for (int from = 0; from < n; ++from) {
            if (from == to) {
                continue;
            }

            if (!(visited_mask & (1ULL << from)) || from == currentVertex) {
                best = std::min(best, instance.distanceMatrix[from][to]);
            }
        }

        return best;
    }

    int computeAssignmentBoundATSP(
            const TSPInstance& instance,
            const BnBNode& node
    ) {
        const int n = instance.dimension;

        if (node.level == n) {
            return node.partial_cost +
                   instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
        }

        long long outBound = node.partial_cost;
        long long inBound = node.partial_cost;

        const int currentOut =
                minOutgoingToUnvisitedOrStart(instance, node.current_vertex, node.visited_mask);
        if (currentOut >= INF) {
            return INF;
        }
        outBound += currentOut;

        const int startIn =
                minIncomingFromUnvisitedOrCurrent(
                        instance,
                        Config::START_VERTEX,
                        node.visited_mask,
                        node.current_vertex
                );
        if (startIn >= INF) {
            return INF;
        }
        inBound += startIn;

        for (int v = 0; v < n; ++v) {
            if (!(node.visited_mask & (1ULL << v))) {
                const int outMin =
                        minOutgoingToUnvisitedOrStart(instance, v, node.visited_mask);

                const int inMin =
                        minIncomingFromUnvisitedOrCurrent(
                                instance,
                                v,
                                node.visited_mask,
                                node.current_vertex
                        );

                if (outMin >= INF || inMin >= INF) {
                    return INF;
                }

                outBound += outMin;
                inBound += inMin;
            }
        }

        const long long total = std::max(outBound, inBound);
        if (total >= INF) {
            return INF;
        }

        return static_cast<int>(total);
    }
}

int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
    if (node.level != instance.dimension) {
        throw std::runtime_error("computeCompletionCost wywolano dla niepelnej sciezki.");
    }

    if (node.current_vertex < 0 || node.current_vertex >= instance.dimension) {
        throw std::runtime_error("Niepoprawny current_vertex w computeCompletionCost.");
    }

    return node.partial_cost + instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
}

int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
    const int n = instance.dimension;

    if (n <= 0) {
        throw std::runtime_error("Niepoprawna instancja w computeLowerBound.");
    }

    if (node.current_vertex < 0 || node.current_vertex >= n) {
        throw std::runtime_error("Niepoprawny current_vertex.");
    }

    if (node.level == n) {
        return computeCompletionCost(instance, node);
    }

    if (instance.symmetric) {
        return computeDegreeBoundTSP(instance, node);
    } else {
        return computeAssignmentBoundATSP(instance, node);
    }
}
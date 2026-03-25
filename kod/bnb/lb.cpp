//#include "lb.h"
//
//#include <algorithm>
//#include <limits>
//#include <stdexcept>
//#include <vector>
//
//
//namespace {
//    constexpr int INF = std::numeric_limits<int>::max() / 4;
//
//    int minOutgoingToUnvisitedOrStart(
//            const TSPInstance& instance,
//            int from,
//            const std::vector<bool>& visited
//    ) {
//        int best = INF;
//        const int n = instance.dimension;
//
//        for (int to = 0; to < n; ++to) {
//            if (to == from) {
//                continue;
//            }
//
//            if (!visited[to] || to == Config::START_VERTEX) {
//                best = std::min(best, instance.distanceMatrix[from][to]);
//            }
//        }
//
//        return best;
//    }
//
//    int minIncomingFromUnvisitedOrCurrent(
//            const TSPInstance& instance,
//            int to,
//            const std::vector<bool>& visited,
//            int currentVertex
//    ) {
//        int best = INF;
//        const int n = instance.dimension;
//
//        for (int from = 0; from < n; ++from) {
//            if (from == to) {
//                continue;
//            }
//
//            if (!visited[from] || from == currentVertex) {
//                best = std::min(best, instance.distanceMatrix[from][to]);
//            }
//        }
//
//        return best;
//    }
//
//    int sumKCheapestResidualEdges(
//            const TSPInstance& instance,
//            int v,
//            int k,
//            const std::vector<int>& remainingDegree
//    ) {
//
//
//        std::vector<int> costs;
//        const int n = instance.dimension;
//        costs.reserve(n - 1);
//
//        for (int u = 0; u < n; ++u) {
//            if (u == v) {
//                continue;
//            }
//
//            if (remainingDegree[u] > 0) {
//                costs.push_back(instance.distanceMatrix[v][u]);
//            }
//        }
//
//        if (static_cast<int>(costs.size()) < k) {
//            return INF;
//        }
//
//        std::nth_element(costs.begin(), costs.begin() + (k - 1), costs.end());
//
//        long long sum = 0;
//        for (int i = 0; i < k; ++i) {
//            sum += costs[i];
//        }
//
//        if (sum >= INF) {
//            return INF;
//        }
//
//        return static_cast<int>(sum);
//    }
//}
//
//int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
//    if (node.level != instance.dimension) {
//        throw std::runtime_error("computeCompletionCost wywolano dla niepelnej sciezki.");
//    }
//
//    if (node.current_vertex < 0 || node.current_vertex >= instance.dimension) {
//        throw std::runtime_error("Niepoprawny current_vertex w computeCompletionCost.");
//    }
//
//    return node.partial_cost + instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
//}
//
//int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
//    const int n = instance.dimension;
//
//    if (n <= 0) {
//        throw std::runtime_error("Niepoprawna instancja w computeLowerBound.");
//    }
//
//    if (static_cast<int>(node.visited.size()) != n) {
//        throw std::runtime_error("Rozmiar visited nie zgadza sie z dimension.");
//    }
//
//    if (node.current_vertex < 0 || node.current_vertex >= n) {
//        throw std::runtime_error("Niepoprawny current_vertex.");
//    }
//
//    if (node.level == n) {
//        return computeCompletionCost(instance, node);
//    }
//
//    if (instance.symmetric) {
//        // ===== TSP =====
//        //
//        // Poprawny bound stopniowy dla sciezki czesciowej:
//        // - nieodwiedzone wierzcholki potrzebuja jeszcze stopnia 2,
//        // - start potrzebuje jeszcze 1, a current jeszcze 1,
//        // - wyjatek: w root start == current, wiec potrzebuje jeszcze 2.
//        //
//        // Dla kazdego wierzcholka zliczamy koszt najtanszych brakujacych
//        // krawedzi w grafie resztowym, a na koncu dzielimy te brakujace
//        // stopnie przez 2, bo kazda przyszla krawedz jest liczona na obu koncach.
//        //
//        // WAŻNE:
//        // partial_cost NIE jest dzielony przez 2.
//
//        std::vector<int> remainingDegree(n, 0);
//
//        for (int v = 0; v < n; ++v) {
//            if (!node.visited[v]) {
//                remainingDegree[v] = 2;
//            }
//        }
//
//        if (node.level == 1) {
//            remainingDegree[Config::START_VERTEX] = 2;
//        } else {
//            remainingDegree[Config::START_VERTEX] += 1;
//            remainingDegree[node.current_vertex] += 1;
//        }
//
//        long long residualSum = 0;
//
//        for (int v = 0; v < n; ++v) {
//            const int req = remainingDegree[v];
//            if (req == 0) {
//                continue;
//            }
//
//            const int add = sumKCheapestResidualEdges(instance, v, req, remainingDegree);
//            if (add >= INF) {
//                return INF;
//            }
//
//            residualSum += add;
//        }
//
//        return static_cast<int>(node.partial_cost + (residualSum + 1) / 2);
//    } else {
//        // ===== ATSP =====
//        //
//        // Bezpieczna wersja:
//        // liczymy osobno ograniczenie na wyjscia i na wejscia,
//        // a potem bierzemy max(outBound, inBound).
//
//        long long outBound = node.partial_cost;
//        long long inBound = node.partial_cost;
//
//        const int currentOut =
//                minOutgoingToUnvisitedOrStart(instance, node.current_vertex, node.visited);
//        if (currentOut >= INF) {
//            return INF;
//        }
//        outBound += currentOut;
//
//        const int startIn =
//                minIncomingFromUnvisitedOrCurrent(
//                        instance,
//                        Config::START_VERTEX,
//                        node.visited,
//                        node.current_vertex
//                );
//        if (startIn >= INF) {
//            return INF;
//        }
//        inBound += startIn;
//
//        for (int v = 0; v < n; ++v) {
//            if (!node.visited[v]) {
//                const int outMin =
//                        minOutgoingToUnvisitedOrStart(instance, v, node.visited);
//
//                const int inMin =
//                        minIncomingFromUnvisitedOrCurrent(
//                                instance,
//                                v,
//                                node.visited,
//                                node.current_vertex
//                        );
//
//                if (outMin >= INF || inMin >= INF) {
//                    return INF;
//                }
//
//                outBound += outMin;
//                inBound += inMin;
//            }
//        }
//
//        return static_cast<int>(std::max(outBound, inBound));
//    }
//}





//
//#include "lb.h"
//
//#include <algorithm>
//#include <cmath>
//#include <limits>
//#include <numeric>
//#include <stdexcept>
//#include <utility>
//#include <vector>
//
//#include "../common/config.h"
//
//namespace {
//    constexpr int INF = std::numeric_limits<int>::max() / 8;
//
//    struct DSU {
//        std::vector<int> parent;
//        std::vector<int> rank;
//
//        explicit DSU(int n) : parent(n), rank(n, 0) {
//            for (int i = 0; i < n; ++i) {
//                parent[i] = i;
//            }
//        }
//
//        int find(int x) {
//            if (parent[x] != x) {
//                parent[x] = find(parent[x]);
//            }
//            return parent[x];
//        }
//
//        bool unite(int a, int b) {
//            a = find(a);
//            b = find(b);
//
//            if (a == b) {
//                return false;
//            }
//
//            if (rank[a] < rank[b]) {
//                std::swap(a, b);
//            }
//
//            parent[b] = a;
//            if (rank[a] == rank[b]) {
//                rank[a]++;
//            }
//
//            return true;
//        }
//    };
//
//    struct OneTreeResult {
//        double lagrangianBound = -1e100;
//        std::vector<int> degree;
//    };
//
//    struct Edge {
//        int u;
//        int v;
//        double adjustedCost;
//        int originalCost;
//    };
//
//    std::vector<int> getUnvisitedVertices(const BnBNode& node) {
//        std::vector<int> unvisited;
//        const int n = static_cast<int>(node.visited.size());
//
//        for (int v = 0; v < n; ++v) {
//            if (!node.visited[v]) {
//                unvisited.push_back(v);
//            }
//        }
//
//        return unvisited;
//    }
//
//    double averageFiniteCost(const std::vector<std::vector<int>>& cost) {
//        long long sum = 0;
//        long long cnt = 0;
//        const int n = static_cast<int>(cost.size());
//
//        for (int i = 0; i < n; ++i) {
//            for (int j = 0; j < n; ++j) {
//                if (i == j) {
//                    continue;
//                }
//                if (cost[i][j] >= INF) {
//                    continue;
//                }
//                sum += cost[i][j];
//                cnt++;
//            }
//        }
//
//        if (cnt == 0) {
//            return 1.0;
//        }
//
//        return static_cast<double>(sum) / static_cast<double>(cnt);
//    }
//
//    std::vector<std::vector<int>> buildContractedSymmetricMatrix(
//            const TSPInstance& instance,
//            const BnBNode& node,
//            const std::vector<int>& unvisited
//    ) {
//        // Wierzcholek 0 = skurczona dotychczasowa sciezka od START do current.
//        // Pozostale wierzcholki = miasta nieodwiedzone.
//        //
//        // Krawedz (supernode, u) ma koszt min(start-u, current-u),
//        // co daje bezpieczne zanizenie kosztu domkniecia.
//
//        const int m = static_cast<int>(unvisited.size()) + 1;
//        std::vector<std::vector<int>> cost(m, std::vector<int>(m, INF));
//
//        for (int i = 0; i < m; ++i) {
//            cost[i][i] = INF;
//        }
//
//        const int start = Config::START_VERTEX;
//        const int current = node.current_vertex;
//
//        for (int idx = 0; idx < static_cast<int>(unvisited.size()); ++idx) {
//            const int u = unvisited[idx];
//            const int cu = std::min(
//                    instance.distanceMatrix[start][u],
//                    instance.distanceMatrix[current][u]
//            );
//
//            cost[0][idx + 1] = cu;
//            cost[idx + 1][0] = cu;
//        }
//
//        for (int i = 0; i < static_cast<int>(unvisited.size()); ++i) {
//            for (int j = 0; j < static_cast<int>(unvisited.size()); ++j) {
//                if (i == j) {
//                    continue;
//                }
//
//                const int u = unvisited[i];
//                const int v = unvisited[j];
//                cost[i + 1][j + 1] = instance.distanceMatrix[u][v];
//            }
//        }
//
//        return cost;
//    }
//
//    OneTreeResult computeOneTree(
//            const std::vector<std::vector<int>>& cost,
//            const std::vector<double>& pi
//    ) {
//        const int n = static_cast<int>(cost.size());
//        OneTreeResult result;
//        result.degree.assign(n, 0);
//
//        if (n < 3) {
//            result.lagrangianBound = -1e100;
//            return result;
//        }
//
//        std::vector<Edge> edges;
//        edges.reserve((n - 1) * (n - 2) / 2);
//
//        // MST na wierzcholkach 1..n-1 (bez root = 0)
//        for (int i = 1; i < n; ++i) {
//            for (int j = i + 1; j < n; ++j) {
//                if (cost[i][j] >= INF) {
//                    continue;
//                }
//
//                edges.push_back({
//                                        i,
//                                        j,
//                                        static_cast<double>(cost[i][j]) + pi[i] + pi[j],
//                                        cost[i][j]
//                                });
//            }
//        }
//
//        std::sort(edges.begin(), edges.end(),
//                  [](const Edge& a, const Edge& b) {
//                      return a.adjustedCost < b.adjustedCost;
//                  });
//
//        DSU dsu(n);
//        double adjustedSum = 0.0;
//        long long originalSum = 0;
//        int mstEdges = 0;
//
//        for (const Edge& e : edges) {
//            if (dsu.unite(e.u, e.v)) {
//                adjustedSum += e.adjustedCost;
//                originalSum += e.originalCost;
//                result.degree[e.u]++;
//                result.degree[e.v]++;
//                mstEdges++;
//
//                if (mstEdges == n - 2) {
//                    break;
//                }
//            }
//        }
//
//        if (mstEdges != n - 2) {
//            result.lagrangianBound = -1e100;
//            return result;
//        }
//
//        // Dwie najtansze krawedzie incydentne do roota = 0
//        int best1 = -1;
//        int best2 = -1;
//        double adj1 = 1e100;
//        double adj2 = 1e100;
//
//        for (int v = 1; v < n; ++v) {
//            if (cost[0][v] >= INF) {
//                continue;
//            }
//
//            const double adj = static_cast<double>(cost[0][v]) + pi[0] + pi[v];
//            if (adj < adj1) {
//                adj2 = adj1;
//                best2 = best1;
//                adj1 = adj;
//                best1 = v;
//            } else if (adj < adj2) {
//                adj2 = adj;
//                best2 = v;
//            }
//        }
//
//        if (best1 == -1 || best2 == -1) {
//            result.lagrangianBound = -1e100;
//            return result;
//        }
//
//        adjustedSum += adj1 + adj2;
//        originalSum += cost[0][best1] + cost[0][best2];
//
//        result.degree[0] += 2;
//        result.degree[best1]++;
//        result.degree[best2]++;
//
//        const double sumPi = std::accumulate(pi.begin(), pi.end(), 0.0);
//        result.lagrangianBound = adjustedSum - 2.0 * sumPi;
//
//        // Dla kontroli numerycznej to powinno byc rownowazne:
//        // originalSum + sum_i pi_i * (deg_i - 2)
//
//        return result;
//    }
//
//    int computeHeldKarpContractedBound(
//            const TSPInstance& instance,
//            const BnBNode& node
//    ) {
//        const std::vector<int> unvisited = getUnvisitedVertices(node);
//
//        if (unvisited.empty()) {
//            return node.partial_cost +
//                   instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
//        }
//
//        if (static_cast<int>(unvisited.size()) == 1) {
//            const int u = unvisited[0];
//            return node.partial_cost
//                   + instance.distanceMatrix[node.current_vertex][u]
//                   + instance.distanceMatrix[u][Config::START_VERTEX];
//        }
//
//        const std::vector<std::vector<int>> cost =
//                buildContractedSymmetricMatrix(instance, node, unvisited);
//
//        const int m = static_cast<int>(cost.size());
//        std::vector<double> pi(m, 0.0);
//
//        double bestLB = -1e100;
//        const double avgCost = averageFiniteCost(cost);
//        double step = avgCost;
//
//        for (int iter = 0; iter < 30; ++iter) {
//            const OneTreeResult tree = computeOneTree(cost, pi);
//            if (tree.lagrangianBound <= -1e90) {
//                return INF;
//            }
//
//            bestLB = std::max(bestLB, tree.lagrangianBound);
//
//            int normSq = 0;
//            for (int i = 0; i < m; ++i) {
//                const int g = tree.degree[i] - 2;
//                normSq += g * g;
//            }
//
//            if (normSq == 0) {
//                break;
//            }
//
//            for (int i = 0; i < m; ++i) {
//                const int g = tree.degree[i] - 2;
//                pi[i] += step * static_cast<double>(g);
//            }
//
//            step *= 0.7;
//            if (step < 1e-3) {
//                break;
//            }
//        }
//
//        if (bestLB <= -1e90) {
//            return INF;
//        }
//
//        const long long total =
//                static_cast<long long>(node.partial_cost) +
//                static_cast<long long>(std::ceil(bestLB - 1e-9));
//
//        if (total >= INF) {
//            return INF;
//        }
//
//        return static_cast<int>(total);
//    }
//
//    // -------------------- AP / HUNGARIAN FOR ATSP --------------------
//
//    long long hungarianMinCost(const std::vector<std::vector<int>>& a) {
//        const int n = static_cast<int>(a.size());
//        if (n == 0) {
//            return 0;
//        }
//
//        std::vector<long long> u(n + 1, 0), v(n + 1, 0);
//        std::vector<int> p(n + 1, 0), way(n + 1, 0);
//
//        for (int i = 1; i <= n; ++i) {
//            p[0] = i;
//            int j0 = 0;
//            std::vector<long long> minv(n + 1, static_cast<long long>(INF) * INF);
//            std::vector<bool> used(n + 1, false);
//
//            do {
//                used[j0] = true;
//                const int i0 = p[j0];
//                long long delta = static_cast<long long>(INF) * INF;
//                int j1 = 0;
//
//                for (int j = 1; j <= n; ++j) {
//                    if (used[j]) {
//                        continue;
//                    }
//
//                    const long long cur =
//                            static_cast<long long>(a[i0 - 1][j - 1]) - u[i0] - v[j];
//
//                    if (cur < minv[j]) {
//                        minv[j] = cur;
//                        way[j] = j0;
//                    }
//
//                    if (minv[j] < delta) {
//                        delta = minv[j];
//                        j1 = j;
//                    }
//                }
//
//                for (int j = 0; j <= n; ++j) {
//                    if (used[j]) {
//                        u[p[j]] += delta;
//                        v[j] -= delta;
//                    } else {
//                        minv[j] -= delta;
//                    }
//                }
//
//                j0 = j1;
//            } while (p[j0] != 0);
//
//            do {
//                const int j1 = way[j0];
//                p[j0] = p[j1];
//                j0 = j1;
//            } while (j0 != 0);
//        }
//
//        std::vector<int> assignment(n + 1, 0);
//        for (int j = 1; j <= n; ++j) {
//            assignment[p[j]] = j;
//        }
//
//        long long value = 0;
//        for (int i = 1; i <= n; ++i) {
//            const int j = assignment[i];
//            value += a[i - 1][j - 1];
//        }
//
//        return value;
//    }
//
//    int computeAssignmentBoundATSP(
//            const TSPInstance& instance,
//            const BnBNode& node
//    ) {
//        const std::vector<int> unvisited = getUnvisitedVertices(node);
//
//        if (unvisited.empty()) {
//            return node.partial_cost +
//                   instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
//        }
//
//        // Rzedy:  current + nieodwiedzone  -> kazdy musi miec jeszcze jedno wyjscie
//        // Kolumny: nieodwiedzone + start   -> kazdy musi miec jeszcze jedno wejscie
//        //
//        // To jest klasyczna relaksacja przyporzadkowania dla ATSP.
//        const int k = static_cast<int>(unvisited.size()) + 1;
//
//        std::vector<int> fromNodes;
//        std::vector<int> toNodes;
//        fromNodes.reserve(k);
//        toNodes.reserve(k);
//
//        fromNodes.push_back(node.current_vertex);
//        for (int u : unvisited) {
//            fromNodes.push_back(u);
//        }
//
//        for (int u : unvisited) {
//            toNodes.push_back(u);
//        }
//        toNodes.push_back(Config::START_VERTEX);
//
//        std::vector<std::vector<int>> cost(k, std::vector<int>(k, INF));
//
//        for (int i = 0; i < k; ++i) {
//            for (int j = 0; j < k; ++j) {
//                const int from = fromNodes[i];
//                const int to = toNodes[j];
//
//                if (from == to) {
//                    cost[i][j] = INF;
//                } else {
//                    cost[i][j] = instance.distanceMatrix[from][to];
//                }
//            }
//        }
//
//        const long long ap = hungarianMinCost(cost);
//        if (ap >= static_cast<long long>(INF) * INF / 4) {
//            return INF;
//        }
//
//        const long long total = static_cast<long long>(node.partial_cost) + ap;
//        if (total >= INF) {
//            return INF;
//        }
//
//        return static_cast<int>(total);
//    }
//}
//
//int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
//    if (node.level != instance.dimension) {
//        throw std::runtime_error("computeCompletionCost wywolano dla niepelnej sciezki.");
//    }
//
//    if (node.current_vertex < 0 || node.current_vertex >= instance.dimension) {
//        throw std::runtime_error("Niepoprawny current_vertex w computeCompletionCost.");
//    }
//
//    return node.partial_cost + instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
//}
//
//int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
//    const int n = instance.dimension;
//
//    if (n <= 0) {
//        throw std::runtime_error("Niepoprawna instancja w computeLowerBound.");
//    }
//
//    if (static_cast<int>(node.visited.size()) != n) {
//        throw std::runtime_error("Rozmiar visited nie zgadza sie z dimension.");
//    }
//
//    if (node.current_vertex < 0 || node.current_vertex >= n) {
//        throw std::runtime_error("Niepoprawny current_vertex.");
//    }
//
//    if (node.level == n) {
//        return computeCompletionCost(instance, node);
//    }
//
//    if (instance.symmetric) {
//        return computeHeldKarpContractedBound(instance, node);
//    } else {
//        return computeAssignmentBoundATSP(instance, node);
//    }
//}



#include "lb.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../common/config.h"

namespace {
    constexpr int INF = std::numeric_limits<int>::max() / 8;

    bool isInternalVisitedVertex(const BnBNode& node, int v) {
        if (!node.visited[v]) {
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

        std::vector<int> costs;
        const int n = instance.dimension;
        costs.reserve(n - 1);

        for (int u = 0; u < n; ++u) {
            if (!isResidualEdgeAllowed(node, v, u, remainingDegree, n)) {
                continue;
            }

            costs.push_back(instance.distanceMatrix[v][u]);
        }

        if (static_cast<int>(costs.size()) < k) {
            return INF;
        }

        std::nth_element(costs.begin(), costs.begin() + k, costs.end());

        long long sum = 0;
        for (int i = 0; i < k; ++i) {
            sum += costs[i];
        }

        if (sum >= INF) {
            return INF;
        }

        return static_cast<int>(sum);
    }

    int computeDegreeBoundTSP(const TSPInstance& instance, const BnBNode& node) {
        const int n = instance.dimension;

        std::vector<int> remainingDegree(n, 0);

        // Nieodwiedzone wierzchołki muszą mieć finalnie stopień 2
        for (int v = 0; v < n; ++v) {
            if (!node.visited[v]) {
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
            const std::vector<bool>& visited
    ) {
        int best = INF;
        const int n = instance.dimension;

        for (int to = 0; to < n; ++to) {
            if (to == from) {
                continue;
            }

            if (!visited[to] || to == Config::START_VERTEX) {
                best = std::min(best, instance.distanceMatrix[from][to]);
            }
        }

        return best;
    }

    int minIncomingFromUnvisitedOrCurrent(
            const TSPInstance& instance,
            int to,
            const std::vector<bool>& visited,
            int currentVertex
    ) {
        int best = INF;
        const int n = instance.dimension;

        for (int from = 0; from < n; ++from) {
            if (from == to) {
                continue;
            }

            if (!visited[from] || from == currentVertex) {
                best = std::min(best, instance.distanceMatrix[from][to]);
            }
        }

        return best;
    }

    long long hungarianMinCost(const std::vector<std::vector<int>>& a) {
        const int n = static_cast<int>(a.size());
        if (n == 0) {
            return 0;
        }

        std::vector<long long> u(n + 1, 0), v(n + 1, 0);
        std::vector<int> p(n + 1, 0), way(n + 1, 0);

        for (int i = 1; i <= n; ++i) {
            p[0] = i;
            int j0 = 0;
            std::vector<long long> minv(n + 1, static_cast<long long>(INF) * INF);
            std::vector<bool> used(n + 1, false);

            do {
                used[j0] = true;
                const int i0 = p[j0];
                long long delta = static_cast<long long>(INF) * INF;
                int j1 = 0;

                for (int j = 1; j <= n; ++j) {
                    if (used[j]) {
                        continue;
                    }

                    const long long cur =
                            static_cast<long long>(a[i0 - 1][j - 1]) - u[i0] - v[j];

                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }

                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }

                for (int j = 0; j <= n; ++j) {
                    if (used[j]) {
                        u[p[j]] += delta;
                        v[j] -= delta;
                    } else {
                        minv[j] -= delta;
                    }
                }

                j0 = j1;
            } while (p[j0] != 0);

            do {
                const int j1 = way[j0];
                p[j0] = p[j1];
                j0 = j1;
            } while (j0 != 0);
        }

        std::vector<int> assignment(n + 1, 0);
        for (int j = 1; j <= n; ++j) {
            assignment[p[j]] = j;
        }

        long long value = 0;
        for (int i = 1; i <= n; ++i) {
            const int j = assignment[i];
            value += a[i - 1][j - 1];
        }

        return value;
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
                minOutgoingToUnvisitedOrStart(instance, node.current_vertex, node.visited);
        if (currentOut >= INF) {
            return INF;
        }
        outBound += currentOut;

        const int startIn =
                minIncomingFromUnvisitedOrCurrent(
                        instance,
                        Config::START_VERTEX,
                        node.visited,
                        node.current_vertex
                );
        if (startIn >= INF) {
            return INF;
        }
        inBound += startIn;

        for (int v = 0; v < n; ++v) {
            if (!node.visited[v]) {
                const int outMin =
                        minOutgoingToUnvisitedOrStart(instance, v, node.visited);

                const int inMin =
                        minIncomingFromUnvisitedOrCurrent(
                                instance,
                                v,
                                node.visited,
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

    if (static_cast<int>(node.visited.size()) != n) {
        throw std::runtime_error("Rozmiar visited nie zgadza sie z dimension.");
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
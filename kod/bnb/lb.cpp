#include "lb.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>


namespace {
    constexpr int INF = std::numeric_limits<int>::max() / 4;

    struct InternalEdgeCandidate {
        int cost;
        int from;
        int to;
        int nextPosInSorted; // pozycja w sortedNeighbors[from], z której pochodzi kandydat

        bool operator>(const InternalEdgeCandidate& other) const {
            if (cost != other.cost) return cost > other.cost;
            if (from != other.from) return from > other.from;
            return to > other.to;
        }
    };


    std::vector<int> getUnvisitedVertices(const TSPInstance& instance, const BnBNode& node) {
        std::vector<int> unvisited;
        unvisited.reserve(instance.dimension - node.level);

        for (int v = 0; v < instance.dimension; ++v) {
            if (!node.visited[v]) {
                unvisited.push_back(v);
            }
        }
        return unvisited;
    }

    int getMinEdgeFromCurrentToUnvisited(
            const TSPInstance& instance,
            int from,
            const std::vector<bool>& visited
    ) {
        for (int to : instance.sortedNeighbors[from]) {
            if (!visited[to]) {
                return instance.distanceMatrix[from][to];
            }
        }
        return INF;
    }

    int getMinEdgeFromUnvisitedToStart(
            const TSPInstance& instance,
            const std::vector<bool>& visited
    ) {
        const int start = Config::START_VERTEX;
        for (int from : instance.sortedInNeighbors[start]) {
            if (!visited[from]) {
                return instance.distanceMatrix[from][start];
            }
        }
        return INF;
    }

    // Zwraca kolejnego poprawnego kandydata krawędzi wewnętrznej wychodzącej z "from",
    // korzystając z sortedNeighbors[from].
    // Dla TSP symetrycznego unikamy duplikatów przez warunek from < to.
    // Dla ATSP bierzemy wszystkie łuki from - to, gdzie oba końce są nieodwiedzone.
    bool getNextInternalCandidate(
            const TSPInstance& instance,
            int from,
            const std::vector<bool>& visited,
            int startPos,
            InternalEdgeCandidate& outCandidate
    ) {
        const auto& neigh = instance.sortedNeighbors[from];
        const bool symmetric = instance.symmetric;

        for (int pos = startPos; pos < static_cast<int>(neigh.size()); ++pos) {
            const int to = neigh[pos];

            if (to == from) {
                continue;
            }

            if (visited[to]) {
                continue;
            }

            // W TSP symetrycznym chcemy każdą krawędź nieskierowaną policzyć tylko raz.
            // Przyjmujemy konwencję: bierzemy tylko te z from < to.
            if (symmetric && from > to) {
                continue;
            }

            outCandidate.cost = instance.distanceMatrix[from][to];
            outCandidate.from = from;
            outCandidate.nextPosInSorted = pos;
            return true;
        }

        return false;
    }

    // Suma (|U|-1) najmniejszych połączeń wewnątrz zbioru nieodwiedzonych,
    // wyciąganych z list sortedNeighbors, bez pełnego skanowania macierzy.
    // - TSP symetryczne: bierzemy (k-1) najmniejszych krawędzi nieskierowanych w U
    // - ATSP: bierzemy (k-1) najmniejszych łuków skierowanych w U
    // "zliczać najmniejsze odległości, które łączą dwa nieodwiedzone węzły"

    int getInternalResidualBound(
            const TSPInstance& instance,
            const std::vector<int>& unvisited,
            const std::vector<bool>& visited
    ) {
        const int k = static_cast<int>(unvisited.size());

        if (k <= 1) {
            return 0;
        }

        const int neededEdges = k - 1;

        std::priority_queue<
                InternalEdgeCandidate,
                std::vector<InternalEdgeCandidate>,
                std::greater<>
        > pq;

        // Startowo wrzucamy po jednym najtańszym poprawnym kandydacie z każdego nieodwiedzonego źródła.
        for (int from : unvisited) {
            InternalEdgeCandidate cand{};
            if (getNextInternalCandidate(instance, from, visited, 0, cand)) {
                pq.push(cand);
            }
        }

        long long sum = 0;
        int taken = 0;

        while (!pq.empty() && taken < neededEdges) {
            InternalEdgeCandidate best = pq.top();
            pq.pop();

            sum += best.cost;
            if (sum >= INF) {
                return INF;
            }
            ++taken;

            // Szukamy kolejnego poprawnego kandydata z tego samego "from"
            InternalEdgeCandidate nextCand{};
            if (getNextInternalCandidate(
                    instance,
                    best.from,
                    visited,
                    best.nextPosInSorted + 1,
                    nextCand
            )) {
                pq.push(nextCand);
            }
        }

        // Jeżeli nie udało się zebrać k-1 połączeń, traktujemy to jako brak sensownego LB.
        if (taken < neededEdges) {
            return INF;
        }

        return static_cast<int>(sum);
    }
}
int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
    return node.partial_cost +
           instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
}

int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
    const int n = instance.dimension;
    const int startNode = Config::START_VERTEX;
    const int currentNode = node.current_vertex;

    // Pełna ścieżka - trzeba tylko domknąć cykl
    if (node.level == n) {
        return node.partial_cost + instance.distanceMatrix[currentNode][startNode];
    }

    const std::vector<int> unvisited = getUnvisitedVertices(instance, node);

    if (unvisited.empty()) {
        return node.partial_cost + instance.distanceMatrix[currentNode][startNode];
    }

    long long lb = node.partial_cost;

    // 1. Wyjście z current do zbioru nieodwiedzonych
    const int outFromCurrent =
            getMinEdgeFromCurrentToUnvisited(instance, currentNode, node.visited);
    if (outFromCurrent >= INF) {
        return INF;
    }
    lb += outFromCurrent;

    // 2. Powrót ze zbioru nieodwiedzonych do startu
    const int inToStart =
            getMinEdgeFromUnvisitedToStart(instance, node.visited);
    if (inToStart >= INF) {
        return INF;
    }
    lb += inToStart;

    // 3. Połączenia wyłącznie pomiędzy nieodwiedzonymi
    const int internalBound =
            getInternalResidualBound(instance, unvisited, node.visited);
    if (internalBound >= INF) {
        return INF;
    }
    lb += internalBound;

    return (lb >= INF) ? INF : static_cast<int>(lb);
}

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
    //
    // Dla TSP symetrycznego unikamy duplikatów przez warunek from < to.
    // Dla ATSP bierzemy wszystkie łuki from -> to, gdzie oba końce są nieodwiedzone.
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
    //
    // Interpretacja:
    // - TSP symetryczne: bierzemy (k-1) najmniejszych krawędzi nieskierowanych w U
    // - ATSP: bierzemy (k-1) najmniejszych łuków skierowanych w U
    //
    // To jest przybliżone LB zgodne z ideą slajdu:
    // "zliczać najmniejsze odległości, które łączą dwa nieodwiedzone węzły".
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


//#include "lb.h"
//
//#include <algorithm>
//#include <limits>
//#include <vector>
//
//namespace {
//    constexpr int INF = std::numeric_limits<int>::max() / 4;
//
//    std::vector<int> getUnvisitedVertices(const TSPInstance& instance, const BnBNode& node) {
//        std::vector<int> unvisited;
//        unvisited.reserve(instance.dimension - node.level);
//
//        for (int v = 0; v < instance.dimension; ++v) {
//            if (!node.visited[v]) {
//                unvisited.push_back(v);
//            }
//        }
//        return unvisited;
//    }
//
//    // =========================
//    // ======= TSP LB ==========
//    // =========================
//
//    int getTspDegreeNeed(const BnBNode& node, int vertex) {
//        const int start = Config::START_VERTEX;
//        const int current = node.current_vertex;
//
//        // Nieodwiedzony wierzcholek musi finalnie miec stopien 2
//        if (!node.visited[vertex]) {
//            return 2;
//        }
//
//        // Root: start == current i jeszcze nie uzyto zadnej krawedzi
//        if (node.level == 1 && vertex == start) {
//            return 2;
//        }
//
//        // Start i current sa koncami czesciowej sciezki - potrzebuja jeszcze po 1 krawedzi
//        if (vertex == start || vertex == current) {
//            return 1;
//        }
//
//        // Pozostale odwiedzone wierzcholki sa juz "zamkniete"
//        return 0;
//    }
//
//    bool isAllowedResidualEdgeTSP(const BnBNode& node, int a, int b) {
//        if (a == b) {
//            return false;
//        }
//
//        const int needA = getTspDegreeNeed(node, a);
//        const int needB = getTspDegreeNeed(node, b);
//
//        if (needA == 0 || needB == 0) {
//            return false;
//        }
//
//        const bool aUnvisited = !node.visited[a];
//        const bool bUnvisited = !node.visited[b];
//
//        // Dozwolone:
//        // - unvisited <-> unvisited
//        // - start/current <-> unvisited
//        // Niedozwolone:
//        // - start <-> current, gdy sa jeszcze nieodwiedzone miasta
//        if (!aUnvisited && !bUnvisited) {
//            return false;
//        }
//
//        return true;
//    }
//
//    int getTspResidualBound(const TSPInstance& instance, const BnBNode& node) {
//        long long degreeSum = 0;
//
//        for (int v = 0; v < instance.dimension; ++v) {
//            const int need = getTspDegreeNeed(node, v);
//            if (need == 0) {
//                continue;
//            }
//
//            int taken = 0;
//            for (int to : instance.sortedNeighbors[v]) {
//                if (!isAllowedResidualEdgeTSP(node, v, to)) {
//                    continue;
//                }
//
//                degreeSum += instance.distanceMatrix[v][to];
//                if (degreeSum >= 2LL * INF) {
//                    return INF;
//                }
//
//                ++taken;
//                if (taken == need) {
//                    break;
//                }
//            }
//
//            if (taken < need) {
//                return INF;
//            }
//        }
//
//        // Kazda przyszla krawedz zostala policzona po obu koncach
//        return static_cast<int>((degreeSum + 1) / 2);
//    }
//
//    // =========================
//    // ======= ATSP LB =========
//    // =========================
//
//    int getAtspOutNeed(const BnBNode& node, int vertex) {
//        const int start = Config::START_VERTEX;
//        const int current = node.current_vertex;
//
//        // Nieodwiedzony wierzcholek musi jeszcze z niego wyjsc 1 luk
//        if (!node.visited[vertex]) {
//            return 1;
//        }
//
//        // Root: start == current i trzeba jeszcze wyjsc 1 raz
//        if (node.level == 1 && vertex == start) {
//            return 1;
//        }
//
//        // Aktualny koniec sciezki musi jeszcze miec jedno wyjscie
//        if (vertex == current) {
//            return 1;
//        }
//
//        return 0;
//    }
//
//    int getAtspInNeed(const BnBNode& node, int vertex) {
//        const int start = Config::START_VERTEX;
//
//        // Nieodwiedzony wierzcholek musi jeszcze dostac 1 wejscie
//        if (!node.visited[vertex]) {
//            return 1;
//        }
//
//        // Root: start == current i trzeba jeszcze wejsc do startu 1 raz
//        if (node.level == 1 && vertex == start) {
//            return 1;
//        }
//
//        // Start musi zostac zamkniety jednym lukiem na koncu
//        if (vertex == start) {
//            return 1;
//        }
//
//        return 0;
//    }
//
//    bool isAllowedResidualArcATSP(const BnBNode& node, int from, int to) {
//        if (from == to) {
//            return false;
//        }
//
//        const int outNeed = getAtspOutNeed(node, from);
//        const int inNeed = getAtspInNeed(node, to);
//
//        if (outNeed == 0 || inNeed == 0) {
//            return false;
//        }
//
//        const int start = Config::START_VERTEX;
//
//        // Z nieodwiedzonego wolno isc:
//        // - do nieodwiedzonego
//        // - do startu
//        if (!node.visited[from]) {
//            return (!node.visited[to] || to == start);
//        }
//
//        // Z current wolno isc tylko do nieodwiedzonego
//        if (from == node.current_vertex) {
//            return !node.visited[to];
//        }
//
//        return false;
//    }
//
//    int getAtspOutgoingResidualBound(const TSPInstance& instance, const BnBNode& node) {
//        long long sumOut = 0;
//
//        for (int from = 0; from < instance.dimension; ++from) {
//            const int need = getAtspOutNeed(node, from);
//            if (need == 0) {
//                continue;
//            }
//
//            int taken = 0;
//            for (int to : instance.sortedNeighbors[from]) {
//                if (!isAllowedResidualArcATSP(node, from, to)) {
//                    continue;
//                }
//
//                sumOut += instance.distanceMatrix[from][to];
//                if (sumOut >= INF) {
//                    return INF;
//                }
//
//                ++taken;
//                if (taken == need) {
//                    break;
//                }
//            }
//
//            if (taken < need) {
//                return INF;
//            }
//        }
//
//        return static_cast<int>(sumOut);
//    }
//
//    int getAtspIncomingResidualBound(const TSPInstance& instance, const BnBNode& node) {
//        long long sumIn = 0;
//
//        for (int to = 0; to < instance.dimension; ++to) {
//            const int need = getAtspInNeed(node, to);
//            if (need == 0) {
//                continue;
//            }
//
//            int taken = 0;
//            for (int from : instance.sortedInNeighbors[to]) {
//                if (!isAllowedResidualArcATSP(node, from, to)) {
//                    continue;
//                }
//
//                sumIn += instance.distanceMatrix[from][to];
//                if (sumIn >= INF) {
//                    return INF;
//                }
//
//                ++taken;
//                if (taken == need) {
//                    break;
//                }
//            }
//
//            if (taken < need) {
//                return INF;
//            }
//        }
//
//        return static_cast<int>(sumIn);
//    }
//
//    int getAtspResidualBound(const TSPInstance& instance, const BnBNode& node) {
//        const int outBound = getAtspOutgoingResidualBound(instance, node);
//        if (outBound >= INF) {
//            return INF;
//        }
//
//        const int inBound = getAtspIncomingResidualBound(instance, node);
//        if (inBound >= INF) {
//            return INF;
//        }
//
//        return std::max(outBound, inBound);
//    }
//}
//
//int computeCompletionCost(const TSPInstance& instance, const BnBNode& node) {
//    return node.partial_cost +
//           instance.distanceMatrix[node.current_vertex][Config::START_VERTEX];
//}
//
//int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
//    const int n = instance.dimension;
//    const int startNode = Config::START_VERTEX;
//    const int currentNode = node.current_vertex;
//
//    // Pelna sciezka - trzeba tylko domknac cykl
//    if (node.level == n) {
//        return node.partial_cost + instance.distanceMatrix[currentNode][startNode];
//    }
//
//    const std::vector<int> unvisited = getUnvisitedVertices(instance, node);
//
//    if (unvisited.empty()) {
//        return node.partial_cost + instance.distanceMatrix[currentNode][startNode];
//    }
//
//    long long lb = node.partial_cost;
//
//    const int residualBound = instance.symmetric
//                              ? getTspResidualBound(instance, node)
//                              : getAtspResidualBound(instance, node);
//
//    if (residualBound >= INF) {
//        return INF;
//    }
//
//    lb += residualBound;
//
//    return (lb >= INF) ? INF : static_cast<int>(lb);
//}
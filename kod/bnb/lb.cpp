#include "lb.h"
#include <algorithm>
#include <limits>
#include <vector>
#include "../common/config.h"

namespace {
    constexpr int INF = std::numeric_limits<int>::max() / 2;

    // Pomocnicza funkcja do szybkiego szukania najtańszej krawędzi wyjściowej
    // używająca posortowanej listy sąsiadów.
    int getMinEdgeToUnvisited(const TSPInstance& instance, int from, const std::vector<bool>& visited) {
        // Zakładamy, że instance.sortedNeighbors[from] zawiera ID sąsiadów
        // posortowanych według wag w distanceMatrix[from][sąsiad]
        for (int to : instance.sortedNeighbors[from]) {
            if (!visited[to]) {
                return instance.distanceMatrix[from][to];
            }
        }
        return INF;
    }

    // Dla 3. metody: najtańszy wjazd do v z innego NIEODWIEDZONEGO u
    int getMinInternalEdge(const TSPInstance& instance, int v, const std::vector<bool>& visited) {
        // Jeśli masz listę posortowanych wjazdów (InNeighbors), użyj jej.
        // Jeśli nie, musimy przeszukać nieodwiedzone węzły.
        int minIn = INF;
        const int n = instance.dimension;

        for (int u = 0; u < n; ++u) {
            if (u != v && !visited[u]) {
                if (instance.distanceMatrix[u][v] < minIn) {
                    minIn = instance.distanceMatrix[u][v];
                }
            }
        }
        return minIn;
    }
}

int computeLowerBound(const TSPInstance& instance, const BnBNode& node) {
    const int n = instance.dimension;
    const int startNode = Config::START_VERTEX;
    const int currentNode = node.current_vertex;

    // 1. Jeśli ścieżka jest pełna - koszt rzeczywisty (domknięcie cyklu)
    if (node.level == n) {
        return node.partial_cost + instance.distanceMatrix[currentNode][startNode];
    }

    long long lb = node.partial_cost;

    // 2. Koszt wyjścia z aktualnego wierzchołka do "chmury" nieodwiedzonych
    int outFromCurrent = getMinEdgeToUnvisited(instance, currentNode, node.visited);
    if (outFromCurrent >= INF) return INF;
    lb += outFromCurrent;

    // 3. Koszt powrotu do startu (S) z "chmury" nieodwiedzonych
    // Szukamy najtańszej krawędzi u -> startNode, gdzie u jest nieodwiedzone
    int inToStart = INF;
    for (int u = 0; u < n; ++u) {
        if (!node.visited[u]) {
            if (instance.distanceMatrix[u][startNode] < inToStart) {
                inToStart = instance.distanceMatrix[u][startNode];
            }
        }
    }
    if (inToStart >= INF) return INF;
    lb += inToStart;

    // 4. KOSZT WEWNĘTRZNY (serce 3. metody)
    // Dla każdego nieodwiedzonego węzła szukamy najtańszego wjazdu
    // z INNEGO nieodwiedzonego węzła.
    for (int v = 0; v < n; ++v) {
        if (!node.visited[v]) {
            int internalIn = getMinInternalEdge(instance, v, node.visited);

            // Jeśli został tylko 1 nieodwiedzony węzeł, getMinInternalEdge zwróci INF.
            // Jest to poprawne, bo ten węzeł jest już połączony
            // krawędziami z pkt 2 i 3 (current -> v -> start).
            if (internalIn < INF) {
                lb += internalIn;
            }
        }
    }

    return (lb > INF) ? INF : static_cast<int>(lb);
}
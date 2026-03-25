#ifndef PEA2_BNB_NODE_H
#define PEA2_BNB_NODE_H
#pragma once

#include <vector>

struct BnBNode {
    std::vector<int> path;            // odwiedzona czesciowa sciezka
    std::vector<bool> visited;        // visited[i] = czy miasto bylo odwiedzone
    int current_vertex = -1;          // ostatni wierzcholek sciezki
    int level = 0;                    // liczba odwiedzonych miast
    int partial_cost = 0;             // koszt czesciowej sciezki
    int lower_bound = 0;              // LB dla stanu
};

struct CompareByLowerBound {
    bool operator()(const BnBNode& a, const BnBNode& b) const {
        return a.lower_bound > b.lower_bound;
    }
};
#endif //PEA2_BNB_NODE_H

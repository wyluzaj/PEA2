#ifndef PEA2_BNB_NODE_H
#define PEA2_BNB_NODE_H
#pragma once

#include <vector>

struct BnBNode {
    std::vector<bool> visited;   // visited[i] = czy miasto bylo odwiedzone
    int parent_index = -1;       // indeks rodzica w puli wezlow
    int current_vertex = -1;     // ostatni wierzcholek sciezki
    int level = 0;               // liczba odwiedzonych miast
    int partial_cost = 0;        // koszt czesciowej sciezki
    int lower_bound = 0;         // LB dla stanu
};

#endif // PEA2_BNB_NODE_H
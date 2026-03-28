#ifndef PEA2_TSP_INSTANCE_H
#define PEA2_TSP_INSTANCE_H
#pragma once

#include <string>
#include <vector>
#include "../common/config.h"

struct City {
    int id = 0;
    double x = 0.0;
    double y = 0.0;
};

struct TSPInstance {
    bool symmetric = true;             // true = TSP, false = ATSP
    std::string name;                  // nazwa instancji
    std::string type;                  // "TSP" albo "ATSP"
    std::string edge_weight_type;      // np. "EUC_2D" albo "EXPLICIT"
    int dimension = 0;

    std::vector<City> cities;          // dla instancji wspolrzednosciowych
    std::vector<std::vector<int>> distanceMatrix;
    std::vector<std::vector<int>> sortedNeighbors;
    std::vector<std::vector<int>> sortedInNeighbors;

    [[nodiscard]] bool isValid() const {
        return dimension > 0 &&
               static_cast<int>(distanceMatrix.size()) == dimension;
    }
};

struct TSPResult {
    std::string algorithm_name;        // BNB_DFS / BNB_BFS / BNB_BESTFS
    std::string instance_name;         // np. berlin52
    std::string instance_type;         // TSP / ATSP
    int vertex_count = 0;              // n

    double total_time_ms = 0.0;        // czas wykonania
    int best_cost = -1;                // koszt najlepszego znalezionego rozwiazania
    std::vector<int> best_path;        // pelna sciezka jako indeksy 0..n-1
    std::string best_path_text;        // tekstowa reprezentacja pelnej sciezki
    int ub_from_nn = -1;               // poczatkowe UB z NN / RNN

    long long visited_nodes = 0;       // ile stanow zdjeto z frontier i sprawdzono
    long long pruned_nodes = 0;        // ile stanow odcieto
    long long generated_nodes = 0;     // ile dzieci rozważono do utworzenia
    long long stored_nodes = 0;        // ile stanow faktycznie trafiło do pamieci
    long long max_frontier_size = 0;   // maksymalny rozmiar frontier
    long long max_node_pool_size = 0;  // maksymalny rozmiar puli nodePool

    bool memory_exhausted = false;
    bool completed_naturally = true;
    std::string stop_reason = "Zakonczono naturalnie";
    std::string summary_file_name;
};

#endif // PEA2_TSP_INSTANCE_H
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

    bool isValid() const {
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
    int best_cost = -1;                // koszt optymalny
    std::vector<int> best_path;        // pelna sciezka jako indeksy 0..n-1
    std::string best_path_text;        // tekstowa reprezentacja pelnej sciezki
    int ub_from_nn = -1;               // poczatkowe UB z NN

    long long visited_nodes = 0;       // pomocniczo do badan
    long long pruned_nodes = 0;        // pomocniczo do badan

    bool completed_naturally = true;
    std::string stop_reason = "Zakonczono naturalnie";
    std::string summary_file_name;
};

#endif // PEA2_TSP_INSTANCE_H
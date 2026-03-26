//#ifndef PEA2_UB_H
//#define PEA2_UB_H
//#pragma once
//
//#include <chrono>
//#include <vector>
//
//#include "../common/tsp_instance.h"
//
//struct UpperBoundResult {
//    int cost = -1;
//    std::vector<int> path;
//    bool finished = false;
//    bool timeoutReached = false;
//};
//
//UpperBoundResult computeInitialUpperBoundNN(
//        const TSPInstance& instance
//);
//#endif //PEA2_UB_H
#ifndef PEA2_UB_H
#define PEA2_UB_H
#pragma once

#include <chrono>
#include <vector>

#include "../common/tsp_instance.h"

struct UpperBoundResult {
    int cost = -1;
    std::vector<int> path;
    bool finished = false;
    bool timeoutReached = false;
};

UpperBoundResult computeInitialUpperBoundNN(
        const TSPInstance& instance
);

UpperBoundResult computeInitialUpperBoundRNN(
        const TSPInstance& instance
);

#endif //PEA2_UB_H
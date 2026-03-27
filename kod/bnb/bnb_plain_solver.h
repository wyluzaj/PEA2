#ifndef PEA2_BNB_PLAIN_SOLVER_H
#define PEA2_BNB_PLAIN_SOLVER_H
#pragma once

#include <string>

#include "../common/tsp_instance.h"
#include "../strategies/search_strategy.h"

enum class InitialUBMode {
    INF,
    RNN
};

TSPResult solveBranchAndBoundPlain(
        const TSPInstance& instance,
        ISearchFrontier& frontier,
        const std::string& algorithmName,
        InitialUBMode ubMode,
        double maxTimeSeconds = 15.0
);

#endif //PEA2_BNB_PLAIN_SOLVER_H
#ifndef PEA2_BNB_SOLVER_H
#define PEA2_BNB_SOLVER_H
#pragma once

#include <chrono>
#include <string>

#include "../common/tsp_instance.h"
#include "../strategies/search_strategy.h"

TSPResult solveBranchAndBound(
        const TSPInstance& instance,
        ISearchFrontier& frontier,
        const std::string& algorithmName,
        double maxTimeSeconds = 15.0
);
#endif //PEA2_BNB_SOLVER_H

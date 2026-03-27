#ifndef PEA2_APP_PLAIN_COMMON_H
#define PEA2_APP_PLAIN_COMMON_H
#pragma once

#include <string>

#include "../bnb/bnb_plain_solver.h"
#include "../strategies/search_strategy.h"

int runPlainApplication(
        int argc,
        char* argv[],
        ISearchFrontier& frontier,
        const std::string& algorithmName,
        InitialUBMode ubMode
);

#endif //PEA2_APP_PLAIN_COMMON_H

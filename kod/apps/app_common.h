#ifndef PEA2_APP_COMMON_H
#define PEA2_APP_COMMON_H
#pragma once

#include <string>

#include "../common/tsp_instance.h"
#include "../strategies/search_strategy.h"

int runApplication(
        int argc,
        char* argv[],
        ISearchFrontier& frontier,
        const std::string& algorithmName
);

#endif //PEA2_APP_COMMON_H

#ifndef PEA2_LB_H
#define PEA2_LB_H

#pragma once

#include "../common/tsp_instance.h"
#include "bnb_node.h"

int computeLowerBound(const TSPInstance& instance, const BnBNode& node);
int computeCompletionCost(const TSPInstance& instance, const BnBNode& node);
#endif //PEA2_LB_H

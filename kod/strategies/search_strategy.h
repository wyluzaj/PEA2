#ifndef PEA2_SEARCH_STRATEGY_H
#define PEA2_SEARCH_STRATEGY_H
#pragma once

#include <cstddef>

#include "../bnb/bnb_node.h"

class ISearchFrontier {
public:
    virtual ~ISearchFrontier() = default;

    virtual void push(const BnBNode& node) = 0;
    virtual BnBNode pop() = 0;
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0;
};

#endif //PEA2_SEARCH_STRATEGY_H

#ifndef PEA2_SEARCH_STRATEGY_H
#define PEA2_SEARCH_STRATEGY_H
#pragma once

#include <cstddef>

using NodeId = int;

class ISearchFrontier {
public:
    virtual ~ISearchFrontier() = default;

    virtual void push(NodeId nodeId, int lowerBound) = 0;
    virtual NodeId pop() = 0;
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0;
};

#endif //PEA2_SEARCH_STRATEGY_H

//#ifndef PEA2_SEARCH_STRATEGY_H
//#define PEA2_SEARCH_STRATEGY_H
//#pragma once
//
//#include <cstddef>
//
//using NodeId = int;
//
//class ISearchFrontier {
//public:
//    virtual ~ISearchFrontier() = default;
//
//    virtual void push(NodeId nodeId, int lowerBound) = 0;
//    virtual NodeId pop() = 0;
//    virtual bool empty() const = 0;
//    virtual std::size_t size() const = 0;
//};
//
//#endif //PEA2_SEARCH_STRATEGY_H
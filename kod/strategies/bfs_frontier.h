#ifndef PEA2_BFS_FRONTIER_H
#define PEA2_BFS_FRONTIER_H
#pragma once

#include <queue>
#include <stdexcept>

#include "search_strategy.h"

class BFSFrontier : public ISearchFrontier {
private:
    std::queue<NodeId> data;

public:
    BFSFrontier() = default;

    void push(NodeId nodeId, int /*lowerBound*/) override {
        data.push(nodeId);
    }

    NodeId pop() override {
        if (data.empty()) {
            throw std::runtime_error("BFSFrontier: proba pobrania z pustej struktury.");
        }

        NodeId nodeId = data.front();
        data.pop();
        return nodeId;
    }

    bool empty() const override {
        return data.empty();
    }

    std::size_t size() const override {
        return data.size();
    }
};

#endif //PEA2_BFS_FRONTIER_H
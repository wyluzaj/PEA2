#ifndef PEA2_DFS_FRONTIER_H
#define PEA2_DFS_FRONTIER_H
#pragma once

#include <stdexcept>
#include <vector>

#include "search_strategy.h"

class DFSFrontier : public ISearchFrontier {
private:
    std::vector<NodeId> data;

public:
    DFSFrontier() = default;

    void push(NodeId nodeId, int /*lowerBound*/) override {
        data.push_back(nodeId);
    }

    NodeId pop() override {
        if (data.empty()) {
            throw std::runtime_error("DFSFrontier: proba pobrania z pustej struktury.");
        }

        NodeId nodeId = data.back();
        data.pop_back();
        return nodeId;
    }

    bool empty() const override {
        return data.empty();
    }

    std::size_t size() const override {
        return data.size();
    }
};

#endif //PEA2_DFS_FRONTIER_H
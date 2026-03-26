#ifndef PEA2_BESTFS_FRONTIER_H
#define PEA2_BESTFS_FRONTIER_H
#pragma once

#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

#include "search_strategy.h"

struct BestFSItem {
    int lower_bound;
    NodeId node_id;

    bool operator>(const BestFSItem& other) const {
        if (lower_bound != other.lower_bound) {
            return lower_bound > other.lower_bound;
        }
        return node_id > other.node_id;
    }
};

class BestFSFrontier : public ISearchFrontier {
private:
    std::priority_queue<
            BestFSItem,
            std::vector<BestFSItem>,
            std::greater<>
    > data;

public:
    BestFSFrontier() = default;

    void push(NodeId nodeId, int lowerBound) override {
        data.push(BestFSItem{lowerBound, nodeId});
    }

    NodeId pop() override {
        if (data.empty()) {
            throw std::runtime_error("BestFSFrontier: proba pobrania z pustej struktury.");
        }

        NodeId nodeId = data.top().node_id;
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

#endif //PEA2_BESTFS_FRONTIER_H
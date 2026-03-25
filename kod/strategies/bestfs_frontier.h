#ifndef PEA2_BESTFS_FRONTIER_H
#define PEA2_BESTFS_FRONTIER_H
#pragma once

#include <queue>
#include <stdexcept>
#include <vector>

#include "../bnb/bnb_node.h"
#include "search_strategy.h"

class BestFSFrontier : public ISearchFrontier {
private:
    std::priority_queue<BnBNode, std::vector<BnBNode>, CompareByLowerBound> data;

public:
    BestFSFrontier() = default;

    void push(const BnBNode& node) override {
        data.push(node);
    }

    BnBNode pop() override {
        if (data.empty()) {
            throw std::runtime_error("BestFSFrontier: proba pobrania z pustej struktury.");
        }

        BnBNode node = data.top();
        data.pop();
        return node;
    }

    bool empty() const override {
        return data.empty();
    }

    std::size_t size() const override {
        return data.size();
    }
};

#endif //PEA2_BESTFS_FRONTIER_H

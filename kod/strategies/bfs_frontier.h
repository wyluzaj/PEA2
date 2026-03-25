#ifndef PEA2_BFS_FRONTIER_H
#define PEA2_BFS_FRONTIER_H
#pragma once

#include <queue>
#include <stdexcept>

#include "search_strategy.h"

class BFSFrontier : public ISearchFrontier {
private:
    std::queue<BnBNode> data;

public:
    BFSFrontier() = default;

    void push(const BnBNode& node) override {
        data.push(node);
    }

    BnBNode pop() override {
        if (data.empty()) {
            throw std::runtime_error("BFSFrontier: proba pobrania z pustej struktury.");
        }

        BnBNode node = data.front();
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

#endif //PEA2_BFS_FRONTIER_H

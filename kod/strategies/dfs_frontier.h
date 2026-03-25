#ifndef PEA2_DFS_FRONTIER_H
#define PEA2_DFS_FRONTIER_H

#pragma once

#include <stdexcept>
#include <vector>

#include "search_strategy.h"

class DFSFrontier : public ISearchFrontier {
private:
    std::vector<BnBNode> data;

public:
    DFSFrontier() = default;

    void push(const BnBNode& node) override {
        data.push_back(node);
    }

    BnBNode pop() override {
        if (data.empty()) {
            throw std::runtime_error("DFSFrontier: proba pobrania z pustej struktury.");
        }

        BnBNode node = data.back();
        data.pop_back();
        return node;
    }

    bool empty() const override {
        return data.empty();
    }

    std::size_t size() const override {
        return data.size();
    }
};

#endif //PEA2_DFS_FRONTIER_H

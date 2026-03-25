
#ifndef PEA2_UTILS_H
#define PEA2_UTILS_H
#pragma once

#include <chrono>
#include <random>
#include <string>
#include <vector>

#include "tsp_instance.h"

int calculateDistance(const City& a, const City& b);
void buildDistanceMatrix(TSPInstance& instance);

int calculatePathCost(const TSPInstance& instance, const std::vector<int>& path);
int calculatePartialPathCost(const TSPInstance& instance, const std::vector<int>& path);

std::string getFileNameWithoutExtension(const std::string& filePath);
std::string normalizePath(const std::string& path);

std::vector<int> makeBasePath(int n);
std::vector<int> generateRandomPermutationKeepingStart(int n, std::mt19937& gen);

std::vector<int> nearestNeighborPath(
        const TSPInstance& instance,
        int startVertex,
        const std::chrono::high_resolution_clock::time_point& globalStartTime,
        double maxTimeSeconds,
        bool* finished = nullptr,
        bool* timeoutReached = nullptr
);

double elapsedMilliseconds(
        const std::chrono::high_resolution_clock::time_point& start,
        const std::chrono::high_resolution_clock::time_point& end
);

std::string pathToString(const std::vector<int>& path, const TSPInstance& instance);
void buildSortedNeighbors(TSPInstance& instance);

#endif //PEA2_UTILS_H

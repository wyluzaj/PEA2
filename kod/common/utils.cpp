#include "utils.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

int calculateDistance(const City& a, const City& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
}
void buildSortedNeighbors(TSPInstance& instance) {
    const int n = instance.dimension;
    instance.sortedNeighbors.assign(n, std::vector<int>());
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                instance.sortedNeighbors[i].push_back(j);
            }
        }
        std::sort(instance.sortedNeighbors[i].begin(), instance.sortedNeighbors[i].end(),
                  [&](int a, int b) {
                      return instance.distanceMatrix[i][a] < instance.distanceMatrix[i][b];
                  });
    }
}
void buildDistanceMatrix(TSPInstance& instance) {
    const int n = instance.dimension;

    if (n <= 0) {
        throw std::runtime_error("Niepoprawny rozmiar instancji.");
    }

    if (static_cast<int>(instance.cities.size()) != n) {
        throw std::runtime_error("Liczba miast nie zgadza sie z DIMENSION.");
    }

    instance.distanceMatrix.assign(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                instance.distanceMatrix[i][j] = 0;
            } else {
                instance.distanceMatrix[i][j] =
                        calculateDistance(instance.cities[i], instance.cities[j]);
            }
        }
    }

    buildSortedNeighbors(instance);
    instance.symmetric = true;
    if (instance.type.empty()) {
        instance.type = "TSP";
    }
    if (instance.edge_weight_type.empty()) {
        instance.edge_weight_type = "EUC_2D";
    }
}

int calculatePartialPathCost(const TSPInstance& instance, const std::vector<int>& path) {
    if (instance.distanceMatrix.empty()) {
        throw std::runtime_error("Macierz odleglosci nie zostala zbudowana.");
    }

    if (path.empty() || path.size() == 1) {
        return 0;
    }

    int totalCost = 0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const int from = path[i];
        const int to = path[i + 1];

        if (from < 0 || from >= instance.dimension || to < 0 || to >= instance.dimension) {
            throw std::runtime_error("Sciezka zawiera niepoprawny indeks miasta.");
        }

        totalCost += instance.distanceMatrix[from][to];
    }

    return totalCost;
}

int calculatePathCost(const TSPInstance& instance, const std::vector<int>& path) {
    if (instance.distanceMatrix.empty()) {
        throw std::runtime_error("Macierz odleglosci nie zostala zbudowana.");
    }

    if (path.empty()) {
        return 0;
    }

    if (path.size() != static_cast<size_t>(instance.dimension)) {
        throw std::runtime_error("Sciezka musi zawierac kazde miasto dokladnie raz.");
    }

    int totalCost = calculatePartialPathCost(instance, path);
    totalCost += instance.distanceMatrix[path.back()][path.front()];
    return totalCost;
}

std::string getFileNameWithoutExtension(const std::string& filePath) {
    std::filesystem::path path(filePath);
    return path.stem().string();
}

std::string normalizePath(const std::string& path) {
    std::string result = path;

    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.front()))) {
        result.erase(result.begin());
    }

    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }

    if (!result.empty() && result.front() == '"' && result.back() == '"' && result.size() >= 2) {
        result = result.substr(1, result.size() - 2);
    }

    return result;
}

std::vector<int> makeBasePath(int n) {
    std::vector<int> path(n);
    std::iota(path.begin(), path.end(), 0);
    return path;
}

std::vector<int> generateRandomPermutationKeepingStart(int n, std::mt19937& gen) {
    std::vector<int> path = makeBasePath(n);
    if (n > 1) {
        std::shuffle(path.begin() + 1, path.end(), gen);
    }
    return path;
}

std::vector<int> nearestNeighborPath(
        const TSPInstance& instance,
        int startVertex,
        const std::chrono::high_resolution_clock::time_point& globalStartTime,
        double maxTimeSeconds,
        bool* finished,
        bool* timeoutReached
) {
    using clock = std::chrono::high_resolution_clock;

    if (instance.distanceMatrix.empty()) {
        throw std::runtime_error("Macierz odleglosci nie zostala zbudowana.");
    }

    if (startVertex < 0 || startVertex >= instance.dimension) {
        throw std::runtime_error("Niepoprawny wierzcholek startowy.");
    }

    if (finished != nullptr) {
        *finished = false;
    }
    if (timeoutReached != nullptr) {
        *timeoutReached = false;
    }

    std::vector<bool> visited(instance.dimension, false);
    std::vector<int> path;
    path.reserve(instance.dimension);

    int current = startVertex;
    visited[current] = true;
    path.push_back(current);

    for (int step = 1; step < instance.dimension; ++step) {
        const double elapsedSeconds =
                std::chrono::duration<double>(clock::now() - globalStartTime).count();

        if (elapsedSeconds >= maxTimeSeconds) {
            if (timeoutReached != nullptr) {
                *timeoutReached = true;
            }
            return path;
        }

        int nearestCity = -1;
        int bestDistance = std::numeric_limits<int>::max();

        for (int candidate = 0; candidate < instance.dimension; ++candidate) {
            if (!visited[candidate] &&
                instance.distanceMatrix[current][candidate] < bestDistance) {
                bestDistance = instance.distanceMatrix[current][candidate];
                nearestCity = candidate;
            }
        }

        if (nearestCity == -1) {
            return path;
        }

        visited[nearestCity] = true;
        path.push_back(nearestCity);
        current = nearestCity;
    }

    if (finished != nullptr) {
        *finished = true;
    }

    return path;
}

double elapsedMilliseconds(
        const std::chrono::high_resolution_clock::time_point& start,
        const std::chrono::high_resolution_clock::time_point& end
) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::string pathToString(const std::vector<int>& path, const TSPInstance& instance) {
    std::ostringstream oss;
    const bool hasCityIds =
            (instance.cities.size() == static_cast<size_t>(instance.dimension));

    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) {
            oss << " -> ";
        }

        const int idx = path[i];

        if (hasCityIds && idx >= 0 && idx < instance.dimension) {
            oss << instance.cities[idx].id;
        } else {
            oss << (idx + 1);
        }
    }

    if (!path.empty()) {
        if (hasCityIds && path.front() >= 0 && path.front() < instance.dimension) {
            oss << " -> " << instance.cities[path.front()].id;
        } else {
            oss << " -> " << (path.front() + 1);
        }
    }

    return oss.str();
}
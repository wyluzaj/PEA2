#include "instance_loader.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "reader.h"
#include "reader_matrix.h"
#include "utils.h"

TSPInstance loadInstance(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filePath);
    }

    const std::string content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
    );

    if (content.find("NODE_COORD_SECTION") != std::string::npos) {
        TSPInstance instance = readTSPFile(filePath);
        buildDistanceMatrix(instance);

        if (instance.type.empty()) {
            instance.type = "TSP";
        }

        return instance;
    }

    if (content.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
        return readMatrixInstance(filePath);
    }

    throw std::runtime_error("Nieznany format instancji: " + filePath);
}
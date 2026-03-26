#include "reader_matrix.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    std::string trim(const std::string& s) {
        const size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return "";
        }

        const size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    }

    std::string getValueAfterColon(const std::string& line) {
        const size_t pos = line.find(':');
        if (pos == std::string::npos) {
            return "";
        }
        return trim(line.substr(pos + 1));
    }

    bool startsWith(const std::string& text, const std::string& prefix) {
        return text.rfind(prefix, 0) == 0;
    }
}

TSPInstance readMatrixInstance(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filePath);
    }

    TSPInstance instance;
    std::string line;
    bool foundEdgeWeightSection = false;
    std::string edgeWeightFormat;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (startsWith(line, "NAME")) {
            instance.name = getValueAfterColon(line);
        }
        else if (startsWith(line, "TYPE")) {
            instance.type = getValueAfterColon(line);

            if (instance.type == "TSP") {
                instance.symmetric = true;
            }
            else if (instance.type == "ATSP") {
                instance.symmetric = false;
            }
            else {
                throw std::runtime_error("Nieznany typ instancji: " + instance.type);
            }
        }
        else if (startsWith(line, "DIMENSION")) {
            instance.dimension = std::stoi(getValueAfterColon(line));

            if (instance.dimension <= 0) {
                throw std::runtime_error("Niepoprawne DIMENSION w pliku: " + filePath);
            }
        }
        else if (startsWith(line, "EDGE_WEIGHT_TYPE")) {
            instance.edge_weight_type = getValueAfterColon(line);

            if (instance.edge_weight_type != "EXPLICIT") {
                throw std::runtime_error(
                        "Obslugiwany jest tylko EDGE_WEIGHT_TYPE = EXPLICIT. Plik: " + filePath
                );
            }
        }
        else if (startsWith(line, "EDGE_WEIGHT_FORMAT")) {
            edgeWeightFormat = getValueAfterColon(line);

            // ZMIANA 1: Rozszerzamy walidację o LOWER_DIAG_ROW
            if (edgeWeightFormat != "FULL_MATRIX" && edgeWeightFormat != "LOWER_DIAG_ROW") {
                throw std::runtime_error(
                        "Obslugiwany format to FULL_MATRIX lub LOWER_DIAG_ROW. Plik: " + filePath
                );
            }
        }
        else if (startsWith(line, "EDGE_WEIGHT_SECTION")) {
            if (instance.dimension <= 0) {
                throw std::runtime_error("EDGE_WEIGHT_SECTION pojawilo sie przed DIMENSION.");
            }

            foundEdgeWeightSection = true;
            break;
        }
        else if (startsWith(line, "EOF")) {
            break;
        }
    }

    if (instance.dimension <= 0) {
        throw std::runtime_error("Brak lub niepoprawny DIMENSION w pliku: " + filePath);
    }

    if (instance.type.empty()) {
        throw std::runtime_error("Brak TYPE w pliku: " + filePath);
    }

    if (instance.edge_weight_type.empty()) {
        instance.edge_weight_type = "EXPLICIT";
    }

    if (!foundEdgeWeightSection) {
        throw std::runtime_error("Brak EDGE_WEIGHT_SECTION w pliku: " + filePath);
    }

    instance.distanceMatrix.assign(
            instance.dimension,
            std::vector<int>(instance.dimension, 0)
    );
    if (edgeWeightFormat == "FULL_MATRIX") {
        for (int i = 0; i < instance.dimension; ++i) {
            for (int j = 0; j < instance.dimension; ++j) {
                if (!(file >> instance.distanceMatrix[i][j])) {
                    throw std::runtime_error("Za malo danych w FULL_MATRIX.");
                }
            }
        }
    }
    else if (edgeWeightFormat == "LOWER_DIAG_ROW") {
        for (int i = 0; i < instance.dimension; ++i) {
            // W tym formacie wiersz 'i' zawiera 'i + 1' wag (od 0 do przekątnej)
            for (int j = 0; j <= i; ++j) {
                int weight;
                if (!(file >> weight)) {
                    throw std::runtime_error("Za malo danych w LOWER_DIAG_ROW.");
                }
                instance.distanceMatrix[i][j] = weight;
                instance.distanceMatrix[j][i] = weight; // Symetria
            }
        }
    }

    return instance;
}
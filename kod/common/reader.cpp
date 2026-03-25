#include "reader.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            --end;
        }

        return s.substr(start, end - start);
    }

    bool startsWith(const std::string& text, const std::string& prefix) {
        return text.rfind(prefix, 0) == 0;
    }

    std::string valueAfterColon(const std::string& line) {
        const size_t pos = line.find(':');
        if (pos == std::string::npos) {
            return "";
        }
        return trim(line.substr(pos + 1));
    }
}

TSPInstance readTSPFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku: " + path);
    }

    TSPInstance instance;
    std::string line;
    bool inNodeSection = false;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (line == "NODE_COORD_SECTION") {
            inNodeSection = true;
            continue;
        }

        if (line == "EOF") {
            break;
        }

        if (!inNodeSection) {
            if (startsWith(line, "NAME")) {
                instance.name = valueAfterColon(line);
            } else if (startsWith(line, "TYPE")) {
                instance.type = valueAfterColon(line);
                if (instance.type == "TSP") {
                    instance.symmetric = true;
                } else if (instance.type == "ATSP") {
                    instance.symmetric = false;
                }
            } else if (startsWith(line, "DIMENSION")) {
                instance.dimension = std::stoi(valueAfterColon(line));
            } else if (startsWith(line, "EDGE_WEIGHT_TYPE")) {
                instance.edge_weight_type = valueAfterColon(line);
            }
        } else {
            std::istringstream iss(line);
            City city{};

            if (!(iss >> city.id >> city.x >> city.y)) {
                throw std::runtime_error("Blad odczytu miasta w pliku: " + path);
            }

            instance.cities.push_back(city);
        }
    }

    if (instance.dimension <= 0) {
        throw std::runtime_error("Brak lub niepoprawne DIMENSION w pliku: " + path);
    }

    if (instance.cities.empty()) {
        throw std::runtime_error("Brak NODE_COORD_SECTION w pliku: " + path);
    }

    if (static_cast<int>(instance.cities.size()) != instance.dimension) {
        throw std::runtime_error("Liczba miast nie zgadza sie z DIMENSION w pliku: " + path);
    }

    if (!instance.edge_weight_type.empty() && instance.edge_weight_type != "EUC_2D") {
        throw std::runtime_error(
                "Ten reader obsluguje tylko EDGE_WEIGHT_TYPE = EUC_2D. Wczytano: " +
                instance.edge_weight_type
        );
    }

    if (instance.type.empty()) {
        instance.type = "TSP";
        instance.symmetric = true;
    }

    return instance;
}
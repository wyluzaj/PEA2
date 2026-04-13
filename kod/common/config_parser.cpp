#include "config_parser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "utils.h"

// ============================================================
// Pomocnicze funkcje lokalne
// ============================================================

namespace {

    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;
        return s.substr(start, end - start);
    }

    std::string toUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::toupper(c); });
        return s;
    }

// Usuwa komentarze #
    std::string stripComment(const std::string& line) {
        const size_t pos = line.find('#');
        if (pos == std::string::npos) return line;
        return line.substr(0, pos);
    }

    SearchType parseSearchType(const std::string& raw, const std::string& configPath) {
        const std::string val = toUpper(trim(raw));
        if (val == "DFS")    return SearchType::DFS;
        if (val == "BFS")    return SearchType::BFS;
        if (val == "BESTFS") return SearchType::BESTFS;
        throw std::runtime_error(
                configPath + ": nieznany search_type = \"" + raw + "\"\n"
                                                                   "  Dozwolone wartosci: DFS, BFS, BestFS"
        );
    }

    AlgorithmType parseAlgorithmType(const std::string& raw, const std::string& configPath) {
        const std::string val = toUpper(trim(raw));
        if (val == "PLAIN_UB_INF") return AlgorithmType::PLAIN_UB_INF;
        if (val == "PLAIN_UB_RNN") return AlgorithmType::PLAIN_UB_RNN;
        if (val == "LB_UB_RNN")    return AlgorithmType::LB_UB_RNN;
        throw std::runtime_error(
                configPath + ": nieznany algorithm_type = \"" + raw + "\"\n"
                                                                      "  Dozwolone wartosci: PLAIN_UB_INF, PLAIN_UB_RNN, LB_UB_RNN"
        );
    }

}

// ============================================================
// Implementacja parseConfigFile
// ============================================================

RunConfig parseConfigFile(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku konfiguracyjnego: " + configPath);
    }

    RunConfig cfg;
    bool gotInstance     = false;
    bool gotSearchType   = false;
    bool gotAlgorithm    = false;

    std::string line;
    int lineNo = 0;

    while (std::getline(file, line)) {
        ++lineNo;
        line = trim(stripComment(line));

        if (line.empty()) continue;

        // Format: klucz = wartosc
        const size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            throw std::runtime_error(
                    configPath + ":" + std::to_string(lineNo) +
                    ": brak znaku '=' w linii: \"" + line + "\""
            );
        }

        const std::string key = toUpper(trim(line.substr(0, eqPos)));
        const std::string val = trim(line.substr(eqPos + 1));

        if (val.empty()) {
            throw std::runtime_error(
                    configPath + ":" + std::to_string(lineNo) +
                    ": pusta wartosc dla klucza \"" + key + "\""
            );
        }

        if (key == "INSTANCE_PATH") {
            cfg.instancePath = normalizePath(val);
            gotInstance = true;
        }

        else if (key == "SEARCH_TYPE") {
            cfg.searchType = parseSearchType(val, configPath);
            gotSearchType = true;
        }
        else if (key == "ALGORITHM_TYPE") {
            cfg.algorithmType = parseAlgorithmType(val, configPath);
            gotAlgorithm = true;
        }
        else {
            // Nieznany klucz - ostrzeĹĽenie, nie bĹ‚Ä…d (Ĺ‚atwa rozszerzalnoĹ›Ä‡)
            std::cerr << "[WARN] " << configPath << ":" << lineNo
                      << ": nieznany klucz \"" << key << "\" - ignorowany.\n";
        }
    }

    // Walidacja kompletnoĹ›ci
    if (!gotInstance) {
        throw std::runtime_error(
                configPath + ": brakuje wymaganego klucza INSTANCE_PATH"
        );
    }
    if (!gotSearchType) {
        throw std::runtime_error(
                configPath + ": brakuje wymaganego klucza SEARCH_TYPE"
        );
    }
    if (!gotAlgorithm) {
        throw std::runtime_error(
                configPath + ": brakuje wymaganego klucza ALGORITHM_TYPE"
        );
    }

    return cfg;
}


void printConfig(const RunConfig& cfg) {
    auto searchName = [](SearchType t) -> std::string {
        switch (t) {
            case SearchType::DFS:    return "DFS";
            case SearchType::BFS:    return "BFS";
            case SearchType::BESTFS: return "BestFS (lowest-cost)";
        }
        return "?";
    };

    auto algoName = [](AlgorithmType t) -> std::string {
        switch (t) {
            case AlgorithmType::PLAIN_UB_INF: return "PLAIN_UB_INF  (brak LB, UB = INT_MAX)";
            case AlgorithmType::PLAIN_UB_RNN: return "PLAIN_UB_RNN  (brak LB, UB z RNN)";
            case AlgorithmType::LB_UB_RNN:    return "LB_UB_RNN     (z LB, UB startowe z RNN)";
        }
        return "?";
    };

    std::cout << "=== Konfiguracja ===\n"
              << "  Instancja:      " << cfg.instancePath   << "\n"
              << "  Przeszukiwanie: " << searchName(cfg.searchType) << "\n"
              << "  Algorytm B&B:   " << algoName(cfg.algorithmType) << "\n"
              << "====================\n";
}

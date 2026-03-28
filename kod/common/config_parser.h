#ifndef PEA2_CONFIG_PARSER_H
#define PEA2_CONFIG_PARSER_H
#pragma once

#include <string>

// ============================================================
// Typy wyliczeniowe opisujÄ…ce wybory z pliku konfiguracyjnego
// ============================================================

enum class SearchType {
    DFS,
    BFS,
    BESTFS
};

enum class AlgorithmType {
    PLAIN_UB_INF,   // bez LB, UB = INT_MAX
    PLAIN_UB_RNN,   // bez LB, UB wyznaczane z RNN
    LB_UB_RNN       // z LB (wyliczanym), UB startowe z RNN
};

// ============================================================
// Struktura przechowujÄ…ca odczytanÄ… konfiguracjÄ™
// ============================================================

struct RunConfig {
    std::string instancePath;   // sciezka do pliku instancji
    SearchType  searchType  = SearchType::DFS;
    AlgorithmType algorithmType = AlgorithmType::LB_UB_RNN;
};

// ============================================================
// Funkcja parsujÄ…ca plik konfiguracyjny
// Rzuca std::runtime_error przy bĹ‚Ä™dach
// ============================================================

RunConfig parseConfigFile(const std::string& configPath);

// Pomocnicze: wypisz konfiguracjÄ™ na cout
void printConfig(const RunConfig& cfg);

#endif // PEA2_CONFIG_PARSER_H

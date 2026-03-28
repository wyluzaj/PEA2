//  ============================================================
//  PEA2 â€“ Branch & Bound solver dla TSP/ATSP
//  Punkt wejĹ›cia: jeden executable sterowany plikiem .cfg
//
//  UĹĽycie:
//    PEA2 <plik_konfiguracyjny.cfg>
//    PEA2                    (program zapyta o Ĺ›cieĹĽkÄ™ do .cfg)
//  ============================================================

#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "../common/config_parser.h"
#include "../common/instance_loader.h"
#include "../common/tsp_instance.h"
#include "../common/utils.h"
#include "../common/writer.h"

#include "../bnb/bnb_solver.h"
#include "../bnb/bnb_plain_solver.h"

#include "../strategies/search_strategy.h"
#include "../strategies/dfs_frontier.h"
#include "../strategies/bfs_frontier.h"
#include "../strategies/bestfs_frontier.h"

// ============================================================
// Pomocnicze
// ============================================================

namespace {

// Tworzy frontier odpowiedni do wybranej strategii
    std::unique_ptr<ISearchFrontier> makeFrontier(SearchType st) {
        switch (st) {
            case SearchType::DFS:    return std::make_unique<DFSFrontier>();
            case SearchType::BFS:    return std::make_unique<BFSFrontier>();
            case SearchType::BESTFS: return std::make_unique<BestFSFrontier>();
        }
        throw std::runtime_error("Nieznany SearchType.");
    }

// Buduje nazwÄ™ algorytmu widocznÄ… w wynikach
    std::string buildAlgorithmName(const RunConfig& cfg) {
        std::string name = "BNB";

        switch (cfg.algorithmType) {
            case AlgorithmType::PLAIN_UB_INF: name += "_PLAIN"; break;
            case AlgorithmType::PLAIN_UB_RNN: name += "_PLAIN"; break;
            case AlgorithmType::LB_UB_RNN:    break; // brak sufiksu
        }

        switch (cfg.searchType) {
            case SearchType::DFS:    name += "_DFS";    break;
            case SearchType::BFS:    name += "_BFS";    break;
            case SearchType::BESTFS: name += "_BESTFS"; break;
        }

        switch (cfg.algorithmType) {
            case AlgorithmType::PLAIN_UB_INF: name += "_UB_INF"; break;
            case AlgorithmType::PLAIN_UB_RNN: name += "_UB_RNN"; break;
            case AlgorithmType::LB_UB_RNN:    name += "_UB_RNN"; break;
        }

        return name;
    }

// Uruchamia wĹ‚aĹ›ciwy solver i zwraca wynik
    TSPResult runSolver(
            const TSPInstance& instance,
            const RunConfig& cfg,
            ISearchFrontier& frontier,
            const std::string& algoName
    ) {
        switch (cfg.algorithmType) {
            case AlgorithmType::PLAIN_UB_INF:
                return solveBranchAndBoundPlain(
                        instance, frontier, algoName,
                        InitialUBMode::INF,
                        Config::DEFAULT_TIME_LIMIT_SECONDS
                );

            case AlgorithmType::PLAIN_UB_RNN:
                return solveBranchAndBoundPlain(
                        instance, frontier, algoName,
                        InitialUBMode::RNN,
                        Config::DEFAULT_TIME_LIMIT_SECONDS
                );

            case AlgorithmType::LB_UB_RNN:
                return solveBranchAndBound(
                        instance, frontier, algoName,
                        Config::DEFAULT_TIME_LIMIT_SECONDS
                );
        }
        throw std::runtime_error("Nieznany AlgorithmType.");
    }

// Pyta uĹĽytkownika o Ĺ›cieĹĽkÄ™ do pliku konfiguracyjnego
    std::string askForConfigPath() {
        std::string path;
        std::cout << "Podaj sciezke do pliku konfiguracyjnego (.cfg):\n> ";
        std::getline(std::cin, path);
        return normalizePath(path);
    }

} // namespace

// ============================================================
// main
// ============================================================

int main(int argc, char* argv[]) {
    try {
        // --- 1. Ustal Ĺ›cieĹĽkÄ™ do pliku konfiguracyjnego ---
        std::string configPath;
        if (argc >= 2) {
            configPath = normalizePath(argv[1]);
        } else {
            configPath = askForConfigPath();
        }

        if (configPath.empty()) {
            throw std::runtime_error("Nie podano sciezki do pliku konfiguracyjnego.");
        }

        // --- 2. Wczytaj konfiguracjÄ™ ---
        const RunConfig cfg = parseConfigFile(configPath);
        printConfig(cfg);

        // --- 3. Wczytaj instancjÄ™ TSP/ATSP ---
        TSPInstance instance = loadInstance(cfg.instancePath);

        if (instance.name.empty()) {
            instance.name = getFileNameWithoutExtension(cfg.instancePath);
        }

        // --- 4. Przygotuj frontier i uruchom solver ---
        auto frontier = makeFrontier(cfg.searchType);
        const std::string algoName = buildAlgorithmName(cfg);

        std::cout << "\nUruchamianie " << algoName << " ...\n";

        TSPResult result = runSolver(instance, cfg, *frontier, algoName);

        // --- 5. Zapisz i wyĹ›wietl wyniki ---
        writeBnBSummaryToFile(result);
        appendBnBResultToCsv(result);
        printBnBResultToConsole(result);

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "\n[BLAD] " << ex.what() << "\n";
        return 2;
    }
    catch (...) {
        std::cerr << "\n[BLAD] Nieznany wyjatek.\n";
        return 3;
    }
}

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>

#include "../bnb/bnb_plain_solver.h"
#include "../common/instance_loader.h"
#include "../common/utils.h"
#include "../common/writer.h"
#include "../strategies/bestfs_frontier.h"
#include "../strategies/bfs_frontier.h"
#include "../strategies/dfs_frontier.h"

namespace {
    std::string toLower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });
        return text;
    }

    void printUsage(const char* programName) {
        std::cerr
                << "Uzycie:\n"
                << "  " << programName
                << " <plik_instancji> <dfs|bfs|bestfs> <inf|rnn>\n\n"
                << "Przyklady:\n"
                << "  " << programName << " dane.tsp dfs inf\n"
                << "  " << programName << " dane.tsp bfs rnn\n"
                << "  " << programName << " dane.tsp bestfs inf\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            printUsage(argv[0]);
            return 1;
        }

        const std::string inputPath = normalizePath(argv[1]);
        const std::string strategyArg = toLower(argv[2]);
        const std::string ubArg = toLower(argv[3]);

        std::unique_ptr<ISearchFrontier> frontier;
        std::string algorithmName;

        if (strategyArg == "dfs") {
            frontier = std::make_unique<DFSFrontier>();
            algorithmName = "BNB_PLAIN_DFS";
        } else if (strategyArg == "bfs") {
            frontier = std::make_unique<BFSFrontier>();
            algorithmName = "BNB_PLAIN_BFS";
        } else if (strategyArg == "bestfs" || strategyArg == "lc" || strategyArg == "lowestcost") {
            frontier = std::make_unique<BestFSFrontier>();
            algorithmName = "BNB_PLAIN_BESTFS";
        } else {
            std::cerr << "Nieznana strategia: " << strategyArg << "\n";
            printUsage(argv[0]);
            return 1;
        }

        InitialUBMode ubMode;

        if (ubArg == "inf") {
            ubMode = InitialUBMode::INF;
            algorithmName += "_UB_INF";
        } else if (ubArg == "rnn") {
            ubMode = InitialUBMode::RNN;
            algorithmName += "_UB_RNN";
        } else {
            std::cerr << "Nieznany tryb UB: " << ubArg << "\n";
            printUsage(argv[0]);
            return 1;
        }

        TSPInstance instance = loadInstance(inputPath);

        if (instance.name.empty()) {
            instance.name = getFileNameWithoutExtension(inputPath);
        }

        TSPResult result = solveBranchAndBoundPlain(
                instance,
                *frontier,
                algorithmName,
                ubMode,
                Config::DEFAULT_TIME_LIMIT_SECONDS
        );

        writeBnBSummaryToFile(result);
        appendBnBResultToCsv(result);
        printBnBResultToConsole(result);

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Blad: " << ex.what() << "\n";
        return 2;
    }
    catch (...) {
        std::cerr << "Blad: nieznany wyjatek.\n";
        return 3;
    }
}
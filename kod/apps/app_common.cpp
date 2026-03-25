#include "app_common.h"

#include <exception>
#include <iostream>
#include <string>

#include "../bnb/bnb_solver.h"
#include "../common/instance_loader.h"
#include "../common/utils.h"
#include "../common/writer.h"

namespace {
    std::string askUserForPath() {
        std::string inputPath;
        std::cout << "Podaj sciezke do pliku instancji:\n";
        std::getline(std::cin, inputPath);

        inputPath = normalizePath(inputPath);

        if (inputPath.empty()) {
            throw std::runtime_error("Nie podano sciezki do pliku.");
        }

        return inputPath;
    }
}

int runApplication(
        int argc,
        char* argv[],
        ISearchFrontier& frontier,
        const std::string& algorithmName
) {
    try {
        std::string inputPath;

        if (argc >= 2) {
            inputPath = normalizePath(argv[1]);
        } else {
            inputPath = askUserForPath();
        }

        TSPInstance instance = loadInstance(inputPath);

        if (instance.name.empty()) {
            instance.name = getFileNameWithoutExtension(inputPath);
        }

        TSPResult result = solveBranchAndBound(
                instance,
                frontier,
                algorithmName,
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
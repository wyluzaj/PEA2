#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "../common/config_parser.h"
#include "../common/instance_loader.h"
#include "../common/tsp_instance.h"
#include "../common/utils.h"
#include "../common/writer.h"
#include "../common/config.h"
#include "../bnb/bnb_solver.h"
#include "../bnb/bnb_plain_solver.h"
#include "../strategies/search_strategy.h"
#include "../strategies/dfs_frontier.h"
#include "../strategies/bfs_frontier.h"
#include "../strategies/bestfs_frontier.h"

namespace {

    void waitForEnter(){
        std::cout<<"\nNacisnij Enter, aby zamknac...\n";
        std::cin.clear();
        if(std::cin.peek()=='\n') std::cin.ignore();
        std::cin.get();
    }

    std::unique_ptr<ISearchFrontier> makeFrontier(SearchType st){
        switch(st){
            case SearchType::DFS:    return std::make_unique<DFSFrontier>();
            case SearchType::BFS:    return std::make_unique<BFSFrontier>();
            case SearchType::BESTFS: return std::make_unique<BestFSFrontier>();
        }
        throw std::runtime_error("Nieznany SearchType.");
    }

    std::string buildAlgorithmName(const RunConfig& cfg){
        std::string n="BNB";
        if(cfg.algorithmType==AlgorithmType::PLAIN_UB_INF||cfg.algorithmType==AlgorithmType::PLAIN_UB_RNN) n+="_PLAIN";
        switch(cfg.searchType){
            case SearchType::DFS:    n+="_DFS";    break;
            case SearchType::BFS:    n+="_BFS";    break;
            case SearchType::BESTFS: n+="_BESTFS"; break;
        }
        switch(cfg.algorithmType){
            case AlgorithmType::PLAIN_UB_INF: n+="_UB_INF"; break;
            case AlgorithmType::PLAIN_UB_RNN: n+="_UB_RNN"; break;
            case AlgorithmType::LB_UB_RNN:    n+="_UB_RNN"; break;
        }
        return n;
    }

    TSPResult runSolver(const TSPInstance& inst,const RunConfig& cfg,ISearchFrontier& frontier,const std::string& name){
        switch(cfg.algorithmType){
            case AlgorithmType::PLAIN_UB_INF:
                return solveBranchAndBoundPlain(inst,frontier,name,InitialUBMode::INF,Config::DEFAULT_TIME_LIMIT_SECONDS);
            case AlgorithmType::PLAIN_UB_RNN:
                return solveBranchAndBoundPlain(inst,frontier,name,InitialUBMode::RNN,Config::DEFAULT_TIME_LIMIT_SECONDS);
            case AlgorithmType::LB_UB_RNN:
                return solveBranchAndBound(inst,frontier,name,Config::DEFAULT_TIME_LIMIT_SECONDS);
        }
        throw std::runtime_error("Nieznany AlgorithmType.");
    }

    std::string askForConfigPath(){
        std::string p;
        std::cout<<"Podaj sciezke do pliku konfiguracyjnego (.cfg):\n> ";
        std::getline(std::cin,p);
        return normalizePath(p);
    }

} // namespace

int main(int argc,char* argv[]){
    try {
        std::string configPath;
        if(argc>=2) configPath=normalizePath(argv[1]);
        else        configPath=askForConfigPath();
        if(configPath.empty()) throw std::runtime_error("Nie podano sciezki do pliku konfiguracyjnego.");

        const RunConfig cfg=parseConfigFile(configPath);
        printConfig(cfg);

        std::cout<<"Wczytywanie instancji: "<<cfg.instancePath<<"\n";
        TSPInstance instance=loadInstance(cfg.instancePath);
        if(instance.name.empty()) instance.name=getFileNameWithoutExtension(cfg.instancePath);
        std::cout<<"Instancja: "<<instance.name<<" ("<<instance.dimension<<" wierzcholkow, "<<instance.type<<")\n";

        auto frontier=makeFrontier(cfg.searchType);
        const std::string algoName=buildAlgorithmName(cfg);
        std::cout<<"\nUruchamianie "<<algoName<<" ...\n";

        TSPResult result=runSolver(instance,cfg,*frontier,algoName);

        writeBnBSummaryToFile(result);
        appendBnBResultToCsv(result);
        std::cout<<"\n=== Wynik ===\n";
        printBnBResultToConsole(result);

        waitForEnter();
        return 0;
    }
    catch(const std::exception& ex){std::cerr<<"\n[BLAD] "<<ex.what()<<"\n";waitForEnter();return 2;}
    catch(...){std::cerr<<"\n[BLAD] Nieznany wyjatek.\n";waitForEnter();return 3;}
}

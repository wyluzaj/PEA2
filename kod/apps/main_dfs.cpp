#include "app_common.h"
#include "../strategies/dfs_frontier.h"

int main(int argc, char* argv[]) {
    DFSFrontier frontier;
    return runApplication(argc, argv, frontier, "BNB_DFS");
}
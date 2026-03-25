#include "app_common.h"
#include "../strategies/bfs_frontier.h"

int main(int argc, char* argv[]) {
    BFSFrontier frontier;
    return runApplication(argc, argv, frontier, "BNB_BFS");
}
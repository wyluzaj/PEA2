#include "app_plain_common.h"
#include "../strategies/bfs_frontier.h"

int main(int argc, char* argv[]) {
    BFSFrontier frontier;
    return runPlainApplication(argc, argv, frontier, "BNB_PLAIN_BFS_UB_RNN", InitialUBMode::RNN);
}

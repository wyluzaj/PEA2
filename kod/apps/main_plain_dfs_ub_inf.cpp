#include "app_plain_common.h"
#include "../strategies/dfs_frontier.h"

int main(int argc, char* argv[]) {
    DFSFrontier frontier;
    return runPlainApplication(argc, argv, frontier, "BNB_PLAIN_DFS_UB_INF", InitialUBMode::INF);
}

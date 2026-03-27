#include "app_plain_common.h"
#include "../strategies/bestfs_frontier.h"

int main(int argc, char* argv[]) {
    BestFSFrontier frontier;
    return runPlainApplication(argc, argv, frontier, "BNB_PLAIN_BESTFS_UB_RNN", InitialUBMode::RNN);
}

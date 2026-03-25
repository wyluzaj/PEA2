#include "app_common.h"
#include "../strategies/bestfs_frontier.h"

int main(int argc, char* argv[]) {
    BestFSFrontier frontier;
    return runApplication(argc, argv, frontier, "BNB_BESTFS");
}
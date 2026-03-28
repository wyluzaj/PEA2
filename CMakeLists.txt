cmake_minimum_required(VERSION 3.26)
project(PEA2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

# ============================================================
# Biblioteka rdzeniowa (bez zmian w stosunku do oryginału)
# ============================================================
add_library(pea2_core
        kod/common/reader.cpp
        kod/common/reader.h
        kod/common/reader_matrix.cpp
        kod/common/reader_matrix.h
        kod/common/instance_loader.cpp
        kod/common/instance_loader.h
        kod/common/utils.cpp
        kod/common/utils.h
        kod/common/writer.cpp
        kod/common/writer.h
        kod/common/config.h
        kod/common/tsp_instance.h

        # Nowy parser pliku konfiguracyjnego
        kod/common/config_parser.cpp
        kod/common/config_parser.h

        kod/bnb/bnb_node.h
        kod/bnb/lb.h
        kod/bnb/lb.cpp
        kod/bnb/ub.cpp
        kod/bnb/ub.h
        kod/bnb/bnb_solver.cpp
        kod/bnb/bnb_solver.h
        kod/bnb/bnb_plain_solver.cpp
        kod/bnb/bnb_plain_solver.h

        kod/strategies/search_strategy.h
        kod/strategies/dfs_frontier.h
        kod/strategies/bfs_frontier.h
        kod/strategies/bestfs_frontier.h
)

# ============================================================
# Jeden executable sterowany plikiem konfiguracyjnym
# ============================================================
add_executable(PEA2
        kod/apps/main.cpp
)

target_link_libraries(PEA2 pea2_core)

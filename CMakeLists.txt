cmake_minimum_required(VERSION 3.26)
project(PEA2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

add_library(pea2_core
        kod/common/reader.cpp
        kod/common/reader_matrix.cpp
        kod/common/instance_loader.cpp
        kod/common/utils.cpp
        kod/common/writer.cpp
        kod/common/config_parser.cpp

        kod/bnb/lb.cpp
        kod/bnb/ub.cpp
        kod/bnb/bnb_solver.cpp
        kod/bnb/bnb_plain_solver.cpp
)

add_executable(PEA2
        kod/apps/main.cpp
)

target_link_libraries(PEA2 pea2_core)

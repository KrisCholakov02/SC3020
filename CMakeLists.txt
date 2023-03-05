cmake_minimum_required(VERSION 3.24)
project(SC3020)

set(CMAKE_CXX_STANDARD 17)

add_executable(SC3020 src/main.cpp src/b_plus_tree.cpp src/b_plus_tree_insert.cpp src/b_plus_tree_search.cpp src/b_plus_tree_remove.cpp src/storage.cpp src/tree_node.cpp)
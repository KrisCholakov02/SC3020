cmake_minimum_required(VERSION 3.24)
project(SC3020)

set(CMAKE_CXX_STANDARD 17)

add_executable(SC3020 src/main.cpp src/storage.cpp src/storage.h src/storage_components.h src/b_plus_tree.cpp src/b_plus_tree.h src/tree_node.cpp src/tree_node.h src/b_plus_tree_remove.cpp src/b_plus_tree_removeHelperFunctions.cpp src/b_plus_tree_rangeQuery.cpp)

#include "b_plus_tree.h"

BPlusTree::BPlusTree(Storage *indexes, Storage *records, size_t nodeSize) {
    // Checking which is the number of keys that a node can contain
    // In the start, the node has a bool and int attributes, so we subtract them from the nodeSize
    // For n keys, we have n+1 pointers, so we subtract one Address too, then we try increasing the key number
    // with having that for every key there is a float (the actual key) and the address of the record
    maxNumKeys = 0;
    while (((maxNumKeys + 1) * (sizeof(Address) + sizeof(float)) + sizeof(int) + sizeof(bool) + sizeof(Address)) <
           nodeSize) {
        maxNumKeys += 1;
    }
    if (maxNumKeys == 0) {
        throw logic_error("The node size is not enough to support the B+ Tree requirements.");
    }
    // Setting the root to a null pointer
    root = nullptr;
    rootAddress = nullptr;
    // Setting the size of the node
    this->nodeSize = nodeSize;
    // Setting the counter of the levels and nodes
    this->numLevels = 0;
    this->numNodes = 0;
    // Setting the storages
    this->indexes = indexes;
    this->records = records;
}


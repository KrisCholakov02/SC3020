#ifndef SC3020_TREE_NODE_H
#define SC3020_TREE_NODE_H


#include "storage_components.h"

// The class for the node of the B+ Tree
// A node in the B+ Tree.
class Node
{
private:
    // Variables
    Address *pointers;      // A pointer to an array of struct {void *blockAddress, short int offset} containing other nodes in disk.
    float *keys;            // Pointer to an array of keys in this node.
    int numKeys;            // Current number of keys in this node.
    bool isLeaf;            // Whether this node is a leaf node.
    friend class BPlusTree; // Let the BPlusTree class access this class' private variables.

public:
    // Methods

    // Constructor
    Node(int maxKeys); // Takes in max keys in a node.
};

#endif //SC3020_TREE_NODE_H

#ifndef SC3020_TREE_NODE_H
#define SC3020_TREE_NODE_H


#include "storage_components.h"

// The class for the node of the B+ Tree
class TreeNode {
private:
    // Whether the node is a leaf node
    bool leafNode;
public:
    // Constructor for the B+ tree node based on the maximum number of keys that a node can have
    TreeNode(int maxNumKeys);

    // Getter of the pointers
    Address *getPointers() const {
        return pointers;
    }

    // Getter if the keys
    float *getKeys() const {
        return keys;
    }

    // Check if the node is a leaf node
    bool isLeafNode() const {
        return leafNode;
    }

    // Getter for the number of keys currently in the node
    int getNumKeys() const {
        return numKeys;
    }

// The number of keys currently in the node
int numKeys;
// The array of the keys in the node
float *keys;
// The array of the pointers in the node
Address *pointers;
};


#endif //SC3020_TREE_NODE_H

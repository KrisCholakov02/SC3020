#ifndef SC3020_B_PLUS_TREE_H
#define SC3020_B_PLUS_TREE_H


#include "storage.h"
#include "tree_node.h"

// A class for the B+ Tree structure
class BPlusTree {
private:
    // The storage space for the indexes of the records
    Storage *indexes;
    // The storage space for the records of the database
    Storage *records;
    // A pointer to the root node of the tree
    TreeNode *root;
    // The address of the root node
    void *rootAddress;
    // Tha maximum number of keys in a node of the tree
    int maxNumKeys;
    // The number of levels the current tree has
    int numLevels;
    // The number of nodes the current tree has
    int numNodes;
    // The size of a node in the tree
    size_t nodeSize;
public:
    // The constructor for the BPlusTree class
    BPlusTree(Storage *indexes, Storage *records, size_t nodeSize);

    // A function to insert a record into the B+ Tree
    void insert(Address recordAddress, float key);

    // A function to remove a record from the B+ Tree
    void remove(float key);

    // A function to search for a record in the B+ Tree
    void searchRange(float start, float end);

    // Getter for the root of the tree
    TreeNode *getRoot() const {
        return root;
    }

    // Getter for the number of levels in the tree
    int getNumLevels() const {
        return numLevels;
    }

    // Getter for the number of nodes in the tree
    int getNumNodes() const {
        return numNodes;
    }

    // Getter fo the maximum number of keys in the tree's nodes
    int getMaxNumKeys() const {
        return maxNumKeys;
    }
};


#endif //SC3020_B_PLUS_TREE_H
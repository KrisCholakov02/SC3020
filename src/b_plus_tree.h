#ifndef SC3020_B_PLUS_TREE_H
#define SC3020_B_PLUS_TREE_H

#include "storage.h"
#include "tree_node.h"

using namespace std;

// A class for the B+ Tree structure
class BPlusTree {
private:
    // Attributes
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

    //Methods
    // A method to update or create a parent node of both nodes
    void insertInternal(int key, TreeNode *parentAddress, TreeNode *childAddress);

    // Balanced B+ tree structure.
    void updateInternal(int key, TreeNode *cursorDiskAddress, TreeNode *childDiskAddress);

    // Insert a record into a linked list of the records with the same key
    Address insertLL(Address LLHead, Address recordAddress, int key);

    // Remove the whole linked list
    void removeLL(Address headAddress);

    // Finds the direct parent of a node in the B+ Tree.
    TreeNode *findParent(TreeNode *currentAddress, TreeNode *childAddress, int key);
public:
    // The constructor for the BPlusTree class
    BPlusTree(Storage *indexes, Storage *records, size_t nodeSize);

    // A function to insert a record into the B+ Tree
    void insert(Address recordAddress, int key);

    // A function to remove a record from the B+ Tree
    int remove(int key);

    // A function to search for a record in the B+ Tree
    void rangeSearch(float start, float end);

    // Getter for the root of the tree
    TreeNode *getRoot() const {
        return root;
    }

    // Getter for the number of levels in the tree
    int getNumLevels();

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

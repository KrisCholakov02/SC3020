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

TreeNode *BPlusTree::findParent(TreeNode *currentAddress, TreeNode *childAddress, float key)
{
    // Load in the current node
    Address currentAddressObject = Address(currentAddress, 0);
    TreeNode *current = (TreeNode *)indexes->loadRecordFromStorage(currentAddressObject, nodeSize);

    // If the current node is a leaf one, no child
    if (current->leafNode) return nullptr;
    // Otherwise, get parent address
    TreeNode *parentAddress = currentAddress;

    // While current is not leaf find continue the traversal
    while (current->leafNode == false)
    {
        // Check all pointers for a match
        for (int i = 0; i < current->numKeys + 1; i++)
        {
            if (current->pointers[i].blockAddress == childAddress) return parentAddress;
        }

        // Find the position within the node
        for (int i = 0; i < current->numKeys; i++)
        {
            // If key is less than the current key, go to the left
            if (key < current->keys[i])
            {
                // Load the next node
                TreeNode *nextNode = (TreeNode *)indexes->loadRecordFromStorage(current->pointers[i], nodeSize);
                // Update parent address
                parentAddress = (TreeNode *)current->pointers[i].blockAddress;
                // Make the next node a current one
                current = (TreeNode *)nextNode;
                break;
            }
            // If key larger than all keys in the node, go to node pointed by the last pointer
            if (i == current->numKeys - 1)
            {
                // Load the next node
                TreeNode *nextNode = (TreeNode *) indexes->loadRecordFromStorage(current->pointers[i + 1], nodeSize);
                //  Update parent address
                parentAddress = (TreeNode *)current->pointers[i + 1].blockAddress;
                // Make the next node a current one
                current = (TreeNode *) nextNode;
                break;
            }
        }
    }
    return nullptr;
}

int BPlusTree::getNumLevels() {
    // If there is not a root, empty tree
    if (rootAddress == nullptr) {
        return 0;
    }

    // Load in the root node the indexes storage
    Address rootAddressObject = Address(rootAddress, 0);
    root = (TreeNode *)indexes->loadRecordFromStorage(rootAddressObject, nodeSize);
    // A current node variable to travers through the levels
    TreeNode *current = root;

    int levels = 1;

    // Until the current node reaches the leaf level
    while (!current->leafNode) {
        current = (TreeNode *)indexes->loadRecordFromStorage(current->pointers[0], nodeSize);
        levels++;
    }
    levels++;

    return levels;
}

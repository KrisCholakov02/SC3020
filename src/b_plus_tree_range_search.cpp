#include "b_plus_tree.h"
#include "tree_node.h"
#include "storage_components.h"

#include <vector>
#include <iostream>

using namespace std;


// Function to handle range queries from start key to end key
void BPlusTree::rangeSearch(float start, float end) {
    // Check if the root exists i.e. it is not a null pointer
    if (rootAddress != nullptr) {
        //Load root node from disk storage to main memory
        Address addr{rootAddress, 0};
        root = (TreeNode *) indexes->loadRecordFromStorage(addr, nodeSize);
        //Node has been loaded from storage
        cout << "Node Accessed";


        // Print the keys and pointers of root node
        // Iterate from first key/pointer to the nu,ber of keys/pointers i.e. accessing all of them
        for (int i = 0; i < root->getNumKeys(); i++) {
            cout << root->getPointers()[i].blockAddress << "|";
            cout << root->getKeys()[i] << "|";
        }

        // Traverse down the tree to the leaf node containing the starting point

        // Set flag variable
        // FoundNode variable to be false, which can be changed later if the node is indeed found
        bool foundNode = false;
        // Set a temporary treeNode to point to root node
        TreeNode *temp = root;

        // Traverse while leaf node is not reached
        while (!temp->isLeafNode()) {
            // Traverse through all keys/ pointers in root(temp here) node to find leaf node containing start key .
            for (int j = 0; j < temp->getNumKeys(); j++) {
                // Find the smallest key greater than the start key
                // Check if the "start" value is less than the current key value in the node "temp" that it's examining.
                if (start < temp->getKeys()[j]) {
                    // Loads the next node from the pointer at index "j" and updates the "temp" pointer to the new node.
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[j], nodeSize);
                    cout << "Node Accessed";
                    // Print the keys and block address of keys of root node
                    // Iterate from first key/pointer to the nu,ber of keys/pointers i.e. accessing all of them
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].blockAddress << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                    break;
                }

                // j reached the end of number of keys in node
                // When none of the keys in the current node are greater than the search start value
                // Loads the last child node pointer of the current node
                if (j == temp->getNumKeys() - 1) {

                    // Expected to contain keys greater than the search start value,
                    // Sets the temp variable to point to this child node.
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[j + 1], nodeSize);
                    // Prints a message indicating that the node was accessed
                    // Outputs the block addresses and keys of the node's children.
                    cout << "Node Accessed";
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].blockAddress << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                    break;
                }
            }
        }

        //leaf node
        // It reaches the leaf node that contains the first key in the range

        // Flag variable to indicate when loop should start
        // Initialize a boolean variable to check if the search is complete
        bool finish = false;
        // Iterate while finish is equal to false
        // Traverse through each leaf node in the tree
        while (finish == false) {
            // Loop through each key in the current leaf node
            for (int k = 0; k < temp->getNumKeys(); k++) {
                // If the key value is within the search range, print the corresponding record
                if (temp->getKeys()[k] >= start && temp->getKeys()[k] <= end) {
                    // Print message for node access
                    cout << "Node Accessed.";
                    // Loop through each key in the current leaf node
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        // Print the block address and key for each node entry
                        cout << temp->getPointers()[i].blockAddress << "|";
                        cout << temp->getKeys()[i] << "|";

                    }

                    cout << endl;
                    //code to display the record
                }
                // Check if the current key value in the leaf node temp is greater than the ending range value end
                // If it is, it sets the finish boolean flag to true, indicating that the range search is complete
                // Break out of the loop

                // all subsequent key values in the node will also be greater than end
                // There's no need to continue searching.
                if (temp->getKeys()[k] > end) {
                    finish = true;
                    break;
                }

                // Check if the current node temp has a right sibling
                // And the last key of temp is not equal to end
                if (temp->getPointers()[temp->getNumKeys()].blockAddress != nullptr &&
                    temp->getKeys()[k] != end) {
                    // Load the right sibling of temp into temp,
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[k], nodeSize);
                    cout << "Node Accessed";
                    // Prints the addresses and keys of the nodes that were accessed in the process.
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].blockAddress << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                }
                // Else we believe we have reached end of leaf node as either of the above conditions were false
                // Sets finish to true to indicate that the range search is finished for the current leaf node.
                else {
                    finish = true;
                }

            }
        }

    }
    //Else root address is a null pointer indicating that the B+ tree is empty, no node
    else {
        throw logic_error("empty B+ Tree");
    }
    return;
}
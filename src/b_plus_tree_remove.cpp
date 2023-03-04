//
// Created by anush on 03-03-2023.
//


#include "b_plus_tree.h"
#include "tree_node.h"

#include <iostream>

using namespace std;

//Removing record
int BPlusTree::remove(float key) {
    // set number of nodes the current tree contains before removing a key
    numNodes = indexes->getAllocated();

    // if root address points to null, tree is empty.
    if (rootAddress == nullptr) {
        throw std::logic_error("B+ tree is empty!");
    } else {
        // if the tree is not empty,find in address of root node from the disk with 0 offset.
        Address rootStorageAddress{rootAddress, 0};
        root = (TreeNode *) indexes->loadRecordFromStorage(rootStorageAddress, nodeSize);

        TreeNode *cursor = root;

        //cursorStorageAddress keeps track of the disk/storage address of the current node being accessed
        void *cursorStorageAddress = rootAddress;
        //Store the indices of left node and right node of current node,
        //So we can borrow from them if possible and if necessary conditions are maintained
        int leftSibling, rightSibling;

        //parentNode tracks the parent of each node so it can be updated if need be
        TreeNode *parentNode;
        //parentStorageAddress keeps track of parent of each node as disk address so we can update if need be
        void *parentStorageAddress = rootAddress;

        // While cursor is not pointing to a leaf node, keep traversing through the tree to find the desired key.
        // This is to find the leaf node that points to the key in disk
        while (cursor->isLeafNode() == false) {
            // If node is not a leaf node, we need to go to next level.
            // Before going ot next level, store address of current node in parent node
            // Also store disk storage address of current node as parent storage address
            // Next we go deeper in the tree - to the next level
            parentNode = cursor;
            parentStorageAddress = cursorStorageAddress;

            // Traverse through all the keys of the tree node to find the suitable key and pointer value
            // We follow them to go deeper in the tree
            for (int x = 0; x < cursor->numKeys; x++) {
                // Store the left and right sibling indices from which we can borrow a node if allowed
                //to the right x+1 index
                rightSibling = x + 1;
                //to the left x-1 index
                leftSibling = x - 1;

                //First need to find the given key in tree

                // Check if given key is greater than all node keys, reached last node
                // Current node pointing to last node
                // If yes go straightaway to the last pointer in TreeNode i.e. the rightmodet pointer
                if (x == cursor->numKeys - 1) {
                    // Load the node from disk storage to main memory.
                    TreeNode *mainMemoryNode = (TreeNode *) indexes->loadRecordFromStorage(cursor->pointers[x + 1],
                                                                                           nodeSize);

                    // Update cursorStorageAddress to keep track of pointer in the node for future updates
                    cursorStorageAddress = cursor->pointers[x + 1].blockAddress;

                    // Update current node to point to a new node in the main memory
                    cursor = (TreeNode *) mainMemoryNode;

                    //Update the left and right sibling indexes
                    leftSibling = x;
                    rightSibling = x + 2;

                    break;
                }

                // Check if key is smaller than the current key,
                // Follow the node pointed by left pointer.
                if (key < cursor->keys[x]) {
                    // Load the node pointed by left ptr in from disk storage to main memory.
                    TreeNode *mainMemoryNode = (TreeNode *) indexes->loadRecordFromStorage(cursor->pointers[x],
                                                                                           nodeSize);

                    // Update cursorStorageAddress to keep track of pointer in the node for future updates
                    cursorStorageAddress = cursor->pointers[x].blockAddress;

                    // Change current node to point to the new TreeNode in main memory.
                    cursor = (TreeNode *) mainMemoryNode;
                    break;
                }

            }
        }

        // After finding the leaf node which points to the desired key
        // Find the position of this key, is it exists
        // Find if the key of the record to be removed, ecen exists in our B+ tree

        //assuming we dont find key, initially set key_found as false
        //position points to the location/ index where key was found
        int position;
        bool key_found = false;

        // Making sure code also works for duplicate key values
        //iterate through all the keys
        for (position = 0; position < cursor->numKeys; position++) {
            //Check is key of current node points to our desired value
            if (key == cursor->keys[position]) {
                //set key found to be true and exit
                key_found = true;
                break;
            }
        }

        // Check is key_found is still false
        // This indicates required key was not found in the b+ tree, thereby we return the logical error message
        if (key_found == false) {
            std::cout << "Failed to find the required key " << key << " to remove as it does not exist in tree!"
                      << endl;

            // Once the node is deleted, store the number of nodes deleted in the process
            // Update the current total number of nodes
            int numNodesDeleted = numNodes - indexes->getAllocated();
            numNodes = indexes->getAllocated();
            return numNodesDeleted;
        }

        // Before proceeding to delete the key, delete entire linked list stored under the key.
        // This is done so we don't lose head of linked list
        removeLinkedList(cursor->pointers[position]);

        // Proceed to delete the key.
        // Shift all remaining keys and pointers forward respectively to replace values of deleted record.
        for (int x = position; x < cursor->numKeys; x++) {
            //Update the keys and pointer positions to next
            cursor->pointers[x] = cursor->pointers[x + 1];
            cursor->keys[x] = cursor->keys[x + 1];
        }
        //update current node pointer
        cursor->numKeys--;

        // If there is a last pointer left, move it forward.
        cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys + 1];

        // Set all remaining pointers from the filled number of key pointers to nullptr
        for (int x = cursor->numKeys + 1; x < maxNumKeys + 1; x++) {
            Address nullAddress{nullptr, 0};
            cursor->pointers[x] = nullAddress;
        }

        // If current node points to rootNode
        if (root == cursor) {
            //Check if this root node/ current node has remaining keys
            if (cursor->numKeys == 0) {
                // Delete the entire rootNode and deallocate it.
                std::cout << "Congratulations! Successfully deleted the entire indices!" << endl;

                // Proceed to deallocate the block in disk storage used to store root node.
                Address rootStorageAddress{rootAddress, 0};
                indexes->deallocateRecord(rootStorageAddress, nodeSize);

                // Reset root pointers in the B+ Tree.
                //Update root address pointer and pointer to root node in B+ tree to null
                rootAddress = nullptr;
                root = nullptr;


            }
            //Inform user of deleting key successfully
            std::cout << "Deletion successful" << key << endl;

            // Once the node is deleted, store the number of nodes deleted in the process
            // Update the current total number of nodes
            int numNodesDeleted = numNodes - indexes->getAllocated();
            numNodes = indexes->getAllocated();

            // Save the changes and these updated record addressed to storage
            Address cursorAddress = {cursorStorageAddress, 0};
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

            return numNodesDeleted;
        }

        //Case 2:
        // If above deletion was unsuccessful
        // Check if requirements for deletion are met
        // Check if we have minimum number of keys required for stable B+ tree structure
        // Check if we have minimum keys ⌊(n+1)/2⌋ for leaf node.

        //If number of keys greater than minimum requirement, no underflow so we're done
        if (cursor->numKeys >= (maxNumKeys + 1) / 2) {
            std::cout << "Deletion Successful " << key << endl;

            // Once the node is deleted, store the number of nodes deleted in the process
            // Update the current total number of nodes
            int numNodesDeleted = numNodes - indexes->getAllocated();
            numNodes = indexes->getAllocated();

            // Save the changes and these updated record addressed to storage
            Address cursorAddress = {cursorStorageAddress, 0};
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

            return numNodesDeleted;
        }


        //However, if we reach to this place in code, it indicates underflow has occurred
        // This means we do not have sufficient keys to maintain structure of balanced b+ tree

        //Case 3:
        // Attempt first to borrow from left sibling of current node(on same level).
        // Check if left sibling even exists by checking if its index is in valid range i.e. greater than 0.
        if (leftSibling >= 0) {
            // Load in left sibling from disk.
            TreeNode *leftSiblingNode = (TreeNode *) indexes->loadRecordFromStorage(parentNode->pointers[leftSibling],
                                                                                    nodeSize);

            // Check if we can borrow a key from the left neighbour node without causing underflow.
            // Check if after borrowing, number of keys in left sibling node remains
            // greater than equal to '>=' minimum number of keys required for stable structure
            if (leftSiblingNode->numKeys >= (maxNumKeys + 1) / 2 + 1) {
                // Since the borrowed key will be smaller than keys in current node
                // Insert it as the leftmost key in current node and shift remaining keys

                // Firstly, shift last pointer back by one place.
                cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

                // Next we shift all remaining keys and pointers back by one.
                for (int x = cursor->numKeys; x > 0; x--) {
                    //Update values of pointers and keys accordingly
                    cursor->pointers[x] = cursor->pointers[x - 1];
                    cursor->keys[x] = cursor->keys[x - 1];
                }

                // Complete the transfer process
                // Move the key, pointer from rightmost of left sibling node to leftmost of current node
                cursor->pointers[0] = leftSiblingNode->pointers[leftSiblingNode->numKeys - 1];
                cursor->keys[0] = leftSiblingNode->keys[leftSiblingNode->numKeys - 1];
                //Update number of keys - add 1 in current node and subtract 1 from left sibling node
                cursor->numKeys++;
                leftSiblingNode->numKeys--;

                // Shift pointers to the left to update left sibling node
                leftSiblingNode->pointers[cursor->numKeys] = leftSiblingNode->pointers[cursor->numKeys + 1];

                // Update keys of parent node of left sibling accordingly
                parentNode->keys[leftSibling] = cursor->keys[0];
                // Save this parent node to disk storage
                Address parentAddress{parentStorageAddress, 0};
                indexes->saveRecordToStorage(parentNode, nodeSize, parentAddress);

                // Save this left sibling node to disk storage
                indexes->saveRecordToStorage(leftSiblingNode, nodeSize, parentNode->pointers[leftSibling]);

                // Save the current node to disk storage
                Address cursorAddress = {cursorStorageAddress, 0};
                indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

                // Once the node is deleted, store the number of nodes deleted in the process
                // Update the current total number of nodes
                int numNodesDeleted = numNodes - indexes->getAllocated();
                numNodes = indexes->getAllocated();
                return numNodesDeleted;
            }
        }

        // Case 4:
        // If unsuccessful in borrowing from the left sibling, attempt to borrow from right from the right.
        // Check if we even have a right sibling.
        if (rightSibling <= parentNode->numKeys) {
            // Load in right sibling from disk.
            TreeNode *rightNode = (TreeNode *) indexes->loadRecordFromStorage(parentNode->pointers[rightSibling],
                                                                              nodeSize);

            // Check if we can borrow a key from the left neighbour node without causing underflow.
            // Check if after borrowing, number of keys in left sibling node remains
            // greater than equal to '>=' minimum number of keys required for stable structure
            if (rightNode->numKeys >= (maxNumKeys + 1) / 2 + 1) {

                // Since the borrowed key will be larger than keys in current node
                // Insert it as the rightmost key in current node
                // It is also the leftmost key of right sibling node
                // Shift remaining keys in right sibling node one place to the left
                cursor->pointers[cursor->numKeys + 1] = cursor->pointers[cursor->numKeys];

                // Complete the transfer process
                // Move the key, pointer from leftmost of right sibling node to rightmost of current node
                cursor->pointers[cursor->numKeys] = rightNode->pointers[0];
                cursor->keys[cursor->numKeys] = rightNode->keys[0];
                //Update number of keys - add 1 in current node and subtract 1 from right sibling node
                cursor->numKeys++;
                rightNode->numKeys--;

                // Balance the right sibling node by shifting remaining keys and pointers to the left
                for (int x = 0; x < rightNode->numKeys; x++) {
                    rightNode->keys[x] = rightNode->keys[x + 1];
                    rightNode->pointers[x] = rightNode->pointers[x + 1];
                }

                // Move right sibling's last pointer left by one too.
                rightNode->pointers[cursor->numKeys] = rightNode->pointers[cursor->numKeys + 1];

                // According to rules, change key of parent node's to be new lower bound of right sibling.
                // This is to manage balance of tree
                parentNode->keys[rightSibling - 1] = rightNode->keys[0];

                // Save address of parent node and node itself to disk storage.
                Address parentAddress{parentStorageAddress, 0};
                indexes->saveRecordToStorage(parentNode, nodeSize, parentAddress);
                // Save node right sibling to disk storage.
                indexes->saveRecordToStorage(rightNode, nodeSize, parentNode->pointers[rightSibling]);

                // Save current node to disk storage
                Address cursorAddress = {cursorStorageAddress, 0};
                indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

                // Once the node is deleted, store the number of nodes deleted in the process
                // Update the current total number of nodes
                int numNodesDeleted = numNodes - indexes->getAllocated();
                numNodes = indexes->getAllocated();
                return numNodesDeleted;
            }
        }

        //Case 5:
        // If we reach this place in code, it indicates unavailability to steal from neighbouring nodes
        // This is due to necessary requirements of minimum number of keys in each node for B+ tree balance

        // Solution: Merge Nodes.
        // Note: Merging will always succeed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current).

        // Left
        // Start with left sibling node if available
        // Proceed to merge with it.
        if (leftSibling >= 0) {
            // Load in left sibling from disk storage.
            TreeNode *leftSiblingNode = (TreeNode *) indexes->loadRecordFromStorage(parentNode->pointers[leftSibling],
                                                                                    nodeSize);

            // Proceed with the transfer of all keys/pointers from current node to left sibling node.
            for (int x = leftSiblingNode->numKeys, j = 0; j < cursor->numKeys; x++, j++) {
                leftSiblingNode->keys[x] = cursor->keys[j];
                leftSiblingNode->pointers[x] = cursor->pointers[j];
            }

            // Update the number of keys in left sibling node
            // By adding number of keys in left sibling node + number of keys in current node
            leftSiblingNode->numKeys += cursor->numKeys;

            // Update respective variables such as keys and pointers
            // Make last pointer of left sibling node point to current which is the next leaf node
            leftSiblingNode->pointers[leftSiblingNode->numKeys] = cursor->pointers[cursor->numKeys];

            // Save this updated left node to disk storage
            indexes->saveRecordToStorage(leftSiblingNode, nodeSize, parentNode->pointers[leftSibling]);

            // Now we need to update the parents node until the time B+ tree becomes stable
            updateInternal(parentNode->keys[leftSibling], (TreeNode *) parentStorageAddress,
                           (TreeNode *) cursorStorageAddress);

            // After updating parent nodes, just delete the current node from disk by deallocating.
            Address cursorAddress{cursorStorageAddress, 0};
            indexes->deallocateRecord(cursorAddress, nodeSize);
        }

            // Right
            // However, If left sibling doesn't exist,
            // Merge with right sibling if it exists.
        else if (rightSibling <= parentNode->numKeys) {
            // Load in right sibling from disk storage.
            TreeNode *rightSiblingNode = (TreeNode *) indexes->loadRecordFromStorage(parentNode->pointers[rightSibling],
                                                                                     nodeSize);

            // Proceed with the transfer of all keys/pointers from right sibling node to current node.
            for (int x = cursor->numKeys, j = 0; j < rightSiblingNode->numKeys; x++, j++) {
                //Update all keys and pointers within current node
                cursor->pointers[x] = rightSiblingNode->pointers[j];
                cursor->keys[x] = rightSiblingNode->keys[j];
            }

            // Update respective variables such as keys and pointers
            // Make last pointer of current node point to head pointer to left sibling node which is the next leaf node
            cursor->pointers[cursor->numKeys] = rightSiblingNode->pointers[rightSiblingNode->numKeys];
            cursor->numKeys += rightSiblingNode->numKeys;


            // Save current node being pointed into disk storage.
            Address cursorAddress{cursorStorageAddress, 0};
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

            // Update all parent/internal nodes until structure of tree balanced to fully remove the right node.
            void *rightNodeAddress = parentNode->pointers[rightSibling].blockAddress;
            updateInternal(parentNode->keys[rightSibling - 1], (TreeNode *) parentStorageAddress,
                           (TreeNode *) rightNodeAddress);

            // After updating parent nodes, just delete the right sibling node from disk by deallocating.
            Address rightNodeDiskAddress{rightNodeAddress, 0};
            indexes->deallocateRecord(rightNodeDiskAddress, nodeSize);
        }
    }

    // Once the node is deleted, store the number of nodes deleted in the process
    // Update the current total number of nodes
    int numNodesDeleted = numNodes - indexes->getAllocated();
    numNodes = indexes->getAllocated();
    return numNodesDeleted;
}




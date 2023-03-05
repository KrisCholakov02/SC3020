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
    numNodes = indexes->getBlocksAllocated();

    // if root address points to null, tree is empty.
    if (rootAddress == nullptr) {
        throw std::logic_error("B+ tree is empty!");
    } else {
        // if the tree is not empty,find in address of root node from the disk with 0 offset.
        Address rootStorageAddress = Address(rootAddress, 0);
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
            int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
            numNodes = indexes->getBlocksAllocated();
            return numNodesDeleted;
        }

        // Before proceeding to delete the key, delete entire linked list stored under the key.
        // This is done so we don't lose head of linked list
        removeLL(cursor->pointers[position]);

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
            Address nullAddress = Address(nullptr, 0);
            cursor->pointers[x] = nullAddress;
        }

        // If current node points to rootNode
        if (root == cursor) {
            //Check if this root node/ current node has remaining keys
            if (cursor->numKeys == 0) {
                // Delete the entire rootNode and deallocate it.
                std::cout << "Congratulations! Successfully deleted the entire indices!" << endl;

                // Proceed to deallocate the block in disk storage used to store root node.
                Address rootStorageAddress = Address(rootAddress, 0);
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
            int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
            numNodes = indexes->getBlocksAllocated();

            // Save the changes and these updated record addressed to storage
            Address cursorAddress = Address(cursorStorageAddress, 0);
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
            int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
            numNodes = indexes->getBlocksAllocated();

            // Save the changes and these updated record addressed to storage
            Address cursorAddress = Address(cursorStorageAddress, 0);
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
                Address parentAddress = Address(parentStorageAddress, 0);
                indexes->saveRecordToStorage(parentNode, nodeSize, parentAddress);

                // Save this left sibling node to disk storage
                indexes->saveRecordToStorage(leftSiblingNode, nodeSize, parentNode->pointers[leftSibling]);

                // Save the current node to disk storage
                Address cursorAddress = Address(cursorStorageAddress, 0);
                indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

                // Once the node is deleted, store the number of nodes deleted in the process
                // Update the current total number of nodes
                int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
                numNodes = indexes->getBlocksAllocated();
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
                Address parentAddress = Address(parentStorageAddress, 0);
                indexes->saveRecordToStorage(parentNode, nodeSize, parentAddress);
                // Save node right sibling to disk storage.
                indexes->saveRecordToStorage(rightNode, nodeSize, parentNode->pointers[rightSibling]);

                // Save current node to disk storage
                Address cursorAddress = Address(cursorStorageAddress, 0);
                indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

                // Once the node is deleted, store the number of nodes deleted in the process
                // Update the current total number of nodes
                int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
                numNodes = indexes->getBlocksAllocated();
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
            Address cursorAddress = Address(cursorStorageAddress, 0);
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
            Address cursorAddress = Address(cursorStorageAddress, 0);
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

            // Update all parent/internal nodes until structure of tree balanced to fully remove the right node.
            void *rightNodeAddress = parentNode->pointers[rightSibling].blockAddress;
            updateInternal(parentNode->keys[rightSibling - 1], (TreeNode *) parentStorageAddress,
                           (TreeNode *) rightNodeAddress);

            // After updating parent nodes, just delete the right sibling node from disk by deallocating.
            Address rightNodeDiskAddress = Address(rightNodeAddress, 0);
            indexes->deallocateRecord(rightNodeDiskAddress, nodeSize);
        }
    }

    // Once the node is deleted, store the number of nodes deleted in the process
    // Update the current total number of nodes
    int numNodesDeleted = numNodes - indexes->getBlocksAllocated();
    numNodes = indexes->getBlocksAllocated();
    return numNodesDeleted;
}


//Recursive function to deallocate records/ entire blocks if safe till no more left to deallocate
void BPlusTree::removeLL(Address headAddress) {
    // Load in the head of node directly from disk storage.
    TreeNode *head = (TreeNode *) indexes->loadRecordFromStorage(headAddress, nodeSize);

    // Deleting the current head.
    // Continue to deallocate the entire block through the list till no more nodes left to deallocate

    // Deallocating current record using deallocateRecord function.
    indexes->deallocateRecord(headAddress, nodeSize);

    //Base condition - stop if end of file reached
    if (head->pointers[head->numKeys].blockAddress == nullptr) {
        std::cout << "Reached the end of linked list";
        return;
    }
    // Nodes still left in linked list
    if (head->pointers[head->numKeys].blockAddress != nullptr) {
        removeLL(head->pointers[head->numKeys]);
    }
}

// Adjusting the parent
// If the parent is not full enough recursively apply the deletion algorithm in parent


// This function runs till all parent/internal nodes are updated to
// Regain B+ Tree structure after removing a node in case 5, no sibling node found to borrow
// Inputs storage address of parent, the child address to delete, and removes the child.
void BPlusTree::updateInternal(int key, TreeNode *cursorDiskAddress, TreeNode *childDiskAddress) {
    // Load in the latest copy of parent which is pointed to by cursor and child from disk storage
    Address cursorAddress = Address(cursorDiskAddress, 0);
    TreeNode *cursor = (TreeNode *) indexes->loadRecordFromStorage(cursorAddress, nodeSize);

    // Proceed to retrieve address of child node to be removed
    Address childAddress = Address(childDiskAddress, 0);

    // Start by checking is cursor storage address is same as root address
    if (cursorDiskAddress == rootAddress) {
        root = cursor;
    }

    if (cursor == root) {
        // All edits happen in main memory

        // If there is only one key in root node
        // We have to remove all keys and need to change the root node to its child.
        if (cursor->numKeys == 1) {
            // If the second (larger)pointer points to child, make it the new root.
            if (cursor->pointers[1].blockAddress == childDiskAddress) {
                // Remove the child completely
                indexes->deallocateRecord(childAddress, nodeSize);

                // Update root to now point to the parent's left pointer
                // Subsequently, load the left pointer into main memory and update root node.
                root = (TreeNode *) indexes->loadRecordFromStorage(cursor->pointers[0], nodeSize);
                rootAddress = (TreeNode *) cursor->pointers[0].blockAddress;

                // We then proceed to deallocate the old parent root
                indexes->deallocateRecord(cursorAddress, nodeSize);

                std::cout << "RootNode updated." << endl;
                return;
            }

                // Else if left pointer (smaller) in root points to the child, delete from there.
            else if (cursor->pointers[0].blockAddress == childDiskAddress) {
                // Delete the child completely
                indexes->deallocateRecord(childAddress, nodeSize);

                // Update root to now point to the parent's left pointer
                // Subsequently, load the right pointer into main memory and update root node.
                root = (TreeNode *) indexes->loadRecordFromStorage(cursor->pointers[1], nodeSize);
                rootAddress = (TreeNode *) cursor->pointers[1].blockAddress;

                // We then proceed to deallocate the old parent root
                indexes->deallocateRecord(cursorAddress, nodeSize);

                std::cout << "RootNode updated." << endl;
                return;
            }
        }
    }

    // If we reach this part of code, indicates nothing has yet been returned
    // It means parent node is NOT the root node.
    // We need to delete (possibly recursively) an internal node

    int position;

    // Based on the lower bounding key of child node, search for key to delete in parent
    for (position = 0; position < cursor->numKeys; position++) {
        //find the position of the key to be removed
        if (cursor->keys[position] == key) {
            break;
        }
    }

    // Proceed to shift all keys forward to delete the key
    for (int x = position; x < cursor->numKeys; x++) {
        //shifting all keys one step forward
        cursor->keys[x] = cursor->keys[x + 1];
    }

    // Search for which pointer to remove in parent node

    // Note: Remember pointers are on the RIGHT for non leaf nodes.

    //run for loop to find position of pointer to be removed
    for (position = 0; position < cursor->numKeys + 1; position++) {
        if (cursor->pointers[position].blockAddress == childDiskAddress) {
            break;
        }
    }
    // Move all the pointer from the position point by one to be able to delete it
    for (int x = position; x < cursor->numKeys + 1; x++) {
        cursor->pointers[x] = cursor->pointers[x + 1];
    }

    // Subtract numKeys by 1 after deleting a key
    cursor->numKeys--;

    //Due to removal of a key there might be an underflow in parent, if yes, need to manage that
    // Check is each node has at least ((maxNumKeys + 1) / 2 - 1) keys
    if (cursor->numKeys >= (maxNumKeys + 1) / 2 - 1) {
        return;
    }

    // If nothing returned, indicates underflw in parentnode after deleting a key
    // We attempt to borrow some key(s) from neighbouring nodes.

    // If this is the root node, just return, no option
    if (cursorDiskAddress == rootAddress) {
        return;
    }

    // If not, we attempt to find the parent of the current parent to get our siblings.
    // Pass in lower bound key of our child to search for it.
    TreeNode *parentStorageAddress = findParent((TreeNode *) rootAddress, cursorDiskAddress, cursor->keys[0]);
    //maintain indexes for left and right sibling nodes
    int rightSibling, leftSibling;

    // Load the parent node into main memory.
    Address parentAddress = Address(parentStorageAddress, 0);
    TreeNode *parent = (TreeNode *) indexes->loadRecordFromStorage(parentAddress, nodeSize);

    // Find left and right sibling of the current node by iterating through pointer positions.
    //Do all the way from 0 to number of keys
    for (position = 0; position < parent->numKeys + 1; position++) {
        if (parent->pointers[position].blockAddress == cursorDiskAddress) {
            //Update the indexes of left and right siblings
            rightSibling = position + 1;
            leftSibling = position - 1;
            break;
        }
    }

    // Attempt first to borrow from left sibling of current node(on same level).
    // Check if left sibling even exists by checking if its index is in valid range i.e. greater than 0.
    if (leftSibling >= 0) {
        // Load in left sibling from disk storage.
        TreeNode *leftNode = (TreeNode *) indexes->loadRecordFromStorage(parent->pointers[leftSibling], nodeSize);

        // Check if we can borrow a key from the left neighbour node without causing underflow.
        // Check if after borrowing, number of keys in left sibling node remains
        // greater than equal to '>=' minimum number of keys required for stable structure
        // Non leaf nodes require a minimum of ⌊n/2⌋
        if (leftNode->numKeys >= (maxNumKeys + 1) / 2) {
            // Since the borrowed key will be smaller than keys in current node
            // Insert it as the leftmost key in current node and shift remaining keys

            // Shift all keys back by one place.
            for (int x = cursor->numKeys; x > 0; x--) {
                cursor->keys[x] = cursor->keys[x - 1];
            }

            // Transfer the borrowed key from neighbour and pointer to current node from left node.
            // We duplicate the current node lower bound key to keep pointers correct and structure intact.
            // Do so at 0th index of current node
            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

            // Shift all pointers back by 1 to fit new one
            for (int x = cursor->numKeys + 1; x > 0; x--) {
                cursor->pointers[x] = cursor->pointers[x - 1];
            }

            // Add pointers from left node to current node
            cursor->pointers[0] = leftNode->pointers[leftNode->numKeys];

            // Change the numbers of keys in left node and current node
            // We subtract 1 from keys in left node add 1 in current node
            leftNode->numKeys--;
            cursor->numKeys++;


            // Updating the left sibling nde by shifting pointers to the left
            leftNode->pointers[cursor->numKeys] = leftNode->pointers[cursor->numKeys + 1];

            // Save parent to disk storage
            Address parentAddress = Address(parentStorageAddress, 0);
            indexes->saveRecordToStorage(parent, nodeSize, parentAddress);

            // Save left sibling to disk storage
            indexes->saveRecordToStorage(leftNode, nodeSize, parent->pointers[leftSibling]);

            // Save current node to disk storage
            Address cursorAddress = Address(cursorDiskAddress, 0);
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    // If unsuccessful in borrowing from the left sibling, attempt to borrow from right from the right.
    // Check if we even have a right sibling.
    if (rightSibling <= parent->numKeys) {
        // If we do, load right sibling node from disk storage
        TreeNode *rightNode = (TreeNode *) indexes->loadRecordFromStorage(parent->pointers[rightSibling], nodeSize);

        // Check if we can borrow a key from the left neighbour node without causing underflow.
        // Check if after borrowing, number of keys in left sibling node remains
        // greater than equal to '>=' minimum number of keys required for stable structure
        if (rightNode->numKeys >= (maxNumKeys + 1) / 2) {
            // We do not even need to shift any remaining keys and pointers because we are inserting on the rightmost.
            // It is also the leftmost key of right sibling node
            // It is the rightmost key of current node
            cursor->keys[cursor->numKeys] = parent->keys[position];
            parent->keys[position] = rightNode->keys[0];

            // Update right sibling by shifting remaining pointers and keys
            for (int x = 0; x < rightNode->numKeys - 1; x++) {
                rightNode->keys[x] = rightNode->keys[x + 1];
            }

            // Move the first pointer from right TreeNode to the current node
            cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

            // Delete first pointer in right node and shift remaining pointers to the left
            for (int x = 0; x < rightNode->numKeys; ++x) {
                rightNode->pointers[x] = rightNode->pointers[x + 1];
            }

            // Update numKeys bu adding a key to current node and subtracting a key fromt he right node
            cursor->numKeys++;
            rightNode->numKeys--;

            // Save parent node to disk storage
            Address parentAddress = Address(parentStorageAddress, 0);
            indexes->saveRecordToStorage(parent, nodeSize, parentAddress);

            // Save right sibling node to disk storage.
            indexes->saveRecordToStorage(rightNode, nodeSize, parent->pointers[rightSibling]);

            // Save current node to disk storage.
            Address cursorAddress = Address(cursorDiskAddress, 0);
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    // If we reach this part of code, there was no sibling node to borrow keys from without causing imbalance in tree structure
    // Solution: Merge nodes
    // Note: Merging will always succeed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current) - never overflow or underflow.

    // First attempt to merge with left node
    if (leftSibling >= 0) {
        // Load in left sibling from disk storage.
        TreeNode *leftNode = (TreeNode *) indexes->loadRecordFromStorage(parent->pointers[leftSibling], nodeSize);

        // Make left node's upper bound to be cursor's lower bound according to requirements.
        leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

        // Transfer all keys from current node to left sibling node.
        for (int x = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++) {
            leftNode->keys[x] = cursor->keys[j];
        }
        // Transfer all pointers also
        Address nullAddress = Address(nullptr, 0);
        for (int x = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++) {
            leftNode->pointers[x] = cursor->pointers[j];
            cursor->pointers[j] = nullAddress;
        }

        // Update respective variables such as keys and pointers
        // Make last pointer of left sibling node point to current which is the next node
        leftNode->numKeys += cursor->numKeys + 1;
        cursor->numKeys = 0;

        // Save left node to disk storage.
        indexes->saveRecordToStorage(leftNode, nodeSize, parent->pointers[leftSibling]);

        // Delete current node
        // Recursive update until root node is found and all parent/internal nodes are balanced
        updateInternal(parent->keys[leftSibling], (TreeNode *) parentStorageAddress, (TreeNode *) cursorDiskAddress);
    }

        // Right
        // However, If left sibling doesn't exist,
        // Merge with right sibling if it exists.
    else if (rightSibling <= parent->numKeys) {
        // Load in right sibling from disk.
        TreeNode *rightNode = (TreeNode *) indexes->loadRecordFromStorage(parent->pointers[rightSibling], nodeSize);

        // Set upper bound of cursor to be lower bound of right sibling.
        cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

        // Proceed with the transfer of all keys from right sibling node to current node
        for (int x = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++) {
            cursor->keys[x] = rightNode->keys[j];
        }

        // Transfer all pointers from right node into current.
        //Separately cause we have an extra ptr in  non leaf nodes
        Address nullAddress = Address(nullptr, 0);
        for (int x = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++) {
            cursor->pointers[x] = rightNode->pointers[j];
            rightNode->pointers[j] = nullAddress;
        }

        // Update respective variables such as keys and pointers
        // Make last pointer of current node point to head pointer to left sibling node which is the next node
        cursor->numKeys += rightNode->numKeys + 1;
        rightNode->numKeys = 0;

        // Save current node to disk storage
        Address cursorAddress = Address(cursorDiskAddress, 0);
        indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

        // Proceed to remove right node.
        // Need to update the parent in order to fully remove the right node.
        void *rightNodeAddress = parent->pointers[rightSibling].blockAddress;

        //recursive call again to updateInternal to do the rest
        updateInternal(parent->keys[rightSibling - 1], (TreeNode *) parentStorageAddress,
                       (TreeNode *) rightNodeAddress);
    }
}
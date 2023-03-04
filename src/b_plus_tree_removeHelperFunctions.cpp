//
// Created by anush on 04-03-2023.
//

#include "b_plus_tree.h"
#include "tree_node.h"

#include <iostream>

using namespace std;

//Recursive function to deallocate records/ entire blocks if safe till no more left to deallocate
void BPlusTree::removeLinkedList(Address LinkedListHeadAddress)
{
    // Load in the head of node directly from disk storage.
    TreeNode *head = (TreeNode *)indexes->loadRecordFromStorage(LinkedListHeadAddress, nodeSize);

    // Deleting the current head.
    // Continue to deallocate the entire block through the list till no more nodes left to deallocate

    // Deallocating current record using deallocateRecord function.
    indexes->deallocateRecord(LinkedListHeadAddress, nodeSize);

    //Base condition - stop if end of file reached
    if (head->pointers[head->numKeys].blockAddress == nullptr)
    {
        std::cout << "Reached the end of linked list";
        return;
    }
    // Nodes still left in linked list
    if (head->pointers[head->numKeys].blockAddress != nullptr)
    {
        removeLinkedList(head->pointers[head->numKeys]);
    }
}

// Adjusting the parent
// If the parent is not full enough recursively apply the deletion algorithm in parent


// This function runs till all parent/internal nodes are updated to
// Regain B+ Tree structure after removing a node in case 5, no sibling node found to borrow
// Inputs storage address of parent, the child address to delete, and removes the child.
void BPlusTree::updateInternal(float key, TreeNode *cursorStorageAddress, TreeNode *childStorageAddress)
{
    // Load in the latest copy of parent which is pointed to by cursor and child from disk storage
    Address cursorAddress{cursorStorageAddress, 0};
    TreeNode *cursor = (TreeNode *)indexes->loadRecordFromStorage(cursorAddress, nodeSize);

    // Proceed to retrieve address of child node to be removed
    Address childAddress{childStorageAddress, 0};

    // Start by checking is cursor storage address is same as root address
    if (cursorStorageAddress == rootAddress)
    {
        root = cursor;
    }

    if (cursor == root)
    {
        // All edits happen in main memory

        // If there is only one key in root node
        // We have to remove all keys and need to change the root node to its child.
        if (cursor->numKeys == 1)
        {
            // If the second (larger)pointer points to child, make it the new root.
            if (cursor->pointers[1].blockAddress == childStorageAddress)
            {
                // Remove the child completely
                indexes->deallocateRecord(childAddress, nodeSize);

                // Update root to now point to the parent's left pointer
                // Subsequently, load the left pointer into main memory and update root node.
                root = (TreeNode *)indexes->loadRecordFromStorage(cursor->pointers[0], nodeSize);
                rootAddress = (TreeNode *)cursor->pointers[0].blockAddress;

                // We then proceed to deallocate the old parent root
                indexes->deallocateRecord(cursorAddress, nodeSize);

                std::cout << "RootNode updated." << endl;
                return;
            }

            // Else if left pointer (smaller) in root points to the child, delete from there.
            else if (cursor->pointers[0].blockAddress == childStorageAddress)
            {
                // Delete the child completely
                indexes->deallocateRecord(childAddress, nodeSize);

                // Update root to now point to the parent's left pointer
                // Subsequently, load the right pointer into main memory and update root node.
                root = (TreeNode *)indexes->loadRecordFromStorage(cursor->pointers[1], nodeSize);
                rootAddress = (TreeNode *)cursor->pointers[1].blockAddress;

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
    for (position = 0; position < cursor->numKeys; position++)
    {
        //find the position of the key to be removed
        if (cursor->keys[position] == key)
        {
            break;
        }
    }

    // Proceed to shift all keys forward to delete the key
    for (int x= position; x< cursor->numKeys; x++)
    {
        //shifting all keys one step forward
        cursor->keys[x] = cursor->keys[x+ 1];
    }

    // Search for which pointer to remove in parent node

    // Note: Remember pointers are on the RIGHT for non leaf nodes.

    //run for loop to find position of pointer to be removed
    for (position = 0; position < cursor->numKeys + 1; position++)
    {
        if (cursor->pointers[position].blockAddress == childStorageAddress)
        {
            break;
        }
    }
    // Move all the pointer from the position point by one to be able to delete it
    for (int x= position; x< cursor->numKeys + 1; x++)
    {
        cursor->pointers[x] = cursor->pointers[x+ 1];
    }

    // Subtract numKeys by 1 after deleting a key
    cursor->numKeys--;

    //Due to removal of a key there might be an underflow in parent, if yes, need to manage that
    // Check is each node has at least ((maxNumKeys + 1) / 2 - 1) keys
    if (cursor->numKeys >= (maxNumKeys + 1) / 2 - 1)
    {
        return;
    }

    // If nothing returned, indicates underflw in parentnode after deleting a key
    // We attempt to borrow some key(s) from neighbouring nodes.

    // If this is the root node, just return, no option
    if (cursorStorageAddress == rootAddress)
    {
        return;
    }

    // If not, we attempt to find the parent of the current parent to get our siblings.
    // Pass in lower bound key of our child to search for it.
    TreeNode *parentStorageAddress = findParent((TreeNode *)rootAddress, cursorStorageAddress, cursor->keys[0]);
    //maintain indexes for left and right sibling nodes
    int rightSibling, leftSibling;

    // Load the parent node into main memory.
    Address parentAddress{parentStorageAddress, 0};
    TreeNode *parent = (TreeNode *)indexes->loadRecordFromStorage(parentAddress, nodeSize);

    // Find left and right sibling of the current node by iterating through pointer positions.
    //Do all the way from 0 to number of keys
    for (position = 0; position < parent->numKeys + 1; position++)
    {
        if (parent->pointers[position].blockAddress == cursorStorageAddress)
        {
            //Update the indexes of left and right siblings
            rightSibling = position + 1;
            leftSibling = position - 1;
            break;
        }
    }

    // Attempt first to borrow from left sibling of current node(on same level).
    // Check if left sibling even exists by checking if its index is in valid range i.e. greater than 0.
    if (leftSibling >= 0)
    {
        // Load in left sibling from disk storage.
        TreeNode *leftNode = (TreeNode *)indexes->loadRecordFromStorage(parent->pointers[leftSibling], nodeSize);

        // Check if we can borrow a key from the left neighbour node without causing underflow.
        // Check if after borrowing, number of keys in left sibling node remains
        // greater than equal to '>=' minimum number of keys required for stable structure
        // Non leaf nodes require a minimum of ⌊n/2⌋
        if (leftNode->numKeys >= (maxNumKeys + 1) / 2)
        {
            // Since the borrowed key will be smaller than keys in current node
            // Insert it as the leftmost key in current node and shift remaining keys

            // Shift all keys back by one place.
            for (int x= cursor->numKeys; x> 0; x--)
            {
                cursor->keys[x] = cursor->keys[x- 1];
            }

            // Transfer the borrowed key from neighbour and pointer to current node from left node.
            // We duplicate the current node lower bound key to keep pointers correct and structure intact.
            // Do so at 0th index of current node
            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];

            // Shift all pointers back by 1 to fit new one
            for (int x= cursor->numKeys + 1; x> 0; x--)
            {
                cursor->pointers[x] = cursor->pointers[x- 1];
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
            Address parentAddress{parentStorageAddress, 0};
            indexes->saveRecordToStorage(parent, nodeSize, parentAddress);

            // Save left sibling to disk storage
            indexes->saveRecordToStorage(leftNode, nodeSize, parent->pointers[leftSibling]);

            // Save current node to disk storage
            Address cursorAddress = {cursorStorageAddress, 0};
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    // If unsuccessful in borrowing from the left sibling, attempt to borrow from right from the right.
    // Check if we even have a right sibling.
    if (rightSibling <= parent->numKeys)
    {
        // If we do, load right sibling node from disk storage
        TreeNode *rightNode = (TreeNode *)indexes->loadRecordFromStorage(parent->pointers[rightSibling], nodeSize);

        // Check if we can borrow a key from the left neighbour node without causing underflow.
        // Check if after borrowing, number of keys in left sibling node remains
        // greater than equal to '>=' minimum number of keys required for stable structure
        if (rightNode->numKeys >= (maxNumKeys + 1) / 2)
        {
            // We do not even need to shift any remaining keys and pointers because we are inserting on the rightmost.
            // It is also the leftmost key of right sibling node
            // It is the rightmost key of current node
            cursor->keys[cursor->numKeys] = parent->keys[position];
            parent->keys[position] = rightNode->keys[0];

            // Update right sibling by shifting remaining pointers and keys
            for (int x= 0; x< rightNode->numKeys - 1; x++)
            {
                rightNode->keys[x] = rightNode->keys[x+ 1];
            }

            // Move the first pointer from right TreeNode to the current node
            cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];

            // Delete first pointer in right node and shift remaining pointers to the left
            for (int x= 0; x< rightNode->numKeys; ++x)
            {
                rightNode->pointers[x] = rightNode->pointers[x+ 1];
            }

            // Update numKeys bu adding a key to current node and subtracting a key fromt he right node
            cursor->numKeys++;
            rightNode->numKeys--;

            // Save parent node to disk storage
            Address parentAddress{parentStorageAddress, 0};
            indexes->saveRecordToStorage(parent, nodeSize, parentAddress);

            // Save right sibling node to disk storage.
            indexes->saveRecordToStorage(rightNode, nodeSize, parent->pointers[rightSibling]);

            // Save current node to disk storage.
            Address cursorAddress = {cursorStorageAddress, 0};
            indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    // If we reach this part of code, there was no sibling node to borrow keys from without causing imbalance in tree structure
    // Solution: Merge nodes
    // Note: Merging will always succeed due to ⌊(n)/2⌋ (left) + ⌊(n-1)/2⌋ (current) - never overflow or underflow.

    // First attempt to merge with left node
    if (leftSibling >= 0)
    {
        // Load in left sibling from disk storage.
        TreeNode *leftNode = (TreeNode *)indexes->loadRecordFromStorage(parent->pointers[leftSibling], nodeSize);

        // Make left node's upper bound to be cursor's lower bound according to requirements.
        leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];

        // Transfer all keys from current node to left sibling node.
        for (int x= leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++)
        {
            leftNode->keys[x] = cursor->keys[j];
        }
        // Transfer all pointers also
        Address nullAddress{nullptr, 0};
        for (int x= leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++)
        {
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
        updateInternal(parent->keys[leftSibling], (TreeNode *)parentStorageAddress, (TreeNode *)cursorStorageAddress);
    }

    // Right
    // However, If left sibling doesn't exist,
    // Merge with right sibling if it exists.
    else if (rightSibling <= parent->numKeys)
    {
        // Load in right sibling from disk.
        TreeNode *rightNode = (TreeNode *)indexes->loadRecordFromStorage(parent->pointers[rightSibling], nodeSize);

        // Set upper bound of cursor to be lower bound of right sibling.
        cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];

        // Proceed with the transfer of all keys from right sibling node to current node
        for (int x= cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++)
        {
            cursor->keys[x] = rightNode->keys[j];
        }

        // Transfer all pointers from right node into current.
        //Separately cause we have an extra ptr in  non leaf nodes
        Address nullAddress = {nullptr, 0};
        for (int x= cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++)
        {
            cursor->pointers[x] = rightNode->pointers[j];
            rightNode->pointers[j] = nullAddress;
        }

        // Update respective variables such as keys and pointers
        // Make last pointer of current node point to head pointer to left sibling node which is the next node
        cursor->numKeys += rightNode->numKeys + 1;
        rightNode->numKeys = 0;

        // Save current node to disk storage
        Address cursorAddress{cursorStorageAddress, 0};
        indexes->saveRecordToStorage(cursor, nodeSize, cursorAddress);

        // Proceed to remove right node.
        // Need to update the parent in order to fully remove the right node.
        void *rightNodeAddress = parent->pointers[rightSibling].blockAddress;

        //recursive call again to updateInternal to do the rest
        updateInternal(parent->keys[rightSibling - 1], (TreeNode *)parentStorageAddress, (TreeNode *)rightNodeAddress);
    }
}


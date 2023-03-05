#include "b_plus_tree.h"
#include "tree_node.h"
#include "storage_components.h"

#include <iostream>

using namespace std;

void BPlusTree::insert(Address recordAddress, int key) {
    // If the tree does not have a root node
    if (rootAddress == nullptr) {
        // Create a new linked list for this key value, in case of duplicated keys in the future
        TreeNode *LLNode = new TreeNode(maxNumKeys);
        LLNode->keys[0] = key;
        LLNode->leafNode = false;
        LLNode->numKeys = 1;
        LLNode->pointers[0] = recordAddress;
        // Save the linked list to the indexes storage
        Address LLNodeAddress = indexes->saveRecordToStorage((void *) LLNode, nodeSize);

        // Create the root node
        root = new TreeNode(maxNumKeys);
        root->keys[0] = key;
        root->leafNode = true;
        root->numKeys = 1;
        root->pointers[0] = LLNodeAddress;
        // Save the root node to the indexes storage
        rootAddress = indexes->saveRecordToStorage(root, nodeSize).blockAddress;

    }
        // Else if root exists already, traverse the nodes to find the proper place to insert the key.
    else {
        // Set a variable to keep track of the current node of the search
        TreeNode *current = root;
        // Set a variable to keep track of the current's parent
        TreeNode *parent;
        // Get their storage addresses
        void *parentAddress = rootAddress;
        void *currentAddress = rootAddress;

        // While the current is not a leaf node
        while (current->leafNode == false) {
            // Set the parent node and its address
            parent = current;
            parentAddress = currentAddress;

            // Loop to find the key that and the pointer to the range the inserted key has to be
            for (int i = 0; i < current->numKeys; i++) {
                // Set a variable for the next node in the search
                TreeNode *nextNode;
                // If the inserted key is less than the current key in the loop, go to the left pointer
                if (key < current->keys[i]) {
                    // Load the new node from the storage
                    nextNode = (TreeNode *) indexes->loadRecordFromStorage(current->pointers[i], nodeSize);
                    // Set the current node to be the newly found one
                    currentAddress = current->pointers[i].blockAddress;
                    current = nextNode;
                    break;
                }
                // Else if the key is larger than all the keys in the current node, get the last assigned pointer
                if (i == current->numKeys - 1) {
                    // Load the new node from the storage
                    nextNode = (TreeNode *) indexes->loadRecordFromStorage(current->pointers[i + 1], nodeSize);
                    // Set the current node to be the newly found one
                    currentAddress = current->pointers[i+1].blockAddress;
                    current = nextNode;
                    break;
                }
            }
        }

        // We exit the while, meaning that we have reached leaf nodes level
        if (current->numKeys < maxNumKeys) {
            int i = 0;
            // Find the position where the new key should be inserted
            while (key > current->keys[i] && i < current->numKeys) i++;

            // If the position already has the same key
            if (current->keys[i] == key) {
                // The key is a duplicating one, so it is inserted into the linked for this key value
                current->pointers[i] = insertLL(current->pointers[i], recordAddress, key);
            }
                // Else if the searched position's key is not the same as the inserted one
                // This key value is inserted for the first time
            else {
                // Set the last pointer to the previous last pointer node
                Address next = current->pointers[current->numKeys];

                // A loop to move all the keys and according pointers, larger than the inserted one
                for (int j = current->numKeys; j > i; j--) {
                    // Swap
                    current->keys[j] = current->keys[j - 1];
                    current->pointers[j] = current->pointers[j - 1];
                }
                // Insert the new key
                current->keys[i] = key;

                // Create a new linked list for this key value, in case of duplicated keys in the future
                TreeNode *LLNode = new TreeNode(maxNumKeys);
                LLNode->keys[0] = key;
                LLNode->leafNode = false;
                LLNode->numKeys = 1;
                LLNode->pointers[0] = recordAddress;
                // Save the linked list to the indexes storage
                Address LLNodeAddress = indexes->saveRecordToStorage((void *) LLNode, nodeSize);

                // Refresh the pointer and the number of keys in the leaf node
                current->pointers[i] = LLNodeAddress;
                current->numKeys++;
                // Update leaf node pointer to point to the next node
                current->pointers[current->numKeys] = next;

                // Save the updated node at the indexes storage
                Address currentAddressObject = Address(currentAddress, 0);
                indexes->saveRecordToStorage(current, nodeSize, currentAddressObject);
            }
        }
            // There is no space for a new key in the leaf node
        else {
            // Create a new leaf node that will take half of the content of the full one
            TreeNode *newNode = new TreeNode(maxNumKeys);
            // Save the current keys plus the new key to a temporary array
            int keysT[maxNumKeys + 1];
            // Save the current pointers plus the new one to a temporary array
            Address *pointersT = (Address *) malloc(sizeof(Address) * (maxNumKeys + 1));
            Address nextNode = current->pointers[current->numKeys];
            // Copy the contents
            for (int i = 0; i < maxNumKeys; i++) {
                keysT[i] = current->keys[i];
                pointersT[i] = current->pointers[i];
            }
            // Insert the new key
            int i = 0;
            while (key > keysT[i] && i < maxNumKeys) i++;

            // Check if the key is a duplicate of an already inserted one
            if (i < current->numKeys) {
                if (current->keys[i] == key) {
                    // Store teh new key to the linked list
                    current->pointers[i] = insertLL(current->pointers[i], recordAddress, key);
                    return;
                }
            }
            // The key is not a duplicating one
            for (int j = maxNumKeys; j > i; j--) {
                // Swap
                keysT[j] = keysT[j - 1];
                pointersT[j] = pointersT[j - 1];
            }

            // Add the new key and pointer to the temporary arrays
            keysT[i] = key;

            // Create a linked list to store records with duplicating keys in the future
            TreeNode *LLNode = new TreeNode(maxNumKeys);
            LLNode->keys[0] = key;
            LLNode->leafNode = false;
            LLNode->numKeys = 1;
            LLNode->pointers[0] = recordAddress;
            // Save the linked list to the indexes storage
            Address LLNodeAddress = indexes->saveRecordToStorage((void *) LLNode, nodeSize);
            pointersT[i] = LLNodeAddress;
            // We set the new node's leafNode attribute to be true because the new node is a leaf node
            newNode->leafNode = true;

            // The splitting into two nodes
            current->numKeys = (maxNumKeys + 1) / 2;
            newNode->numKeys = (maxNumKeys + 1) - ((maxNumKeys + 1) / 2);

            // Set the last pointer of the new leaf node to point to the previous last pointer before the insertion
            newNode->pointers[newNode->numKeys] = nextNode;

            // Add the keys and pointers to the previous node
            for (i = 0; i < current->numKeys; i++) {
                current->keys[i] = keysT[i];
                current->pointers[i] = pointersT[i];
            }

            // Add the keys and pointers to the new node
            for (int j = 0; j < newNode->numKeys; i++, j++) {
                newNode->keys[j] = keysT[i];
                newNode->pointers[j] = pointersT[i];
            }

            // Save the new node to the indexes storage
            Address newNodeAddress = indexes->saveRecordToStorage(newNode, nodeSize);

            // Now to set the current's pointer to the disk address of the leaf and save it in place
            current->pointers[current->numKeys] = newNodeAddress;

            // clean the trash key values from the current node
            for (int i = current->numKeys; i < maxNumKeys; i++) {
                current->keys[i] = int();
            }
            // clean the trash pointer values from the current node
            for (int i = current->numKeys + 1; i < maxNumKeys + 1; i++) {
                Address nullAddress = Address(nullptr, 0);
                current->pointers[i] = nullAddress;
            }

            // Save the current node to the indexes storage
            Address currentAddressObject = Address(currentAddress, 0);
            indexes->saveRecordToStorage(current, nodeSize, currentAddressObject);

            // If the root is a leaf node
            if (current == root) {
                TreeNode *newRoot = new TreeNode(maxNumKeys);
                // Set the new's root first key to be the left bound of the right child
                newRoot->keys[0] = newNode->keys[0];

                // Set the new pointers of the new root node
                newRoot->pointers[0] = currentAddressObject;
                newRoot->pointers[1] = newNodeAddress;
                // Refresh the attributes of newRoot
                newRoot->leafNode = false;
                newRoot->numKeys = 1;

                // Write the new root node to indexes storage
                Address newRootAddress = indexes->saveRecordToStorage(newRoot, nodeSize);

                // Update the root address of the B+ tree
                rootAddress = newRootAddress.blockAddress;
                root = newRoot;
            }
                // If we are not at the root
            else {
                // Insert/Update a new node to be a parent among the internal nodes
                insertInternal(newNode->keys[0], (TreeNode *) parentAddress, (TreeNode *) newNodeAddress.blockAddress);
            }
        }
    }
    // Refresh the number of nodes in the tree
    numNodes = indexes->getBlocksAllocated();
}

void BPlusTree::insertInternal(int key, TreeNode *parentAddress, TreeNode *childAddress) {
    // Get the current parent from the storage
    Address parentAddressObject = Address(parentAddress, 0);
    TreeNode *parent = (TreeNode *) indexes->loadRecordFromStorage(parentAddressObject, nodeSize);

    // If the current's address in the root address, then the root is the current node
    if (parentAddress == rootAddress) {
        root = parent;
    }

    Address childAddressObject = Address(childAddress, 0);
    TreeNode *child = (TreeNode *) indexes->loadRecordFromStorage(childAddressObject, nodeSize);

    // If parent has space, we can add the child node as a pointer.
    if (parent->numKeys < maxNumKeys) {
        // Find the position where to put the child
        int i = 0;
        while (key > parent->keys[i] && i < parent->numKeys) i++;

        // Swap all keys to insert the child one
        for (int j = parent->numKeys; j > i; j--) {
            parent->keys[j] = parent->keys[j - 1];
        }
        // Move the pointers to right with one position
        for (int j = parent->numKeys + 1; j > i + 1; j--) {
            parent->pointers[j] = parent->pointers[j - 1];
        }

        // Add the child's key to the parent's correct position
        parent->keys[i] = key;
        parent->numKeys++;
        //Add the child's pointer to the parent
        parent->pointers[i + 1] = childAddressObject;

        // Update the parent's record in the storage
        indexes->saveRecordToStorage(parent, nodeSize, parentAddressObject);
    }
        // If parent node doesn't have space
    else {
        // Create a new internal node
        TreeNode *newNode = new TreeNode(maxNumKeys);

        // Again create temporary arrays for the keys and pointers, but now we have one more pointer to track the child
        int keysT[maxNumKeys + 1];
        Address *pointersT = (Address *) malloc(sizeof(Address) * (maxNumKeys + 2));
        // Copy the keys into the temporary array
        for (int i = 0; i < maxNumKeys; i++) {
            keysT[i] = parent->keys[i];
        }
        // Copy the pointers into the temporary array
        for (int i = 0; i < maxNumKeys + 1; i++) {
            pointersT[i] = parent->pointers[i];
        }

        // Find the position of the inserted key
        int i = 0;
        while (key > keysT[i] && i < maxNumKeys) i++;

        // Swap all elements higher than the index
        int j;
        for (int j = maxNumKeys; j > i; j--) {
            keysT[j] = keysT[j - 1];
        }
        // Insert the key into the correct spot
        keysT[i] = key;

        // Shift the pointers too
        for (int j = maxNumKeys + 1; j > (i + 1); j--) {
            pointersT[j] = pointersT[j - 1];
        }
        // Insert the pointer at the correct position
        pointersT[i + 1] = childAddressObject;

        // The new node is a parent, so it cannot be a leaf node
        newNode->leafNode = false;

        // Splitting the two nodes
        parent->numKeys = (maxNumKeys + 1) / 2;
        newNode->numKeys = maxNumKeys - (maxNumKeys + 1) / 2;

        // Putting the keys in the parent
        for (int i = 0; i < parent->numKeys; i++) {
            parent->keys[i] = keysT[i];
        }
        // Putting the values in the new internal node.
        for (i = 0, j = parent->numKeys + 1; i < newNode->numKeys; i++, j++) {
            newNode->keys[i] = keysT[j];
        }

        // Putting the pointers in the parent.
        for (i = 0, j = parent->numKeys + 1; i < newNode->numKeys + 1; i++, j++) {
            newNode->pointers[i] = pointersT[j];
        }

        // Clean trash values
        for (int i = parent->numKeys; i < maxNumKeys; i++) {
            parent->keys[i] = int();
        }
        for (int i = parent->numKeys + 1; i < maxNumKeys + 1; i++) {
            Address nullAddress = Address(nullptr, 0);
            parent->pointers[i] = nullAddress;
        }

        // Set the pointer from parent to child
        parent->pointers[parent->numKeys] = childAddressObject;

        // Save the old parent and the new internal node to the storage
        indexes->saveRecordToStorage(parent, nodeSize, parentAddressObject);
        Address newNodeAddress = indexes->saveRecordToStorage(newNode, nodeSize);

        // If current cursor is the root of the tree, we need to create a new root
        if (parent == root) {
            TreeNode *newRoot = new TreeNode(nodeSize);
            newRoot->keys[0] = parent->keys[parent->numKeys];

            // Set the new root node's children to be the previous parent and the new internal node
            newRoot->pointers[0] = parentAddressObject;
            newRoot->pointers[1] = newNodeAddress;

            // Refresh new root's attributes
            newRoot->leafNode = false;
            newRoot->numKeys = 1;
            root = newRoot;

            // Save new root node into the indexes storage
            Address newRootAddress = indexes->saveRecordToStorage(root, nodeSize);
            // Refresh the root address of the tree
            rootAddress = newRootAddress.blockAddress;
        }
            // Else if the parent is not a root, thus we need to split again
        else {
            TreeNode *newParent = findParent((TreeNode *) rootAddress, parentAddress, parent->keys[0]);
            insertInternal(keysT[parent->numKeys], newParent, (TreeNode *) newNodeAddress.blockAddress);
        }
    }
}

Address BPlusTree::insertLL(Address LLHead, Address recordAddress, int key) {
    // Load the head of the linked list
    TreeNode *head = (TreeNode *) indexes->loadRecordFromStorage(LLHead, nodeSize);

    // Check if the head node has a space for the new record
    if (head->numKeys < maxNumKeys) {
        // The head has space for the new record
        // Move all keys to right with one position
        for (int i = head->numKeys; i > 0; i--) {
            head->keys[i] = head->keys[i - 1];
        }
        // ove all pointers to right with one position
        for (int i = head->numKeys + 1; i > 0; i--) {
            head->pointers[i] = head->pointers[i - 1];
        }

        // Insert new record into the head node
        head->keys[0] = key;
        head->pointers[0] = recordAddress;
        head->numKeys++;

        // Save the updated head to the storage
        LLHead = indexes->saveRecordToStorage((void *) head, nodeSize, LLHead);
        return LLHead;
    }
        // There is no space for the record in the head
    else {
        // Create a new linked list node
        TreeNode *LLNode = new TreeNode(maxNumKeys);
        LLNode->leafNode = false;
        LLNode->keys[0] = key;
        LLNode->numKeys = 1;

        // Insert the record into the new node
        LLNode->pointers[0] = recordAddress;
        // Make the new node head of the linked list
        LLNode->pointers[1] = LLHead;

        // Save the new node to the index storage
        Address LLNodeAddress = indexes->saveRecordToStorage((void *) LLNode, nodeSize);
        return LLNodeAddress;
    }
}
#include "b_plus_tree.h"
#include "tree_node.h"
#include "storage_components.h"

#include <vector>
#include <iostream>

using namespace std;


// Function to handle range queries from start key to end key
void BPlusTree::search(int lowerBoundKey, int upperBoundKey)
{
    // Tree is empty.
    if (rootAddress == nullptr)
    {
        throw std::logic_error("Tree is empty!");
    }
        // Else iterate through root node and follow the keys to find the correct key.
    else
    {
        // Load in root from disk.
        Address rootDiskAddress{rootAddress, 0};
        root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);

        // for displaying to output file
        std::cout << "Index node accessed. Content is -----";
        displayNode(root);

        Node *cursor = root;

        bool found = false;

        // While we haven't hit a leaf node, and haven't found a range.
        while (cursor->isLeaf == false)
        {
            // Iterate through each key in the current node. We need to load nodes from the disk whenever we want to traverse to another node.
            for (int i = 0; i < cursor->numKeys; i++)
            {
                // If lowerBoundKey is lesser than current key, go to the left pointer's node to continue searching.
                if (lowerBoundKey < cursor->keys[i])
                {
                    // Load node from disk to main memory.
                    cursor = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

                    // for displaying to output file
                    std::cout << "Index node accessed. Content is -----";
                    displayNode(cursor);

                    break;
                }
                // If we reached the end of all keys in this node (larger than all), then go to the right pointer's node to continue searching.
                if (i == cursor->numKeys - 1)
                {
                    // Load node from disk to main memory.
                    // Set cursor to the child node, now loaded in main memory.
                    cursor = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

                    // for displaying to output file
                    std::cout << "Index node accessed. Content is -----";
                    displayNode(cursor);
                    break;
                }
            }
        }

        // When we reach here, we have hit a leaf node corresponding to the lowerBoundKey.
        // Again, search each of the leaf node's keys to find a match.
        // vector<Record> results;
        // unordered_map<void *, void *> loadedBlocks; // Maintain a reference to all loaded blocks in main memory.

        // Keep searching whole range until we find a key that is out of range.
        bool stop = false;

        while (stop == false)
        {
            int i;
            for (i = 0; i < cursor->numKeys; i++)
            {
                // Found a key within range, now we need to iterate through the entire range until the upperBoundKey.
                if (cursor->keys[i] > upperBoundKey)
                {
                    stop = true;
                    break;
                }
                if (cursor->keys[i] >= lowerBoundKey && cursor->keys[i] <= upperBoundKey)
                {
                    // for displaying to output file
                    std::cout << "Index node (LLNode) accessed. Content is -----";
                    displayNode(cursor);

                    // Add new line for each leaf node's linked list printout.
                    std::cout << endl;
                    std::cout << "LLNode: tconst for average rating: " << cursor->keys[i] << " > ";

                    // Access the linked list node and print records.
                    displayLL(cursor->pointers[i]);
                }
            }

            // On the last pointer, check if last key is max, if it is, stop. Also stop if it is already equal to the max
            if (cursor->pointers[cursor->numKeys].blockAddress != nullptr && cursor->keys[i] != upperBoundKey)
            {
                // Set cursor to be next leaf node (load from disk).
                cursor = (Node *)index->loadFromDisk(cursor->pointers[cursor->numKeys], nodeSize);

                // for displaying to output file
                std::cout << "Index node accessed. Content is -----";
                displayNode(cursor);

            }
            else
            {
                stop = true;
            }
        }
    }
    return;
}

void BPlusTree::displayNode(Node *node)
{
    // Print out all contents in the node as such |pointer|key|pointer|
    int i = 0;
    std::cout << "|";
    for (int i = 0; i < node->numKeys; i++)
    {
        std::cout << node->pointers[i].blockAddress << " | ";
        std::cout << node->keys[i] << " | ";
    }

    // Print last filled pointer
    if (node->pointers[node->numKeys].blockAddress == nullptr) {
        std::cout << " Null |";
    }
    else {
        std::cout << node->pointers[node->numKeys].blockAddress << "|";
    }

    for (int i = node->numKeys; i < maxKeys; i++)
    {
        std::cout << " x |";      // Remaining empty keys
        std::cout << "  Null  |"; // Remaining empty pointers
    }

    std::cout << endl;
}

// Display a block and its contents in the disk. Assume it's already loaded in main memory.
void BPlusTree::displayBlock(void *blockAddress)
{
    // Load block into memory
    void *block = operator new(nodeSize);
    std::memcpy(block, blockAddress, nodeSize);

    unsigned char testBlock[nodeSize];
    memset(testBlock, '\0', nodeSize);

    // Block is empty.
    if (memcmp(testBlock, block, nodeSize) == 0)
    {
        std::cout << "Empty block!" << '\n';
        return;
    }

    unsigned char *blockChar = (unsigned char *)block;

    int i = 0;
    while (i < nodeSize)
    {
        // Load each record
        void *recordAddress = operator new(sizeof(Record));
        std::memcpy(recordAddress, blockChar, sizeof(Record));

        Record *record = (Record *)recordAddress;

        std::cout << "[" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "]  ";
        blockChar += sizeof(Record);
        i += sizeof(Record);
    }

}

// Print the tree
void BPlusTree::display(Node *cursorDiskAddress, int level)
{
    // Load in cursor from disk.
    Address cursorMainMemoryAddress{cursorDiskAddress, 0};
    Node *cursor = (Node *)index->loadFromDisk(cursorMainMemoryAddress, nodeSize);

    // If tree exists, display all nodes.
    if (cursor != nullptr)
    {
        for (int i = 0; i < level; i++)
        {
            std::cout << "   ";
        }
        std::cout << " level " << level << ": ";

        displayNode(cursor);

        if (cursor->isLeaf != true)
        {
            for (int i = 0; i < cursor->numKeys + 1; i++)
            {
                // Load node in from disk to main memory.
                Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

                display((Node *)mainMemoryNode, level + 1);
            }
        }
    }
}

void BPlusTree::displayLL(Address LLHeadAddress)
{
    // Load linked list head into main memory.
    Node *head = (Node *)index->loadFromDisk(LLHeadAddress, nodeSize);

    // Print all records in the linked list.
    for (int i = 0; i < head->numKeys; i++)
    {
        // Load the block from disk.
        // void *blockMainMemoryAddress = operator new(nodeSize);
        // std::memcpy(blockMainMemoryAddress, head->pointers[i].blockAddress, nodeSize);

        std::cout << "\nData block accessed. Content is -----";
        displayBlock(head->pointers[i].blockAddress);
        std::cout << endl;

        Record result = *(Record *)(disk->loadFromDisk(head->pointers[i], sizeof(Record)));
        std::cout << result.tconst << " | ";


    }

    // Print empty slots
    for (int i = head->numKeys; i < maxKeys; i++)
    {
        std::cout << "x | ";
    }

    // End of linked list
    if (head->pointers[head->numKeys].blockAddress == nullptr)
    {
        std::cout << "End of linked list" << endl;
        return;
    }

    // Move to next node in linked list.
    if (head->pointers[head->numKeys].blockAddress != nullptr)
    {
        displayLL(head->pointers[head->numKeys]);
    }
}

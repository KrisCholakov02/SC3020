#include "b_plus_tree.h"

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
    // Get size left for keys and pointers in a node after accounting for node's isLeaf and numKeys attributes.
    size_t nodeBufferSize = blockSize - sizeof(bool) - sizeof(int);

    // Set max keys available in a node. Each key is a float, each pointer is a struct of {void *blockAddress, short int offset}.
    // Therefore, each key is 4 bytes. Each pointer is around 16 bytes.

    // Initialize node buffer with a pointer. P | K | P , always one more pointer than keys.
    size_t sum = sizeof(Address);
    maxKeys = 0;

    // Try to fit as many pointer key pairs as possible into the node block.
    while (sum + sizeof(Address) + sizeof(float) <= nodeBufferSize)
    {
        sum += (sizeof(Address) + sizeof(float));
        maxKeys += 1;
    }

    if (maxKeys == 0)
    {
        throw std::overflow_error("Error: Keys and pointers too large to fit into a node!");
    }

    // Initialize root to NULL
    rootAddress = nullptr;
    root = nullptr;

    // Set node size to be equal to block size.
    nodeSize = blockSize;

    // Initialize initial variables
    levels = 0;
    numNodes = 0;

    // Initialize disk space for index and set reference to disk.

    this->disk = disk;
    this->index = index;
}

Node *BPlusTree::findParent(Node *cursorDiskAddress, Node *childDiskAddress, float lowerBoundKey)
{
    // Load in cursor into main memory, starting from root.
    Address cursorAddress{cursorDiskAddress, 0};
    Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

    // If the root cursor passed in is a leaf node, there is no children, therefore no parent.
    if (cursor->isLeaf)
    {
        return nullptr;
    }

    // Maintain parentDiskAddress
    Node *parentDiskAddress = cursorDiskAddress;

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {
        // Check through all pointers of the node to find match.
        for (int i = 0; i < cursor->numKeys + 1; i++)
        {
            if (cursor->pointers[i].blockAddress == childDiskAddress)
            {
                return parentDiskAddress;
            }
        }

        for (int i = 0; i < cursor->numKeys; i++)
        {
            // If key is lesser than current key, go to the left pointer's node.
            if (lowerBoundKey < cursor->keys[i])
            {
                // Load node in from disk to main memory.
                Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

                // Update parent address.
                parentDiskAddress = (Node *)cursor->pointers[i].blockAddress;

                // Move to new node in main memory.
                cursor = (Node *)mainMemoryNode;
                break;
            }

            // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
            if (i == cursor->numKeys - 1)
            {
                // Load node in from disk to main memory.
                Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

                // Update parent address.
                parentDiskAddress = (Node *)cursor->pointers[i + 1].blockAddress;

                // Move to new node in main memory.
                cursor = (Node *)mainMemoryNode;
                break;
            }
        }
    }

    // If we reach here, means cannot find already.
    return nullptr;
}


int BPlusTree::getLevels() {

    if (rootAddress == nullptr) {
        return 0;
    }

    // Load in the root node from disk
    Address rootDiskAddress{rootAddress, 0};
    root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);
    Node *cursor = root;

    levels = 1;

    while (!cursor->isLeaf) {
        cursor = (Node *)index->loadFromDisk(cursor->pointers[0], nodeSize);
        levels++;
    }

    // Account for linked list (count as one level)
    levels++;

    return levels;
}

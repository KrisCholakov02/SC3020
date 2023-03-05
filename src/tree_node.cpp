#include "tree_node.h"
#include "storage_components.h"

bool myNullPtr = false;

Node::Node(int maxKeys)
{
    // Initialize empty array of keys and pointers.
    keys = new float[maxKeys];
    pointers = new Address[maxKeys + 1];

    for (int i = 0; i < maxKeys + 1; i++)
    {
        Address nullAddress{(void *)myNullPtr, 0};
        pointers[i] = nullAddress;
    }
    numKeys = 0;
}
#include "tree_node.h"
#include "storage_components.h"


TreeNode::TreeNode(int maxNumKeys) {
    // Creating the keys' array
    keys = new float[maxNumKeys];
    // Creating the pointers' array
    Address *pointers = (Address *) malloc(sizeof(Address) * (maxNumKeys + 1));

    // Setting all pointers in the array to null pointers
    for (int i = 0; i < maxNumKeys + 1; i++) {
        Address nullAddress = Address((void *) false, 0);
        pointers[i] = nullAddress;
    }
    // Setting the number of keys currently in the node to 0 as there are no keys upon creation
    numKeys = 0;
}
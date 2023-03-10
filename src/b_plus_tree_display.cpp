#include "b_plus_tree.h"
#include "storage_components.h"

#include<iostream>
#include<cstring>

using namespace std;


void BPlusTree:: displayTreeNode(TreeNode *node){
    cout << "|";
    for (int i = 0; i < node->numKeys; i++) {
        cout << node->pointers[i].blockAddress << "|";
        cout << node->keys[i] << "|";
    }

    if(node->pointers[node->numKeys].blockAddress == nullptr){
        cout << "NULL |";
    }
    else{
        cout << node->pointers[node->numKeys].blockAddress << "|";
    }

    for(int i = node->numKeys; i<maxNumKeys; i++){
        cout << "x |";
        cout << " Null |";
    }

    cout << endl;
}


void BPlusTree::displayBlk(void *blkAddr){
    void *block = operator new(nodeSize);
    memcpy(block, blkAddr, nodeSize);

    unsigned  char testBlock[nodeSize];
    memset(testBlock, '\0', nodeSize);

    if(memcpy(testBlock, block, nodeSize) == 0){
        cout << "Empty block!" << '\n';
        return;
    }

    unsigned char *blockChar= (unsigned  char*)block;

    int i = 0;
    while(i<nodeSize){
        void *recAddress = operator new(sizeof (Record));
        memcpy(recAddress, blockChar, sizeof (Record));

        Record *rec = (Record *)recAddress;

        cout << "[" << rec->t << "|" << rec->rating << "|" << rec->numVotes << "] ";
        blockChar += sizeof (Record);
        i+=sizeof (Record);

    }
}

void BPlusTree ::displayRec(Address addr) {
    TreeNode *head = (TreeNode*)indexes->loadRecordFromStorage(addr,nodeSize);

    for(int i =0; i<head->getNumKeys(); i++){
        cout << "\n Data block accessed. Content is ----";
        displayBlk(head->pointers[i].blockAddress);
        cout << endl;

        Record res = *(Record*)(records->loadRecordFromStorage(head->pointers[i], sizeof (Record)));
        cout << res.t << "|";
    }

    for(int i = head->numKeys; i<maxNumKeys; i++){
        cout << "x | ";
    }

    if(head->pointers[head->numKeys].blockAddress == nullptr){
        cout << "End of Linked List" << endl;
        return;
    }

    if(head->pointers[head->numKeys].blockAddress != nullptr){
        displayRec(head->pointers[head->numKeys]);
    }
}

void BPlusTree::display(TreeNode* cursorDiskAddress, int level){
    Address cursorMainMemoryAddress{cursorDiskAddress, 0};
    TreeNode *cursor = (TreeNode*) indexes->loadRecordFromStorage(cursorMainMemoryAddress, nodeSize);

    if(cursor != nullptr){
        for(int i=0; i<level;i++){
            cout << " ";
        }
        cout << " level" << level << ": ";

        displayTreeNode(cursor);

        if (cursor->isLeafNode() != true)
        {
            for (int i = 0; i < cursor->numKeys + 1; i++)
            {
                // Load node in from disk to main memory.
                TreeNode *mainMemoryNode = (TreeNode *)indexes->loadRecordFromStorage(cursor->pointers[i], nodeSize);

                display((TreeNode *)mainMemoryNode, level + 1);
            }
        }
    }
}

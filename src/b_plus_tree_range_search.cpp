#include "b_plus_tree.h"
#include "tree_node.h"
#include "storage_components.h"

#include <vector>
#include <iostream>

using namespace std;

void BPlusTree::rangeSearch(float start, float end) {
    if (rootAddress != nullptr) {
        Address addr = Address(rootAddress, 0);
        root = (TreeNode *) indexes->loadRecordFromStorage(addr, nodeSize);
        cout << "Node Accessed";
        for (int i = 0; i < root->getNumKeys(); i++) {
            cout << root->getPointers()[i].getBlockAddress() << "|";
            cout << root->getKeys()[i] << "|";
        }

        bool foundNode = false;
        TreeNode *temp = root;

        while (!temp->isLeafNode()) {
            for (int j = 0; j < temp->getNumKeys(); j++) {
                if (start < temp->getKeys()[j]) {
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[j], nodeSize);
                    cout << "Node Accessed";
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].getBlockAddress() << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                    break;
                }
                if (j == temp->getNumKeys() - 1) {
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[j + 1], nodeSize);
                    cout << "Node Accessed";
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].getBlockAddress() << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                    break;
                }
            }
        }

        //leaf node

        bool finish = false;

        while (finish == false) {
            for (int k = 0; k < temp->getNumKeys(); k++) {
                if (temp->getKeys()[k] >= start && temp->getKeys()[k] <= end) {
                    cout << "Node Accessed.";
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].getBlockAddress() << "|";
                        cout << temp->getKeys()[i] << "|";

                    }

                    cout << endl;
                    //code to display the record
                }
                if (temp->getKeys()[k] > end) {
                    finish = true;
                    break;
                }
                if (temp->getPointers()[temp->getNumKeys()].getBlockAddress() != nullptr &&
                    temp->getKeys()[k] != end) {
                    temp = (TreeNode *) indexes->loadRecordFromStorage(temp->getPointers()[k], nodeSize);
                    cout << "Node Accessed";
                    for (int i = 0; i < temp->getNumKeys(); i++) {
                        cout << temp->getPointers()[i].getBlockAddress() << "|";
                        cout << temp->getKeys()[i] << "|";

                    }
                } else {
                    finish = true;
                }

            }
        }

    } else {
        throw logic_error("empty B+ Tree");
    }
    return;
}
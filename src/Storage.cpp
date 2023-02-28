//
// Created by anush on 28-02-2023.
//

#include "Storage.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>

// Constants for disk-based storage
const int BLOCK_SIZE = 200;
const int NUM_BLOCKS = 2500;

class Record {
public:
    int id;
    std::string name;
    double salary;
};

class Storage {
private:
    char *disk;
    bool *blocksInUse;
    int numBlocksInUse;

public:
    Storage() {
        // Allocate memory for disk storage and initialize blocks in use array
        disk = new char[BLOCK_SIZE * NUM_BLOCKS];
        blocksInUse = new bool[NUM_BLOCKS];
        numBlocksInUse = 0;
        memset(blocksInUse, false, NUM_BLOCKS);
    }

    ~Storage() {
        // Deallocate memory for disk storage and blocks in use array
        delete[] disk;
        delete[] blocksInUse;
    }

    int findFreeBlock() {
        // Find first free block and mark it as in use
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (!blocksInUse[i]) {
                blocksInUse[i] = true;
                numBlocksInUse++;
                return i;
            }
        }
        return -1;
    }

    void freeBlock(int blockNum) {
        // Mark block as free
        blocksInUse[blockNum] = false;
        numBlocksInUse--;
    }

    bool isBlockInUse(int blockNum) {
        // Check if block is in use
        return blocksInUse[blockNum];
    }

    bool writeBlock(int blockNum, char *data) {
        // Write data to disk block
        if (blockNum < 0 || blockNum >= NUM_BLOCKS) {
            std::cout << "Error: block number out of range" << std::endl;
            return false;
        }
        memcpy(disk + blockNum * BLOCK_SIZE, data, BLOCK_SIZE);
        return true;
    }

    char *readBlock(int blockNum) {
        // Read data from disk block
        if (blockNum < 0 || blockNum >= NUM_BLOCKS) {
            std::cout << "Error: block number out of range" << std::endl;
            return NULL;
        }
        char *blockData = new char[BLOCK_SIZE];
        memcpy(blockData, disk + blockNum * BLOCK_SIZE, BLOCK_SIZE);
        return blockData;
    }

    std::string serializeRecord(const Record &r) {
        // Serialize record into string
        std::string data = std::to_string(r.id) + " " + r.name + " " + std::to_string(r.salary);
        return data;
    }

    Record deserializeRecord(const std::string &data) {
        // Deserialize string into record
        Record r;
        std::stringstream ss(data);
        ss >> r.id >> r.name >> r.salary;
        return r;
    }

    void writeRecord(int id, std::string name, double salary) {
        Record r{id, name, salary};
        std::string serializedRecord = serializeRecord(r);

        int blockNum = findFreeBlock();
        if (blockNum == -1) {
            std::cout << "Error: no free blocks available" << std::endl;
            return;
        }

        int remainingBytes = BLOCK_SIZE - sizeof(int);
        int offset = 0;

        while (serializedRecord.length() > 0) {
            char *blockData = readBlock(blockNum);

            if (blockData == NULL) {
                std::cout << "Error: could not read block " << blockNum << std::endl;
                return;
            }

            if (remainingBytes <= 0) {
                // No more space in this block, move on to the next one
                int nextBlockNum = findFreeBlock();
                if (nextBlockNum == -1) {
                    std::cout << "Error: no free blocks available" << std::endl;
                    freeBlock(blockNum);
                    return;
                }
                *(int *) (blockData + offset) = nextBlockNum;
                writeBlock(blockNum, blockData);
                free(blockData);
                blockData = readBlock(nextBlockNum);
                blockNum = nextBlockNum;
                offset = sizeof(int);
                remainingBytes = BLOCK_SIZE - sizeof(int);
            }

            int copyLength = std::min(remainingBytes, (int) serializedRecord.length());
            memcpy(blockData + offset, serializedRecord.data(), copyLength);
            serializedRecord.erase(0, copyLength);
            offset += copyLength;
            remainingBytes -= copyLength;

            if (serializedRecord.length() == 0) {
                *(int *) (blockData + offset) = -1;
                writeBlock(blockNum, blockData);
            }

            free(blockData);
        }
    }

    void deallocateBlock(int blockNum) {
        if (blockNum < 0 || blockNum >= NUM_BLOCKS) {
            std::cout << "Error: block number out of range" << std::endl;
            return;
        }
        if (!isBlockInUse(blockNum)) {
            std::cout << "Error: block is not in use" << std::endl;
            return;
        }
        freeBlock(blockNum);
    }

    int allocateBlock() {
        // Find first free block and mark it as in use
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (!blocksInUse[i]) {
                blocksInUse[i] = true;
                numBlocksInUse++;
                return i;
            }
        }
        return -1;
    }
};


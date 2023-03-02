//
// Created by anush on 28-02-2023.
//

#ifndef SC3020_STORAGE_H
#define SC3020_STORAGE_H

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
    char* disk;
    bool* blocksInUse;
    int numBlocksInUse;

public:
    Storage();
    ~Storage();

    int findFreeBlock();
    void freeBlock(int blockNum);
    bool isBlockInUse(int blockNum);
    bool writeBlock(int blockNum, char* data);
    char* readBlock(int blockNum);
    std::string serializeRecord(const Record& r);
    Record deserializeRecord(const std::string& data);
    void writeRecord(int id, std::string name, double salary);
    void allocateBlock(int blockNum);
    void deallocateBlock(int blockNum);
};


#endif //SC3020_STORAGE_H

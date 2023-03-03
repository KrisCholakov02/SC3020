#ifndef SC3020_STORAGE_H
#define SC3020_STORAGE_H

#include <iostream>
#include <cstring>
#include "storage_components.h"

using namespace std;

// A class for creating, allocating and modifying the storage
class Storage {
private:
    // Maximum storage size
    size_t maxSize;
    // The fixed block size
    size_t blockSize;
    // Current used storage size based on the number of records
    size_t actualSize;
    // Current used storage size based on the number of blocks
    size_t currentSize;
    // The used storage size of the current block
    size_t currentBlockSize;

    // The number of blocks allocated
    int blocksAllocated;
    // The number of blocks accessed
    int blocksAccessed;

    // Pointer to the storage
    void *storage;
    // Pointer to the current block
    void *block;
public:
    // Constructor for the Storage class
    Storage(size_t maxSize, size_t blockSize);

    // Allocate a block from the storage
    bool allocateBlock();

    // Allocate record within the block
    Address allocateRecord(size_t recordSize);

    // Deallocate a record from a block
    bool deallocateRecord(Address recordAddress, size_t recordSize);

    // Load the record from the storage
    void *loadRecordFromStorage(Address recordAddress, size_t recordSize);

    // Save a record to the storage
    Address saveRecordToStorage(void *record, size_t recordSize);

    // Overloading the saveRecordToStorage function
    // Update an already existing record to the storage
    Address saveRecordToStorage(void *record, size_t recordSize, Address recordAddress);

    // Getter for the maximum size of the storage
    size_t getMaxSize() const {
        return maxSize;
    }
    // Getter for the fixed size of the blocks in the storage
    size_t getBlockSize() const {
        return blockSize;
    }

    // Getter for the actual of the storage (based on number of records)
    size_t getActualSize() const {
        return actualSize;
    }

    // Getter for the current size of the storage (based on number of block)
    size_t getCurrentSize() const {
        return currentSize;
    }

    // Getter for the used storage size of the current block
    size_t getCurrentBlockSize() const {
        return currentBlockSize;
    }

    // Getter for the number of allocated blocks
    int getAllocated() const {
        return blocksAllocated;
    }

    // Getter for the number of accessed blocks
    int getBlocksAccessed() const {
        return blocksAccessed;
    }

    // Set the number of accessed blocks to 0
    void resetBlocksAccessed() {
        blocksAccessed = 0;
    }

    // Destructor for the Storage class
    ~Storage();
};

#endif //SC3020_STORAGE_H

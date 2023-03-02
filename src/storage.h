//
// Created by Кристиян Каменов Чолаков on 2.03.23.
//

#ifndef SC3020_STORAGE_H
#define SC3020_STORAGE_H

#include <iostream>
#include "storage_components.h"

using namespace std;

class Storage {
private:
    size_t maxSize;
    size_t blockSize;
    size_t actualSize;
    size_t currentSize;
    size_t currentBlockSize;

    int allocated;
    int blocksAccessed;

    void *storage;
    void *block;
public:
    Storage(size_t maxSize, size_t blockSize);

    bool allocateBlock();

    Address allocate(size_t size);

    bool deallocate(Address address, size_t size);

    void *loadFromDisk(Address address, size_t size);

    Address saveToDisk(void *itemAddress, size_t size);

    Address saveToDisk(void *itemAddress, size_t size, Address diskAddress);

    size_t getMaxSize() const {
        return maxSize;
    }

    size_t getBlockSize() const {
        return blockSize;
    }

    size_t getActualSize() const {
        return actualSize;
    }

    size_t getCurrentSize() const {
        return currentSize;
    }

    size_t getCurrentBlockSize() const {
        return currentBlockSize;
    }

    int getAllocated() const {
        return allocated;
    }

    int getBlocksAccessed() const {
        return blocksAccessed;
    }

    int resetBlocksAccessed();

    ~Storage();
};


#endif //SC3020_STORAGE_H

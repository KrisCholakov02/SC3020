#include "storage.h"

using namespace std;

Storage::Storage(size_t maxSize, size_t blockSize) : maxSize(maxSize), blockSize(blockSize) {
    this->maxSize = maxSize;
    this->blockSize = blockSize;
    this->actualSize = 0;
    this->currentSize = 0;
    this->blocksAllocated = 0;

    this->storage = operator new(maxSize);
    memset(storage, '\0', maxSize);
    this->block = nullptr;
    this->currentBlockSize = 0;
    this->blocksAccessed = 0;
}


bool Storage::allocateBlock() {
    // In case that the storage is almost full and cannot accept any more new blocks
    if (currentSize + blockSize > maxSize) {
        cout << "The storage is almost full, no space for a new block." << endl;
        return false;
    }
    // The storage has a space for at least one more block, so a new block is allocated
    currentSize += blockSize;
    block = (char *) storage + blocksAllocated * blockSize;
    blocksAllocated += 1;
    currentBlockSize = 0;
    return true;
}

Address Storage::allocateRecord(size_t recordSize) {
    // If the record recordSize is larger than the fixed recordSize of the block, throw an exception
    if (recordSize > blockSize) {
        cout << "The record recordSize is larger than the fixed block recordSize." << endl;
        throw invalid_argument("The record's recordSize should be less than the block's recordSize.");
    }
    // If there are no blocks allocated, try to allocate a new one
    if (blocksAllocated == 0 || (currentBlockSize + recordSize > blockSize)) {
        // If a new block cannot be created because of storage not having enough space for it, throw an exception
        if (!allocateBlock()) throw logic_error("A new block cannot be created.");
    }
    short int offset = currentBlockSize;

    // If the current block has enough space for the record or a block is successfully created, allocate record to it
    Address newRecordAddress {block, offset};
    currentBlockSize += recordSize;
    currentSize += recordSize;
    return newRecordAddress;
}

bool Storage::deallocateRecord(Address recordAddress, size_t recordSize) {
    try {
        // Deallocating the record from the block
        void *toDelete = (char *) recordAddress.blockAddress + recordAddress.offset;
        memset(toDelete, '\0', recordSize);
        // Decreasing the storage size with the size of the deleted record
        actualSize -= recordSize;
        // Creating an empty block to compare it with the current one later
        unsigned char emptyBlock[blockSize];
        memset(emptyBlock, '\0', blockSize);
        // Comparing if the current block is empty
        if (memcmp(emptyBlock, recordAddress.blockAddress, blockSize) == 0) {
            // Decreasing the used storage size and the number of allocated blocks if the block is empty
            currentSize -= blockSize;
            blocksAllocated--;
        }
        // If current block is not empty, we just proceed without decreasing the storage used, as the block stays
        return true;
    } catch (...) {
        // Catch if an exception happens and return false
        cout << "Cannot deallocate the record." << endl;
        return false;
    }
}

void *Storage::loadRecordFromStorage(Address recordAddress, size_t recordSize) {
    // Get a record from a block based on the block's address, offset and the record's size
    void *address = operator new(recordSize);
    memcpy(address, (char *) recordAddress.blockAddress + recordAddress.offset, recordSize);
    // Increase the number of blocks that are accessed
    blocksAccessed++;
    // Return the address with the record that was copied
    return address;
}

Address Storage::saveRecordToStorage(void *record, size_t recordSize) {
    // Creating the address for the record to be stored and copying it there
    Address address = allocateRecord(recordSize);
    memcpy((char *) address.blockAddress + address.offset, record, recordSize);
    // Increase the number of blocks that are accessed
    blocksAccessed++;
    // Return the address with the record that was copied
    return address;
}

Address Storage::saveRecordToStorage(void *record, size_t recordSize, Address recordAddress) {
    // Calculating the address for the record to be stored and copying it there
    memcpy((char *) recordAddress.blockAddress + recordAddress.offset, record, recordSize);
    // Increase the number of blocks that are accessed
    blocksAccessed++;
    // Return the address with the record that was copied
    return recordAddress;
}

Storage::~Storage() = default;;

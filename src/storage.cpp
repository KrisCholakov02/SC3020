//
// Created by Кристиян Каменов Чолаков on 2.03.23.
//

#include "storage.h"

using namespace std;

Storage::Storage(size_t maxSize, size_t blockSize) : maxSize(maxSize), blockSize(blockSize) {
    this->maxSize = maxSize;
    this->blockSize = blockSize;
    this->actualSize = 0;
    this->currentSize = 0;
    this->allocated = 0;

    this->storage = operator new(maxSize);
    memset(storage, '\0', maxSize);
    this->block = nullptr;
    this->currentBlockSize = 0;
    this->blocksAccessed = 0;
}

Address Storage::allocate(size_t size)
{
    // If record size exceeds block size, throw an error.
    if (size > blockSize)
    {
        std::cout << "Error: Size required larger than block size (" << size << " vs " << blockSize << ")! Increase block size to store data." << '\n';
        throw std::invalid_argument("Requested size too large!");
    }

    // If no current block, or record can't fit into current block, make a new block.
    if (allocated == 0 || (currentBlockSize + size > blockSize))
    {
        bool isSuccessful = allocateBlock();
        if (!isSuccessful)
        {
            throw std::logic_error("Failed to allocate new block!");
        }
    }

    // Update variables
    short int offset = currentBlockSize;

    currentBlockSize += size;
    actualSize += size;

    // Return the new memory space to put in the record.
    Address recordAddress = {block, offset};

    return recordAddress;
}

bool Storage::deallocate(Address address, size_t size)
{
    try
    {
        // Remove record from block.
        void *addressToDelete = (char *)address.getBlockAddress() + address.getOffset();
        std::memset(addressToDelete, '\0', size);

        // Update actual size used.
        actualSize -= size;

        // If block is empty, just remove the size of the block (but don't deallocate block!).
        // Create a new test block full of NULL to test against the actual block to see if it's empty.
        unsigned char testBlock[blockSize];
        memset(testBlock, '\0', blockSize);

        // Block is empty, remove size of block.
        if (memcmp(testBlock, address.getBlockAddress(), blockSize) == 0)
        {
            currentSize -= blockSize;
            allocated--;
        }

        return true;
    }
    catch (...)
    {
        std::cout << "Error: Could not remove record/block at given address (" << address.getBlockAddress() << ") and offset (" << address.getOffset() << ")." << '\n';
        return false;
    };
}

// Give a block address, offset and size, returns the data there.
void *Storage::loadFromDisk(Address address, size_t size)
{
    void *mainMemoryAddress = operator new(size);
    std::memcpy(mainMemoryAddress, (char *)address.getBlockAddress() + address.getOffset(), size);

    // Update blocks accessed
    blocksAccessed++;

    return mainMemoryAddress;
}

// Saves something into the disk. Returns disk address.
Address Storage::saveToDisk(void *itemAddress, size_t size)
{
    Address diskAddress = allocate(size);
    std::memcpy((char *)diskAddress.getBlockAddress() + diskAddress.getOffset(), itemAddress, size);

    // Update blocks accessed
    blocksAccessed++;

    return diskAddress;
}

// Update data in disk if I have already saved it before.
Address Storage::saveToDisk(void *itemAddress, std::size_t size, Address diskAddress)
{
    std::memcpy((char *)diskAddress.getBlockAddress() + diskAddress.getOffset(), itemAddress, size);

    // Update blocks accessed
    blocksAccessed++;

    return diskAddress;
}

Storage::~Storage(){};

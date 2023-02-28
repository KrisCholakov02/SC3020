//
// Created by Public on 26/02/2023.
//
using namespace std;
#include "storage_component.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <tuple>

//Constructor for StorageComponent class
StorageComponent::StorageComponent(size_t maxStrSize, size_t blockSize) {
    this->maxStorageSize = maxStrSize;
    this->blkSize = blockSize;
    this->usedSizeInBLk = 0;
    this->usedSizeInRec = 0;
    //the number of blocks that have been allocated so far.
    this->numAllocated = 0;

    //Allocate space for memory block
    this->sptr = operator new(maxStrSize);

    //Initialize memory block to all 0s
    memset(sptr, '\0', maxStrSize);

    //Set block pointer and block offset variable to default values
    this->blk = nullptr;
    //Representing the amount of used space in the current block.
    this->usedBlkSize = 0;

    //Set number of blocks accessed to 0
    this->numBlksAccessed = 0;
}

//Function to allocate new block of memory
bool StorageComponent::allocateBlock() {

    //Check if there is enough space to allocate a new block
    if (usedSizeInBLk + blkSize <= maxStorageSize)  {
        // Update respective variables
        usedSizeInBLk = usedSizeInBLk + blkSize;

        // Set current block pointer to new block.
        blk = (char *)sptr + numAllocated * blkSize;

        // Reset offset to 0.
        usedBlkSize = 0;

        //increment the number of allocated blocks
        numAllocated = numAllocated + 1;

        //return true to indicate successful block allocation
        return true;
    }
    else
    {
        //Print error message and return false to indicate failed block allocation
        std::cout << "Error: Maximum storage size reached - unable to allocate a new block for database(" << usedSizeInBLk << "/" << maxStorageSize << " used)." << '\n';
        return false;
    }
}

//Function to deallocate a block of memory
bool StorageComponent::deallocateBlock(Address addr, size_t dSize) {
    //try-catch block to catch possible exceptions while deallocating memory
    try
    {
        // Remove record from block.
        // Calculate the address of the block to be deleted
        void *deleteAddress = (char *)addr.blkAddr + addr.offset;
        // set the deleteAddress pointer to all null characters ('\0') in order to "delete" the contents of that block of memory.
        std::memset(deleteAddress, '\0', dSize);

        // Update the size used for record.
        usedSizeInRec -= dSize;

        // If a block is empty, just remove the size of the block (but don't deallocate block!).
        // Create a new test block and set all its values to null
        // We need to test against the actual block using testBlock to see if it's empty.
        unsigned char testBlock[blkSize];
        memset(testBlock, '\0', blkSize);

        //check if the block being deleted is empty
        // If the block turns out to be empty, subtract size of block.
        if (memcmp(testBlock, addr.blkAddr, blkSize) == 0)
        {
            usedSizeInBLk -= blkSize; //update used block size
            numAllocated--; //decrement the number of allocated blocks
        }

        return true;
    }
    catch (...)
    {
        //If there was an error, output an error message and return false
        std::cout << "Error: Unable to remove record/block at provided address (" << addr.blkAddr << addr.offset <<")." << '\n';
        return false;
    };
}

// Allocate space in a block
// return the address where the new record should be stored
Address StorageComponent::allocateRecordInBlock(size_t size) {

    //check is requested size is greater than block size
    if (size > blkSize)
    {
        // If the requested size is larger than the block size, throw an exception
        std::cout << "Error: Requested size is greater than block size (" << size << " vs " << blkSize << ")! Increase block size to store data." << '\n';
        throw std::invalid_argument("Requested size too large!"); //Throw an exception
    }

    // check If no blocks have been allocated yet or if the remaining space in the current block is insufficient
    // If there is no current allocated block or if the record cannot fit into current block, we make a new block.
    if (numAllocated == 0 || (usedBlkSize + size > blkSize))
    {
        //Attempt to allocate a new block
        bool isSuccessful = allocateBlock();
        if (!isSuccessful)
        {
            //if the allocation fails, throw an exception
            throw std::logic_error("Failed to allocate new block!");
        }
    }

    // Update variables

    // Calculate the offset where the new record should be stored
    // the offset in bytes from the beginning of the block where the data will be stored.
    short int offset = usedBlkSize;

    // Update the used block size and used size in record
    usedBlkSize += size;
    usedSizeInRec += size;

    // Return the address where the new record should be stored
    Address recordAddr = {blk, offset};

    // the address (block and offset) where the data will be stored.
    return recordAddr;
}

// Retrieve an item of a given size (variable field lengths) from a specific address in disk
void *StorageComponent::getFromDisk(Address addr, size_t size) {

    // Allocate memory in main memory to store the retrieved item
    void *mainMemAddr = operator new(size);
    // Copy the item from disk to main memory
    memcpy(mainMemAddr, (char*) addr.blkAddr + addr.offset, size);

    // Increment the number of blocks accessed
    numBlksAccessed++;
    // Return the address where the item was stored in main memory
    return mainMemAddr;
}

// Save an item of a given size (variable field length) to disk and return the address where it was stored
Address StorageComponent::SaveInDisk(void *itemAddr, size_t size) {
    // Allocate space in disk for the new item
    Address diskAddr = allocateRecordInBlock(size);
    // Copy the item from main memory to disk
    memcpy((char*)diskAddr.blkAddr+diskAddr.offset, itemAddr,size);
    // Increment the number of blocks accessed
    numBlksAccessed++;
    // Return the address where the item was stored in disk
    return diskAddr;
}

// Update an item of a given size (variable field length) at a specific address in disk
Address StorageComponent::UpdateToDisk(void *itemAddr, size_t size, Address diskAddr) {
    // Copy the updated item from main memory to disk
    memcpy((char*)diskAddr.blkAddr+diskAddr.offset, itemAddr,size);
    // Increment the number of blocks accessed
    numBlksAccessed++;
    // Return the address where the item was updated in disk
    return diskAddr;

}

//Deconstructor - release storage space
StorageComponent::~StorageComponent() {};


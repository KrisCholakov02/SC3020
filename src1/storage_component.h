//
// Created by Public on 26/02/2023.
//

#ifndef SC3020_STORAGE_COMPONENT_H
#define SC3020_STORAGE_COMPONENT_H

using namespace std;
#include <vector>
#include <unordered_map>
#include <tuple>

struct Address
{
    // A pointer to the start of a block in memory
    void *blkAddr;
    // A short integer representing the offset within a block.
    short int offset;
};

// Defines a single movie record (read from data file).
struct Record
{
    char tconst[10];     // ID of the movie.
    float averageRating; // Average rating of this movie.
    int numVotes;        // Number of votes of this movie.
};


class StorageComponent
{
public:
    StorageComponent(size_t maxStrSize,size_t blockSize);

    bool allocateBlock();

    bool deallocateBlock(Address addr, size_t dSize);

    Address allocateRecordInBlock(size_t size);

    void *getFromDisk(Address addr, size_t size);

    Address SaveInDisk(void *itemAddr, size_t size);

    Address UpdateToDisk(void *itemAddr, size_t size, Address diskAddr);

    ~StorageComponent();

    size_t getMaxStorage(){
        return maxStorageSize;
    };

    size_t getBlkSize(){
        return blkSize;
    };

    size_t getUsedBlkSize(){
        return usedBlkSize;
    };

    size_t getUsedSizeInBlk(){
        return usedSizeInBLk;
    };

    size_t getUsedSizeInRec(){
        return getUsedSizeInRec();
    };

    int getNumAllocated(){
        return numAllocated;
    };

    int getNumBlkAccessed(){
        return numBlksAccessed;
    };

    int resetBlksAccessed(){
        int tempBlkAccessed = numBlksAccessed;
        numBlksAccessed = 0;
        return tempBlkAccessed;
    };

private:

    size_t maxStorageSize;
    size_t blkSize;
    size_t usedSizeInBLk;
    size_t usedSizeInRec;
    size_t usedBlkSize;

    int numAllocated;
    int numBlksAccessed;

    void *sptr;
    void *blk;


};
#endif //SC3020_STORAGE_COMPONENT_H

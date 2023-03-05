#ifndef SC3020_STORAGE_COMPONENTS_H
#define SC3020_STORAGE_COMPONENTS_H

#include <iostream>
#include <cstring>

using namespace std;

// Class for the addresses in the storage
class Address {
private:
    // The offset of the record in the block
    short int offset;
public:
    // The constructor of the Address class
    Address(void *blockAddress, short offset) : blockAddress(blockAddress), offset(offset) {}

    // Getter for the block address
    void *getBlockAddress() const {
        return blockAddress;
    }

    // Getter for the offset of the record within the block
    short getOffset() const {
        return offset;
    }

// The address of the block the record is located at
    void *blockAddress;
};

class Record {
private:
    // The tconst field of the records
    char t[10];
    // The rating of the record's movie
    float rating;
    // The number of votes of the record's movie
    int numVotes;
public:
    // Constructor for the Record class
    Record(char *t, float rating, int numVotes) : rating(rating), numVotes(numVotes) {
        strncpy(this->t, t, 10);
    }

    // Getter for the tconst
    const char *getT() const {
        return t;
    }

    // Getter for the rating
    float getRating() const {
        return rating;
    }

    // Getter for the number of votes
    int getNumVotes() const {
        return numVotes;
    }
};

#endif //SC3020_STORAGE_COMPONENTS_H

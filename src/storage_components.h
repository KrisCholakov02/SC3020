#ifndef SC3020_STORAGE_COMPONENTS_H
#define SC3020_STORAGE_COMPONENTS_H

#include <iostream>
#include <cstring>

using namespace std;

// Class for the addresses in the storage
struct Address {
    // The address of the block the record is located at
    void *blockAddress;
    // The offset of the record in the block
    short int offset;

};

struct Record {
    // The tconst field of the records
    char t[10];
    // The rating of the record's movie
    float rating;
    // The number of votes of the record's movie
    int numVotes;

};

#endif //SC3020_STORAGE_COMPONENTS_H

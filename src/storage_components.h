#ifndef SC3020_STORAGE_COMPONENTS_H
#define SC3020_STORAGE_COMPONENTS_H

#include <iostream>
#include <cstring>

// Defines an address of a record stored as a block address with an offset.
struct Address
{
    void *blockAddress;
    short int offset;
};

// Defines a single movie record (read from data file).
struct Record
{
    char tconst[11];     // ID of the movie.
    float averageRating; // Average rating of this movie.
    int numVotes;        // Number of votes of this movie.
};

#endif //SC3020_STORAGE_COMPONENTS_H

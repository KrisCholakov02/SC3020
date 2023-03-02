//
// Created by Кристиян Каменов Чолаков on 2.03.23.
//

#ifndef SC3020_STORAGE_COMPONENTS_H
#define SC3020_STORAGE_COMPONENTS_H

// Class for
class Address {
private:
    void *blockAddress;
    short int offset;
public:
    Address(void *blockAddress, short offset) : blockAddress(blockAddress), offset(offset) {}

    void *getBlockAddress() const {
        return blockAddress;
    }

    void setBlockAddress(void *blockAddress) {
        Address::blockAddress = blockAddress;
    }

    short getOffset() const {
        return offset;
    }

    void setOffset(short offset) {
        Address::offset = offset;
    }
};

class Record {
private:
    char t[10];
    float rating;
    int numVotes;
public:
    Record(char *t, float rating, int numVotes) : t(t), rating(rating), numVotes(numVotes) {}


};

#endif //SC3020_STORAGE_COMPONENTS_H

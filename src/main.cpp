#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "storage.h"
#include "b_plus_tree.h"

using namespace std;

// A function to print a line with a certain length to the terminal.
void printLine(int displaySize) {
    for (int i = 0; i < displaySize; i++) {
        cout << "-";
    }
    cout << endl;
}

// A function to print a text as a title for nicer displaying features
void printTitle(int displaySize, string text) {
    // Getting half of the remaining length of the display after the text and printing spaces to align
    int n = (displaySize - text.length()) / 2;
    for (int i = 0; i < n; i++) {
        cout << " ";
    }
    // Printing the text and ending the line
    cout << text << endl;
}

// A function to show the current of the data loading dynamically in one line
void displayProgress(int progress, int total) {
    float percentage = static_cast<float>(progress) / total * 100;
    cout << "\r" << setw(3) << static_cast<int>(percentage) << "% [";
    int progressLength = static_cast<int>(percentage) / 2;
    for (int i = 0; i < 50; i++) {
        if (i < progressLength) {
            cout << "=";
        } else if (i == progressLength) {
            cout << ">";
        } else {
            cout << " ";
        }
    }
    cout << "]";
    cout.flush();
}

int main() {
    //Constants:

    // Using this constant to set the length of the displayed lines in the terminal.
    int const DISPLAY_SIZE = 60;

    // A constant for the number of lines in the data.tsv file
    int const NUM_RECORDS = 1070318;
    // A constant for the size of one block - 200 B (as said in the requirements).
    int const BLOCK_SIZE = 200;
    // A constant for the size of the storage part holding the records of the database - 150 MB.
    int const RECORDS_SIZE = 150;
    // A constant for the size of the storage part holding the indexes for the records - 350 MB.
    int const INDEXES_SIZE = 350;

    // Create storages for the records and the indexes
    Storage records = Storage(RECORDS_SIZE * pow(2, 20), BLOCK_SIZE);
    Storage indexes = Storage(INDEXES_SIZE * pow(2, 20), BLOCK_SIZE);

    // Creating the B+ Tree
    BPlusTree tree = BPlusTree(&indexes, &records, BLOCK_SIZE);

    // Getting the file with the records
    ifstream file("../data/data.tsv");

    // Setting a counter for the number of records
    int numRecords = 0;
    // Opening the file to allocate the records to the storage
    if (file.is_open()) {
        // A string to hold every line of the data
        string recordLine;
        cout << "Loading the data" << endl;

        // Get the lines of the data.tsv file
        while (getline(file, recordLine)) {
            Record temporary;

            // Converting the record line to a string stream, so the fields can be assigned
            stringstream recordStream(recordLine);

            // Assigning the tconst field
            strcpy(temporary.t, recordLine.substr(0, recordLine.find("\t")).c_str());
            // A check for the first line of the data.tsv, get only records without the header

            if (strcmp(temporary.t, "tconst") == 0) {
                continue;
            }
            string data;
            getline(recordStream, data, '\t');

            //assigning rating and numVotes fields
            recordStream >> temporary.rating >> temporary.numVotes;

            // Saving it to the storage and increasing the counter of saved records in the storage
            Address currentRecordAddress = records.saveRecordToStorage(&temporary, sizeof(Record));

            // Inserting the keys into the B+ tree
            tree.insert(currentRecordAddress, temporary.numVotes);
            cout << numRecords <<  endl;
            tree.display(tree.getRoot(), tree.getNumLevels());

            numRecords += 1;
            //displayProgress(numRecords, NUM_RECORDS);
        }
        cout << endl;
        file.close();
    }

    // Experiment 1:
    printLine(DISPLAY_SIZE);
    printTitle(DISPLAY_SIZE, "Experiment 1");
    printLine(DISPLAY_SIZE);
    cout << "Number of records: " << numRecords << endl;
    cout << "Size of a record: " << sizeof(Record) << " B" << endl;
    cout << "(Max whole) Records per block: " << (int) BLOCK_SIZE / sizeof(Record) << endl;
    cout << "Number of blocks: " << records.getBlocksAllocated() << endl;

    indexes.resetBlocksAccessed();
    records.resetBlocksAccessed();

    // Experiment 2:
    printLine(DISPLAY_SIZE);
    printTitle(DISPLAY_SIZE, "Experiment 2");
    printLine(DISPLAY_SIZE);
    cout << "Parameter n of the B+ tree: " << tree.getMaxNumKeys() << endl;
    cout << "Number of nodes of the B+ tree: " << tree.getNumNodes() << endl;
    cout << "Height of the B+ tree: " << tree.getNumLevels() << endl;
    cout << "Root nodes and child nodes:" << endl;
    for (int i = 0; i < tree.getRoot()->getNumKeys(); i++) {
        cout << tree.getRoot()->getKeys()[i] << ", ";
    }
    std::cout << endl;

    indexes.resetBlocksAccessed();
    records.resetBlocksAccessed();

    // Experiment 3:

    return 0;
}

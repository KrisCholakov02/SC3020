#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include "storage.h"

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

int main() {
    //Constants:

    // Using this constant to set the length of the displayed lines in the terminal.
    int const DISPLAY_SIZE = 60;

    // A constant for the size of one block - 200 B (as said in the requirements).
    int const BLOCK_SIZE = 200;
    // A constant for the size of the storage part holding the records of the database - 150 MB.
    int const RECORDS_SIZE = 150;
    // A constant for the size of the storage part holding the indexes for the records - 350 MB.
    int const INDEXES_SIZE = 350;

    // Experiment 1:

    // Create storages for the records and the indexes
    Storage indexes = Storage(150 * pow(2, 20), BLOCK_SIZE);
    Storage records = Storage(150 * pow(2, 20), BLOCK_SIZE);

    // Getting the file with the records
    ifstream file("../data/data.tsv");

    int numRecords = 0;
    if (file.is_open()) {
        string recordLine;

        while (getline(file, recordLine)) {
            char t[10];
            float rating;
            int numVotes;

            stringstream recordStream(recordLine);

            strcpy(t, recordLine.substr(0, recordLine.find("\t")).c_str());
            if (strcmp(t, "tconst") == 0) {
                continue;
            }
            string data;
            getline(recordStream, data, '\t');

            //assigning temp.averageRating & temp.numVotes values
            recordStream >> rating >> numVotes;

            Record current = Record(t, rating, numVotes);
            cout << t << " " << rating << " " << numVotes << "-->" << numRecords + 1 << endl;

            Address currentRecordAddress = records.saveRecordToStorage(&current, sizeof(Record));
            numRecords += 1;
        }
        file.close();
    }

    // Printing the results from Experiment 1
    printLine(DISPLAY_SIZE);
    printTitle(DISPLAY_SIZE, "Experiment 1");
    printLine(DISPLAY_SIZE);
    cout << "Number of records: " << numRecords << endl;
    cout << "Size of a record: " << sizeof(Record) << " B" << endl;
    cout << "(Max whole) Records per block: " << (int) BLOCK_SIZE / sizeof(Record) << endl;
    cout << "Number of blocks: " << records.getBlocksAllocated() << endl;


    return 0;
}

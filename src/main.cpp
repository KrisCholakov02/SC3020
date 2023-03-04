#include <iostream>

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

    //Experiment 1:

    printLine(DISPLAY_SIZE);
    printTitle(DISPLAY_SIZE, "Experiment 1");




    return 0;
}

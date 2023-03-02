#include <iostream>

using namespace std;

void printHeader(int displaySize) {
    for (int i = 0; i < displaySize; i++) {
        cout << "-";
    }
    cout << endl;
}

int const DISPLAY_SIZE = 60;
int const BLOCK_SIZE = 200;

int main() {
    printHeader(DISPLAY_SIZE);
    cout << "Select the Storage Size (int MBs):     " << endl;
    int storageSize = 0;
    try {
        cin >> storageSize;
        if (storageSize > 500 || storageSize < 100) {
            cout << "Enter an integer between 100 and 500." << endl;
            return 0;
        }

    } catch (...) {
        cout << "Enter a valid integer!" << endl;
    }


    return 0;
}

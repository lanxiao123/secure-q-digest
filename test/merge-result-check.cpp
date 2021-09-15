#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>

using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }
    //read files to compare
    ifstream ifs_result1(argv[1]);
    ifstream ifs_result2(argv[2]); 
    if (ifs_result1 && ifs_result2) {
        string line_result1;
        string line_result2;
        while (getline(ifs_result1, line_result1)) {
            if (line_result1.compare("") != 0) {
                getline(ifs_result2, line_result2);
                if (line_result1.compare(line_result2) != 0) {
                    cout << 0 << endl;
                    return 0;
                }
            }

        }
    }
    cout << 1 << endl;
    return 0;
}

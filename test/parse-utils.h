//
// Created on 2020/11/11.
//
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
#ifndef Q_DIGEST_MPC_UTILS_H
#define Q_DIGEST_MPC_UTILS_H

#endif //Q_DIGEST_MPC_UTILS_H

bool parse_parameter(int64_t *n, int64_t *len, int64_t *k, int64_t *d, string path) {
    ifstream ifs(path);
    if (ifs) {
        string line;
        getline(ifs, line);
        //the first line
        int index = line.find(' ');
        string str_n(line, 0, index);

        line = line.substr(index + 1);
        index = line.find(' ');
        string str_len(line, 0, index);

        line = line.substr(index + 1);
        index = line.find(' ');
        string str_k(line, 0, index);

        string str_d = line.substr(index + 1);

        *n = (int64_t) stoll(str_n);
        *len = (int64_t) stoll(str_len);
        *k = (int64_t) stoll(str_k);
        *d = (int64_t) stoll(str_d);

        return true;
    }
    return false;
}

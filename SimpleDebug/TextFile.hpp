#ifndef TEXTFILE_HPP
#define TEXTFILE_HPP

#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class TextFile {
private:
    ifstream is;
    // bool isValid = false;
public:
    TextFile(string file) {
        is.open(file);
    }
    ~TextFile() {
        is.close();
    }
    bool exists() {
        return is.good();
    }
    void findLine(int line) {
        is.seekg(0, is.beg);
        string dummy;
        for(int i = 0; i < line-1; i++)
            getline(is, dummy);
    }
    string getLine(int line = 0) {
        string result;
        if(line)
            findLine(line);
        getline(is, result);
        return result;
    }
    bool eof() {
        return is.eof();
    }
    bool good() {
        return is.good();
    }
};
#endif
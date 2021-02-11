#ifndef IntList__h
#define IntList__h

// determine how much memory has been used
#include <vector>
#include <iostream>
#include <fstream>
#include "TObject.h" // not used, but present for some header 

using namespace std;

struct IntList : public TObject {
    vector<int> list;
    bool operator()(int); // check to see number in list
    bool has(int);
    bool has_not(int);
    IntList(const char* in_file, ofstream& log, bool print=true);
    IntList(const char* in_file, bool print=true);
    ClassDef (IntList,1);
    private:
    string make(const char* in_file, bool print);
};
#endif

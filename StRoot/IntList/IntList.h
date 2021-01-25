#ifndef IntList__h
#define IntList__h

#include <vector>
#include <iostream>
#include <fstream>
#include "TObject.h" 

using namespace std;

struct IntList : public TObject {
    vector<int> list;
    bool operator()(int); // check if argument is in the list
    bool has(int);
    bool has_not(int);
    IntList(const char* in_file, ofstream& log, bool print=true);
    IntList(const char* in_file, bool print=true);
    ClassDef (IntList,1);
    private:
    string make(const char* in_file, bool print);
};
#endif

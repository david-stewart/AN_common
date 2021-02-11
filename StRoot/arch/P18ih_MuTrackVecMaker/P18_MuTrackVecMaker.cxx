#include "IntList.h"
#include <sstream>
#include <algorithm>
#include "TString.h" // not used, but present for some header 

ClassImp(IntList)

string IntList::make(const char* in_file, bool print) {
    ostringstream msg;
    ifstream file;
    file.open(in_file);
    if (!file.is_open()) {
        msg << "Could not open int list \"" << in_file << "\". No entries entered." << endl;
        cout << msg.str() << endl;
        return msg.str();
    }
    string line;
    while (getline(file,line)) {
        line.append(" ");
        stringstream words(line);
        TString word;
        while (words >> word) {
            if (word.BeginsWith("//") || word.BeginsWith("#")) break;
            list.push_back(word.Atoi());
        }
    }
    sort(list.begin(),list.end());
    file.close();
    msg << " Successfully read in list from \"" << in_file << "\"";
    if (print) msg << ". Values: ";
    msg << endl;
    if (print) {
        for (auto& i : list) msg << "  " << i << endl;
    }
    cout << msg.str();
    return msg.str();
};

IntList::IntList(const char* in_file, ofstream& log, bool print) {
    log << make(in_file, print);
};
IntList::IntList(const char* in_file, bool print) { make(in_file, print); };

bool IntList::operator()(int val) {
    return std::binary_search(list.begin(), list.end(), val);
};
bool IntList::has(int val) { return this->operator()(val); };
bool IntList::has_not(int val) { return !(this->operator()(val)); };


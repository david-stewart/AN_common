#ifndef TrackTowerMatcher__h
#define TrackTowerMatcher__h

/*
 * Simple class which recieves input tracks and which tower they are matched to,
 * and then, on a tower-by-tower bases, tells the sum of track Et matched to that tower.
 */

// determine how much memory has been used

#define PI_MASS2 0.01947978

#include "stdlib.h"
#include "stdio.h"
#include <vector>
#include "TObject.h"
#include "TMath.h"

using namespace std;

struct IntDouble {
    int i;
    double d;
    IntDouble(int, double);
    IntDouble();
};

class TrackTowerMatcher : public TObject {
    public:
    TrackTowerMatcher();
    ~TrackTowerMatcher();

    void clear(); 

    void   add_hadronE_p2 (double p2, int i_tower, double m2=PI_MASS2);
    void   add_towerEt    (int i_tower, double Et_uncorr);

    double get_hadronE  (int i_tower);
    double get_towerEt  (int i_tower);


    private:
    vector<IntDouble> hadronE;//  for pair<i_tower, E from tracks>
    vector<IntDouble> towerEt;     //  for pair<i_tower, Et uncorrected> for tracks

    bool is_sorted_hadronE; // set to false with clear; 
    bool is_sorted_towerEt;
    
    ClassDef (TrackTowerMatcher, 1);
};
#endif

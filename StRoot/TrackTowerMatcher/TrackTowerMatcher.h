#ifndef TrackTowerMatcher__h
#define TrackTowerMatcher__h

/*
 * Simple class which recieves input tracks and which tower they are matched to,
 * and then, on a tower-by-tower bases, tells the sum of track Et matched to that tower.
 */

// determine how much memory has been used

#include "stdlib.h"
#include "stdio.h"
#include <vector>
#include "TObject.h"
#include "TMath.h"

using namespace std;

struct int_double {
    int i;
    double d;
    int_double(int, double);
    int_double();
};

struct TrackTowerMatcher : public TObject {
    TrackTowerMatcher();
    ~TrackTowerMatcher();
    vector<int_double> data; // for pair<i_tower, E>
    void clear(); 
    double sum_matched_E(int i_tower);
    void add_p2_itower(double p2, int i_tower, double m2=0.0182196); 

    bool is_sorted; // set to false with clear; will self-sort with first sum_E call.
    /* bool comp(const int i, const pair<int,double>); */

    ClassDef (TrackTowerMatcher, 1);
};
#endif

#include "TrackTowerMatcher.h"

#include <algorithm>

ClassImp(TrackTowerMatcher)
TrackTowerMatcher::TrackTowerMatcher() : 
    is_sorted_hadronE{false},
    is_sorted_towerEt{false} 
{};
TrackTowerMatcher::~TrackTowerMatcher() {};

IntDouble::IntDouble(int _i, double _d) :
    i{_i}, d{_d} 
{};
IntDouble::IntDouble() : i{0}, d{0.} {};

bool operator < (const IntDouble& l, const IntDouble& r ) { return l.i < r.i; };
bool operator < (const IntDouble& l, const int   i ) { return l.i < i; };
bool operator < (const int   i, const IntDouble& l ) { return i   < l.i; };
bool operator ==(const IntDouble& l, const IntDouble& r ) { return l.i ==r.i; };
bool operator ==(const IntDouble& l, const int   i ) { return l.i ==i; };
bool operator ==(const int   i, const IntDouble& l ) { return i   ==l.i; };
bool operator !=(const IntDouble& l, const int   i ) { return l.i !=i; };
bool operator !=(const int   i, const IntDouble& l ) { return i   !=l.i; };

void TrackTowerMatcher::clear() {
    hadronE.clear();
    towerEt.clear();
    is_sorted_hadronE = false;
    is_sorted_towerEt = false;
};

void TrackTowerMatcher::add_hadronE_p2(double p2, int i_tower, double m2) {
    hadronE.push_back({i_tower,  TMath::Sqrt(p2+m2)});
};
void TrackTowerMatcher::add_towerEt(int i_tower, double Et) {
    towerEt.push_back({i_tower, Et});
};

double TrackTowerMatcher::get_hadronE(int i_tower) {
    // return sum E of all the tracks matching i_tower
    if (!is_sorted_hadronE) {
        std::sort(hadronE.begin(), hadronE.end());
        is_sorted_hadronE = true;
    }
    double E{0.};
    if (binary_search( hadronE.begin(), hadronE.end(), i_tower)) {
        const unsigned int i_match { 
            static_cast<unsigned int>(
              std::lower_bound(hadronE.begin(), hadronE.end(),i_tower)-hadronE.begin()) 
        };
        for (unsigned int i {i_match}; i< hadronE.size(); ++i) {
            if (hadronE[i] != i_tower) break;
            E += hadronE[i].d;
        }
        return E;
    }
    return 0.;
};

double TrackTowerMatcher::get_towerEt(int i_tower) {
    // return sum E of all the tracks matching i_tower
    if (!is_sorted_towerEt) {
        std::sort(towerEt.begin(), towerEt.end());
        is_sorted_towerEt = true;
    }
    auto iter = lower_bound(towerEt.begin(), towerEt.end(), i_tower);
    if (iter == towerEt.end()) return 0.;
    else return iter->d;
};

#include "TrackTowerMatcher.h"

#include <algorithm>

ClassImp(TrackTowerMatcher)
TrackTowerMatcher::TrackTowerMatcher() {};
TrackTowerMatcher::~TrackTowerMatcher() {};

int_double::int_double(int _i, double _d) :
    i{_i}, d{_d} 
{};
int_double::int_double() : i{0}, d{0.} {};

bool operator < (const int_double& l, const int_double& r ) { return l.i < r.i; };
bool operator < (const int_double& l, const int   i ) { return l.i < i; };
bool operator < (const int   i, const int_double& l ) { return i   < l.i; };
bool operator ==(const int_double& l, const int_double& r ) { return l.i ==r.i; };
bool operator ==(const int_double& l, const int   i ) { return l.i ==i; };
bool operator ==(const int   i, const int_double& l ) { return i   ==l.i; };
bool operator !=(const int_double& l, const int   i ) { return l.i !=i; };
bool operator !=(const int   i, const int_double& l ) { return i   !=l.i; };

void TrackTowerMatcher::clear() {
    data.clear();
    is_sorted = false;
};

void TrackTowerMatcher::add_p2_itower(double p2, int i_tower, double m2) {
    data.push_back( {i_tower,  TMath::Sqrt(p2+m2)} );
};

double TrackTowerMatcher::sum_matched_E(int i_tower) {
    // return sum E of all the tracks matching i_tower
    if (!is_sorted) {
        std::sort(data.begin(), data.end());
        is_sorted = true;
    }
    double E{0.};
    if (binary_search( data.begin(), data.end(), i_tower)) {
        const unsigned int i_match { 
            static_cast<unsigned int>(
                    std::lower_bound(data.begin(), data.end(),i_tower)-data.begin()) 
        };
        for (unsigned int i {i_match}; i< data.size(); ++i) {
            if (data[i] != i_tower) break;
            E += data[i].d;
        }
        return E;
    }
    return 0.;
};

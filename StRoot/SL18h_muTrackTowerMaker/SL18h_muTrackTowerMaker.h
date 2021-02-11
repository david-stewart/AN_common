#ifndef SL18h_muTrackTowerMaker__h
#define SL18h_muTrackTowerMaker__h

#include "StMaker.h"
#include "StEmcUtil/geometry/StEmcGeom.h"
#include "TreeObj/TreeObj.h"
#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StEmcUtil/projection/StEmcPosition.h"
#include "TrackTowerMatcher/TrackTowerMatcher.h"
#include "StEmcUtil/database/StBemcTables.h"
#include "St_db_Maker/St_db_Maker.h"
#include "IntList/IntList.h"
#include <iostream>
#include <fstream>
#include <set>
#include <map>

using namespace std;

struct FloatInt {
    float f_val;
    int   i_val;
    FloatInt (float, int);
    bool operator < (const FloatInt& rhs) const; // const { return pT > rhs.pT; };
};


class SL18h_muTrackTowerMaker : public TObject {
    private:
        StMuDstMaker*  muDstMaker;
        IntList&       bad_tower_list;        

        ofstream&      log;
        StEmcGeom*     mBEMCGeom;
        StEmcPosition* mEmcPosition;
        St_db_Maker*   starDb;
        StBemcTables*  mBemcTables;
    public:
        int debug_level;
    private:

        set<int>      id_set;
        map<int,int>  id_to_TrackIndex;
        vector<int>   pt_track_order;
        vector<mupicoTrack> tracks;
        vector<mupicoTower> towers;
        /* vector<FloatInt>  track_pt_id; */

        const int MaxNTracks;
        const int MaxNTowers;

        int iTrack;
        int iTower;

    public:
        virtual ~SL18h_muTrackTowerMaker() {};
        SL18h_muTrackTowerMaker(
                StMuDstMaker*, 
                IntList& bad_tower_list, 
                ofstream& log, 
                St_db_Maker* _starDb,
                int debug_level = 0,
                int _MaxNTracks = 200,
                int _MaxNTowers = 400
        );
        /* TrackTowerMatcher   matched_tower; */
        void clear();
        short index_id(int id);
        void make();

        int nTracks;
        int nTowers;
        mupicoTrack& get_track(int i);
        mupicoTower& get_tower(int i);

        bool next_tower();
        bool next_track();
        mupicoTrack& track();
        mupicoTower& tower();
        /* short index(); */

    ClassDef(SL18h_muTrackTowerMaker,1);
};

#endif

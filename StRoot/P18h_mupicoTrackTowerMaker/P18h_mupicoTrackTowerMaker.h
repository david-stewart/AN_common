#ifndef P18h_vTrackTowerMaker__h
#define P18h_vTrackTowerMaker__h

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

using namespace std;

class P18h_mupicoTrackTowerMaker : public TObject {
    private:
        StMuDstMaker*  muDstMaker;
        IntList&       bad_tower_list;        

        ofstream&      log;
        StEmcGeom*     mBEMCGeom;
        StEmcPosition* mEmcPosition;
        St_db_Maker*  starDb;
        StBemcTables*  mBemcTables;

    public:
        virtual ~P18h_mupicoTrackTowerMaker() {};
        P18h_mupicoTrackTowerMaker(
                StMuDstMaker*, 
                IntList& bad_tower_list, 
                ofstream& log, 
                St_db_Maker* _starDb,
                int debug_level = 0);
        /* P18h_mupicoTrackTowerMaker(); */
        vector<mupicoTrack> makeVec_mupicoTracks(TrackTowerMatcher* matched_tower=nullptr);
        vector<mupicoTower> makeVec_mupicoTowers(TrackTowerMatcher* matched_tower=nullptr);
        int debug_level;
        /* static constexpr double Pi    ; */
        /* static constexpr double TwoPi ; */

    ClassDef(P18h_mupicoTrackTowerMaker,1);
};

#endif

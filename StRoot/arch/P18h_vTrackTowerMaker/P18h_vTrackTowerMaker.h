#ifndef P18h_vTrackTowerMaker__h
#define P18h_vTrackTowerMaker__h

#include "StMaker.h"
#include "StEmcUtil/geometry/StEmcGeom.h"
#include "TreeEvent/TreeEvent.h"
#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StEmcUtil/projection/StEmcPosition.h"
#include "StEmcUtil/database/StBemcTables.h"
#include "St_db_Maker/St_db_Maker.h"
#include "IntList/IntList.h"
#include <iostream>
#include <fstream>

using namespace std;

class P18h_vTrackTowerMaker : public TObject {
    private:
        StMuDstMaker*  muDstMaker;
        IntList&       bad_tower_list;        

        ofstream&      log;
        StEmcGeom*     mBEMCGeom;
        StEmcPosition* mEmcPosition;
        St_db_Maker*  starDb;
        StBemcTables*  mBemcTables;

    public:
        virtual ~P18h_vTrackTowerMaker() {};
        P18h_vTrackTowerMaker(StMuDstMaker*, IntList& bad_tower_list, ofstream& log, St_db_Maker* _starDb);
        /* P18h_vTrackTowerMaker(); */
        vector<myTrack> makeVecMyTracks();
        vector<myTower> makeVecMyTowers();
        bool print_level;
        /* static constexpr double Pi    ; */
        /* static constexpr double TwoPi ; */

    ClassDef(P18h_vTrackTowerMaker,1);
};

#endif

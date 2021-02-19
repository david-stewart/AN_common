#ifndef SL20a_picoTrackTowerMaker__h
#define SL20a_picoTrackTowerMaker__h

#include "StEmcUtil/geometry/StEmcGeom.h"
#include "TreeObj/TreeObj.h"
/* #include "StMuDSTMaker/COMMON/StMuDstMaker.h" */
/* #include "StMuDSTMaker/COMMON/StMuDst.h" */
/* #include "StEmcUtil/projection/StEmcPosition.h" */
#include "StRoot/StPicoEvent/StPicoDst.h"
#include "StRoot/StPicoEvent/StPicoTrack.h"
#include "StRoot/StPicoEvent/StPicoEvent.h"
#include "StRoot/StPicoDstMaker/StPicoDstMaker.h"
#include "StRoot/StPicoEvent/StPicoEmcTrigger.h"
#include "StRoot/StPicoEvent/StPicoBTowHit.h"
#include "StPicoEvent/StPicoBEmcPidTraits.h"

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


class SL20a_picoTrackTowerMaker : public TObject {
    private:
        StPicoDstMaker* picoMaker;
        IntList&        bad_tower_list;
        ofstream&       log;
        // BEAR
        /* StEmcGeom*     mBEMCGeom; */
        /* StEmcPosition* mEmcPosition; */
        /* St_db_Maker*   starDb; */
        /* StBemcTables*  mBemcTables; */
    public:
        int debug_level;
    private:

        // note: these are used with the MuDst, because the 
        // MuDst is used in conjunction with the embedding.
        // These are left in this version because the MuDst->PicoDst
        // is 1:1, and so may be used with a check in the future
        // However they are not actually filled for now, becuase,
        // ultimately, the track id is only used to connect the 
        // embedded tracks to the reconstructed tracks, and I don't
        // have an embedding related to a SL20a picoDst library...
        //
        // See "generate_ids" flag below
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

        bool generate_ids;
        StEmcGeom*      bemcGeom;

    public:
        virtual ~SL20a_picoTrackTowerMaker() {};
        SL20a_picoTrackTowerMaker(
                StPicoDstMaker*, 
                IntList& bad_tower_list, 
                ofstream& log, 
                /* St_db_Maker* _starDb, */
                int debug_level = 0,
                int _MaxNTracks = 200,
                int _MaxNTowers = 400,
                bool generate_ids = false
        );
        void  clear();
        short index_id(int id);
        void  make();

        int nTracks;
        int nTowers;
        mupicoTrack& get_track(int i);
        mupicoTower& get_tower(int i);

        bool next_tower();
        bool next_track();
        mupicoTrack& track();
        mupicoTower& tower();
        /* short index(); */
        

    ClassDef(SL20a_picoTrackTowerMaker,1);
};

#endif

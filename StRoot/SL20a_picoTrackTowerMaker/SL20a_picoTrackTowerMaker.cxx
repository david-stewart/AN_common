#include "SL20a_picoTrackTowerMaker.h"

#include "TrackTowerMatcher/TrackTowerMatcher.h"
#include "StEmcCollection.h"
/* #include "StEmcRawHit.h" */
/* #include "StEmcModule.h" */
/* #include "StEmcDetector.h" */

ClassImp(SL20a_picoTrackTowerMaker)

FloatInt::FloatInt (float _d, int _i) : f_val{_d}, i_val{_i} {};
bool FloatInt::operator < (const FloatInt& rhs) const { return f_val > rhs.f_val; };


SL20a_picoTrackTowerMaker::SL20a_picoTrackTowerMaker (
        StPicoDstMaker* _picoMaker,
        IntList&        _bad_tower_list,
        ofstream&       _log,
        St_db_Maker*    _starDb,
        int             _debug_level,
        int             _MaxNTracks,
        int             _MaxNTowers,
        bool            _generate_ids
) :
    picoMaker    {_picoMaker},
    bad_tower_list(_bad_tower_list),
    log           (_log),
    /* mBEMCGeom     {StEmcGeom::getEmcGeom("bemc")}, */
    /* mEmcPosition  { new StEmcPosition }, */
    starDb        { _starDb },
    mBemcTables   {new StBemcTables},
    debug_level   {_debug_level},
    MaxNTracks    {_MaxNTracks},
    MaxNTowers    {_MaxNTowers},
    generate_ids  {_generate_ids},
    bemcGeom      { StEmcGeom::getEmcGeom("bemc") }
{
    clear();
};

bool SL20a_picoTrackTowerMaker::next_track(){
    ++iTrack;
    if (iTrack >= nTracks) {
        iTrack = -1;
        return false;
    }
    return true;
};
bool SL20a_picoTrackTowerMaker::next_tower(){
    ++iTower;
    if (iTower >= nTowers) {
        iTower = -1;
        return false;
    }
    return true;
};
mupicoTrack& SL20a_picoTrackTowerMaker::track() { 
    return tracks[pt_track_order[iTrack]];
};
mupicoTower& SL20a_picoTrackTowerMaker::tower() { 
    return towers[iTower];
};

mupicoTrack& SL20a_picoTrackTowerMaker::get_track(int i) {
    return tracks[pt_track_order[i]];
};
mupicoTower& SL20a_picoTrackTowerMaker::get_tower(int i) {
    return towers[i];
};
/* short index() { */
    /* return index_id(towers */
/* }: */

void SL20a_picoTrackTowerMaker::clear() {
    id_to_TrackIndex.clear();
    pt_track_order.clear();
    tracks.clear();
    towers.clear();
    id_set.clear();

    nTracks = 0;
    nTowers = 0;
    iTrack  = -1;
    iTower  = -1;
};

short SL20a_picoTrackTowerMaker::index_id(int id) {
    if (id_to_TrackIndex.count(id)) return id_to_TrackIndex[id];
    if (id_set.count(id))           return -1;
    return -2;
};


void SL20a_picoTrackTowerMaker::make() {
    clear();

    // Get access to picoDst data
    StPicoDst*       picoDst   = picoMaker->picoDst();
    StPicoEvent*     picoEvent = picoDst->event();

    TrackTowerMatcher matched_tower{}; // keep track of hadronE and towerEt matches
    // Fill the vector of mupicoTracks
   
    vector<FloatInt>    pt_to_index;   // sort for index of tracks to pT high to low
    map<int,int>        index_to_id;   // the id of each track in tracks

    for (unsigned int i = 0; i < picoDst->numberOfTracks(); ++i) {
        StPicoTrack* pico_track = picoDst->track(i);
        if (!pico_track || !(pico_track->isPrimary()))   continue;
        TVector3 Ptrack = pico_track->pMom();
        float eta  { static_cast<float>(Ptrack.PseudoRapidity()) };
        if (TMath::Abs(eta) > 1.) continue;
        float pt {(float) Ptrack.Perp() };
        if (pt < 0.2 || pt > 30.) continue;
        if (generate_ids)  id_set.insert((short)pico_track->id());
        bool pass_cuts { true };

        // otherwise, keep the track, but perhaps with a pass_cuts = false
        short nHitsFit  = pico_track->nHitsFit() ;
        short nHitsPoss = pico_track->nHitsPoss();
        short nHitsdEdx = pico_track->nHitsDedx();
        /* float nHitsRat { static_cast<float>(nHitsFit)/nHitsPoss}; */
        // if (nHitsRat < 0.52 || nHitsFit < 10) pass_cuts = false;
        if (nHitsFit < 15) pass_cuts = false;

        TVector3 dca = pico_track->origin() - picoEvent->primaryVertex(); 
        float dcaXYZ { static_cast<float>(dca.Mag()) };
        if (dcaXYZ > 3.) pass_cuts = false;
        bool isTofMatch  {pico_track->isTofTrack()};

        short towerID {0};
        bool isBEMCMatch { pico_track->isBemcMatchedTrack() };
        if (isBEMCMatch) { // true if mBEmcMatchedTowerIndex != 0
            towerID =  pico_track->bemcTowerIndex()+1 ;
            matched_tower.add_hadronE_p2( Ptrack.Mag2(), towerID );
            if (!pico_track->isBemcMatchedExact()) towerID *= -1;
        }
        if (towerID) {
            int i_check { TMath::Abs(towerID) };
            if (bad_tower_list(i_check)) {
                if (towerID>0) towerID += 10000;
                if (towerID<0) towerID -= 10000;
            }
        }

        tracks.push_back({
            pt, 
            eta, 
            static_cast<float>(__0to2pi(Ptrack.Phi())), 
            static_cast<float>(dca.Perp()), 
            dcaXYZ,  // update later XYZ here
            isTofMatch, 
            isBEMCMatch,
            towerID,
            0., // matched tower energy
            nHitsFit, 
            nHitsPoss, 
            nHitsdEdx,
            pass_cuts
        });
        pt_to_index.push_back({(float)pt,nTracks});
        if (generate_ids) index_to_id[nTracks] = (short) pico_track->id();
        ++nTracks;

        /* if (towerID != 0) matched_tower.add_hadronE_p2(Ptrack.Mag2(), TMath::Abs(towerID)); */
    }

    // Now get the order of indices for the tracsk to be read
    sort(pt_to_index.begin(), pt_to_index.end()); // tracks are sorted by pT
    for (auto& p : pt_to_index) pt_track_order.push_back(p.i_val);
    if (generate_ids) for (int i{0};i<nTracks;++i)  {
        id_to_TrackIndex[index_to_id[pt_track_order[i]]] = i;
    }


    mBemcTables->loadTables(starDb);
    const int detector {1};
    for (int towerID = 1; towerID < 4801; ++towerID){
        StPicoBTowHit* bTowHit = static_cast<StPicoBTowHit*>(picoDst->btowHit(towerID-1)); 

        if (!bTowHit) { 
            if (debug_level > 2) cout << " missing: " << towerID << endl;  
            continue; 
        }
        if (debug_level > 1 && bTowHit->isBad()) cout << " --- isBad: " << towerID << endl;
        
        if (debug_level > 2) {
            bool status0 { mBemcTables->status(detector, towerID)             != 1 };
            bool status1 { mBemcTables->status(detector, towerID, "pedestal") != 1 };
            bool status2 { mBemcTables->status(detector, towerID, "gain")     != 1 };
            bool status3 { mBemcTables->status(detector, towerID, "calib")    != 1 };

            if (status0  || status1 || status2 || status3) cout << Form(" bad: %4i %i %i %i %i || %i",
                    towerID, status0, status1, status2, status3, bTowHit->isBad()) << endl;
        }

        if ( 
                mBemcTables->status(detector, towerID)             != 1
             || mBemcTables->status(detector, towerID, "pedestal") != 1
             || mBemcTables->status(detector, towerID, "gain")     != 1
             || mBemcTables->status(detector, towerID, "calib")    != 1
        ) { continue; }

		if (bad_tower_list(towerID)) continue;

        // compare the energy
        double ped  { mBemcTables->pedestal(detector, towerID) };
        double calib { mBemcTables->calib(detector,   towerID) };
        double towerEnergy = (bTowHit->adc() - ped)*calib;
        if (debug_level > 3) {
            double hitE = bTowHit->energy();
            cout << Form(" id %5i  %6.3f %6.3f  Delta %6.3f", 
                    towerID, hitE, towerEnergy, towerEnergy-hitE) << endl;
        }

        const double minTowEnergy {0.2};
        if (towerEnergy < minTowEnergy) continue;
        float xT, yT, zT;
        bemcGeom->getXYZ(towerID, xT, yT, zT);
        TVector3 towLoc {xT, yT, zT};
        TVector3 relPos { towLoc - picoEvent->primaryVertex()   };
        float eta  { static_cast<float>(relPos.PseudoRapidity())};
        float phi  { static_cast<float>(__0to2pi(relPos.Phi())) };

        float towerEt = (float) towerEnergy / TMath::CosH(eta);
        if (towerEt < minTowEnergy) continue;
        float hadron_corr = matched_tower.get_hadronE(towerID);
        if (hadron_corr) hadron_corr /= TMath::CosH(eta);

        towers.push_back({
             towerEt, 
             eta,
             phi,
             hadron_corr,
             (short) towerID
        });
        matched_tower.add_towerEt(towerID, towerEt);
    }
    sort(towers.begin(), towers.end()); // towers are sorted by Et
    nTowers = towers.size();

    // go back and add track tower energies
    for (auto& _track : tracks) {
        int towerID { TMath::Abs(_track.towerID) };
        if (towerID) {
            _track.towerEt = matched_tower.get_towerEt(towerID);
        }
    }

    if (nTracks > MaxNTracks) {
        cout << " Warning! More tracks found that MaxNTracks allowed." << endl
             << " found("<<nTracks<<") Allowed ("<<MaxNTracks<<")" << endl
             << " Keeping allowed number of highest pT tracks." << endl;
        log  << " Warning! More tracks found that MaxNTracks allowed." << endl
             << " found("<<nTracks<<") Allowed ("<<MaxNTracks<<")" << endl
             << " Keeping allowed number of highest pT tracks." << endl;
        nTracks = MaxNTracks;
    }

    if (nTowers > MaxNTowers) {
        cout << " Warning! More towers found that MaxNTowers allowed." << endl
             << " found("<<nTowers<<") Allowed ("<<MaxNTowers<<")" << endl
             << " Keeping allowed number of highest pT towers." << endl;
        log  << " Warning! More towers found that MaxNTowers allowed." << endl
             << " found("<<nTowers<<") Allowed ("<<MaxNTowers<<")" << endl
             << " Keeping allowed number of highest pT towers." << endl;
        nTowers = MaxNTowers;
    }
};

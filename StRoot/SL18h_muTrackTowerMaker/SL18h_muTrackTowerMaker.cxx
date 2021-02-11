#include "SL18h_muTrackTowerMaker.h"

#include "TrackTowerMatcher/TrackTowerMatcher.h"

#include "StEmcCollection.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StEmcUtil/others/emcDetectorName.h"
#include "StEmcUtil/projection/StEmcPosition.h"
#include "StEmcUtil/filters/StEmcFilter.h"
#include "StEmcRawHit.h"
#include "StEmcModule.h"
#include "StEmcDetector.h"

ClassImp(SL18h_muTrackTowerMaker)

FloatInt::FloatInt (float _d, int _i) : f_val{_d}, i_val{_i} {};
bool FloatInt::operator < (const FloatInt& rhs) const { return f_val > rhs.f_val; };


SL18h_muTrackTowerMaker::SL18h_muTrackTowerMaker (
        StMuDstMaker* _muDstMaker,
        IntList&      _bad_tower_list,
        ofstream&     _log,
        St_db_Maker*  _starDb,
        int           _debug_level,
        int _MaxNTracks,
        int _MaxNTowers
) :
    muDstMaker    {_muDstMaker},
    bad_tower_list(_bad_tower_list),
    log           (_log),
    mBEMCGeom     {StEmcGeom::getEmcGeom("bemc")},
    mEmcPosition  { new StEmcPosition },
    starDb        { _starDb },
    mBemcTables   {new StBemcTables},
    debug_level   {_debug_level},
    MaxNTracks    {_MaxNTracks},
    MaxNTowers    {_MaxNTowers}
{
    if (debug_level > 0) cout << " ------------------- star_dB TIGER " << starDb << endl;
    clear();
};

bool SL18h_muTrackTowerMaker::next_track(){
    ++iTrack;
    if (iTrack >= nTracks) {
        iTrack = -1;
        return false;
    }
    return true;
};
bool SL18h_muTrackTowerMaker::next_tower(){
    ++iTower;
    if (iTower >= nTowers) {
        iTower = -1;
        return false;
    }
    return true;
};
mupicoTrack& SL18h_muTrackTowerMaker::track() { 
    return tracks[pt_track_order[iTrack]];
};
mupicoTower& SL18h_muTrackTowerMaker::tower() { 
    return towers[iTower];
};

mupicoTrack& SL18h_muTrackTowerMaker::get_track(int i) {
    return tracks[pt_track_order[i]];
};
mupicoTower& SL18h_muTrackTowerMaker::get_tower(int i) {
    return towers[i];
};
/* short index() { */
    /* return index_id(towers */
/* }: */

void SL18h_muTrackTowerMaker::clear() {
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

short SL18h_muTrackTowerMaker::index_id(int id) {
    if (id_to_TrackIndex.count(id)) return id_to_TrackIndex[id];
    if (id_set.count(id))           return -1;
    return -2;
};


void SL18h_muTrackTowerMaker::make() {
    clear();

    // Get access to MuDst data
    StMuDst*         mMuDst         = muDstMaker->muDst();
    StMuEvent*       muEvent        = mMuDst->event();

    // get magetic field (needed for track to tower matching)
    float mBField = muEvent->magneticField(); // needed for mupicoTower matching
    mBEMCGeom = StEmcGeom::instance("bemc");
    Double_t bemc_radius = mBEMCGeom->Radius();
    Double_t mBField_tesla = mBField / 10.0;
    // See rhic.bnl.gov/star/packages/SL18b/StRoot/StJetMaker/mudst/StMuEmcPosition.cxx
    if (debug_level == 1) cout << " mBField " << mBField_tesla << endl;
    if (debug_level == 1) cout << " bemc_radius " << bemc_radius << endl;

    TrackTowerMatcher matched_tower{}; // keep track of hadronE and towerEt matches
    // Fill the vector of mupicoTracks
    vector<FloatInt>    pt_to_index;   // sort for index of tracks to pT high to low
    map<int,int>        index_to_id;   // the id of each track in tracks
    for (int i_track{0}; i_track< (int) mMuDst->numberOfPrimaryTracks(); ++i_track){
        StMuTrack* muTrack = (StMuTrack*) mMuDst->primaryTracks(i_track);
        if ( muTrack->charge() == -9999 ) continue;

        bool pass_cuts = true;
        short id = muTrack->id();
        id_set.insert(id);

        // two cuts that results in an id of -1:
        // The track exists, but it is not worth keeping
        double eta { muTrack->eta() };
        if (TMath::Abs(eta) > 1.) continue;
        double pt { muTrack->pt() };
        if (pt < 0.2) continue;

        // otherwise, keep the track, but perhaps with a pass_cuts = false
        StMuTrack* gTrk = (StMuTrack*)muTrack->globalTrack();
        if (!gTrk) {
            cout << "Error: missing global track for primary"<<endl;
            continue;
        }
        int nHitsFit { gTrk->nHitsFit(kTpcId) };
        int nHitsPoss { gTrk->nHitsPoss(kTpcId) };
        int nHitsdEdx { gTrk->nHitsDedx() };
        /* float nHitsRat { static_cast<float>(nHitsFit)/nHitsPoss}; */
        /* if (nHitsRat < 0.52 || nHitsFit < 10) pass_cuts = false; */
        if (nHitsFit < 15) pass_cuts = false;

        StThreeVectorF dcaGlobal = muTrack->dcaGlobal();
        double dcaXYZ = dcaGlobal.mag();
        double dcaXY  = dcaGlobal.perp();

        if (dcaXYZ > 3.) pass_cuts = false;//continue;
        if (debug_level == 1) cout << " pt " << pt << endl;

        bool isTofMatch = (muTrack->index2BTofHit() >= 0);
        bool isBEMCMatch = (muTrack->matchBEMC()); // update later
        if (debug_level == 1) cout << " isTofMatch " << isTofMatch << endl;

        int  towerID = 0; // 0 for no match, <0 for near match, >0 exact match, +/- 1000

        // --------------------------------------
        // BEGIN: Do the track to tower matching
        //get inner radius of BEMC
        StThreeVectorD bemc_pos, bemc_mom;
        // BEMC hardware indices 
        Int_t h_m, h_e, h_s = 0;
        Int_t tow_id = 0;
        Bool_t close_match = false;

        // Check if the track can be projected onto the current radius
        // if not, track can't be matched.
        // By JetCorr request the global track projection to BEMC is used.


        if ( mEmcPosition->projTrack(&bemc_pos, &bemc_mom, gTrk, mBField_tesla, bemc_radius) ) {
            if (debug_level == 1) cout << " alpha 0   bemc_pos " << bemc_pos << endl;
            if (debug_level == 1) cout << "  bemc_mom " << bemc_mom << endl;

            // First, examine track eta. If it falls in two regions:
            // 0 < |eta| < etaMin()
            // etaMax() < |eta| < 1.0
            // then shift the eta for the projection slightly into the neighboring tower
            if ( fabs(bemc_pos.pseudoRapidity()) < mBEMCGeom->EtaMin() ) {
                Double_t unsigned_eta = mBEMCGeom->EtaMin() + 0.001;
                Double_t unsigned_theta = 2.0 * atan(exp(-1.0 * unsigned_eta));
                Double_t signed_theta = (bemc_pos.pseudoRapidity() >= 0 ? 1.0 : -1.0) * unsigned_theta;
                bemc_pos.setTheta(signed_theta);
                close_match = true;
            } 
            else if ( fabs(bemc_pos.pseudoRapidity()) > mBEMCGeom->EtaMax() &&
                    fabs(bemc_pos.pseudoRapidity()) < 1.0 ) {
                Double_t unsigned_eta = mBEMCGeom->EtaMax() - 0.001;
                Double_t unsigned_theta = 2.0 * atan(exp(-1.0 * unsigned_eta));
                Double_t signed_theta = (bemc_pos.pseudoRapidity() >= 0 ? 1.0 : -1.0) * unsigned_theta;
                bemc_pos.setTheta(signed_theta);
                close_match = true;
            } 
            // Get the BEMC hardware location in (m, e, s) and translate to id
            // If StEmcGeom::getBin() != 0: track was not matched to a tower.
            // Its outside of the BEMC eta range (> 1.0).
            if ( mBEMCGeom->getBin(bemc_pos.phi(),bemc_pos.pseudoRapidity(),h_m,h_e,h_s) == 0 ) {
                // If StEmcGeom::getId() == 0: the track was matched successfully. Otherwise, 
                // the track was not matched to a tower at this radius, the track was projected
                // into the gap between modules in phi. 
                if ( h_s != -1 ) {
                    mBEMCGeom->getId(h_m,h_e,h_s,tow_id);
                    /* bemcExactMatch = !close_match; */
                    if (close_match) {
                        towerID = -1*tow_id;
                    } else {
                        towerID = tow_id;
                    }
                } else {
                    // Value of the "dead space" per module in phi:
                    // 2*pi/60 (amount of azimuth covered per module)
                    // 2*0.0495324 (active size of module)
                    Double_t dphi = (TMath::Pi() / 60.0) - 0.0495324;

                    // Shift the projected phi by dphi in positive and negative directions
                    // if we look for the projection for both of these, only one should give
                    // a tower id, and the other should still be in the inter-tower space
                    StThreeVectorD bemc_pos_shift_pos(bemc_pos); 
                    bemc_pos_shift_pos.setPhi(bemc_pos_shift_pos.phi() + dphi);
                    StThreeVectorD bemc_pos_shift_neg(bemc_pos); 
                    bemc_pos_shift_neg.setPhi(bemc_pos_shift_neg.phi() - dphi);

                    if ( mBEMCGeom->getBin(bemc_pos_shift_pos.phi(),
                                bemc_pos_shift_pos.pseudoRapidity(),
                                h_m,h_e,h_s) == 0 &&
                            h_s != -1 ) {
                        mBEMCGeom->getId(h_m,h_e,h_s,tow_id);
                        /* bemcExactMatch = false; */
                        towerID = -1*tow_id;
                        /* picoTrk->setBEmcMatchedTowerIndex(-1*tow_id); */
                    }
                    else if ( mBEMCGeom->getBin(bemc_pos_shift_neg.phi(),
                                bemc_pos_shift_neg.pseudoRapidity(),
                                h_m,h_e,h_s) == 0 &&
                            h_s != -1 ) {
                        mBEMCGeom->getId(h_m,h_e,h_s,tow_id);
                        /* bemcExactMatch = false; */
                        towerID = -1*tow_id;
                        /* picoTrk->setBEmcMatchedTowerIndex(-1*tow_id); */
                    }
                } // else
            } // if ( mBEMCGeom->getBin(bemc_pos.phi(),bemc_pos.pseudoRapidity(),h_m,h_e,h_s) == 0 ) cout << " tow_id " << tow_id << endl;
        } else {
            cout << " warning: fail b/c function mEmcPosition->projTrack failed " << endl;
            throw std::runtime_error("fail b/c function mEmcPosition->projTrackfailed");
        }
        // END: Do the track to tower matching
        // --------------------------------------

        if (towerID) {
            int i_check { TMath::Abs(towerID) };
            if (bad_tower_list(i_check)) {
                if (towerID>0) towerID += 10000;
                if (towerID<0) towerID -= 10000;
            }
        }

        tracks.push_back({
            static_cast<float>(pt), 
            static_cast<float>(eta), 
            static_cast<float>(__0to2pi(muTrack->phi())), 
            static_cast<float>(dcaXY), 
            static_cast<float>(dcaXYZ),  // update later XYZ here
            isTofMatch, 
            isBEMCMatch,
            (short) towerID,
            0.,
            (short) nHitsFit, 
            (short) nHitsPoss, 
            (short) nHitsdEdx,
            pass_cuts
        });
        pt_to_index.push_back({(float)pt,nTracks});
        index_to_id[nTracks] = id;
        ++nTracks;

        if (towerID != 0) matched_tower.add_hadronE_p2(muTrack->p().mag2(), TMath::Abs(towerID));
    }
    // Now get the order of indices for the tracsk to be read
    sort(pt_to_index.begin(), pt_to_index.end()); // tracks are sorted by pT
    for (auto& p : pt_to_index) pt_track_order.push_back(p.i_val);
    for (int i{0};i<nTracks;++i) id_to_TrackIndex[index_to_id[pt_track_order[i]]] = i;

    // Now fill (and correct for jet matchs, all of the towers)
    /* vector<mupicoTower> vTowers; */
    const UInt_t   mBEMCModules         = 120;
    const UInt_t   mBEMCTowPerModule    = 20;
    const Int_t    mBEMCSubSections     = 2;
    const int BTOW {1};
    /* vector<int> bb; */

    /* const double Pi    = 3.1415926535897932384; */
    const double TwoPi = 6.2831853071795864769;

    /* if (starDb == nullptr) { */
        /* cout << " Fatal error: Star_db_Maker* starDb has not been initialized! " << endl; */
    /* } */
    
    mBemcTables->loadTables(starDb);
    /* StMuDst*         mMuDst         = muDstMaker->muDst(); */
    StEmcCollection* mEmcCollection = (StEmcCollection*) mMuDst->emcCollection();
    if (mEmcCollection == nullptr) {
        throw std::runtime_error("Fatal error in SL18h_muTrackTowerMaker:"
                "can't get EmcCollection.");
    }
    StEmcDetector* mBEMC = mEmcCollection->detector(kBarrelEmcTowerId);

    for (UInt_t i = 1; i <= mBEMCModules; ++i) {
        StSPtrVecEmcRawHit& mEmcTowerHits = mBEMC->module(i)->hits();
        /* loop over all hits in the module */
        for (UInt_t j = 0; j < mEmcTowerHits.size(); ++j) {
            /* cout << " j: " << j << endl; */
            /* cout << " hits.size: " << mEmcTowerHits.size() << endl; */
            StEmcRawHit* tow = mEmcTowerHits[j];

            if (       abs(tow->module()) > mBEMCModules  
                    || abs(tow->eta())    > mBEMCTowPerModule 
                    || tow->sub()         >  mBEMCSubSections) continue;

            int towerID, towerStatus;
            mBEMCGeom->getId((int)tow->module(), (int)tow->eta(), (int)tow->sub(), towerID);
            /* cout << Form(" A towerID: %4i",towerID) << endl; */
            if (bad_tower_list.has(towerID)) continue;
            // { cout << " ALEPH bad tower list: " << towerID << endl; continue;}
            mBemcTables->getStatus(BTOW, towerID, towerStatus);
            /* cout << Form(" B towerID: %4i   towerStatus: %i",towerID, towerStatus) << endl; */

            if (towerStatus != 1) continue;
            //{ cout << " BETA bad tower status: " << towerID << endl; continue; }
            /* if (binary_search(bb.begin(), bb.end(), towerID)) cout << " bb : " << towerID << endl; */
            Float_t towerEnergy = tow->energy();
            const double minTowEnergy {0.2};
            if (towerEnergy < minTowEnergy) continue;
            Float_t towerEta, towerPhi;
            mBEMCGeom->getEtaPhi(towerID, towerEta, towerPhi);
            while (towerPhi < 0.)    towerPhi += TwoPi;
            while (towerPhi > TwoPi) towerPhi -= TwoPi;

            double towerEt { towerEnergy / TMath::CosH(towerEta) };
            if (towerEt < minTowEnergy) continue;
            float hadron_corr = matched_tower.get_hadronE(towerID);
            if (hadron_corr) hadron_corr /= TMath::CosH(towerEta);

            towers.push_back({
                    static_cast<float>(towerEt), 
                    static_cast<float>(towerEta), 
                    static_cast<float>(towerPhi),
                    hadron_corr,
                    (short) towerID
            });
            matched_tower.add_towerEt(towerID, towerEt);
        }
    }
    sort(towers.begin(), towers.end()); // towers are sorted by Et
    nTowers = towers.size();

    // go back and add track tower energies
    for (int i{0}; i<TMath::Abs(nTracks); ++i) {
        int towerID { TMath::Abs(tracks[i].towerID) };
        if (towerID) {
            tracks[i].towerEt = matched_tower.get_towerEt(towerID);
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

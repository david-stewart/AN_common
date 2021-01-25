#include "P18h_mupicoTrackTowerMaker.h"

#include "StEmcCollection.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StEmcUtil/others/emcDetectorName.h"
#include "StEmcUtil/projection/StEmcPosition.h"
#include "StEmcUtil/filters/StEmcFilter.h"
#include "StEmcRawHit.h"
#include "StEmcModule.h"
#include "StEmcDetector.h"

ClassImp(P18h_mupicoTrackTowerMaker)

P18h_mupicoTrackTowerMaker::P18h_mupicoTrackTowerMaker (
        StMuDstMaker* _muDstMaker,
        IntList&      _bad_tower_list,
        ofstream&     _log,
        St_db_Maker*  _starDb,
        int           _debug_level
) :
    muDstMaker    {_muDstMaker},
    bad_tower_list(_bad_tower_list),
    log           (_log),
    mBEMCGeom     {StEmcGeom::getEmcGeom("bemc")},
    mEmcPosition  { new StEmcPosition },
    starDb        { _starDb },
    mBemcTables   {new StBemcTables},
    debug_level   {_debug_level}
{
    if (debug_level > 0) cout << " ------------------- star_dB TIGER " << starDb << endl;
};

vector<mupicoTrack> P18h_mupicoTrackTowerMaker::makeVec_mupicoTracks(TrackTowerMatcher* matched_tower) {
    vector<mupicoTrack>  vTracks;
    StMuDst*         mMuDst         = muDstMaker->muDst();
    StMuEvent*       muEvent        = mMuDst->event();

    float mBField = muEvent->magneticField(); // needed for tower matching
    mBEMCGeom = StEmcGeom::instance("bemc");
    Double_t bemc_radius = mBEMCGeom->Radius();
    Double_t mBField_tesla = mBField / 10.0;
    // See rhic.bnl.gov/star/packages/SL18b/StRoot/StJetMaker/mudst/StMuEmcPosition.cxx
    if (debug_level == 1) cout << " mBField " << mBField_tesla << endl;
    if (debug_level == 1) cout << " bemc_radius " << bemc_radius << endl;

    for (int i_track{0}; i_track< (int) mMuDst->numberOfPrimaryTracks(); ++i_track){
        StMuTrack* muTrack = (StMuTrack*) mMuDst->primaryTracks(i_track);
        if ( muTrack->charge() == -9999 ) continue;
        double eta { muTrack->eta() };
        if (TMath::Abs(eta) > 1.) continue;
        int nHitsFit { muTrack->nHitsFit() };
        int nHitsPoss { muTrack->nHitsPoss() };
        int nHitsdEdx { muTrack->nHitsDedx() };
        float nHitsRat { static_cast<float>(nHitsFit)/nHitsPoss};
        if (nHitsRat < 0.52 || nHitsFit < 15) continue;
        StThreeVectorF dcaGlobal = muTrack->dcaGlobal();
        double dcaXYZ = dcaGlobal.mag();
        double dcaXY  = dcaGlobal.perp();
        if (dcaXYZ > 3.) continue;
        double pt { muTrack->pt() };
        if (debug_level == 1) cout << " pt " << pt << endl;
        if (pt < 0.2) continue;
        if (debug_level == 1) cout << " debug C1 " << endl;

        bool isTofMatch = (muTrack->index2BTofHit() >= 0);
        bool isBEMCMatch = (muTrack->matchBEMC()); // update later
        if (debug_level == 1) cout << " isTofMatch " << isTofMatch << endl;

        int  i_tower = 0; // 0 for no match, <0 for near match, >0 exact match
        StMuTrack* gTrk = (StMuTrack*)muTrack->globalTrack();
        if (!gTrk) {
            cout << "Error: missing global track for primary"<<endl;
            continue;
        }

        // Do the track matching
        
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
                        i_tower = -1*tow_id;
                    } else {
                        i_tower = tow_id;
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
                        i_tower = -1*tow_id;
                        /* picoTrk->setBEmcMatchedTowerIndex(-1*tow_id); */
                    }
                    else if ( mBEMCGeom->getBin(bemc_pos_shift_neg.phi(),
                                bemc_pos_shift_neg.pseudoRapidity(),
                                h_m,h_e,h_s) == 0 &&
                            h_s != -1 ) {
                        mBEMCGeom->getId(h_m,h_e,h_s,tow_id);
                        /* bemcExactMatch = false; */
                        i_tower = -1*tow_id;
                        /* picoTrk->setBEmcMatchedTowerIndex(-1*tow_id); */
                    }
                } // else
            } // if ( mBEMCGeom->getBin(bemc_pos.phi(),bemc_pos.pseudoRapidity(),h_m,h_e,h_s) == 0 ) cout << " tow_id " << tow_id << endl;
        } else {
            cout << " warning: fail b/c function mEmcPosition->projTrack failed " << endl;
            return  vTracks;
        }
        vTracks.push_back({
            static_cast<float>(pt), 
            static_cast<float>(muTrack->phi()), 
            static_cast<float>(eta), 
            static_cast<float>(dcaXY), 
            static_cast<float>(dcaXYZ),  // update later XYZ here
            isTofMatch, 
            isBEMCMatch,
            (short) i_tower,
            (short) nHitsFit, (short) nHitsPoss, (short) nHitsdEdx
        });
        if (i_tower != 0 && matched_tower != nullptr) {
            matched_tower->add_p2_itower(muTrack->p().mag2(), TMath::Abs(i_tower));
        }
    }
    sort(vTracks.begin(), vTracks.end()); // tracks are sorted by pT
    return vTracks;

};

vector<mupicoTower> P18h_mupicoTrackTowerMaker::makeVec_mupicoTowers(
        TrackTowerMatcher* matched_tower)
{
    vector<mupicoTower> vTowers;
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
    StMuDst*         mMuDst         = muDstMaker->muDst();
    StEmcCollection* mEmcCollection = (StEmcCollection*) mMuDst->emcCollection();
    if (mEmcCollection == nullptr) {
        cout << "Fatal error in P18h_mupicoTrackTowerMaker: can't get EmcCollection." << endl;
        return vTowers;
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
            /* if (bad_tower_list.has(towerID)) continue; */
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
            while (towerPhi < 0.) towerPhi += TwoPi;
            while (towerPhi > TwoPi) towerPhi -= TwoPi;

            double towerEt { towerEnergy / TMath::CosH(towerEta) };
            float hadron_corr=0.;
            if (matched_tower != nullptr) hadron_corr = matched_tower->sum_matched_E(towerID);

            vTowers.push_back({
                    static_cast<float>(towerEt), 
                    static_cast<float>(towerPhi),
                    static_cast<float>(towerEta), 
                    hadron_corr,
                    (short) towerID
            });
        }
    }
    sort(vTowers.begin(), vTowers.end()); // towers are sorted by Et
    return vTowers;

};

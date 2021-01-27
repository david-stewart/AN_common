#include "TreeObj.h"

ClassImp(myTriggers)
    myTriggers::myTriggers() {};
    myTriggers::myTriggers(TTree* tree, vector<int>  vec_triggers) {
        add_triggers(tree,vec_triggers);
    };
    myTriggers::~myTriggers() {
        for (auto& pair : trig_map) delete pair.second;
    };
    void myTriggers::add_triggers(TTree* tree, vector<int>& vec_triggers) {
        for (auto trigger : vec_triggers){
            trig_map[trigger] = new bool;
            tree->Branch(Form("is%i",trigger),trig_map[trigger]);
            trigger_ids.push_back(trigger);
        }
    };
    void myTriggers::set_trigger(int trigger,bool truefalse) {
        *trig_map[trigger] = truefalse;
    };
    void myTriggers::reset_triggers() {
        for (auto pair : trig_map) *(pair.second) = false;
    };
    bool myTriggers::has_any_triggers() {
        for (auto pair : trig_map) {
            if (*(pair.second)) return true;
        }
        return false;
    };


ClassImp(mupicoTrack)
    mupicoTrack::mupicoTrack() : 
        pT{0.}, phi{0.}, eta{0.}, 
        dcaXY{0.}, dcaXYZ{0.}, 
        TOF_match{false}, BEMC_match{false},
        i_tower{0}, nHitsFit{0}, 
        nHitsPoss{0}, nHitsdEdx{0}
    {};
    mupicoTrack::mupicoTrack(
        float _pT, float _phi, float _eta, 
        float _dcaXY, float _dcaXYZ, 
        bool _TOF_match, bool _BEMC_match, 
        short _i_tower, 
        short _nhitsfit, 
        short _nhitsposs,
        short _nhitsdedx
    ) :
        pT{_pT}, phi{_phi}, eta{_eta},
        dcaXY{_dcaXY}, dcaXYZ{_dcaXYZ}, 
        TOF_match{_TOF_match}, BEMC_match{_BEMC_match},
        i_tower{_i_tower}, nHitsFit{_nhitsfit}, 
        nHitsPoss{_nhitsposs}, nHitsdEdx{_nhitsdedx}
    {};
    bool mupicoTrack::operator < (const mupicoTrack& rhs) const { return pT > rhs.pT; };

ClassImp(mupicoTower)
    mupicoTower::mupicoTower() :
        Et{0.}, phi{0.}, eta{0.}, Et_hadroncorr{0.}, i_tower{0} 
    {};
    mupicoTower::mupicoTower(
        const float _Et, 
        const float _phi,
        const float _eta, 
        const float _Et_hadroncorr,
        const short _i_tower
    ) :
        Et{_Et}, phi{_phi}, eta{_eta}, 
        Et_hadroncorr{_Et_hadroncorr}, 
        i_tower{_i_tower} 
    {};
    bool mupicoTower::operator < (const mupicoTower& rhs) const { return Et > rhs.Et; };

ClassImp(embReconTrack)
    embReconTrack::embReconTrack() :
        geantId{0}, 
        is_matched{false}, pass_cuts{false},
        ptMc{0.},  etaMc{0.},  phiMc{0.}, nHitMc{0},
        ptPr{0.}, etaPr{0.}, phiPr{0.},
        dcaGl{0.}, dcaXYGl{0.} ,
        fitPts{0}, nPossiblePts{0}, dedxPts{0}
    {};
    embReconTrack::embReconTrack(
        int _geantId, bool _is_matched, 
        bool _pass_cuts,
        float _ptMc,  float _etaMc,  float _phiMc,
        short _nHitMc, 
        float _ptPr, float _etaPr, float _phiPr,
        float _dcaGl, float _dcaXYGl,
        short _fitPts, short _nPossiblePts, short _dedxPts
    ) :
        geantId{_geantId}, is_matched{_is_matched}, 
        pass_cuts{_pass_cuts}, 
        ptMc{_ptMc}, etaMc{_etaMc}, phiMc{_phiMc}, 
        nHitMc{_nHitMc},
        ptPr{_ptPr}, etaPr{_etaPr}, phiPr{_phiPr}, 
        dcaGl{_dcaGl}, dcaXYGl{_dcaXYGl}, fitPts{_fitPts},
        nPossiblePts{_nPossiblePts}, dedxPts{_dedxPts}
    {};

ClassImp(embNeutPart)
    embNeutPart::embNeutPart() :
        geantId{0}, 
        ptMc{0.},  etaMc{0.},  phiMc{0.}
    {};
    embNeutPart::embNeutPart(int _geantId,   float _pT, 
                 float _eta, float _phi
    ) : 
        geantId{_geantId}, 
        ptMc{_pT},  etaMc{_eta},  phiMc{_phi}
    {};

ClassImp(mupicoEventHeader)
    mupicoEventHeader::mupicoEventHeader() :
        runId{0}, eventId{0}, ZDCx{0.},
        vx{0.}, vy{0.}, vz{0.},
        BBC_Ein{0}, BBC_Eout{0},
        BBC_Win{0}, BBC_Wout{0},
        vzVpd{0.}, ranking{0.}, 
        ZdcSumAdcEast{0}, ZdcSumAdcWest{0}
    {};
    mupicoEventHeader::mupicoEventHeader(
        int _runId, int _eventId, float _ZDCx, 
        float _vx, float _vy, float _vz, 
        int _BBC_Ein, int _BBC_Eout,
        int _BBC_Win, int _BBC_Wout,
        float _vzVpd, float _ranking,
        int _ZdcSumAdcEast, int _ZdcSumAdcWest
    ) :
        runId{_runId}, eventId{_eventId}, ZDCx{_ZDCx},
        vx{_vx}, vy{_vy}, vz{_vz},
        BBC_Ein{_BBC_Ein}, BBC_Eout{_BBC_Eout},
        BBC_Win{_BBC_Win}, BBC_Wout{_BBC_Wout},
        vzVpd{_vzVpd}, ranking{_ranking},
        ZdcSumAdcEast{_ZdcSumAdcEast}, ZdcSumAdcWest{_ZdcSumAdcWest}
    {};

ClassImp(mupicoJetwArea)
    mupicoJetwArea::mupicoJetwArea() :
        pT{0.}, phi{0.}, eta{0.}, area{0.}
    {};
    mupicoJetwArea::mupicoJetwArea(float _pT, float _phi, 
            float _eta, float _area) :
        pT{_pT}, phi{_phi}, eta{_eta}, area{_area}
    {};

ClassImp(mupicoJet)
    mupicoJet::mupicoJet() :
        pT{0.}, phi{0.}, eta{0.}
    {};
    mupicoJet::mupicoJet(float _pT, float _phi, float _eta) :
        pT{_pT}, phi{_phi}, eta{_eta}
    {};

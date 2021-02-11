#include "TreeObj.h"
float __0to2pi(float phi){ //  put phi in range 0..2pi
    while (phi< 0 ) phi += __loc_2pi;
    while (phi> __loc_2pi) phi -= __loc_2pi;
    return phi;
}

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
        pt{0.},
        eta{0.}, 
        phi{0.}, 
        dcaXY{0.},
        dcaXYZ{0.}, 
        TOF_match{false}, 
        BEMC_match{false},

        towerID{0}, 
        towerEt{0.},

        nHitsFit{0}, 
        nHitsPoss{0}, 
        nHitsDedx{0},
        pass_cuts{false}
    {};
    mupicoTrack::mupicoTrack(
        float _pt, 
        float _eta, 
        float _phi, 
        float _dcaXY,
        float _dcaXYZ, 

        bool  _TOF_match, 
        bool  _BEMC_match, 

        short _towerID, 
        float _towerEt,

        short _nhitsfit, 
        short _nhitsposs,
        short _nhitsdedx,
        bool  _pass_cuts
    ) :
        pt{_pt}, 
        eta{_eta},
        phi{_phi}, 
        dcaXY{_dcaXY}, 
        dcaXYZ{_dcaXYZ}, 

        TOF_match{_TOF_match}, 
        BEMC_match{_BEMC_match},

        towerID{_towerID}, 
        towerEt{_towerEt}, // 0., unless there is a tower with passing Et

        nHitsFit{_nhitsfit}, 
        nHitsPoss{_nhitsposs}, 
        nHitsDedx{_nhitsdedx},
        pass_cuts{_pass_cuts}
    {};
    bool mupicoTrack::operator < (const mupicoTrack& rhs) const { return pt > rhs.pt; };

ClassImp(mupicoTower)
    mupicoTower::mupicoTower() :
        Et{0.}, 
        eta{0.}, 
        phi{0.}, 
        Et_hadroncorr{0.}, 
        towerID{0} 
    {};
    mupicoTower::mupicoTower(
        const float _Et, 
        const float _eta, 
        const float _phi,
        const float _Et_hadroncorr,
        const short _towerID
    ) :
        Et{_Et}, 
        eta{_eta}, 
        phi{_phi}, 
        Et_hadroncorr{_Et_hadroncorr}, 
        towerID{_towerID} 
    {};
    bool mupicoTower::operator < (const mupicoTower& rhs) const { return Et > rhs.Et; };

ClassImp(embTrack)
    embTrack::embTrack() :
        geantId{0}, 
        id{-2}, // id is -2 if not matched, -1 if matched but failed too manuy cuts, otherwise
                   // otherwise 0 to point to the Track_id
        pt{0.},  
        eta{0.}, 
        phi{0.}
    {};
    embTrack::embTrack(
        short _geantId, 
        short _id,
        float _pt,
        float _eta,
        float _phi
    ) :
        geantId{_geantId}, 
         id {_id},
        pt{_pt}, 
        eta{_eta}, 
        phi{_phi} 
    {};

ClassImp(embNeutPart)
    embNeutPart::embNeutPart() :
        geantId{0}, 
        pt{0.},  
        eta{0.},  
        phi{0.}
    {};
    embNeutPart::embNeutPart(short _geantId,   float _pt, 
                 float _eta, float _phi
    ) : 
        geantId{_geantId}, 
        pt{_pt}, 
        eta{_eta},  
        phi{_phi}
    {};

ClassImp(mupicoEventHeader)
    mupicoEventHeader::mupicoEventHeader() :
        runId{0}, 
        eventId{0},
        ZDCx{0.},
        vx{0.}, 
        vy{0.}, 
        vz{0.},
        BBC_Ein{0}, 
        BBC_Eout{0},
        BBC_Win{0}, 
        BBC_Wout{0},
        vzVpd{0.}, 
        ranking{0.}, 
        ZdcSumAdcEast{0}, 
        ZdcSumAdcWest{0}
    {};
    mupicoEventHeader::mupicoEventHeader(
        int _runId, 
        int _eventId, 
        float _ZDCx, 
        float _vx, 
        float _vy, 
        float _vz, 
        int _BBC_Ein, 
        int _BBC_Eout,
        int _BBC_Win, 
        int _BBC_Wout,
        float _vzVpd, 
        float _ranking,
        int _ZdcSumAdcEast, 
        int _ZdcSumAdcWest
    ) :
        runId{_runId}, 
        eventId{_eventId}, 
        ZDCx{_ZDCx},
        vx{_vx}, 
        vy{_vy}, 
        vz{_vz},
        BBC_Ein{_BBC_Ein},
        BBC_Eout{_BBC_Eout},
        BBC_Win{_BBC_Win}, 
        BBC_Wout{_BBC_Wout},
        vzVpd{_vzVpd}, 
        ranking{_ranking},
        ZdcSumAdcEast{_ZdcSumAdcEast}, 
        ZdcSumAdcWest{_ZdcSumAdcWest}
    {};

ClassImp(JetwArea)
    JetwArea::JetwArea() :
        pt{0.}, 
        eta{0.}, 
        phi{0.}, 
        area{0.}
    {};
    JetwArea::JetwArea(
            float _pt,
            float _eta,
            float _phi, 
            float _area
            ) :
        pt{_pt}, eta{_eta}, phi{_phi}, area{_area}
    {};

ClassImp(Jet)
    Jet::Jet() :
        pt{0.}, 
        eta{0.},
        phi{0.} 
    {};
    Jet::Jet(float _pt, 
            float _eta, 
            float _phi) :
        pt{_pt}, eta{_eta}, phi{_phi}
    {};

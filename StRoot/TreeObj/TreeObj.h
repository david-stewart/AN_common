#ifndef TreeObj_h
#define TreeObj_h

#include <iostream>
#include <fstream>
#include "TArrayF.h"
#include "TObject.h"
#include "TTree.h"

static const float __loc_pi  { 3.14159265359 };
static const float __loc_2pi { 6.28318530718 };
using namespace std;
float __0to2pi(float _phi_in); // put phi in range 0..2pi

class myTriggers : public TObject {
    public:
        virtual ~myTriggers() ;
        myTriggers();
        myTriggers(TTree* tree, vector<int>  triggers);
        void add_triggers(TTree* tree, vector<int>& triggers);

        void set_trigger(int, bool);
        bool has_any_triggers(); // see if any triggers are true 
        void reset_triggers(); // set all triggers to false
        vector<int> trigger_ids; // copy of keys in trig_map
    private:
        map<int,bool*> trig_map;
    ClassDef(myTriggers,1);
};

// Same track object as for PicoToTree
/* class Track_id; */
struct mupicoTrack : public TObject {
	public:
    mupicoTrack() ;
    mupicoTrack(
            float _pt, 
            float _eta, 
            float _phi, 
            float _dcaXY, 
            float _dcaXYZ, 

            bool _TOF_match, 
            bool _BEMC_match, 

            short _towerID, 
            float _towerEt, // Et of the matched tower

            short _nhitsfit, 
            short _nhitsposs,
            short _nhitsdedx,
            bool  _pass_cuts
    );
    /* mupicoTrack(const Track_id&); */
	virtual ~mupicoTrack() {};
    float   pt;
    float   eta;
    float   phi;
    float   dcaXY;
    float   dcaXYZ;
    bool    TOF_match;
    bool    BEMC_match;

    short   towerID; // towerID < 0 is an approx match, > 0 is an exact match
    float   towerEt;

    short   nHitsFit;
    short   nHitsPoss;
    short   nHitsDedx;
    bool    pass_cuts; // local convenience; for sake of locally changing which tracks pass cuts
    bool operator < (const mupicoTrack& rhs) const; // const { return pt > rhs.pt; };
	ClassDef(mupicoTrack,1);
};

class mupicoTower : public TObject {
	public:
    mupicoTower();
	virtual ~mupicoTower() {};

    mupicoTower(
        const float _Et, 
        const float _eta, 
        const float _phi,
        const float _Et_hadroncorr,
        const short _towerID
    ) ;
    float   Et;
    float   eta;
    float   phi;
    float   Et_hadroncorr; // correction from charged tracks. Positive value.
    short   towerID;
    bool operator < (const mupicoTower & rhs) const;
	ClassDef(mupicoTower,1);
};

// geantid can be used to get particle charge
class embTrack : public TObject {
    public:
    embTrack(); 
    embTrack(
        short _geantId, 
        short _id, // id to mupicoTrack (from MuDst); 
                      // -2 if not matched
                      // -1 if matched but not mupicoTrack not written (becuase high |eta|, etc...)
        float _pt,  
        float _eta,  
        float _phi
        /* short _nHit //, */  
        /* float _ptPr, float _etaPr, float _phiPr, */
        /* float _dcaGl, float _dcaXYGl, */
        /* short _fitPts, short _nPossiblePts, short _dedxPts */
    ); 

	virtual ~embTrack() {};
    short geantId;
    short id; 
    float pt; 
    float eta;
    float phi;
	ClassDef(embTrack,1);
};

class embNeutPart : public TObject {
    public:
    embNeutPart(); 
    embNeutPart(short _geantId, float _pt, float _etat, float _phi);

    short geantId;
    float pt;
    float eta;
    float phi;

    ClassDef(embNeutPart,1);
};

class mupicoEventHeader : public TObject {
    public:
    virtual ~mupicoEventHeader() {};
    mupicoEventHeader();
    mupicoEventHeader(
        int runId, int eventId, float ZDCx, 
        float _vx, float _vy, float _vz, 
        int _BBC_Ein, int _BBC_Eout,
        int _BBC_Win, int _BBC_Wout,
        float _vzVpd, float _ranking,
        int _ZdcSumAdcEast, int _ZdcSumAdcWest
    );

    int    runId;
    int    eventId;
    float  ZDCx;
    float  vx, vy, vz;

    int    BBC_Ein;   // bbc East adc
    int    BBC_Eout;  // bbc East adc

    int    BBC_Win;   // bbc East adc
    int    BBC_Wout;  // bbc East adc

    float  vzVpd;
    float  ranking;

    int    ZdcSumAdcEast;
    int    ZdcSumAdcWest;

    ClassDef(mupicoEventHeader,1);
};

// better to store the float[24] BBC tiles locally in a tree array
// also ntracks and ntowers directly, if desired
//    /* _tree->Branch("EastBBC", east, "s[24]/s"); */
//    /* _tree->Branch("WestBBC", west, "s[24]/s"); */

// to be used with both charged and full jets
class JetwArea : public TObject {
    public:
    float pt;
    float eta;
    float phi;
    float area;
    vector<short> index_track;
    vector<short> index_tower;
    JetwArea(float _pt, float _eta, float _phi, float _area);
    JetwArea();
	ClassDef(JetwArea,1);
};

class Jet : public TObject {
    public:
    float pt;
    float eta;
    float phi;
    Jet(float _pt, float _eta, float _phi);
    Jet();
	ClassDef(Jet,1);
};

#endif

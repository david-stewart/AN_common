#ifndef TreeObj_h
#define TreeObj_h

#include <iostream>
#include <fstream>
#include "TArrayF.h"
#include "TObject.h"
#include "TTree.h"

using namespace std;

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
struct mupicoTrack : public TObject {
	public:
    mupicoTrack() ;
    mupicoTrack(float _pT, float _phi, float _eta, 
            float _dcaXY, float _dcaXYZ, 
            bool _TOF_match, bool _BEMC_match, 
            short _i_tower, 
            short _nhitsfit, 
            short _nhitsposs,
            short _nhitsdedx
    );
	virtual ~mupicoTrack() {};
    float   pT;
    float   phi;
    float   eta;
    float   dcaXY;
    float   dcaXYZ;
    bool    TOF_match;
    bool    BEMC_match;
    short   i_tower; // i_tower < 0 is an approx match, > 0 is an exact match
    short   nHitsFit;
    short   nHitsPoss;
    short   nHitsdEdx;
    bool operator < (const mupicoTrack& rhs) const; // const { return pT > rhs.pT; };
	ClassDef(mupicoTrack,1);
};

// 
class mupicoTower : public TObject {
	public:
    mupicoTower();
	virtual ~mupicoTower() {};

    mupicoTower(
        const float _Et, 
        const float _phi,
        const float _eta, 
        const float _Et_hadroncorr,
        const short _i_tower
    ) ;
    float   Et;
    float   phi;
    float   eta;
    float   Et_hadroncorr; // correction from charged tracks. Positive value.
    short   i_tower;
    bool operator < (const mupicoTower & rhs) const;
	ClassDef(mupicoTower,1);
};

// geantid can be used to get particle charge
class embReconTrack : public TObject {
    public:
    embReconTrack(); 
    embReconTrack(
        int _geantId, bool _is_matched, 
        bool _pass_cuts,
        float _ptMc,  float _etaMc,  float _phiMc,
        short _nHitMc, 
        float _ptPr, float _etaPr, float _phiPr,
        float _dcaGl, float _dcaXYGl,
        short _fitPts, short _nPossiblePts, short _dedxPts
    ); 

	virtual ~embReconTrack() {};

    int geantId;

    bool  is_matched; 
    bool  pass_cuts; 

    float ptMc; 
    float etaMc;
    float phiMc;
    short nHitMc;

    float ptPr;
    float etaPr;
    float phiPr;

    float dcaGl;
    float dcaXYGl;

    short   fitPts;
    short   nPossiblePts;
    short   dedxPts;


	ClassDef(embReconTrack,1);
};

class embNeutPart : public TObject {
    public:
    embNeutPart(); 
    embNeutPart(int _geantId, float _ptMc, float _etatMc, float _phiMc); 

    int geantId;

    float ptMc;
    float etaMc;
    float phiMc;

    ClassDef (embNeutPart,1);
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
class mupicoJetwArea : public TObject {
    public:
    float pT;
    float phi;
    float eta;
    float area;
    mupicoJetwArea(float, float, float, float);
    mupicoJetwArea();
	ClassDef(mupicoJetwArea,1);
};

class mupicoJet : public TObject {
    public:
    float pT;
    float phi;
    float eta;
    mupicoJet(float, float, float);
    mupicoJet();
	ClassDef(mupicoJet,1);
};

#endif

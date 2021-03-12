#ifndef GetBranchMaker__h
#define GetBranchMaker__h

// determine how much memory has been used

#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <vector>
#include "TStopwatch.h"
#include "TreeObj/TreeObj.h"
#include "TClonesArray.h"
/* #include "fastjet/config.h" */
/* #include "fastjet/PseudoJet.hh" */
/* using fastjet::PseudoJet; */

int parseLine(char* line);
int getMemValue();

using namespace std;
/* using namespace fastjet; */

// note: on RCAF for reasons that I do not fully understand, I cannot `cons` compile
// code using psuedojet in the header files. Therefore, I use the local named
// ennumeartion jetAlgorithm
struct JetBranchMaker : public TObject {
    /* const double PI0MASS2   { 0.0182196 }; */
    const double PI_MASS = 0.13957;

    enum jetAlgorithm { antikt, kt, cambridge };
    // Initialize with a TTree*, a name, and jet parameters, and it will make
    // and fill jets into that TTree with lists of fed in particles
	~JetBranchMaker() {};
    JetBranchMaker(
        TTree* tree, 
        const char*   name_tag, 
        bool          calc_areas      = false,
        string        jet_algo        = "anti-kt",
        unsigned int  max_njets       = 200,
        unsigned int  max_nconstituents = 1000,
        double        jet_R           = 0.4,
        double        ghost_R         = 0.01, 
        double        ghost_max_rap   = 4.,
        double        max_abs_eta_jet = -1.);  // if negative, defaults to 1.-jet_R

    void clear(); // clear the internal jet clones
    void fill(vector<pair<Jet,int>>& particles); // if calc is true, calculate jets
    void fill(pair<Jet,int> particle); // if calc is true, calculate jets
    void generate(); // generate the jets (and areas, if appropriate)
    

    bool           calc_areas;
    unsigned int   max_njets;
	/* unsigned int   njets; */
	unsigned int   max_nconstituents;
    // jet definition parameters:
    jetAlgorithm jet_algo;
    double jet_R;
    double max_abs_eta_jet;
    double ghost_R;
    double ghost_max_rap;
	double min_jet_pt;

    int n_tracks;
    int n_towers;
    int n_particles;

    // internal values:
    TClonesArray clones;
    /* TClonesArray clones_index; // constituent index */
    vector<pair<Jet,int>> particles;

    /* vector<short> itow; */ 
    /* vector<short> i0tow; */
    /* vector<short> i1tow; */

    /* vector<short> itrk; */
    /* vector<short> i0trk; */
    /* vector<short> i1trk; */

                                 // Jet is really just pT, phi, eta
    int remove_nhardest_jets;    // to use if calculating areas

    /* AreaDefinition area_def; */
    /* fastjet::JetDefinition  jet_def; */
    float rho;
    float rho_sigma;

    ClassDef (JetBranchMaker,1);

};
#endif

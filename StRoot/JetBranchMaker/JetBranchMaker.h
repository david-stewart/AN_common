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

int parseLine(char* line);
int getMemValue();

using namespace std;
/* using namespace fastjet; */

// note: on RCAF for reasons that I do not fully understand, I cannot `cons` compile
// code using psuedojet in the header files. Therefore, I use the local named
// ennumeartion jetAlgorithm
struct JetBranchMaker : public TObject {
    /* const double PI0MASS2   { 0.0182196 }; */
    const double pi0mass = 0.13498;
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
        double        jet_R           = 0.4,
        double        ghost_R         = 0.01, 
        double        ghost_max_rap   = 4.,
        double        max_abs_eta_jet = -1.);  // if negative, defaults to 1.-jet_R

    void clear(); // clear the internal jet clones
    void fill(vector<mupicoJet>& particles, bool generate=true); // if calc is true, calculate jets
    void fill(mupicoJet          particle,  bool generate=false); // if calc is true, calculate jets
    void generate(); // generate the jets (and areas, if appropriate)
    

    bool           calc_areas;
    unsigned int   max_njets;
	unsigned int   njets;
    // jet definition parameters:
    jetAlgorithm jet_algo;
    double jet_R;
    double max_abs_eta_jet;
    double ghost_R;
    double ghost_max_rap;
	double min_jet_pt;
    

    // internal values:
    TClonesArray clones;
    vector<mupicoJet> particles; // collect input particles. 
                                 // mupicoJet is really just pT, phi, eta
    int remove_nhardest_jets;    // to use if calculating areas

    /* AreaDefinition area_def; */
    /* fastjet::JetDefinition  jet_def; */
    float rho;
    float rho_sigma;

    ClassDef (JetBranchMaker,1);
};
#endif

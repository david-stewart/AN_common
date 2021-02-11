#include "JetBranchMaker.h"

#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <iostream>
#include "TString.h"
#include "TreeObj/TreeObj.h"

#include "fastjet/config.h"
#include <algorithm>
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/Subtractor.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"


using namespace fastjet;
ClassImp(JetBranchMaker)

JetBranchMaker::JetBranchMaker(
    TTree* tree, 
    const char* name_tag, 
    bool  _calc_areas,
    string         _jet_algo,
    unsigned int   _max_njets,
    double _jet_R,
    double _ghost_R,
    double _ghost_max_rap,
    double _max_abs_eta_jet )
:
    calc_areas { _calc_areas },
    max_njets  { _max_njets },
    jet_R      { _jet_R },
    max_abs_eta_jet { _max_abs_eta_jet < 0 ? 1.-jet_R:_max_abs_eta_jet },
    ghost_R    { _ghost_R },
    ghost_max_rap { _ghost_max_rap },
	min_jet_pt {0.2},
    clones     { calc_areas ? "JetwArea" : "Jet", static_cast<int>(max_njets) }
{
    if (_jet_algo == "anti-kt" || _jet_algo == "antikt" || _jet_algo == "anti_kt") {
        jet_algo = antikt;
    } else if ( _jet_algo == "kt" ) {
        jet_algo = kt;
    } else if ( _jet_algo == "cambridge" ) {
        jet_algo = cambridge;
    } else {
        cout << " error: selected jet algorithm("<< _jet_algo << ") not recoginized." << endl;
        cout << " Options allowed: anti-kt, kt, cambridge" << endl;
        cout << " Reverting to anti-kt default. " << endl;
        jet_algo = antikt;
    }

    tree->Branch(Form("%s_njets", name_tag), &njets);
    tree->Branch(name_tag, &clones);
    if (calc_areas) {
        tree->Branch( Form("%s_rho", name_tag), &rho);
        tree->Branch( Form("%s_rho_sigma", name_tag), &rho_sigma);
    } 
};

void JetBranchMaker::clear() {
    clones.Clear();
    particles.clear();
};

void JetBranchMaker::fill(vector<Jet>& v_part, bool gen) {
    for (auto& p : v_part) particles.push_back(p);
    if (gen) generate();
};

void JetBranchMaker::fill(Jet p, bool gen) {
    particles.push_back(p);
    if (gen) generate();
};

void JetBranchMaker::generate(){
    // transfer each Jet into a Pseudojet
    int index{0};
    vector<PseudoJet> v_input;
    for (auto& p : particles) {
        v_input.push_back(PseudoJet());
        v_input[index++].reset_PtYPhiM( p.pt, p.eta, p.phi, pi0mass );
    }
   
    fastjet::Selector      jetrap  = fastjet::SelectorAbsEtaMax(max_abs_eta_jet);
    fastjet::Selector      not_pure_ghost = !SelectorIsPureGhost();
    fastjet::Selector      selection      = jetrap && not_pure_ghost;


    fastjet::JetDefinition jet_def ( 
        jet_algo == antikt ? antikt_algorithm :
        jet_algo == kt     ? kt_algorithm     :
                             cambridge_algorithm, 
        jet_R
    );

    if (calc_areas) {
        AreaDefinition area_def( 
            active_area_explicit_ghosts, 
            GhostedAreaSpec(ghost_max_rap, 1, ghost_R)
        );
        fastjet::ClusterSequenceArea clustSeq(v_input, jet_def, area_def);
        vector<PseudoJet> jets = sorted_by_pt( selection( clustSeq.inclusive_jets(min_jet_pt) ));
        njets = (jets.size() > max_njets) ? max_njets : jets.size();
        // fill clones
        for (unsigned int i{0}; i<njets; ++i) {
            JetwArea* jet = (JetwArea*) clones.ConstructedAt(i);
            jet->area = jets[i].area();
            jet->pt = jets[i].perp();
            jet->eta = jets[i].eta();
            jet->phi = jets[i].phi();
        }

        // Get rho and rho_sigma
        JetDefinition jet_def_bkgd(kt_algorithm, jet_R); // <--
        Selector selector_rm2 = SelectorAbsEtaMax(1.0) * (!SelectorNHardest(2)); // <--
        fastjet::JetMedianBackgroundEstimator bge_rm2 {selector_rm2, jet_def_bkgd, area_def};
        bge_rm2.set_particles(v_input);
        rho = bge_rm2.rho();
        rho_sigma = bge_rm2.sigma();
    } else {
        fastjet::ClusterSequence clustSeq(v_input, jet_def);
        vector<PseudoJet> jets = sorted_by_pt( selection( clustSeq.inclusive_jets(min_jet_pt) ));
        njets = (jets.size() > max_njets) ? max_njets : jets.size();
        // fill clones
        for (unsigned int i{0}; i<njets; ++i) {
            Jet* jet = (Jet*) clones.ConstructedAt(i);
            jet->pt = jets[i].perp();
            jet->eta = jets[i].eta();
            jet->phi = jets[i].phi();
        }
    }
};

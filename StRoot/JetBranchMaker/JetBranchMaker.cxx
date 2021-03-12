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
    unsigned int  _max_nconstituents,
    double _jet_R,
    double _ghost_R,
    double _ghost_max_rap,
    double _max_abs_eta_jet )
:
    calc_areas { _calc_areas },
    max_njets  { _max_njets },
    max_nconstituents  { _max_nconstituents },
    jet_R      { _jet_R },
    max_abs_eta_jet { _max_abs_eta_jet < 0 ? 1.-jet_R:_max_abs_eta_jet },
    /* max_abs_eta_jet { 1.0 }, */
    ghost_R    { _ghost_R },
    ghost_max_rap { _ghost_max_rap },
	min_jet_pt {0.2},
    /* n_tracks   {0}, */
    /* n_towers   {0}, */
    /* n_particles {0}, */
    clones     { calc_areas ? "JetwArea" : "JetnoArea", static_cast<int>(max_njets) }
    /* tower_indices { "constituent_index", static_cast<int>(max_nconstituents) } */
    /* track_indices { "constituent_index", static_cast<int>(max_nconstituents) } */
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

    /* tree->Branch(Form("%s_itow", name_tag), &itow); */
    /* tree->Branch(Form("%s_i0tow", name_tag), &i0tow); */
    /* tree->Branch(Form("%s_i1tow",  name_tag),&i1tow); */

    /* tree->Branch(Form("%s_itrk",  name_tag),&itrk); */
    /* tree->Branch(Form("%s_i0trk", name_tag), &i0trk); */
    /* tree->Branch(Form("%s_i1trk",  name_tag),&i1trk); */

    /* tree->Branch(Form("%s_njets", name_tag), &njets); */
    tree->Branch(name_tag, &clones);


    /* tower_indices = 0; */
    /* track_indices = 0; */

    /* vector<short> tower_indices; */
    /* vector<short> track_indices; */

    /* tree->Branch(Form("%s_iTow",name_tag),&tower_indices, name_tag); */
    /* tree->Branch(Form("%s_iTrk",name_tag),&track_indices, name_tag); */

    /* tree->Branch(Form("%s_tow_index", name_tag, &tower_indices); */
    if (calc_areas) {
        tree->Branch( Form("%s_rho", name_tag), &rho);
        tree->Branch( Form("%s_rho_sigma", name_tag), &rho_sigma);
    } 
};

void JetBranchMaker::clear() {
    clones.Clear();
    /* clones_index.Clear(); */
    /* n_tracks = 0; */
    /* n_towers = 0; */
    /* n_particles = 0; */
    particles.clear();

    /* itow.clear(); */
    /* i0tow.clear(); */
    /* i1tow.clear(); */

    /* itrk.clear(); */
    /* i0trk.clear(); */
    /* i1trk.clear(); */
};

// NOTE:
// assumed that tracks filled first, and then towers second,
// and that they are filled in the same order as written to
// the trees.

void JetBranchMaker::fill(vector<pair<Jet,int>>& v_part) {
    for (auto& p : v_part) particles.push_back(p);
};

void JetBranchMaker::fill(pair<Jet,int> p) {
    particles.push_back(p);
};

void JetBranchMaker::generate(){
    // transfer each Jet into a Pseudojet
    int index{0};
    vector<PseudoJet> v_particles;
    for (auto& pp : particles) {
        auto& p = pp.first;
        v_particles.push_back(PseudoJet());
        v_particles[index].reset_PtYPhiM( p.pt, p.eta, p.phi, PI_MASS );
        v_particles[index].set_user_index(pp.second);
        /* cout << " set: " << pp.second << endl; */
        ++index;
    }
   
    fastjet::Selector      jetrap         = fastjet::SelectorAbsEtaMax(max_abs_eta_jet);
    fastjet::Selector      not_pure_ghost = !SelectorIsPureGhost();
    fastjet::Selector      selection      = jetrap && not_pure_ghost;


    fastjet::JetDefinition jet_def ( 
        jet_algo == antikt ? antikt_algorithm :
        jet_algo == kt     ? kt_algorithm     :
                             cambridge_algorithm, 
        jet_R
    );

    // unzip the jet and make the constituent_index clones, too
    if (calc_areas) {
        AreaDefinition area_def( 
            active_area_explicit_ghosts, 
            GhostedAreaSpec(ghost_max_rap, 1, ghost_R)
        );
        fastjet::ClusterSequenceArea clustSeq(v_particles, jet_def, area_def);
        vector<PseudoJet> jets = sorted_by_pt( selection( clustSeq.inclusive_jets(min_jet_pt) ));
        unsigned int njets = (jets.size() > max_njets) ? max_njets : jets.size();

        // fill clones
        /* int cnt_tracks = 0; */
        /* int cnt_towers = 0; */
        /* int ntrk = 0; */
        /* int ntow = 0; */
        for (unsigned int i{0}; i<njets; ++i) {
            /* i0tow.push_back(ntow); */
            /* i0trk.push_back(ntrk); */

            /* cout << " jet: " << i << endl; */
            JetwArea* jet = (JetwArea*) clones.ConstructedAt(i);
            jet->clear(); // required to clear the internal indicees
            jet->area = jets[i].area();
            jet->pt   = jets[i].perp();
            jet->eta  = jets[i].eta();
            jet->phi  = jets[i].phi();

            // look at the pieces of jet[i]:
            auto pieces = jets[i].constituents();
            /* int ntow = 0; */
            for (auto& p : pieces) {
                if (p.pt() >= 0.2) {
                    short index = p.user_index() ;
                    if (index<0) {
                        jet->index_tower.push_back(-index-1);
                        /* itow.push_back(-index-1); */
                        /* ++ntow; */
                    } else {
                        /* cout << " out: " << index << endl; */
                        jet->index_track.push_back(index);
                        /* itrk.push_back(index); */
                        /* ++ntrk; */
                    }
                } else {
                    /* cout << " GHOST " << endl; */
                }
            };
            /* i1tow.push_back(ntow); */
            /* i1trk.push_back(ntrk); */
        }


        // Get rho and rho_sigma
        JetDefinition jet_def_bkgd(kt_algorithm, jet_R); // <--
        Selector selector_rm2 = SelectorAbsEtaMax(1.0) * (!SelectorNHardest(2)); // <--
        fastjet::JetMedianBackgroundEstimator bge_rm2 {selector_rm2, jet_def_bkgd, area_def};
        bge_rm2.set_particles(v_particles);
        rho = bge_rm2.rho();
        rho_sigma = bge_rm2.sigma();
    } else {
        fastjet::ClusterSequence clustSeq(v_particles, jet_def);
        vector<PseudoJet> jets = sorted_by_pt( selection( clustSeq.inclusive_jets(min_jet_pt) ));
        unsigned int njets = (jets.size() > max_njets) ? max_njets : jets.size();
        // fill clones
        /* int ntrk = 0; */
        /* int ntow = 0; */
        for (unsigned int i{0}; i<njets; ++i) {
            JetnoArea* jet = (JetnoArea*) clones.ConstructedAt(i);
            jet->clear();
            jet->pt = jets[i].perp();
            jet->eta = jets[i].eta();
            jet->phi = jets[i].phi();
            auto pieces = jets[i].constituents();
            for (auto& p : pieces) {
                if (p.pt() >= 0.2) {
                    short index = p.user_index() ;
                    if (index<0) {
                        jet->index_tower.push_back(-index-1);
                    } else {
                        jet->index_track.push_back(index);
                    }
                } else {
                    /* cout << " GHOST " << endl; */
                }
            };
        }
    }
};

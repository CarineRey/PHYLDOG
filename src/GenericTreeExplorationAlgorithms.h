/*
Copyright or © or Copr. Centre National de la Recherche Scientifique
contributor : Bastien Boussau (2009-2013)

bastien.boussau@univ-lyon1.fr

This software is a bioinformatics computer program whose purpose is to
simultaneously build gene and species trees when gene families have
undergone duplications and losses. It can analyze thousands of gene
families in dozens of genomes simultaneously, and was presented in
an article in Genome Research. Trees and parameters are estimated
in the maximum likelihood framework, by maximizing theprobability
of alignments given the species tree, the gene trees and the parameters
of duplication and loss.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef _GENERICTREEEXPLORATIONALGORITHM_H_
#define _GENERICTREEEXPLORATIONALGORITHM_H_

// From PhylLib:
#include <Bpp/Phyl/Tree.h>
#include <Bpp/Phyl/App/PhylogeneticsApplicationTools.h>
#include <Bpp/Phyl/Node.h>
//#include <Bpp/Phyl/Model/JCnuc.h>
#include <Bpp/Phyl/Model/Nucleotide/JCnuc.h>
#include <Bpp/Phyl/Distance/BioNJ.h>
#include <Bpp/Phyl/Distance/DistanceEstimation.h>

// From NumCalc:
#include <Bpp/Numeric/VectorTools.h>
#include <Bpp/Numeric/Random/RandomTools.h>
#include <Bpp/Numeric/NumConstants.h>
#include <Bpp/Numeric/Prob/ConstantDistribution.h>


//From Seqlib:
#include <Bpp/Seq/Alphabet/DNA.h>


#include "ReconciliationTools.h"


using namespace bpp;
using namespace std;


void changeRoot(TreeTemplate<Node> &tree, int newOutGroup);
std::vector<Node*>  makeSPR(TreeTemplate<Node> &tree, 
                            int cutNodeId, int newBrotherId, 
                            bool verbose = true, 
                            bool returnNodesToUpdate = false);
void makeMuffatoSPR(TreeTemplate<Node> &tree, 
             Node* cutNode, 
             Node* newFather, 
             bool verbose = true) ;

void makeNNI(TreeTemplate<Node> &tree, int nodeId);
void buildVectorOfRegraftingNodes(TreeTemplate<Node> &tree, int nodeForSPR, std::vector <int> & nodeIdsToRegraft);
std::vector<int> getRemainingNeighbors(const Node * node1, const Node * node2);
void getRemainingNeighborsUntilDistance(const Node * node1, const Node * node2, int distance, int d, std::vector <int> & neighbors);
void getNeighboringNodesIdLimitedDistance (TreeTemplate<Node> &tree, int nodeId, int distance, std::vector <int> & neighboringNodeIds);
//void getRemainingNeighborsUntilDistanceLowerNodes(TreeTemplate<Node> &tree, const Node * node1, const Node * node2, int distance, int d, std::vector <int> & neighbors);
void getNeighboringNodesIdLimitedDistanceLowerNodes (TreeTemplate<Node> &tree, int nodeId, int distance, std::vector <int> & neighboringNodeIds);
void buildVectorOfRegraftingNodesLimitedDistance(TreeTemplate<Node> &tree, int nodeForSPR, int distance, std::vector <int> & nodeIdsToRegraft);
void buildVectorOfRegraftingNodesLimitedDistanceLowerNodes(TreeTemplate<Node> &tree, int nodeForSPR, int distance, std::vector <int> & nodeIdsToRegraft);
void makeDeterministicModifications(TreeTemplate<Node> &tree, size_t & nodeForNNI, size_t & nodeForSPR, size_t & nodeForRooting);
void makeDeterministicNNIsAndRootChangesOnly(TreeTemplate<Node> &tree, size_t & nodeForNNI, size_t & nodeForRooting, const bool fixedOutgroupSpecies_) ;
bool checkChangeHasNotBeenDone(TreeTemplate<Node> &tree, TreeTemplate<Node> *bestTree, size_t & nodeForNNI, 
                               size_t & nodeForRooting, std::vector < double >  &NNILks, 
                               std::vector < double >  &rootLks);
void dropLeaves(TreeTemplate<Node> & tree, const std::vector<string> &spToDrop);
Tree* MRP(const vector<Tree*>& vecTr);
void rootTreeWithOutgroup (TreeTemplate<Node> &tree, 
						   const std::vector<std::string> outgroupTaxa) throw ( TreeException );
bool isTreeRootedWithOutgroup (const TreeTemplate<Node> &tree, const std::vector<std::string> outgroupTaxa) ;
#endif 

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



extern "C" {
#include <pll/pll.h>
}

#include <iostream>
#include <string>
#include <fstream>
#include <boost/graph/graph_traits.hpp>

#include <Bpp/Seq/Container/SiteContainerTools.h>
#include <Bpp/Phyl/Node.h>
#include <Bpp/Phyl/TreeTemplateTools.h>


#include "LikelihoodEvaluator.h"
#include "ReconciliationTools.h"




using namespace std;
using namespace bpp;


void LikelihoodEvaluator::PLL_initializePLLInstance(){
  
  /* Set the PLL instance attributes */
  PLL_attributes.rateHetModel     = PLL_GAMMA;
  PLL_attributes.fastScaling      = PLL_TRUE;
  PLL_attributes.saveMemory       = PLL_FALSE;
  PLL_attributes.useRecom         = PLL_FALSE;
  PLL_attributes.randomNumberSeed = 0xDEADBEEF;
  PLL_attributes.numberOfThreads  = 8;            /* This only affects the pthreads version */
  
  PLL_instance = pllCreateInstance (&PLL_attributes);
}


LikelihoodEvaluator::LikelihoodEvaluator(map<string, string> params):
  params(params)
{
  initialized=false;
  
  string methodString = ApplicationTools::getStringParameter("likelihood.evaluator",params,"PLL");
  method = (methodString == "PLL"? PLL:BPP);
  
  // loading data, imported from GeneTreeLikelihood (Bastien)
  
  std::vector <std::string> spNames;
  bool cont = false;
  alphabet =  getAlphabetFromOptions(params, cont);

  if (!cont)
    throw(Exception("Unable to load this family"));
 
  sites = getSequencesFromOptions(params, alphabet, cont);

  if (!cont)
    throw(Exception("Unable to load this family"));
 
  substitutionModel = getModelFromOptions(params, alphabet, sites, cont);

  if (!cont)
    throw(Exception("Unable to load this family"));
    
//   if (substitutionModel->getName() != "RE08")
//     SiteContainerTools::changeGapsToUnknownCharacters(*sites, cont);
 
  if (!cont)
    throw(Exception("Unable to load this family"));
 
  rateDistribution = getRateDistributionFromOptions(params, substitutionModel, cont);
  
  try 
  {
    bool cont = true;
    tree = getTreeFromOptions(params, alphabet, sites, substitutionModel, rateDistribution, cont);
  }
  catch (std::exception& e)
  {
    std::cout << e.what() <<"; Unable to get a proper gene tree for family <<file<< avoiding this family."<<std::endl;
    cont=false;
  }
    
}

// TODO: is clonable?
// LikelihoodEvaluator::LikelihoodEvaluator(LikelihoodEvaluator &levaluator){
//   nniLk = levaluator.nniLk->clone();
//   tree = levaluator.tree->clone();
// }

void LikelihoodEvaluator::PLL_loadAlignment(string path)
{
  /* Parse a PHYLIP/FASTA file */
  pllAlignmentData * alignmentData = pllParseAlignmentFile (PLL_FORMAT_FASTA, path.c_str());
  if (!alignmentData)
  {
    throw Exception("PLL: Error while parsing " + path);
  }
}


void LikelihoodEvaluator::PLL_loadNewick_fromFile(string path)
{
  PLL_newick = pllNewickParseFile(path.c_str());
  if (!PLL_newick)
  {
    throw Exception("PLL: Error while parsing newick file");
  }
  if (!pllValidateNewick (PLL_newick))  /* check whether the valid newick tree is also a tree that can be processed with our nodeptr structure */
  {
    throw Exception("PLL: Invalid phylogenetic tree.");
  }
}

void LikelihoodEvaluator::PLL_loadNewick_fromString(string newick)
{
  PLL_newick = pllNewickParseString (newick.c_str());
  if (!PLL_newick)
  {
    throw Exception("PLL: Error while parsing newick string: " + newick);
  }
  if (!pllValidateNewick (PLL_newick))  /* check whether the valid newick tree is also a tree that can be processed with our nodeptr structure */
  {
    throw Exception("PLL: Invalid phylogenetic tree.");
  }
}


void LikelihoodEvaluator::PLL_loadPartitions(string path)
{
  /* Parse the partitions file into a partition queue structure */
  PLL_partitionInfo = pllPartitionParse (path.c_str());
  
  /* Validate the partitions */
  if (!pllPartitionsValidate (PLL_partitionInfo, PLL_alignmentData))
  {
    throw Exception("PLL: Partitions do not cover all sites.");
  }
  
  /* Commit the partitions and build a partitions structure */
  PLL_partitions = pllPartitionsCommit (PLL_partitionInfo, PLL_alignmentData);
  
  /* We don't need the the intermedia partition queue structure anymore */
  pllQueuePartitionsDestroy (&PLL_partitionInfo);
  
  /* eliminate duplicate sites from the alignment and update weights vector */
  pllAlignmentRemoveDups (PLL_alignmentData, PLL_partitions);
}

void LikelihoodEvaluator::PLL_connectTreeAndAlignment()
{
  pllTreeInitTopologyNewick (PLL_instance, PLL_newick, PLL_FALSE);
    
  // cout << "PLL: Connect the alignment and partition structure with the tree structure" << std::endl ;
  /* Connect the alignment and partition structure with the tree structure */
  if (!pllLoadAlignment (PLL_instance, PLL_alignmentData, PLL_partitions, PLL_DEEP_COPY))
  {
    throw Exception("PLL: Incompatible tree/alignment combination.");
  }
}


void LikelihoodEvaluator::initialize_BPP_nniLk()
{
  nniLk = new NNIHomogeneousTreeLikelihood(*tree, *sites, substitutionModel, rateDistribution, mustUnrootTrees, verbose);
  
  nniLk->initParameters();
  nniLk->initialize();
}

void LikelihoodEvaluator::initialize_PLL()
{
  // #1 PREPARING
  // must have the strict names loaded
  loadStrictNamesFromAlignment_forPLL();
  writeAlignmentFilesForPLL();
  
  // preparing the tree
  
  // PLL process
  PLL_initializePLLInstance();
  PLL_loadAlignment(fileNamePrefix + "alignment.fasta");
  PLL_loadPartitions(fileNamePrefix + "partition.txt");
  
  logLikelihood = PLL_evaluate(tree);
  
}

double LikelihoodEvaluator::PLL_evaluate(TreeTemplate<Node>* treeToEvaluate)
{
  // preparing the tree
  TreeTemplate<Node>* treeForPLL = treeToEvaluate->clone();
  convertTreeToStrict(treeForPLL);
  
  // getting the root
  bool wasRooted = (treeForPLL->isRooted() ? true : false);
  set<Node*> leaves1, leaves2;
  if(wasRooted)
  {
    Node* root = treeForPLL->getRootNode();
    Node* son1 = root->getSon(0);
    Node* son2 = root->getSon(1);
    vector<Node*> leaves1Vector = TreeTemplateTools::getLeaves<Node>(*son1);
    vector<Node*> leaves2Vector = TreeTemplateTools::getLeaves<Node>(*son1);
    leaves1.insert(leaves1Vector.begin(),leaves1Vector.end());
    leaves2.insert(leaves2Vector.begin(),leaves2Vector.end());
    treeForPLL->unroot();
  }
  
  
  Newick newickForPll;
  stringstream newickStingForPll;
  newickForPll.write(*treeToEvaluate,newickStingForPll);
  PLL_loadNewick_fromString(newickStingForPll.str());
  delete treeForPLL;
  
  // processing by PLL
  PLL_connectTreeAndAlignment();
  pllInitModel(PLL_instance, PLL_partitions, PLL_alignmentData);
  
  // getting the new tree with now branch lengths
  newickStingForPll.str(PLL_instance->tree_string);
  delete treeToEvaluate;
  treeToEvaluate = newickForPll.read(newickStingForPll);
  
  
  //re-rooting if needed
  if(wasRooted)
  {
    // the plyogenetic root is between the current topological
    // root and one of the three sons
    // so, one of the three sons of a root is the outgroup
    vector<Node*> rootSons = treeToEvaluate->getRootNode()->getSons();
    for(vector<Node*>::iterator currSon = rootSons.begin(); currSon != rootSons.end(); currSon++)
    {
      vector<Node*> currLeavesVector = TreeTemplateTools::getLeaves<Node>(**currSon);
      set<Node*> currLeavesSet(currLeavesVector.begin(),currLeavesVector.end());
      if(currLeavesSet == leaves1 || currLeavesSet == leaves2)
        treeToEvaluate->newOutGroup(*currSon);
    }
    if(!treeToEvaluate->isRooted())
      throw Exception("Unable to re-root the tree.");
    
  }
  
  restoreTreeFromStrict(treeToEvaluate);
  
  return(PLL_instance->likelihood);
}


// CLONABLE?
// LikelihoodEvaluator* LikelihoodEvaluator::clone()
// {
//   return(new LikelihoodEvaluator(this));
// }


LikelihoodEvaluator::~LikelihoodEvaluator()
{
  if(method == PLL){
    delete tree;
  }
  else
  {
    delete nniLk;
    if(nniLkAlternative)
      delete nniLkAlternative;
    //TODO delete all the trees, etc
  }
}

void LikelihoodEvaluator::initialize()
{
  // ### common requirements for initialization
  // make a name for this family from the alignemnt file name
  istringstream ss_alignFile(ApplicationTools::getStringParameter("likelihood.evaluator",params,"PLL"));
  while(getline(ss_alignFile,name,'/'))
    ; // do nothing
  
  if(method == PLL)
    initialize_PLL();
  else
    initialize_BPP_nniLk();
  
  //
  initialized = true;
}


void LikelihoodEvaluator::setAlternativeTree(TreeTemplate< Node >* newAlternative)
{
  if(alternativeTree != 00)
    delete alternativeTree;
  alternativeTree = newAlternative->clone();
  if(method == PLL){
    alternativeLogLikelihood = PLL_evaluate(alternativeTree);
  }
  else
  {
    delete nniLkAlternative;
    nniLkAlternative =  new NNIHomogeneousTreeLikelihood (*alternativeTree, *sites, substitutionModel, rateDistribution, mustUnrootTrees, verbose);
    alternativeLogLikelihood = nniLkAlternative->getLogLikelihood();
  }
}

void LikelihoodEvaluator::acceptAlternativeTree()
{
  delete tree;
  tree = alternativeTree->clone();
  logLikelihood = alternativeLogLikelihood;
  if(method == BPP)
  {
    delete nniLk;
    nniLk = nniLkAlternative->clone();
  }
}





TreeTemplate< Node >* LikelihoodEvaluator::getAlternativeTree()
{
  return alternativeTree;
}

bool LikelihoodEvaluator::isInitialized()
{
  return initialized;
}

Alphabet* LikelihoodEvaluator::getAlphabet()
{
  return alphabet;
}

double LikelihoodEvaluator::getAlternativeLogLikelihood()
{
  return alternativeLogLikelihood;
}

double LikelihoodEvaluator::getLogLikelihood()
{
  return logLikelihood;
}

DiscreteDistribution* LikelihoodEvaluator::getRateDistribution()
{
  return rateDistribution;
}

VectorSiteContainer* LikelihoodEvaluator::getSites()
{
  return sites;
}

TreeTemplate< Node >* LikelihoodEvaluator::getTree()
{
  return tree;
}

SubstitutionModel* LikelihoodEvaluator::getSubstitutionModel()
{
  return substitutionModel;
}


void LikelihoodEvaluator::loadStrictNamesFromAlignment_forPLL()
{
  vector<string> seqNames = sites->getSequencesNames();
  string currStrictName, currName;
  for(unsigned int currIdx = 0; currIdx != seqNames.size(); currIdx++)
  {
    currName = seqNames.at(currIdx);
    currStrictName = "seq" + currIdx;
    realToStrict[currName] = currStrictName;
    strictToReal[currStrictName] = currName;
  }
  
}

void LikelihoodEvaluator::convertTreeToStrict(TreeTemplate< Node >* targetTree)
{
  vector<Node*> leaves = targetTree->getLeaves();
  for(vector<Node*>::iterator currLeaf = leaves.begin(); currLeaf != leaves.end(); currLeaf++){
    (*currLeaf)->setName(realToStrict[(*currLeaf)->getName()]);
  }
}

void LikelihoodEvaluator::restoreTreeFromStrict(TreeTemplate< Node >* targetTree)
{
  vector<Node*> leaves = targetTree->getLeaves();
  for(vector<Node*>::iterator currLeaf = leaves.begin(); currLeaf != leaves.end(); currLeaf++){
    (*currLeaf)->setName(strictToReal[(*currLeaf)->getName()]);
  }
}


void LikelihoodEvaluator::writeAlignmentFilesForPLL()
{
  fileNamePrefix = name + "_" ;
  ofstream alignementFile(string(fileNamePrefix + "alignment.fasta").c_str(), ofstream::out);
  
  //preparing the file for the alignment
  BasicSequence currSequence(sites->getAlphabet());
  for(unsigned int currSeqIndex = 0; currSeqIndex != sites->getNumberOfSequences(); currSeqIndex++)
  {
    currSequence = sites->getSequence(currSeqIndex);
    alignementFile << ">" << realToStrict[currSequence.getName()] << "\n" << currSequence.toString() << "\n";
  }
  alignementFile.close();
  
  ofstream partitionFile(string(fileNamePrefix + "partition.txt").c_str(), ofstream::out);
  //TODO: take into account the alphabet
  partitionFile << "DNA, p1=1-" << sites->getNumberOfSites() << "\n";
  partitionFile.close();
}

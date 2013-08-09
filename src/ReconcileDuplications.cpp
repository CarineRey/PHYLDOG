//
// File: ReconcileDuplications.cpp
// Created by: Bastien Boussau
// Created on: Aug 04 23:36 2007
//
/*
Copyright or © or Copr. CNRS

This software is a computer program whose purpose is to estimate
phylogenies and evolutionary parameters from a dataset according to
the maximum likelihood principle.

This software is governed by the CeCILL  license under French law and
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

// From the STL:
#include <iostream>
#include <iomanip>
#include <algorithm>

// From SeqLib:
#include <Bpp/Seq/Alphabet/Alphabet.h>
#include <Bpp/Seq/Container/VectorSiteContainer.h>
#include <Bpp/Seq/Container/SiteContainerTools.h>
#include <Bpp/Seq/SiteTools.h>
#include <Bpp/Seq/SequenceTools.h>
#include <Bpp/Seq/App/SequenceApplicationTools.h>

// From PhylLib:
/*#include <Bpp/Phyl/Tree.h>
#include <Bpp/Phyl/Likelihood/DiscreteRatesAcrossSitesTreeLikelihood.h>
#include <Bpp/Phyl/Likelihood/HomogeneousTreeLikelihood.h>
#include <Bpp/Phyl/Likelihood/DRHomogeneousTreeLikelihood.h>
//#include <Phyl/NNIHomogeneousTreeLikelihood.h>
#include <Bpp/Phyl/Likelihood/ClockTreeLikelihood.h>
#include <Bpp/Phyl/PatternTools.h>
#include <Bpp/Phyl/App/PhylogeneticsApplicationTools.h>
#include <Bpp/Phyl/Likelihood/MarginalAncestralStateReconstruction.h>
#include <Bpp/Phyl/OptimizationTools.h>
#include <Bpp/Phyl/Likelihood/RASTools.h>
#include <Bpp/Phyl/Io/Newick.h>
#include <Bpp/Phyl/TreeTools.h>
#include <Bpp/Phyl/Distance/BioNJ.h>
#include <Bpp/Phyl/OptimizationTools.h>
*/
#include <Bpp/Phyl/Io/Nhx.h>


// From NumCalc:
/*#include <Bpp/Numeric/Prob/DiscreteDistribution.h>
#include <Bpp/Numeric/Prob/ConstantDistribution.h>
#include <Bpp/Numeric/DataTable.h>
#include <Bpp/Numeric/Matrix/MatrixTools.h>
#include <Bpp/Numeric/VectorTools.h>
#include <Bpp/Numeric/AutoParameter.h>
#include <Bpp/Numeric/Random/RandomTools.h>
#include <Bpp/Numeric/NumConstants.h>
#include <Bpp/Numeric/Function/PowellMultiDimensions.h>
*/
// From Utils:
/*#include <Bpp/Utils/AttributesTools.h>
#include <Bpp/App/ApplicationTools.h>
#include <Bpp/Io/FileTools.h>
#include <Bpp/Clonable.h>
#include <Bpp/Numeric/Number.h>
#include <Bpp/BppString.h>
#include <Bpp/Text/KeyvalTools.h>
#include <Bpp/Text/TextTools.h>
*/

//#include <Phyl/SAHomogeneousTreeLikelihood.h>
//#include "ReconciliationTools.h"
//#include "ReconciliationTreeLikelihood.h"
#include "GeneTreeLikelihood.h"
#include "DLGeneTreeLikelihood.h"
#include "COALGeneTreeLikelihood.h"

//#include "SpeciesTreeExploration.h"
#include "SpeciesTreeLikelihood.h"
#include "GeneTreeAlgorithms.h"



//From the BOOST library 
#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>
namespace mpi = boost::mpi;

//#include "mpi.h" 
//using namespace std;
//using namespace bpp;




/*
This program takes a species tree file, sequence alignments and files describing the relations between sequences and species.
To compile it, use the Makefile !
*/

/******************************************************************************/

void help()
{
    (*ApplicationTools::message << "__________________________________________________________________________").endLine();
    (*ApplicationTools::message << "phyldog parameter1_name=parameter1_value parameter2_name=parameter2_value"   ).endLine();
    (*ApplicationTools::message << "      ... param=option_file").endLine();
    (*ApplicationTools::message << "Example of some options: ").endLine();
    (*ApplicationTools::message).endLine();
    
    /*SequenceApplicationTools::printInputAlignmentHelp();
     PhylogeneticsApplicationTools::printInputTreeHelp();
     PhylogeneticsApplicationTools::printSubstitutionModelHelp();
     PhylogeneticsApplicationTools::printRateDistributionHelp();
     PhylogeneticsApplicationTools::printCovarionModelHelp();
     PhylogeneticsApplicationTools::printOptimizationHelp(true, false);
     PhylogeneticsApplicationTools::printOutputTreeHelp();*/
    //  (*ApplicationTools::message << "output.infos                      | file where to write site infos").endLine();
    //  (*ApplicationTools::message << "output.estimates                  | file where to write estimated parameter values").endLine();
    (*ApplicationTools::message << "PATH                                 | path to the directory containing input and output files").endLine();
    (*ApplicationTools::message << "init.species.tree                    | user or random or mrp").endLine();
    (*ApplicationTools::message << "species.tree.file                    | if the former is set to \"user\", path to a species tree" ).endLine();
    (*ApplicationTools::message << "species.names.file                   | if instead it was set to \"random\", or \"mrp\" path to a list of species names ").endLine();
    (*ApplicationTools::message << "starting.tree.file                   | file where to write the initial species tree").endLine();
    (*ApplicationTools::message << "output.tree.file                     | file where to write the end species tree").endLine();
    (*ApplicationTools::message << "output.temporary.tree.file           | file where to write the temporary species tree in case the job is ended because of time constraints.").endLine();

    //  (*ApplicationTools::message << "heuristics.level                  | 0, 1, 2, or 3; 0: DR exact algorithm (default, best); 1 fast heuristics; 2: exhaustive and fast; 3: exhaustive and slow").endLine();
    //(*ApplicationTools::message << "species.id.limit.for.root.position| Threshold for trying root positions").endLine();
    (*ApplicationTools::message << "genelist.file                        | file containing a list of gene option files to analyse").endLine();
    (*ApplicationTools::message << "branch.expected.numbers.optimization | average, branchwise, average_then_branchwise or no: how we optimize duplication and loss probabilities").endLine();
    (*ApplicationTools::message << "genome.coverage.file                 | file giving the percent coverage of the genomes used").endLine();
    (*ApplicationTools::message << "spr.limit                            | integer giving the breadth of SPR movements, in number of nodes. 0.1* number of nodes in the species tree might be OK.").endLine();
    (*ApplicationTools::message << "reconciliation.model                 | 'DL' or 'COAL' giving the type of model to reconcile gene trees against the species tree.").endLine();

    (*ApplicationTools::message << "  Refer to the README file or the Bio++ Program Suite Manual for a list of supplementary options.").endLine();
    (*ApplicationTools::message << "__________________________________________________________________________").endLine();
}


/******************************************************************************/
/***************************Adding two std::vectors*********************************/


std::vector <int> operator + (std::vector <int> x, std::vector <int> y) {
	unsigned int temp=x.size();
	if (temp!=y.size()) {
		std::cout <<"problem : adding two std::vectors of unequal sizes."<<std::endl;
        MPI::COMM_WORLD.Abort(1);
		exit (-1);
	}
 std::vector<int> result;
	for(unsigned int i = 0 ; i<temp ; i++){
		result.push_back(x[i]+y[i]);
	}
	return result;
}

std::vector <double> operator + (std::vector <double> x, std::vector <double> y) {
	unsigned int temp=x.size();
	if (temp!=y.size()) {
		std::cout <<"problem : adding two std::vectors of unequal sizes."<<std::endl;
        MPI::COMM_WORLD.Abort(1);
		exit (-1);
	}
 std::vector<double> result;
	for(unsigned int i = 0 ; i<temp ; i++){
		result.push_back(x[i]+y[i]);
	}
	return result;
}

/******************************************************************************/
// This function parses a vector of gene family files, 
// discards the ones that do not pass certain criteria, 
// and initializes the others. Used by clients.
/******************************************************************************/


void parseAssignedGeneFamilies(const mpi::communicator & world, 
                               unsigned int & server, 
                               unsigned int & rank,
                               std::vector<std::string> & assignedFilenames, 
                               std::map<std::string, std::string> & params, 
                               unsigned int & numDeletedFamilies, TreeTemplate<Node> * geneTree, 
                               TreeTemplate<Node>* &tree, std::vector <int> & num0Lineages, 
                               std::vector <std::vector<int> > & allNum0Lineages, 
                               std::vector <std::vector<int> > & allNum1Lineages, 
                               std::vector <std::vector<int> > & allNum2Lineages, 
                               std::vector <double> & lossExpectedNumbers, 
                               std::vector <double> & duplicationExpectedNumbers,
                               std::vector < std::vector < std::vector < std::vector <unsigned int> > > > coalCounts,
                               std::vector <double> & coalBls,
                               std::vector <std::vector<unsigned int> > & allNum12Lineages, 
                               std::vector <std::vector<unsigned int> > & allNum22Lineages, 
                               std::map <std::string, int> & spId, 
                               int speciesIdLimitForRootPosition, 
                               int heuristicsLevel, int MLindex, 
                               std::vector <double> & allLogLs, 
                               std::vector <GeneTreeLikelihood *> & treeLikelihoods,
                               std::vector <GeneTreeLikelihood *> & backupTreeLikelihoods,
                               std::vector <std::map<std::string, std::string> > & allParams, 
                               std::vector <Alphabet *> & allAlphabets, 
                               std::vector <VectorSiteContainer *> & allDatasets, 
                               std::vector <SubstitutionModel *> & allModels, 
                               std::vector <DiscreteDistribution *> & allDistributions, 
                               std::vector <TreeTemplate<Node> *> & allGeneTrees, 
                               std::vector <TreeTemplate<Node> *> & allUnrootedGeneTrees, 
							   std::vector < std::map <std::string, std::string> >& allSeqSps, 
							   std::vector<unsigned int>& allSprLimitGeneTree,
                               string &reconciliationModel) 
{
  bool avoidFamily;
  std::string initTree;
  std::map<std::string, std::string> famSpecificParams;
  std::vector <std::string> spNames;

  //Here we are going to get all necessary information regarding all gene families the client is in charge of.
  for (unsigned int i = 0 ; i< assignedFilenames.size() ; i++) 
  { //For each file
      double startingFamilyTime = ApplicationTools::getTime();  
      std::cout <<"Examining family "<<assignedFilenames[i]<<std::endl;
      avoidFamily = false;
      std::string file =assignedFilenames[i];
      TreeTemplate<Node> * unrootedGeneTree = 0;
      SubstitutionModel*    model    = 0;
      DiscreteDistribution* rDist    = 0;
      
      if(!FileTools::fileExists(file))
      {
          std::cerr << "Error: Parameter file "<< file <<" not found." << std::endl;
          fflush(0);
          MPI::COMM_WORLD.Abort(1);
          exit(-1);
      }
      else
      {
          famSpecificParams = AttributesTools::getAttributesMapFromFile(file, "=");
          AttributesTools::resolveVariables(famSpecificParams);
      }
      
      AttributesTools::actualizeAttributesMap(params, famSpecificParams);
	  //COAL or DL?
	  reconciliationModel = ApplicationTools::getStringParameter("reconciliation.model", params, "DL", "", true, false);

      //Sequences and model of evolution
      Alphabet * alphabet = SequenceApplicationTools::getAlphabet(params, "", false);
      std::string seqFile = ApplicationTools::getStringParameter("input.sequence.file",params,"none");
      if(!FileTools::fileExists(seqFile))
      {
          std::cerr << "Error: Sequence file "<< seqFile <<" not found." << std::endl;
          MPI::COMM_WORLD.Abort(1);
          exit(-1);
      }
      VectorSiteContainer * allSites = SequenceApplicationTools::getSiteContainer(alphabet, params);       
      
      unsigned int numSites = allSites->getNumberOfSites();
      ApplicationTools::displayResult("Number of sequences", TextTools::toString(allSites->getNumberOfSequences()));
      ApplicationTools::displayResult("Number of sites", TextTools::toString(numSites));
	  
	  std::vector <std::string> seqsToRemove;
	  VectorSiteContainer * sites;
	  
	  if (numSites == 0 ) {
		  std::cout<<"WARNING: Discarding a family whose alignment is 0 site long: "<< seqFile <<std::endl;
		  avoidFamily = true;
          numDeletedFamilies = numDeletedFamilies+1;
		  delete allSites;   
	  }
	  if (!avoidFamily) {
		  unsigned int minPercentSequence = ApplicationTools::getIntParameter("sequence.removal.threshold",params,0);
		  unsigned int threshold = (int) ((double)minPercentSequence * (double)numSites / 100 );
		  
		  
		  if (minPercentSequence > 0) {
			  for ( int j = allSites->getNumberOfSequences()-1 ; j >= 0 ; j--) {
				  if (SequenceTools::getNumberOfCompleteSites(allSites->getSequence(j) ) < threshold ) {
					  ApplicationTools::displayResult("Removing a short sequence:", allSites->getSequence(j).getName()  );
					  // allSites->deleteSequence(i);
					  seqsToRemove.push_back(allSites->getSequence(j).getName());
				  }
			  }
		  }
		  
		  for (unsigned int j =0 ; j<seqsToRemove.size(); j++) 
		  {
			  std::vector <std::string> seqNames = allSites->getSequencesNames();
			  if ( VectorTools::contains(seqNames, seqsToRemove[j]) ) 
			  {
				  allSites->deleteSequence(seqsToRemove[j]);
			  }
			  else 
				  std::cout<<"Sequence "<<seqsToRemove[j] <<"is not present in the gene alignment."<<std::endl;
		  }
		  
		  
		  ApplicationTools::displayResult("# sequences post size-based removal:", TextTools::toString(allSites->getNumberOfSequences()));
		  seqsToRemove.clear();
		  
		  if (allSites->getNumberOfSequences() <= 1 ) {
			  std::cout << "Only one sequence left: discarding gene family "<< file<<std::endl;
			  avoidFamily = true;
			  numDeletedFamilies = numDeletedFamilies+1;
		  }
		  sites = SequenceApplicationTools::getSitesToAnalyse(*allSites, params);     
		  delete allSites;   
	  }
      
      //method to optimize the gene tree root; only useful if heuristics.level!=0.
      bool rootOptimization = false;
      
      /****************************************************************************
       //Then we need to get the file containing links between sequences and species.
       *****************************************************************************/
      std::string taxaseqFile = ApplicationTools::getStringParameter("taxaseq.file",params,"none");
      if (!avoidFamily) {
          if (taxaseqFile=="none" ){
              std::cout << "\n\nNo taxaseqfile was provided. Cannot compute a reconciliation between a species tree and a gene tree using sequences if the relation between the sequences and the species is not explicit !\n" << std::endl;
              std::cout << "phyldog species.tree.file=bigtree taxaseq.file=taxaseqlist gene.tree.file= genetree sequence.file=sequences.fa output.tree.file=outputtree\n"<<std::endl;
              MPI::COMM_WORLD.Abort(1);
              exit(-1);
          }
          if(!FileTools::fileExists(taxaseqFile))
          {
              std::cerr << "Error: taxaseqfile "<< taxaseqFile <<" not found." << std::endl;
              MPI::COMM_WORLD.Abort(1);
              exit(-1);
          }
      }
      //Getting the relations between species and sequence names
      //In this file, the format is expected to be as follows :
      /*
       SpeciesA:sequence1;sequence2
       SpeciesB:sequence5
       SpeciesC:sequence3;sequence4;sequence6
       ...
       */
      //We use a std::map to record the links between species names and sequence names
      //For one species name, we can have several sequence names
      std::map<std::string, std::deque<std::string> > spSeq;
      //We use another std::map to store the link between sequence and species.
      std::map<std::string, std::string> seqSp;
      
      std::ifstream inSpSeq (taxaseqFile.c_str());
      std::string line;
      if (!avoidFamily) {
          while(getline(inSpSeq,line)) {
              //We divide the line in 2 : first, the species name, second the sequence names
              StringTokenizer st1 (line, ":", true);
              //Then we divide the sequence names
              if (st1.numberOfRemainingTokens ()>1) {
                  StringTokenizer st2(st1.getToken(1), ";", true);
                  if (spSeq.find(st1.getToken(0)) == spSeq.end())
                      spSeq.insert( make_pair(st1.getToken(0),st2.getTokens()));
                  else {
                      for (unsigned int j = 0 ; j < (st2.getTokens()).size() ; j++)
                          spSeq.find(st1.getToken(0))->second.push_back(st2.getTokens()[j]);
                  }
              }
          }
      }
      //Printing the contents and building seqSp 
      //At the same time, we gather sequences we will have to remove from the 
      //alignment and from the gene tree
      std::vector <std::string> spNamesToTake = tree->getLeavesNames(); 
      if (!avoidFamily) {
          for(std::map<std::string, std::deque<std::string> >::iterator it = spSeq.begin(); it != spSeq.end(); it++){
              spNames.push_back(it->first);
              for( std::deque<std::string >::iterator it2 = (it->second).begin(); it2 != (it->second).end(); it2++){
                  seqSp.insert(make_pair(*it2, it->first));
              }
              if (!VectorTools::contains(spNamesToTake,it->first)) {
                  for( std::deque<std::string >::iterator it2 = (it->second).begin(); it2 != (it->second).end(); it2++){
                      std::cout<<"Removing sequence of species not considered"<<std::endl;
                      seqsToRemove.push_back(*it2);
                  }
              }
          }
      }
      std::map <std::string, std::string> spSelSeq;
      
      if (!avoidFamily) {
          //If we need to remove all sequences or all sequences except one, 
          //better remove the gene family
          if (seqsToRemove.size()>=sites->getNumberOfSequences()-1) {
              numDeletedFamilies = numDeletedFamilies+1;
              avoidFamily=true;
              std::cout <<"All or almost all sequences have been removed: avoiding family "<<assignedFilenames[i-numDeletedFamilies+1]<<std::endl;
          }
      }
      
      if (!avoidFamily) {
          //We need to prune the alignment so that they contain
          //only sequences from the species under study.
          for (unsigned int j =0 ; j<seqsToRemove.size(); j++) 
          {
              std::vector <std::string> seqNames = sites->getSequencesNames();
              if ( VectorTools::contains(seqNames, seqsToRemove[j]) ) 
              {
                  sites->deleteSequence(seqsToRemove[j]);
              }
              else 
                  std::cout<<"Sequence "<<seqsToRemove[j] <<"is not present in the gene alignment."<<std::endl;
          }
          
          /****************************************************************************
           //Then we need to get the substitution model.
           *****************************************************************************/

          model = PhylogeneticsApplicationTools::getSubstitutionModel(alphabet, sites, params); 

          if (model->getName() != "RE08") SiteContainerTools::changeGapsToUnknownCharacters(*sites);

          if (model->getNumberOfStates() > model->getAlphabet()->getSize())
          {
              // Markov-modulated Markov model!
              rDist = new ConstantDistribution(1.);
          }
          else
          {
              rDist = PhylogeneticsApplicationTools::getRateDistribution(params);
          }
          
          /****************************************************************************
           * Then we need to get the file containing the gene tree, 
           * or build the gene tree.
           *****************************************************************************/
          // Get the initial gene tree
          initTree = ApplicationTools::getStringParameter("init.gene.tree", params, "user", "", false, false);
          ApplicationTools::displayResult("Input gene tree", initTree);
          try 
          {
              if(initTree == "user")
              {
                  std::string geneTreeFile =ApplicationTools::getStringParameter("gene.tree.file",params,"none");
                  if (geneTreeFile=="none" )
                  {
                      std::cout << "\n\nNo Gene tree was provided. The option init.gene.tree is set to user (by default), which means that the option gene.tree.file must be filled with the path of a valid tree file. \nIf you do not have a gene tree file, the program can start from a random tree, if you set init.gene.tree at random, or can build a gene tree with BioNJ or a PhyML-like algorithm with options bionj or phyml.\n\n" << std::endl;
                      MPI::COMM_WORLD.Abort(1);
                      exit(-1);
                  }
                  Newick newick(true);
                  if(!FileTools::fileExists(geneTreeFile))
                  {
                      std::cerr << "Error: geneTreeFile "<< geneTreeFile <<" not found." << std::endl;
                      std::cerr << "Building a bionj tree instead for gene " << geneTreeFile << std::endl;
                      unrootedGeneTree = buildBioNJTree (params, sites, model, rDist, alphabet);
                      if (geneTree) 
                      {
                          delete geneTree;
                          geneTree =0;
                      }            
                      geneTree = unrootedGeneTree->clone();
                      geneTree->newOutGroup(0); 
                      //exit(-1);
                  }
                  else {
                      if (geneTree) 
                      {
                          delete geneTree;
                          geneTree =0;
                      }            
                      geneTree = dynamic_cast < TreeTemplate < Node > * > (newick.read(geneTreeFile));
                  }
                  if (!geneTree->isRooted()) 
                  {
                      if (unrootedGeneTree) {
                          delete unrootedGeneTree;
                          unrootedGeneTree = 0;
                      }
                      unrootedGeneTree = geneTree->clone();
                      //std::cout << "The gene tree is not rooted ; the root will be searched."<<std::endl;
                      geneTree->newOutGroup(0);
                  }
                  else 
                  {
                      if (unrootedGeneTree) {
                          delete unrootedGeneTree;
                          unrootedGeneTree = 0;
                      }
                      unrootedGeneTree = geneTree->clone();
                      unrootedGeneTree->unroot();
                  }
                  ApplicationTools::displayResult("Gene Tree file", geneTreeFile);
                  ApplicationTools::displayResult("Number of leaves", TextTools::toString(geneTree->getNumberOfLeaves()));
              }
              else if ( (initTree == "bionj") || (initTree == "phyml") ) //build a BioNJ starting tree, and possibly refine it using PhyML algorithm
              {
                  unrootedGeneTree = buildBioNJTree (params, sites, model, rDist, alphabet);

                  if (initTree == "phyml")//refine the tree using PhyML algorithm (2003)
                  { 
                      refineGeneTreeUsingSequenceLikelihoodOnly (params, unrootedGeneTree, sites, model, rDist, file, alphabet);
                  }
                  if (geneTree) 
                  {
                      delete geneTree;
                      geneTree = 0;
                  }        
                  geneTree = unrootedGeneTree->clone(); 
				  breadthFirstreNumber (*geneTree);
				  std::cout << " Problem tree? : "<<TreeTemplateTools::treeToParenthesis(*geneTree, true) << std::endl;
 					
				  geneTree->newOutGroup( geneTree->getLeavesId()[0] ); 
              }
              else throw Exception("Unknown init gene tree method. init.gene.tree should be 'user', 'bionj', or 'phyml'.");
          }
          catch (std::exception& e)
          {
              std::cout << e.what() <<"; Unable to get a proper gene tree for family "<<file<<"; avoiding this family."<<std::endl;
              numDeletedFamilies = numDeletedFamilies+1;
              avoidFamily=true;
          }
      }

      if (!avoidFamily) 
      { //This family is phylogenetically informative

          //Going through the gene tree to see if leaves have branches that are too long.
          std::vector <Node*> leaves = geneTree->getLeaves();
         // std::cout << "leaves.size(): "<<leaves.size() <<std::endl;
          for (unsigned int j = 0 ; j < leaves.size() ; j++) {
              if ( leaves[j] -> hasFather() && leaves[j]->getDistanceToFather() >= 2.0 ) {
                  std::cout << "WARNING: Removing sequence "<< leaves[j]->getName() <<" from family "<<file<< " because its branch is unreasonably long (>=2.0)."<<std::endl;
                  seqsToRemove.push_back( leaves[j]->getName() );
                  //removing the corresponding sequence, if present
                  if (sites->hasSequence(leaves[j]->getName() ) )
                      sites->deleteSequence( leaves[j]->getName() );
              }
          }

          if (sites->getNumberOfSequences() >1) {
              //Pruning sequences from the gene tree
              for (unsigned int j =0 ; j<seqsToRemove.size(); j++) 
              {
                  std::vector <std::string> leafNames = geneTree->getLeavesNames();
                  if ( VectorTools::contains(leafNames, seqsToRemove[j]) )
                  {
                      removeLeaf(*geneTree, seqsToRemove[j]);
                      if (unrootedGeneTree) {
                          delete unrootedGeneTree;
                          unrootedGeneTree = 0;
                      }
                      unrootedGeneTree = geneTree->clone();
                      if (!geneTree->isRooted()) {
                          std::cout <<"gene tree is not rooted!!! "<< taxaseqFile<<std::endl;
                      }
                      unrootedGeneTree->unroot();
                  }
                  else 
                      std::cout<<"Sequence "<<seqsToRemove[j] <<" is not present in the gene tree."<<std::endl;
              }
          }
          //If we have only one sequence in the end, we do not make a tree
          else {
              numDeletedFamilies = numDeletedFamilies+1;
              avoidFamily=true;
              std::cout <<"All or almost all sequences have been removed: avoiding family "<<assignedFilenames[i-numDeletedFamilies+1]<<std::endl;
          }
      }
      if (!avoidFamily) 
      { //This family is phylogenetically informative

          /****************************************************************************
           //Then we initialize the losses and duplication numbers on this tree.
           *****************************************************************************/
          std::vector<int> numbers = num0Lineages;
          allNum0Lineages.push_back(numbers);
          allNum1Lineages.push_back(numbers);
          allNum2Lineages.push_back(numbers);
          std::vector<unsigned int> uNumbers (tree->getNumberOfNodes(), 0);

          allNum12Lineages.push_back(uNumbers);
          allNum22Lineages.push_back(uNumbers);

          resetLossesAndDuplications(*tree, lossExpectedNumbers, duplicationExpectedNumbers);
          resetVector(allNum0Lineages[i-numDeletedFamilies]);
          resetVector(allNum1Lineages[i-numDeletedFamilies]);
          resetVector(allNum2Lineages[i-numDeletedFamilies]);
          resetVector(allNum12Lineages[i-numDeletedFamilies]);
          resetVector(allNum22Lineages[i-numDeletedFamilies]);

          /************************************************************************************************************/
          /********************************************COMPUTING LIKELIHOOD********************************************/
          /************************************************************************************************************/
          bool computeLikelihood = ApplicationTools::getBooleanParameter("compute.likelihood", params, true, "", false, false);
          if(!computeLikelihood)
          {
              if (alphabet)
                  delete alphabet;
              if (sites)
                  delete sites;
              if (tree)
                  delete tree;
              std::cout << "PHYLDOG's done. Bye." << std::endl;
              MPI::COMM_WORLD.Abort(1);
              exit(-1);
          }
          
          
          /****************************************************************************
           //Then we can change the gene tree so that its topology minimizes the number of duplications and losses.
           *****************************************************************************/
          std::string alterStartingTopologyWithDL = ApplicationTools::getStringParameter("DL.starting.gene.tree.optimization", params, "no", "", true, false);
          
          
          bool DLStartingGeneTree;
          if (alterStartingTopologyWithDL == "yes") 
          {
              DLStartingGeneTree = true;
          }
          else 
          {
              DLStartingGeneTree = false;
          }
          
          if (DLStartingGeneTree)
          {
              //we temporarily build a ReconciliationTreeLikelihood object, 
              //but won't consider sequences, to save computational time
              std::cout << "Changing the starting gene tree to minimize the numbers of duplications and losses"<<std::endl;
              std::set <int> nodesToTryInNNISearch;
              refineGeneTreeDLOnly (tree, 
                                    geneTree, 
                                    seqSp,
                                    spId,
                                    lossExpectedNumbers, 
                                    duplicationExpectedNumbers, 
                                    MLindex, 
                                    allNum0Lineages[i-numDeletedFamilies], 
                                    allNum1Lineages[i-numDeletedFamilies], 
                                    allNum2Lineages[i-numDeletedFamilies], 
                                    nodesToTryInNNISearch);
              if (unrootedGeneTree) {
                  delete unrootedGeneTree;
                  unrootedGeneTree = 0;
              }
              unrootedGeneTree = geneTree->clone();
              unrootedGeneTree->unroot();
              
              //Refining branch lengths for this altered gene tree.
              refineGeneTreeBranchLengthsUsingSequenceLikelihoodOnly (params, unrootedGeneTree, sites, model, rDist, file, alphabet);
          }
          
          //printing the gene trees with the species names instead of the sequence names
          //This is useful to build an input for duptree for instance
          TreeTemplate<Node> * treeWithSpNames = unrootedGeneTree->clone();
          std::vector <Node*> leaves = treeWithSpNames->getLeaves();
          for (unsigned int j =0; j<leaves.size() ; j++) 
          {
              leaves[j]->setName(seqSp[leaves[j]->getName()]);
          }
          //Now we have a gene tree with species names and bootstrap values and branch lengths
          std::vector <Node *> nodes =  treeWithSpNames->getNodes();
          for (unsigned int j =0; j<nodes.size() ; j++) 
          {
              if (nodes[j]->hasFather()) 
              {
                  nodes[j]->deleteDistanceToFather(); 
              }
              if (nodes[j]->hasBootstrapValue()) 
              {
                  nodes[j]->deleteBranchProperty(TreeTools::BOOTSTRAP); 
              }
          }
          
          //Outputting the starting tree, with species names, and with sequence names
          Newick newick(true);
          std::string startingGeneTreeFile =ApplicationTools::getStringParameter("output.starting.gene.tree.file",params,"none");
		  try
		  {
			  Nhx *nhx = new Nhx();
			  annotateGeneTreeWithDuplicationEvents (*tree, 
													 *geneTree, 
													 geneTree->getRootNode(), 
													 seqSp, spId); 
			  nhx->write(*geneTree, startingGeneTreeFile, true);

			 // newick.write(*geneTree, startingGeneTreeFile, true);
		  }
		  catch (IOException e)
		  {
			  cout << "Problem writing tree to file "<< startingGeneTreeFile <<"\n Is the file path correct and do you have the proper authorizations? "  << endl;
		  }
		  try
		  {
			  newick.write(*treeWithSpNames, startingGeneTreeFile+"_sp_names", true);
		  }
		  catch (IOException e)
		  {
			  cout << "Problem writing tree to file "<< startingGeneTreeFile+"_sp_names" <<"\n Is the file path correct and do you have the proper authorizations? "  << endl;
		  }
          //std::cout << " Rooted tree? : "<<TreeTemplateTools::treeToParenthesis(*geneTree, true) << std::endl;
          
          GeneTreeLikelihood* tl;
          std::string optimizeClock = ApplicationTools::getStringParameter("optimization.clock", params, "no", "", true, false);

          int sprLimitGeneTree = ApplicationTools::getIntParameter("SPR.limit.gene.tree", params, 2, "", false, false);  
          //ApplicationTools::displayResult("Clock", optimizeClock);
          
          if(optimizeClock == "global")
          {
              std::cout<<"Sorry, clocklike trees have not been implemented yet."<<std::endl;
              MPI::COMM_WORLD.Abort(1);
              exit(0);
          }// This has not been implemented!
          else if(optimizeClock == "no")
          {
              if (reconciliationModel == "DL")
              {
                  tl = new DLGeneTreeLikelihood(*unrootedGeneTree, *sites, 
                                                model, rDist, *tree, 
                                                *geneTree, *treeWithSpNames, seqSp, spId, 
                                                lossExpectedNumbers, 
                                                duplicationExpectedNumbers, 
                                                allNum0Lineages[i-numDeletedFamilies], 
                                                allNum1Lineages[i-numDeletedFamilies], 
                                                allNum2Lineages[i-numDeletedFamilies], 
                                                speciesIdLimitForRootPosition, 
                                                heuristicsLevel, MLindex, 
                                                true, true, rootOptimization, true, DLStartingGeneTree, sprLimitGeneTree);
                ///  dynamic_cast<DLGeneTreeLikelihood*> (tl)->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
                  
              }
              else if (reconciliationModel == "COAL")
              {
                  //coalCounts: vector of genetreenbnodes vectors of 3 (3 directions) vectors of sptreenbnodes vectors of 2 ints
                  std::vector< std::vector< std::vector< unsigned int > > > coalCounts2;
                  std::vector< std::vector<unsigned int> > coalCounts3;
                  std::vector< unsigned int > coalCounts4;
                  for (unsigned int j = 0 ; j < 2 ; j++ ) {
                      coalCounts4.push_back(0);
                  }
                  for (unsigned int j = 0 ; j < tree->getNumberOfNodes() ; j++ ) {
                      coalCounts3.push_back(coalCounts4);
                  }
                  for (unsigned int j = 0 ; j < 3 ; j++ ) {
                      coalCounts2.push_back(coalCounts3);
                  }
                  for (unsigned int j = 0 ; j < geneTree->getNumberOfNodes() ; j++ ) {
                      coalCounts.push_back(coalCounts2);
                  }
                  tl = new COALGeneTreeLikelihood(*unrootedGeneTree, *sites, 
                                                  model, rDist, *tree, 
                                                  *geneTree, *treeWithSpNames, seqSp, spId, 
                                                  coalCounts, coalBls,  
                                                  speciesIdLimitForRootPosition, 
                                                  heuristicsLevel, MLindex, 
                                                  true, true, rootOptimization, true, sprLimitGeneTree);
              }
              else {
                  std::cerr <<"Unknown reconciliation model: "<< reconciliationModel <<std::endl;
                  exit(-1);
              }
              
          }
          else throw Exception("Unknown option for optimization.clock: " + optimizeClock);
          delete treeWithSpNames;
          treeLikelihoods.push_back(tl);          
          allParams.push_back(params); 
          allAlphabets.push_back(alphabet);
          allDatasets.push_back(sites);
          allModels.push_back(model);
          allDistributions.push_back(rDist);
          allGeneTrees.push_back(geneTree->clone());
          allUnrootedGeneTrees.push_back(unrootedGeneTree->clone());
		  allSeqSps.push_back(seqSp);
		  allSprLimitGeneTree.push_back(sprLimitGeneTree);
		//  delete rDist;
		//  delete model;
		  //delete sites;
		
		  // delete alphabet;
		  
/*          if (tl)
          delete tl;
          if (params)
          delete params;
          if (alphabet)
          delete alphabet;
          delete sites;
          delete model;
          delete rDist;
          delete geneTree;
          delete unrootedGeneTree;*/
      }
      else 
      {
          if (sites)
              delete sites;
          if (alphabet)
              delete alphabet;
      }
      double familyTime = ApplicationTools::getTime() - startingFamilyTime ;  
      std::cout <<"Examined family "<<assignedFilenames[i] << " in "<<familyTime<<" s."<<std::endl;
      if (unrootedGeneTree) {
          delete unrootedGeneTree;
          unrootedGeneTree = 0;
      }
      if (geneTree) {
          delete geneTree;
          geneTree = 0;
      }

  }//End for each file
    
    
    if (numDeletedFamilies == assignedFilenames.size()) 
    {
        std::cout<<"WARNING: A client with rank "<< rank << " is in charge of 0 gene family after gene family filtering!"<<std::endl;        
        std::cout<<"A processor will be idle most of the time, the load could probably be better distributed."<<std::endl; 
    }

    
    unsigned int numberOfGeneFamilies = assignedFilenames.size()-numDeletedFamilies;
    numberOfFilteredFamiliesCommunicationsServerClient (world, server, 
                                   rank, numberOfGeneFamilies);    
    
    //Building a MRP species tree, if the options say so
    initTree = ApplicationTools::getStringParameter("init.species.tree", 
                                                    params, "user", 
                                                    "", false, false);
    if (initTree == "mrp") {
        //Build a string containing all trees with one gene per species
        string trees1PerSpecies = "";
        vector<string> allTrees1PerSpecies;
        stringstream ss (stringstream::in | stringstream::out);
        for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
        {
            if (reconciliationModel == "DL")
            {
            if ( ( dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i]) )->isSingleCopy() ) {
                ss << TreeTemplateTools::treeToParenthesis((dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i]) )->getGeneTreeWithSpNames(), false);
            }
            }
            else {
                if ((dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i]) )->isSingleCopy() ) {
                    ss << TreeTemplateTools::treeToParenthesis((dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i]) )->getGeneTreeWithSpNames(), false);
                }

            }
        }
        trees1PerSpecies = ss.str() ;     
        mrpCommunicationsServerClient (world, server, 
                                       rank, trees1PerSpecies, 
                                       allTrees1PerSpecies);

    }
    vector<unsigned int> numbersOfGeneFamilies;
    std::string currentSpeciesTree;
    
    secondCommunicationsServerClient (world , server, 
                                      rank, numberOfGeneFamilies, 
                                      numbersOfGeneFamilies, 
                                      lossExpectedNumbers, duplicationExpectedNumbers, coalBls, 
                                      currentSpeciesTree);

    delete tree;

    tree=TreeTemplateTools::parenthesisToTree(currentSpeciesTree, false, "", true);

    spId = computeSpeciesNamesToIdsMap(*tree);
    
    for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
    {        
        treeLikelihoods[i]->setSpTree(*tree);

        treeLikelihoods[i]->setSpId(spId);        
        //GeneTreeLikelihood* tl ;
        if (reconciliationModel == "DL")
        {
            DLGeneTreeLikelihood* tl = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i]);
            tl->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
            tl->optimizeNumericalParameters(params); //Initial optimization of all numerical parameters
            tl->initParameters();
            allLogLs.push_back(tl->getValue());
            if(std::isinf(allLogLs[i]))
            {
                // This may be due to null branch lengths, leading to null likelihood!
                ApplicationTools::displayWarning("!!! Warning!!! Initial likelihood is zero.");
                ApplicationTools::displayWarning("!!! This may be due to branch length == 0.");
                ApplicationTools::displayWarning("!!! All null branch lengths will be set to 0.000001.");
                std::vector<Node*> nodes = treeLikelihoods[i]->getRootedTree().getNodes();
                for(unsigned int k = 0; k < nodes.size(); k++)
                {
                    if(nodes[k]->hasDistanceToFather() && nodes[k]->getDistanceToFather() < 0.000001) nodes[k]->setDistanceToFather(0.000001);
                }
                tl->initParameters();
                // allLogLs[i-numDeletedFamilies]= tl->f(tl->getParameters());
                allLogLs[i]= tl->getValue();
            }
            ApplicationTools::displayResult("Initial sequence and reconciliation likelihood", TextTools::toString(- allLogLs[i], 15));
            if(std::isinf(allLogLs[i]))
            {
                ApplicationTools::displayError("!!! Unexpected initial likelihood == 0.");
                ApplicationTools::displayError("!!! Looking at each site:");
                /*   for(unsigned int k = 0; k < sites->getNumberOfSites(); k++)
                 {
                 (*ApplicationTools::error << "Site " << sites->getSite(k).getPosition() << "\tlog likelihood = " << tl->getLogLikelihoodForASite(k)).endLine();
                 }*/
                ApplicationTools::displayError("!!! 0 values (inf in log) may be due to computer overflow, particularly if datasets are big (>~500 sequences).");
                MPI::COMM_WORLD.Abort(1);
                exit(-1);
            }            
        }
        else if (reconciliationModel == "COAL") {
            COALGeneTreeLikelihood* tl = dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i] );
            tl->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
            tl->optimizeNumericalParameters(params); //Initial optimization of all numerical parameters
            tl->initParameters();
            allLogLs.push_back(tl->getValue());
            if(std::isinf(allLogLs[i]))
            {
                // This may be due to null branch lengths, leading to null likelihood!
                ApplicationTools::displayWarning("!!! Warning!!! Initial likelihood is zero.");
                ApplicationTools::displayWarning("!!! This may be due to branch length == 0.");
                ApplicationTools::displayWarning("!!! All null branch lengths will be set to 0.000001.");
                std::vector<Node*> nodes = treeLikelihoods[i]->getRootedTree().getNodes();
                for(unsigned int k = 0; k < nodes.size(); k++)
                {
                    if(nodes[k]->hasDistanceToFather() && nodes[k]->getDistanceToFather() < 0.000001) nodes[k]->setDistanceToFather(0.000001);
                }
                tl->initParameters();
                // allLogLs[i-numDeletedFamilies]= tl->f(tl->getParameters());
                allLogLs[i]= tl->getValue();
            }
            ApplicationTools::displayResult("Initial sequence and reconciliation likelihood", TextTools::toString(- allLogLs[i], 15));
            if(std::isinf(allLogLs[i]))
            {
                ApplicationTools::displayError("!!! Unexpected initial likelihood == 0.");
                ApplicationTools::displayError("!!! Looking at each site:");
                /*   for(unsigned int k = 0; k < sites->getNumberOfSites(); k++)
                 {
                 (*ApplicationTools::error << "Site " << sites->getSite(k).getPosition() << "\tlog likelihood = " << tl->getLogLikelihoodForASite(k)).endLine();
                 }*/
                ApplicationTools::displayError("!!! 0 values (inf in log) may be due to computer overflow, particularly if datasets are big (>~500 sequences).");
                MPI::COMM_WORLD.Abort(1);
                exit(-1);
            }

        }
    }

    return;
}


/******************************************************************************/
// This function outputs gene trees from the clients.
/******************************************************************************/
void outputGeneTrees (std::vector<std::string> & assignedFilenames, 
                      std::vector <GeneTreeLikelihood *> treeLikelihoods,
                      std::vector< std::map<std::string, std::string> > & allParams, 
                      unsigned int & numDeletedFamilies, 
                      std::vector <std::vector <std::string> > & reconciledTrees,                       
                      std::vector <std::vector <std::string> > & duplicationTrees, 
                      std::vector <std::vector <std::string> > & lossTrees, 
                      unsigned int & bestIndex, int & startRecordingTreesFrom, 
                      std::string &reconciliationModel)
{
  std::string suffix;
  std::string reconcTree;
  std::ofstream out;
  std::string dupTree;
  std::string lossTree;
  int rightIndex = bestIndex-startRecordingTreesFrom ;
  for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
    {
        suffix = ApplicationTools::getStringParameter("output.file.suffix", allParams[i], "", "", false, false);
        reconcTree = ApplicationTools::getStringParameter("output.reconciled.tree.file", allParams[i], "reconciled.tree", "", false, false);

        reconcTree = reconcTree + suffix;
        Nhx *nhx = new Nhx();
        string temp = reconciledTrees[i][rightIndex];
         if (reconciliationModel == "DL") {
            TreeTemplate<Node> * geneTree=nhx->parenthesisToTree(temp);
            temp = duplicationTrees[i][rightIndex];

            TreeTemplate<Node> * spTree=nhx->parenthesisToTree( temp);
            breadthFirstreNumber (*spTree);        
            std::map <std::string, int> spId = computeSpeciesNamesToIdsMap(*spTree);
            std::map <std::string, std::string> seqSp = treeLikelihoods[i]->getSeqSp();

            annotateGeneTreeWithDuplicationEvents (*spTree, 
                                                   *geneTree, 
                                                   geneTree->getRootNode(), 
                                                   treeLikelihoods[i]->getSeqSp(),
                                                   spId); 
            out.open (reconcTree.c_str(), std::ios::out);
            nhx->write(*geneTree, out);
            out.close();
            dupTree = ApplicationTools::getStringParameter("output.duplications.tree.file", allParams[i], "duplications.tree", "", false, false);

            dupTree = dupTree + suffix;
            out.open (dupTree.c_str(), std::ios::out);
            out << duplicationTrees[i][rightIndex]<<std::endl;
            out.close();
            lossTree = ApplicationTools::getStringParameter("output.losses.tree.file", allParams[i], "losses.tree", "", false, false);
            lossTree = lossTree + suffix;
            out.open (lossTree.c_str(), std::ios::out);
            out << lossTrees[i][rightIndex]<<std::endl;
            delete geneTree;
            delete spTree;
        }
        else if (reconciliationModel == "COAL") {
            TreeTemplate<Node> * geneTree=TreeTemplateTools::parenthesisToTree(temp);
            out.open (reconcTree.c_str(), std::ios::out);
            nhx->write(*geneTree, out);
            out.close();
            delete geneTree;

        }

        out.close();
        delete nhx;

    }
  return;
}






/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
//////////////////////////////////////////////////////MAIN/////////////////////////////////////////////////
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/


int main(int args, char ** argv)
{
	if(args == 1)
	{
		help();
		exit(0);
	}

	unsigned int rank, size;
	unsigned int server = 0;

	//Using BOOST :
	mpi::environment env(args, argv);
	mpi::communicator world;
	rank = world.rank();
	size = world.size();
	std::string line;

  if (size==1) {
    std::cout <<"\n\n\n\t\tError: this program can only run if 2 or more processes are used."<<std::endl;
    std::cout <<"\t\tUse 'mpirun -np k phyldog ...', where k>=2"<<std::endl;
    MPI::COMM_WORLD.Abort(1);
    exit(-1);
  }
  
  
	try {
		ApplicationTools::startTimer();
        //All processors parse the main options
		std::map<std::string, std::string> params = AttributesTools::parseOptions(args, argv);

    
        //##################################################################################################################
        //##################################################################################################################
        //############################################# IF AT THE SERVER NODE ##############################################
        //##################################################################################################################
        //##################################################################################################################
        
		if (rank == server) { 
            
            /*
            int z = 0;
            //   char hostname[256];
            //gethostname(hostname, sizeof(hostname));
            std::cout <<"PID: "<<getpid()<<std::endl;
            std::cout <<"z: "<<z<<std::endl;
            //printf("PID %d on %s ready for attach\n", getpid(), hostname);
            // fflush(stdout);
            while (0 == z){
                std::cout << z <<std::endl;
                sleep(5);
            }*/
            
            
            std::cout << "******************************************************************" << std::endl;
            std::cout << "*                   PHYLDOG, version 1.1.0                       *" << std::endl;
            std::cout << "* Author: B. Boussau                            Created 16/07/07 *" << std::endl;
            std::cout << "******************************************************************" << std::endl;
            std::cout << std::endl;
            
            std::cout <<"Server of rank "<<rank <<" with PID "<< TextTools::toString((int)getpid())<<std::endl;

            SpeciesTreeLikelihood spTL = SpeciesTreeLikelihood(world, server, size, params);
            //initialization, first communications, first likelihood computation

            spTL.initialize();

            spTL.MLSearch();
                        
			std::cout << "PHYLDOG's done. Bye." << std::endl;
			ApplicationTools::displayTime("Total execution time:");
            MPI_Barrier(world);  
            MPI::Finalize( );
		}//End if at the server node
    
    
    
    
    
        
		//##################################################################################################################
        //##################################################################################################################
        //############################################# IF AT A CLIENT NODE ################################################
        //##################################################################################################################
        //##################################################################################################################
        if (rank >server)
        {
	/*  
	  int z = 0;
             //   char hostname[256];
             //gethostname(hostname, sizeof(hostname));
             std::cout <<"PID: "<<getpid()<<std::endl;
             std::cout <<"z: "<<z<<std::endl;
             //printf("PID %d on %s ready for attach\n", getpid(), hostname);
             // fflush(stdout);
             while (0 == z){
             std::cout << z <<std::endl;
             sleep(5);
             }*/
	  
            ApplicationTools::startTimer();
            bool debug = ApplicationTools::getBooleanParameter("debug",params,false);
            string path = ApplicationTools::getStringParameter("PATH", params, "", "", true, false);
            string outputFile = path + "Client_"+TextTools::toString(rank)+".out";
            streambuf *psbuf, *backup, *backupcerr;
            ofstream filestr;
            if (!debug) {
                //Redirecting stdout to a specific file for this client
                filestr.open (outputFile.c_str());
                backup = cout.rdbuf();     // back up cout's streambuf 
                backupcerr = cerr.rdbuf(); // back up cerr's streambuf
                psbuf = filestr.rdbuf();   // get file's streambuf                                                                                                                                                               
                cout.rdbuf(psbuf);         // assign streambuf to cout     
                cerr.rdbuf(psbuf);
            }
            /****************************************************************************
             * First communications between the server and the clients.
             *****************************************************************************/      
            bool optimizeSpeciesTreeTopology;
            unsigned int assignedNumberOfGenes;
            std::vector <std::string> assignedFilenames;
            std::vector <unsigned int> numbersOfGenesPerClient;
            std::vector <std::vector<std::string> > listOfOptionsPerClient;
            int SpeciesNodeNumber;
            std::vector <double> lossExpectedNumbers;
            std::vector <double> duplicationExpectedNumbers;
            std::vector <double> backupLossExpectedNumbers;
            std::vector <double> backupDuplicationExpectedNumbers;
            std::vector <int> num0Lineages;
            std::vector <int> num1Lineages; 
            std::vector <int> num2Lineages;
            std::vector <std::vector<int> > allNum0Lineages;
            std::vector <std::vector<int> > allNum1Lineages;
            std::vector <std::vector<int> > allNum2Lineages;
            std::vector < std::vector < std::vector < std::vector< unsigned int > > > > coalCounts;
            std::vector < double > coalBls;
            std::vector <unsigned int> num22Lineages; 
            std::vector <unsigned int> num12Lineages; 
            std::vector < std::vector <unsigned int> > allNum22Lineages; 
            std::vector < std::vector <unsigned int> > allNum12Lineages; 

			std::string SPRalgorithm;
            std::string currentSpeciesTree;
            std::string reconciliationModel;
            
            firstCommunicationsServerClient (world , server, numbersOfGenesPerClient, assignedNumberOfGenes,
                                             assignedFilenames, listOfOptionsPerClient, 
                                             optimizeSpeciesTreeTopology, SpeciesNodeNumber, 
                                             lossExpectedNumbers, duplicationExpectedNumbers, num0Lineages, 
                                             num1Lineages, num2Lineages, 
                                             num12Lineages, num22Lineages, coalBls, currentSpeciesTree);
            
            /****************************************************************************
             * Various initializations.
             *****************************************************************************/  
            TreeTemplate<Node> * tree = 0;
            std::string initTree;
            std::string allFileNames;
            
            //TEST:
            /*
             currentSpeciesTree =    "((((((((((((((((((Macaca_mulatta,((((Homo_sapiens,Pan_troglodytes),Gorilla_gorilla),Pongo_abelii),Nomascus_leucogenys)),Callithrix_jacchus),Tarsius_syrichta),(Microcebus_murinus,Otolemur_garnettii)),Tupaia_belangeri),(((((Mus_musculus,Rattus_norvegicus),Dipodomys_ordii),Cavia_porcellus),Spermophilus_tridecemlineatus),(Oryctolagus_cuniculus,Ochotona_princeps))),((((Vicugna_pacos,(Tursiops_truncatus,Bos_taurus)),Sus_scrofa),((Equus_caballus,(Felis_catus,(Ailuropoda_melanoleuca,Canis_lupus_familiaris))),(Myotis_lucifugus,Pteropus_vampyrus))),(Erinaceus_europaeus,Sorex_araneus))),(((Loxodonta_africana,Procavia_capensis),Echinops_telfairi),(Dasypus_novemcinctus,Choloepus_hoffmanni))),(Monodelphis_domestica,(Macropus_eugenii,Sarcophilus_harrisii))),Ornithorhynchus_anatinus),(((Gallus_gallus,Meleagris_gallopavo),Taeniopygia_guttata),Anolis_carolinensis)),Xenopus_tropicalis),(((Tetraodon_nigroviridis,Takifugu_rubripes),(Gasterosteus_aculeatus,Oryzias_latipes)),Danio_rerio)),Petromyzon_marinus),(Ciona_savignyi,Ciona_intestinalis)),Drosophila_melanogaster),Caenorhabditis_elegans),Saccharomyces_cerevisiae);";
            */
            
            //First we read the species tree from the char[] sent by the server
            tree=TreeTemplateTools::parenthesisToTree(currentSpeciesTree, false, "", true);
         //TEST
        //    breadthFirstreNumber (*tree);        

            resetLossesAndDuplications(*tree, lossExpectedNumbers, duplicationExpectedNumbers);
            //To make the correspondance between species name and id:
            std::map <std::string, int> spId = computeSpeciesNamesToIdsMap(*tree);
            //These std::vectors contain all relevant numbers for all gene families the client is in charge of.
            std::vector <double> allLogLs;
            std::vector <std::map<std::string, std::string> > allParams;
            TreeTemplate<Node> * geneTree = 0;
            int MLindex = 0;
            std::vector <GeneTreeLikelihood *> treeLikelihoods;
            std::vector <GeneTreeLikelihood *> backupTreeLikelihoods;
            std::vector <Alphabet *> allAlphabets;
            std::vector <VectorSiteContainer *>   allDatasets;
            std::vector <SubstitutionModel *> allModels;
            std::vector <DiscreteDistribution *> allDistributions;
            std::vector <TreeTemplate<Node> *> allGeneTrees;
            std::vector <TreeTemplate<Node> *> allUnrootedGeneTrees;
			std::vector < std::map <std::string, std::string> > allSeqSps ;
			std::vector < unsigned int> allSprLimitGeneTree ;
            unsigned int numDeletedFamilies=0;
            unsigned int bestIndex = 0;
            bool stop = false; 
            bool rearrange = false;
            double logL;
            
            /****************************************************************************
             // Then, we get 2 options related to the algorithms used.
             * Meaning of heuristicsLevel:
             * 0: exact double-recursive algorithm. 
             All possible root likelihoods are computed with only 2 tree traversals (default).
             * 1: fastest heuristics : only a few nodes are tried for the roots of the gene trees 
             (the number of these nodes tried depends upon speciesIdLimitForRootPosition), 
             and for each root tried, the events are re-computed only for a subset of the tree.
             * 2: All roots are tried, and for each root tried, 
             the events are re-computed only for a subset of the tree.
             * 3: All roots are tried, and for each root tried, 
             the events are re-computed for all nodes of the tree 
             (which should be useless unless there is a bug in the selection of the subset of the nodes.
             * WARNING: options 1, 2, 3 are probably bugged now.
             *****************************************************************************/
            int heuristicsLevel= 0 ;
            //ApplicationTools::getIntParameter("heuristics.level", params, 0, "", false, false);
            int speciesIdLimitForRootPosition= 2 ;
            //ApplicationTools::getIntParameter("species.id.limit.for.root.position", params, 2, "", false, false);
            
            /****************************************************************************
             * Gene family parsing and first likelihood computation.
             *****************************************************************************/            
            string toPrint = "";
            //TextTools::toString(rank) + " " + TextTools::toString((int)getpid()) + "\t";
            std::cout <<"Client  of rank "<<rank <<" with PID "<< TextTools::toString((int)getpid()) <<" is in charge of " << assignedFilenames.size()<<" gene families:"<<std::endl;
            for (unsigned int i = 0 ; i< assignedFilenames.size() ; i++ ) {
                toPrint = toPrint + assignedFilenames[i] + "\t";
            }
            std::cout << toPrint <<std::endl;
            //Gets gene family-specific options, and computes the likelihood
            parseAssignedGeneFamilies(world , server, rank, assignedFilenames, params, numDeletedFamilies, 
                                      geneTree, tree, num0Lineages, allNum0Lineages, 
                                      allNum1Lineages, allNum2Lineages, 
                                      lossExpectedNumbers, duplicationExpectedNumbers, coalCounts, coalBls,
                                      allNum12Lineages, allNum22Lineages,
                                      spId, speciesIdLimitForRootPosition, heuristicsLevel, 
                                      MLindex, allLogLs, treeLikelihoods, 
                                      backupTreeLikelihoods, allParams, 
                                      allAlphabets, allDatasets, allModels, 
                                      allDistributions, allGeneTrees, allUnrootedGeneTrees, allSeqSps, 
									  allSprLimitGeneTree, reconciliationModel);
            std::vector <std::vector <std::string> > reconciledTrees;
            std::vector <std::vector <std::string> > duplicationTrees;
            std::vector <std::vector <std::string> > lossTrees;
            std::vector <std::string> t;  
            std::vector <std::map<std::string, std::string> > allParamsBackup = allParams;
            bool resetGeneTrees = ApplicationTools::getBooleanParameter("reset.gene.trees",params,true);
            unsigned int currentStep = ApplicationTools::getIntParameter("current.step",params,0);

            for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
            {
                reconciledTrees.push_back(t);
                duplicationTrees.push_back(t);
                lossTrees.push_back(t);
                //This is to avoid optimizing gene tree parameters in the first steps of the program, 
                //if we optimize the species tree topology.
                if (optimizeSpeciesTreeTopology && currentStep < 3) 
                { 
                    if (ApplicationTools::getBooleanParameter("optimization.topology", allParams[i], false, "", true, false))
                    {
                        allParams[i][ std::string("optimization.topology")] = "false";
                    }
                    allParams[i][ std::string("optimization")] = "None"; //Quite extreme, but the sequence likelihood has no impact on the reconciliation !
                    treeLikelihoods[i]->OptimizeSequenceLikelihood(false);
                }
            }

            bool recordGeneTrees; 
          //  bool firstTimeImprovingGeneTrees = false; //When for the first time we optimize gene trees, we set it at true
            if (optimizeSpeciesTreeTopology && currentStep < 3)
            {//At the beginning, we do not record the gene trees.
                recordGeneTrees = false;
            }
            else {
                recordGeneTrees = true;
               // firstTimeImprovingGeneTrees=true;
            }
            int startRecordingTreesFrom = 0; //This int is incremented until the gene trees start to be backed-up, when we start the second phase of the algorithm.
          //  MPI_Barrier(world);
            broadcast(world, stop, server);
            broadcastsAllInformationButStop(world, server, rearrange, 
                                            lossExpectedNumbers, 
                                            duplicationExpectedNumbers, 
                                            coalBls,
                                            currentSpeciesTree,
                                            currentStep, 
                                            reconciliationModel
                                            );
            if (assignedFilenames.size()-numDeletedFamilies > 0) 
            {
                if (tree) delete tree;
                tree=TreeTemplateTools::parenthesisToTree(currentSpeciesTree, false, "", true);
                spId = computeSpeciesNamesToIdsMap(*tree);
            }
            startRecordingTreesFrom = 1;
            for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
            {
                treeLikelihoods[i]->setSpTree(*tree);
                treeLikelihoods[i]->setSpId(spId);
                if (reconciliationModel == "DL") {
                    dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->setExpectedNumbers(duplicationExpectedNumbers, lossExpectedNumbers);
                }
                else if (reconciliationModel == "COAL") {
                    dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->setCoalBranchLengths(coalBls);
                }
            }
            /*
            //We make a backup of the gene tree likelihoods.
            if (reconciliationModel == "DL") {
                for (unsigned int i =0 ; i<treeLikelihoods.size() ; i++) 
                {
                    backupTreeLikelihoods.push_back(dynamic_cast<DLGeneTreeLikelihood*> (dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->clone() ) );
                    // backupTreeLikelihoods.push_back(treeLikelihoods[i]->clone());
                }    
            }
            else if (reconciliationModel == "COAL") {
                for (unsigned int i =0 ; i<treeLikelihoods.size() ; i++) 
                {
                    backupTreeLikelihoods.push_back(dynamic_cast<COALGeneTreeLikelihood*> ( dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->clone() ) );
                }
            }*/

            /****************************************************************************
             ****************************************************************************
             * Main loop: iterative likelihood computations
             ****************************************************************************
             ****************************************************************************/
            Nhx *nhx = new Nhx();
            string rearrangementType;
            bool timing = true;
            double startingTime, totalTime;
            while (!stop)            
            {      
                logL=0.0;
                resetVector(num0Lineages);
                resetVector(num1Lineages);
                resetVector(num2Lineages);
				resetVector(num12Lineages);
                resetVector(num22Lineages);
                for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
                {
                    if (rearrange) //(firstTimeImprovingGeneTrees) 
                    {
                        treeLikelihoods[i]->OptimizeSequenceLikelihood(true);
                        allParams[i][ std::string("optimization.topology")] = "true";
                    }
                    else {
                        treeLikelihoods[i]->OptimizeSequenceLikelihood(false);
                        allParams[i][ std::string("optimization.topology")] = "false";
                    }
                    // std::cout <<  TreeTemplateTools::treeToParenthesis(*geneTree, true)<<std::endl;
                    
                    rearrangementType = ApplicationTools::getStringParameter("rearrangement.gene.tree", allParams[i], "nni", "", true, false);
                    if (rearrangementType == "nni" && currentStep !=4 ) {
                        //PhylogeneticsApplicationTools::optimizeParameters(treeLikelihoods[i], treeLikelihoods[i]->getParameters(), allParams[i], "", true, false);
                        if (timing) 
                            startingTime = ApplicationTools::getTime();
                        // NNI optimization:  
                         if (reconciliationModel == "DL") {
                             dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeNNIs(allParams[i]);
                         }
                         else if (reconciliationModel == "COAL") {
                             dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeNNIs(allParams[i]);
                         }
                        if (timing) 
                        {
                            totalTime = ApplicationTools::getTime() - startingTime;
                            std::cout << "Family "<< assignedFilenames[i] <<"; Time for NNI exploration: "<<  totalTime << " s." <<std::endl;
                        }
                    }
                    else {
                        if (timing) 
                            startingTime = ApplicationTools::getTime();
                        //SPR optimization:    
                        //std::cout <<"Before optimization: "<<TreeTemplateTools::treeToParenthesis(treeLikelihoods[i]->getRootedTree(), true)<<std::endl;
						SPRalgorithm = ApplicationTools::getStringParameter("spr.gene.tree.algorithm", allParams[i], "normal", "", true, false);
                        if (reconciliationModel == "DL") {
							dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeMuffato(allParams[i]);
							if (timing) 
							{
								totalTime = ApplicationTools::getTime() - startingTime;
								std::cout << "Family "<< assignedFilenames[i] <<"; Time for Muffato exploration: "<<  totalTime << " s." <<std::endl;
								startingTime = ApplicationTools::getTime();
							}
							if (SPRalgorithm == "normal")
								dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeSPRsFast2(allParams[i]);
							else
								dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeSPRsFast3(allParams[i]);
                        }
                        else if (reconciliationModel == "COAL") {
                            dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->refineGeneTreeSPRsFast(allParams[i]);
                        }
                        //   treeLikelihoods[i]->refineGeneTreeSPRs(allParams[i]);
                        //  treeLikelihoods[i]->refineGeneTreeSPRs2(allParams[i]);
						if (timing) 
						{
							totalTime = ApplicationTools::getTime() - startingTime;
							std::cout << "Family "<< assignedFilenames[i] <<"; Time for SPR exploration: "<<  totalTime << " s." <<std::endl;
						}
                    }     
                    if (geneTree) {
                        delete geneTree;
                        geneTree = 0;
                    }
                    geneTree = new TreeTemplate<Node>(treeLikelihoods[i]->getRootedTree());

                    ///LIKELIHOOD OPTIMIZED
                    
                   // resetLossesAndDuplications(*tree, lossExpectedNumbers, duplicationExpectedNumbers);
                    if ( reconciliationModel == "DL" ) {
                        allNum0Lineages[i] = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->get0LineagesNumbers();
                        allNum1Lineages[i] = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->get1LineagesNumbers();
                        allNum2Lineages[i] = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->get2LineagesNumbers();
                        num0Lineages = num0Lineages + allNum0Lineages[i];
                        num1Lineages = num1Lineages + allNum1Lineages[i];
                        num2Lineages = num2Lineages + allNum2Lineages[i];
                        MLindex = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->getRootNodeindex();
                        allLogLs[i] = dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->getValue();  
                    }
                    else if (reconciliationModel == "COAL") {
                        allNum12Lineages[i] = dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->getNum12Lineages();
                        allNum22Lineages[i] = dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->getNum22Lineages();
                        num12Lineages = num12Lineages + allNum12Lineages[i];
                        num22Lineages = num22Lineages + allNum22Lineages[i];
                        MLindex = dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->getRootNodeindex();
                        allLogLs[i] = dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->getValue();  
                    }
                    logL = logL + allLogLs[i];
                    std::cout<<"Gene Family: " << assignedFilenames[i] << " total logLk: "<< - allLogLs[i]<< " scenario loglk: "<< treeLikelihoods[i]->getScenarioLikelihood() <<std::endl;

                    if (std::isnan(allLogLs[i])) 
                    {
                        std::cout<<TreeTemplateTools::treeToParenthesis (*geneTree, false, EVENT)<<std::endl;
                        std::cout<<TreeTemplateTools::treeToParenthesis (*tree, false, DUPLICATIONS)<<std::endl;
                    }
                    if (recordGeneTrees) 
                    {
                        reconciledTrees[i].push_back(nhx->treeToParenthesis (*geneTree));
                        if (reconciliationModel == "DL") {
                            duplicationTrees[i].push_back(nhx->treeToParenthesis (*tree));
                            lossTrees[i].push_back(nhx->treeToParenthesis (*tree));
                        }
                    }
                    if (geneTree) 
                    {
                        delete geneTree;
                        geneTree = 0;
                    }
                }//end for each filename
               /* if (firstTimeImprovingGeneTrees) 
                {
                    firstTimeImprovingGeneTrees = false;
                }*/
                if (!recordGeneTrees) 
                {
                    startRecordingTreesFrom++;
                }
                //Clients send back stuff to the server.
               /* gather(world, logL, server); 
                gather(world, num0Lineages, allNum0Lineages, server); 
                gather(world, num1Lineages, allNum1Lineages, server);   
                gather(world, num2Lineages, allNum2Lineages, server);*/
				if (timing) 
				{
					startingTime = ApplicationTools::getTime();
				}

                gathersInformationFromClients (world, 
                                               server,
                                               rank, 
                                               logL, 
                                               num0Lineages, 
                                               num1Lineages, 
                                               num2Lineages, 
                                               allNum0Lineages, 
                                               allNum1Lineages, 
                                               allNum2Lineages, 
                                               num12Lineages, num22Lineages, reconciliationModel);
				if (timing) 
				{
					totalTime = ApplicationTools::getTime() - startingTime;
					std::cout << "Time for gathering information: "<<  totalTime << " s." <<std::endl;
				}

                //Should the computations stop? The server tells us.
                broadcast(world, stop, server);
                if (!stop) 
                {	// we continue the loop
                    //We get the new values from the server
					if (timing) 
					{
						startingTime = ApplicationTools::getTime();
					}
                    broadcastsAllInformationButStop(world, server, rearrange, 
                                                    lossExpectedNumbers, 
                                                    duplicationExpectedNumbers, 
                                                    coalBls,
                                                    currentSpeciesTree,
                                                    currentStep, 
                                                    reconciliationModel);
					if (timing) 
					{
						totalTime = ApplicationTools::getTime() - startingTime;
						std::cout << "Time for broadcasting information: "<<  totalTime << " s." <<std::endl;
						startingTime = ApplicationTools::getTime();
					}

                    if (assignedFilenames.size()-numDeletedFamilies > 0) 
                    {
                        if (tree) delete tree;
                        tree=TreeTemplateTools::parenthesisToTree(currentSpeciesTree, false, "", true);
                        spId = computeSpeciesNamesToIdsMap(*tree);
                    }
                    //if we reset the gene trees by resetting treeLikelihoods:
                    //we always start from ML trees according to sequences only
                    //when we optimize dl expected numbers, we may not want to 
                    //rearrange the gene trees between the first computation of the species tree
                    //lk and the second one; in this case we set rearrange to false.
                    //Then there is no need to reset the gene tree!

                    if (resetGeneTrees && currentStep !=4 && rearrange == true) {
                        for (unsigned int i =0 ; i<treeLikelihoods.size() ; i++) {
                            if    (treeLikelihoods[i])
                                delete treeLikelihoods[i];
                        }
                        treeLikelihoods.clear();
						/*
                        if (reconciliationModel == "DL") {
                            for (unsigned int i=0 ; i<backupTreeLikelihoods.size() ; i++) 
                            {
                                treeLikelihoods.push_back(dynamic_cast<DLGeneTreeLikelihood*> (backupTreeLikelihoods[i])->clone());
                            }
                        }
                        else if (reconciliationModel == "COAL") {
                            for (unsigned int i=0 ; i<backupTreeLikelihoods.size() ; i++) 
                            {
                                treeLikelihoods.push_back(dynamic_cast<COALGeneTreeLikelihood*> (backupTreeLikelihoods[i])->clone());
                            }
                        }
                         */
						
						if (reconciliationModel == "DL")
						{
							for (unsigned int i=0 ; i<allDatasets.size() ; i++) 
                            {
								TreeTemplate<Node> * treeWithSpNames = allUnrootedGeneTrees[i]->clone();
								std::vector <Node*> leaves = treeWithSpNames->getLeaves();
								for (unsigned int j =0; j<leaves.size() ; j++) 
								{
									leaves[j]->setName(allSeqSps[i][leaves[j]->getName()]);
								}
								treeLikelihoods.push_back( new DLGeneTreeLikelihood(*(allUnrootedGeneTrees[i]), *(allDatasets[i]), 
																					allModels[i], allDistributions[i], *tree, 
																					*allGeneTrees[i], *treeWithSpNames, allSeqSps[i], spId, 
																					lossExpectedNumbers, 
																					duplicationExpectedNumbers, 
																					allNum0Lineages[i], 
																					allNum1Lineages[i], 
																					allNum2Lineages[i], 
																					speciesIdLimitForRootPosition, 
																					heuristicsLevel, MLindex, 
																					true, true, true, true, false, allSprLimitGeneTree[i]) );
								delete treeWithSpNames;
							}
							///  dynamic_cast<DLGeneTreeLikelihood*> (tl)->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
							
						}
						else if (reconciliationModel == "COAL")
						{
							for (unsigned int i=0 ; i<allDatasets.size() ; i++) 
                            {
								//coalCounts: vector of genetreenbnodes vectors of 3 (3 directions) vectors of sptreenbnodes vectors of 2 ints
								std::vector< std::vector< std::vector< unsigned int > > > coalCounts2;
								std::vector< std::vector<unsigned int> > coalCounts3;
								std::vector< unsigned int > coalCounts4;
								for (unsigned int j = 0 ; j < 2 ; j++ ) {
									coalCounts4.push_back(0);
								}
								for (unsigned int j = 0 ; j < tree->getNumberOfNodes() ; j++ ) {
									coalCounts3.push_back(coalCounts4);
								}
								for (unsigned int j = 0 ; j < 3 ; j++ ) {
									coalCounts2.push_back(coalCounts3);
								}
								for (unsigned int j = 0 ; j < geneTree->getNumberOfNodes() ; j++ ) {
									coalCounts.push_back(coalCounts2);
								}
								TreeTemplate<Node> * treeWithSpNames = allUnrootedGeneTrees[i]->clone();
								std::vector <Node*> leaves = treeWithSpNames->getLeaves();
								for (unsigned int j =0; j<leaves.size() ; j++) 
								{
									leaves[j]->setName(allSeqSps[i][leaves[j]->getName()]);
								}
								
								treeLikelihoods.push_back( new COALGeneTreeLikelihood(*allUnrootedGeneTrees[i], *allDatasets[i], 
																					  allModels[i], allDistributions[i], *tree, 
																					  *allGeneTrees[i], *treeWithSpNames, allSeqSps[i], spId, 
																					  coalCounts, coalBls,  
																					  speciesIdLimitForRootPosition, 
																					  heuristicsLevel, MLindex, 
																					  true, true, true, true, allSprLimitGeneTree[i]) );
								delete treeWithSpNames;
								
							}
						}
                    }
                    if (currentStep>=3)// rearrange) //?
                    {
                        allParams = allParamsBackup;
                        if (recordGeneTrees==false) 
                        {
                           // firstTimeImprovingGeneTrees = true;
                            recordGeneTrees=true;
                           // bestIndex=startRecordingTreesFrom;
                        }
                    }	
                    for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
                    {
                        treeLikelihoods[i]->setSpTree(*tree);
                        treeLikelihoods[i]->setSpId(spId);
                        if (reconciliationModel == "DL") {
							/*VectorTools::print(duplicationExpectedNumbers);
							VectorTools::print(lossExpectedNumbers);*/

                            dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->setExpectedNumbers(duplicationExpectedNumbers, lossExpectedNumbers);
							//If not using the backuplks
							if (! dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->isInitialized() ) {
							dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
							}
							dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->optimizeNumericalParameters(params); //Initial optimization of all numerical parameters
							dynamic_cast<DLGeneTreeLikelihood*> (treeLikelihoods[i])->initParameters();
                        }
                        else if (reconciliationModel == "COAL") {
							if (! dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->isInitialized() ) {
                            dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->setCoalBranchLengths(coalBls);
							}
							//If not using the backuplks
							dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->initialize();//Only initializes the parameter list, and computes the likelihood through fireParameterChanged
							dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->optimizeNumericalParameters(params); //Initial optimization of all numerical parameters
							dynamic_cast<COALGeneTreeLikelihood*> (treeLikelihoods[i])->initParameters();
                        }
                    }
					if (timing) 
					{
						totalTime = ApplicationTools::getTime() - startingTime;
						std::cout << "Time for resetting gene tree likelihoods: "<<  totalTime << " s." <<std::endl;
					}
                }
                else 
                { 
                    /****************************************************************************
                     * The end, outputting the results.
                     *****************************************************************************/      
                    if (recordGeneTrees) 
                    {
                        broadcast(world, bestIndex, server);
                        std::cout << "bestIndex: "<<bestIndex<<" startRecordingTreesFrom: "<<startRecordingTreesFrom<<std::endl;
                        outputGeneTrees(assignedFilenames, treeLikelihoods, allParams, numDeletedFamilies, 
                                        reconciledTrees, duplicationTrees, lossTrees, 
                                        bestIndex, startRecordingTreesFrom, reconciliationModel);
                    }
                    break;
                }
            }//End while, END OF MAIN LOOP
            	for (unsigned int i = 0 ; i< assignedFilenames.size()-numDeletedFamilies ; i++) 
                {  			
                    if (allAlphabets[i])
                        delete allAlphabets[i];

                    if (allDatasets[i])
                        delete allDatasets[i];

                    if (allModels[i])
                        delete allModels[i];

                    if (allDistributions[i])
                        delete allDistributions[i];

                    if (allGeneTrees[i])
                        delete allGeneTrees[i];

                  /*  if (backupTreeLikelihoods[i])
                        delete backupTreeLikelihoods[i];*/
                    if (treeLikelihoods[i])
                        delete treeLikelihoods[i];

                    if (allUnrootedGeneTrees[i])
                        delete allUnrootedGeneTrees[i];  

                }

            delete nhx;
            if (geneTree) 
            {
                delete geneTree;
            }

            if (tree) delete tree;

            if (!debug) {
                cerr.rdbuf(backupcerr);    // restore cerr's original streambuf
                cout.rdbuf(backup);        // restore cout's original streambuf                                                                                                                                                  
                filestr.close();
            }
            MPI_Barrier(world);  
            MPI::Finalize( );
        }//end if a client node
	}
	catch(std::exception & e)
	{
		std::cout << e.what() << std::endl;
        MPI::COMM_WORLD.Abort(1);
		exit(-1);
	}
	return (0);
}










/*
 // This bit of code is useful to use GDB on clients, when put into the client's code:
 //launch the application, which will output the client pid
 //then launch gdb, attach to the given pid ("attach pid" or "gdb phyldog pid"), 
 //use "up" to go up the stacks, and set the variable z to !=0 to get out of the loop with "set var z = 8".
 int z = 0;
 //   char hostname[256];
 //gethostname(hostname, sizeof(hostname));
 std::cout <<"PID: "<<getpid()<<std::endl;
 std::cout <<"z: "<<z<<std::endl;
 //printf("PID %d on %s ready for attach\n", getpid(), hostname);
 // fflush(stdout);
 while (0 == z){
 std::cout <<z<<std::endl;
 sleep(5);
 }
 */





/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////RUBBISH////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//ATTEMPT 18 02 2010
//OPTIMIZE RATES
/*  SpeciesTreeLikelihood *slk = new SpeciesTreeLikelihood (world, server, tree, index, bestIndex, stop, logL, bestlogL, num0Lineages, num1Lineages, num2Lineages, bestNum0Lineages, bestNum1Lineages, bestNum2Lineages, allNum0Lineages, allNum1Lineages, allNum2Lineages, lossExpectedNumbers, duplicationExpectedNumbers, rearrange, numIterationsWithoutImprovement, branchProbaOptimization, genomeMissing);
 
 slk->getParameters().printParameters (std::cout);
 
 
 //Optimization of rates:
 PowellMultiDimensions * optimizer = new PowellMultiDimensions(slk);
 optimizer->setConstraintPolicy(AutoParameter::CONSTRAINTS_AUTO);
 optimizer->setProfiler(NULL);
 optimizer->setMessageHandler(NULL);
 optimizer->setVerbose(0);
 
 optimizer->getStopCondition()->setTolerance(0.1);
 //optimizer_->setInitialInterval(brLen.getValue(), brLen.getValue()+0.01);
 optimizer->init(slk->getParameters()); 
 optimizer->optimize(); 
 
 std::cout <<"optimization Done: "<<logL<<std::endl;
 
 lossExpectedNumbers = lossExpectedNumbers * optimizer->getParameters().getParameter("coefLoss").getValue();
 duplicationExpectedNumbers = duplicationExpectedNumbers * optimizer->getParameters().getParameter("coefDup").getValue();
 
 std::cout <<"Dup coef: "<<optimizer->getParameters().getParameter("coefDup").getValue() <<" Loss coef: "<<optimizer->getParameters().getParameter("coefLoss").getValue()<<std::endl;
 
 
 delete optimizer;
 delete slk; */


/*
 //ATTEMPT 18 02 2010
 //OPTIMIZE RATES
 SpeciesTreeLikelihood *slk = new SpeciesTreeLikelihood (world, server, tree, index, bestIndex, stop, logL, bestlogL, num0Lineages, num1Lineages, num2Lineages, bestNum0Lineages, bestNum1Lineages, bestNum2Lineages, allNum0Lineages, allNum1Lineages, allNum2Lineages, lossExpectedNumbers, duplicationExpectedNumbers, rearrange, numIterationsWithoutImprovement, branchProbaOptimization, genomeMissing);
 
 slk->getParameters().printParameters (std::cout);
 
 //Optimization:
 PowellMultiDimensions * optimizer = new PowellMultiDimensions(slk);
 optimizer->setConstraintPolicy(AutoParameter::CONSTRAINTS_AUTO);
 optimizer->setProfiler(NULL);
 optimizer->setMessageHandler(NULL);
 optimizer->setVerbose(0);
 
 optimizer->getStopCondition()->setTolerance(0.1);
 //optimizer_->setInitialInterval(brLen.getValue(), brLen.getValue()+0.01);
 optimizer->init(slk->getParameters()); 
 optimizer->optimize(); 
 
 std::cout <<"optimization Done: "<<logL<<std::endl;
 
 lossExpectedNumbers = lossExpectedNumbers * optimizer->getParameters().getParameter("coefLoss").getValue();
 duplicationExpectedNumbers = duplicationExpectedNumbers * optimizer->getParameters().getParameter("coefDup").getValue();
 
 std::cout <<"Dup coef: "<<optimizer->getParameters().getParameter("coefDup").getValue() <<" Loss coef: "<<optimizer->getParameters().getParameter("coefLoss").getValue()<<std::endl;
 
 
 delete optimizer;
 delete slk;
 */



/*This code permits outputting trees with numbers of duplication or loss events at branches
 //set total numbers of loss and duplications on branches
 setLossesAndDuplications(*bestTree, bestLossNumbers, bestDuplicationNumbers);
 std::string dupTree = ApplicationTools::getStringParameter("output.duplications.tree.file", params, "AllDuplications.tree", "", false, false); 
 std::ofstream out (dupTree.c_str(), std::ios::out);
 out << treeToParenthesisWithIntNodeValues (*bestTree, false, DUPLICATIONS)<<std::endl;
 out.close();
 std::string lossTree = ApplicationTools::getStringParameter("output.losses.tree.file", params, "AllLosses.tree", "", false, false);
 out.open (lossTree.c_str(), std::ios::out);
 out << treeToParenthesisWithIntNodeValues (*bestTree, false, LOSSES)<<std::endl;
 out.close();
 */








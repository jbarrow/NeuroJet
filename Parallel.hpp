#if defined(MULTIPROC)
////////////////////////////////////////////////////////////////
//
// FILE:                Parallel.hpp
// PURPOSE:             Provides structures and functions to manage parallel networks
// LAST MODIFIED:       11 Aug. 1999 by APS
//                      Significant portions of code due to PR
//
//                      November 1999 by PR
//                      changed order of initializing blocks in set_connectivty
//                      added a2i function so that each PE will
//                        read a punit.cfg# file, where # is PE number
//                      
//                      June 2001 by DWS and Joe
//                      updated data structures to support decaying
//                      synapses according to the RUNIT 3.0 implementation
//                      
//                      Feb 2002 by DWS
/////////////////////////////////////////////////////////////////

#  if !defined(PARALLEL_HPP)
#    define PARALLEL_HPP

///////////////////////////////
//
// Important assumption--Portions of the code below ASSUME
//                       that all nodes have the same number of neurons
//                       (ni)--this can be rectified, but it implies
//                       the following: 1) all nodes will have to
//                       know how many neurons are on their afferent
//                       neighbors; 2) the root node has to know
//                       how many neurons are on ALL nodes
//
///////////////////////////////

#    if !defined(MPI_H)
#      include "mpi.h"
#      define MPI_H
#    endif

#    include <string>
#    include <vector>
using std::string;
using std::vector;

#    if !defined(PARALLELRAND_HPP)
#      include "ParallelRand.hpp"
#    endif
#    if !defined(SYNAPSE_HPP)
#      include "Synapse.hpp"
#    endif

// This message prefaces everything written when running in
// multi-processor mode
const string P_StdMsg = "PE: ";

// This contains the node rank ID of the root processing node
const unsigned int P_ROOT_NODE_NUM = 0;

#    if !defined(MSG)
#      define MSG P_StdMsg << ParallelInfo::getRank() << ": "
#    endif

#    define IFROOTNODE if (ParallelInfo::getRank() == P_ROOT_NODE_NUM)
#    define IFCHILDNODE if (ParallelInfo::getRank() <> P_ROOT_NODE_NUM)

// Structure type ParallelInfo tracks all the information a node
// needs to store about 1 of its neighbors
class ParallelInfo {
public:
   inline ParallelInfo() : P_zi(NULL), P_sumwz(NULL) {};
   inline void initialize() {
      P_zi = new int[totalNumNrns];
      P_sumwz = new double[totalNumNrns];
   };
   inline ~ParallelInfo() { delArray(P_zi); delArray(P_sumwz); };

   static string ChildRcv();
   static void staticInitialize(int argc, char *argv[], ParallelRand &pRandComm);
   static inline void resetRandComm(const int s) { P_RandComm->ResetSeed(s); }
   static inline void setRandCommSuccess(const double p) { P_RandComm->SetParams(p); }
   static void AllocateArrays(const unsigned int ni);
   static void Barrier();
   static inline unsigned int getRank() { return P_MyRank; }
   static inline unsigned int getNumNodes() { return P_NumNodes; }
   static UIMatrix rcvNodeConn(const UIVector &Shuffle, const UIVector &FanInCon);
   static UIVector rcvNrnConn(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, const UIVector &UnShuffle, const UIVector &FanInCon, const DendriticSynapse * const * const InMatrix);
   static UIVector rcvNrnAij(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, const UIVector &FanInCon,
      const DendriticSynapse * const * const inMatrix, const UIMatrix &FanOutCon,
      const AxonalSynapse * const * const * const outMatrix,
      const unsigned int minAxonalDelay, const unsigned int maxAxonalDelay);
   static DataList rcvNrnWij(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix);
   static DataList exchangeSumwz(DataList &mySumwz); // P2P
   static DataList rcvSumwz();
   static Pattern rcvZi();
   static void RootSend(const string msg);
   static void sendNodeConn(const UIVector &Shuffle, const UIVector &FanInCon);
   static void sendNrnConn(const UIVector &Shuffle, const UIVector &UnShuffle,
      const UIVector &FanInCon, const DendriticSynapse * const * const InMatrix);
   static void sendNrnAij(const UIVector &Shuffle, const UIVector &FanInCon,
      const DendriticSynapse * const * const inMatrix, const UIMatrix &FanOutCon,
      const AxonalSynapse * const * const * const outMatrix,
      const unsigned int minAxonalDelay, const unsigned int maxAxonalDelay);
   static void sendNrnWij(const UIVector &Shuffle, const UIVector &FanInCon,
                          const DendriticSynapse * const * const inMatrix);
   static void sendSumwz(const DataList &toSend);
   static void sendZi(const Pattern &toSend);

private:
   static unsigned int totalNumNrns;
   // Total number of nodes (including root)
   static unsigned int P_NumNodes;
   // The rank (id#) of this node
   static unsigned int P_MyRank;
   static ParallelInfo *P_Nodes;
   static ParallelRand *P_RandComm;
   int *P_zi;
   double *P_sumwz;
};

#  endif                          // Termindates #if defined(PARALLEL_HPP)
#else
// We provide a dummy definition for MSG when we're compiling the
// single-processor version
#  define MSG ""
// If single-processor, this node is the root node for all intents and purposes
#  define IFROOTNODE if (true)
#endif                          // Terminates #if defined(MULTIPROC)

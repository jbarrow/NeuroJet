#if !defined(PARALLEL_HPP)
#   include "Parallel.hpp"
#endif

unsigned int ParallelInfo::totalNumNrns;
// Total number of nodes (including root)
unsigned int ParallelInfo::P_NumNodes;
// The rank (id#) of this node
unsigned int ParallelInfo::P_MyRank;
ParallelInfo * ParallelInfo::P_Nodes;
ParallelRand * ParallelInfo::P_RandComm;

void ParallelInfo::staticInitialize(int argc, char *argv[],
                                    ParallelRand &pRandComm)
{
   // Initialize MPI
   MPI_Init(&argc, &argv);
   // Get the processor number of the current node
   int rank; // can't be unsigned
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   P_MyRank = rank;
   // Get the total number of nodes
   int numNodes; // can't be unsigned
   MPI_Comm_size(MPI_COMM_WORLD, &numNodes);
   P_NumNodes = numNodes;

   P_RandComm = &pRandComm;
}

void ParallelInfo::AllocateArrays(const unsigned int ni)
{
   totalNumNrns = ni;
   P_Nodes = new ParallelInfo[P_NumNodes];
   for (unsigned int i = 0; i < P_NumNodes; i++)
      P_Nodes[i].initialize();
}

void ParallelInfo::Barrier()
{
   MPI_Barrier(MPI_COMM_WORLD);
}

UIMatrix ParallelInfo::rcvNodeConn(const UIVector &Shuffle, const UIVector &FanInCon)
{
   int  P_Tag = 0;
   MPI_Status P_Status;
   // rows are neurons, columns are nodes
   UIMatrix toReturn(totalNumNrns);
   for (unsigned int i = 0; i < totalNumNrns; i++) {
      toReturn.at(i) = UIVector(P_NumNodes);
#if !defined(PARENT_CHLD)
     //Get Root Data
     const unsigned int shuffRow = Shuffle.at(i);
     toReturn.at(i).at(0) = FanInCon.at(shuffRow);
#endif
      for (unsigned int j = 1; j < P_NumNodes; j++) {
         int  NumCon;
         MPI_Recv(&NumCon, 1, MPI_INT, j, P_Tag, MPI_COMM_WORLD, &P_Status);
         toReturn.at(i).at(j) = NumCon;
      }
   }
   return toReturn;
}

UIVector ParallelInfo::rcvNrnConn(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, const UIVector &UnShuffle, const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix)
{
   int  P_Tag = 0;
   MPI_Status P_Status;
   // the cmatrix data!!! (receive one row of cmatrix)
   UIVector toReturn;

#if !defined(PARENT_CHLD)
   //Root node accounts for its own data
   const unsigned int shuffRow = Shuffle.at(nrn);
   const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
   for (unsigned int c = 0; c < FanInCon.at(shuffRow); ++c) {
     toReturn.push_back(UnShuffle.at( dendriticTree[c].getSrcNeuron()) );
   }
#endif


   //Now get data from all other nodes 
   for (unsigned int nodeNum = 1; nodeNum < P_NumNodes; nodeNum++) {
      int NumConPerNode = nrnNodeConn.at(nodeNum);
      int *tmp_cin = new int[NumConPerNode];
      MPI_Recv(&tmp_cin[0], NumConPerNode, MPI_INT, nodeNum, P_Tag,
               MPI_COMM_WORLD, &P_Status);
      for (int i = 0; i < NumConPerNode; i++)
         toReturn.push_back(tmp_cin[i]);
      delete[] tmp_cin;
   }
   return toReturn;
}

UIVector ParallelInfo::rcvNrnAij(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, 
             const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix,
             const UIMatrix &FanOutCon, const AxonalSynapse * const * const * const outMatrix, 
             const unsigned int minAxonalDelay, const unsigned int maxAxonalDelay)
{
   int  P_Tag = 0;
   MPI_Status P_Status;
   // the cmatrix data!!! (receive one row of cmatrix)
   UIVector toReturn;

#if !defined(PARENT_CHLD)
   //Root node needs to account for its own data
    const unsigned int shuffRow = Shuffle.at(nrn);
   const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
   for (unsigned int axonalCol = 0; axonalCol < FanInCon[shuffRow]; ++axonalCol) {
     unsigned int inNeuron = dendriticTree[axonalCol].getSrcNeuron();
     const AxonalSynapse * const * const inAxon = outMatrix[inNeuron];
     unsigned int axonalDelay = 0; // not a valid value!
     for (unsigned int refTime = minAxonalDelay-1; refTime < maxAxonalDelay; ++refTime) {
       const AxonalSynapse * const axonalSegment = inAxon[refTime];
       for (unsigned int outNeuron = 0; outNeuron < FanOutCon[inNeuron][refTime]; ++outNeuron) {
    // Pointer comparsion, not value comparison
    if (axonalSegment[outNeuron].connectsTo(dendriticTree[axonalCol])) {
      axonalDelay = refTime + 1;
      break;
    }
       }
     }
     toReturn.push_back(axonalDelay);
   }
#endif

   //Now get data from all other nodes
   for (unsigned int nodeNum = 1; nodeNum < P_NumNodes; nodeNum++) {
      int NumConPerNode = nrnNodeConn.at(nodeNum);
      int *tmp_ain = new int[NumConPerNode];
      MPI_Recv(&tmp_ain[0], NumConPerNode, MPI_INT, nodeNum, P_Tag,
               MPI_COMM_WORLD, &P_Status);
      for (int i = 0; i < NumConPerNode; i++)
         toReturn.push_back(tmp_ain[i]);
      delete[] tmp_ain;
   }
   return toReturn;
}

DataList ParallelInfo::rcvNrnWij(const UIVector &nrnNodeConn, const int nrn, const UIVector &Shuffle, const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix)
{
   int  P_Tag = 0;
   MPI_Status P_Status;
   // the wmatrix data!!! (receive one row of wmatrix)
   DataList toReturn;

#if !defined(PARENT_CHLD)
   //Root node needs to account for its own data
   const unsigned int shuffRow = Shuffle.at(nrn);
   const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
   for (unsigned int c = 0; c < FanInCon[shuffRow]; ++c)
      toReturn.push_back( dendriticTree[c].getWeight() );
#endif

   //Now get data from all other nodes
   for (unsigned int nodeNum = 1; nodeNum < P_NumNodes; nodeNum++) {
      int NumConPerNode = nrnNodeConn.at(nodeNum);
      float *tmp_win = new float[NumConPerNode];
      MPI_Recv(&tmp_win[0], NumConPerNode, MPI_FLOAT, nodeNum, P_Tag,
               MPI_COMM_WORLD, &P_Status);
      for (int i = 0; i < NumConPerNode; i++)
         toReturn.push_back(tmp_win[i]);
      delete[] tmp_win;            
   }
   return toReturn;
}

DataList ParallelInfo::rcvSumwz()
{
   int P_Tag = 0;
   MPI_Status P_Status;

   // P_Info points to the info struct for the neigbor under
   // consideration
   ParallelInfo *P_Info;
   // I'm pretty sure this ParallelInfo::Barrier() call is unnecessary
   // ParallelInfo::Barrier();
   // Obvious(but EXTREMELY VITAL) point: FIRST transmit, THEN receive
   // in Punit2.0, we send only to the root node

#if defined(TIMING_MODE)
   long long elapsed, start;

   elapsed = 0;
#endif

   // Now, receive from each of our neighbors
   for (unsigned int i = 1; i < P_NumNodes; i++) {
      P_Info = &P_Nodes[i];
#if defined(TIMING_MODE)
      start = rdtsc();
#endif
      MPI_Recv(&P_Info->P_sumwz[0], totalNumNrns, MPI_DOUBLE, i, P_Tag,
               MPI_COMM_WORLD, &P_Status);
#if defined(TIMING_MODE)
      elapsed += rdtsc() - start;
#endif
   }

#if defined(TIMING_MODE)
   Output::Out() << "Elapsed root_rcv time = " << elapsed * 1.0 / TICKS_PER_SEC
           << " seconds" << endl;
#endif

   DataList toReturn(totalNumNrns, 0.0L);
   //next, consolidate the nonlocal stuff
   for (unsigned int i = 1; i < P_NumNodes; i++) {
      P_Info = &P_Nodes[i];
      for (unsigned int j = 0; j < totalNumNrns; j++) {
         toReturn.at(j) += P_Info->P_sumwz[j];
      }
   }
   return toReturn;
}

// This function replaces rcvSumwz() and sendSumwz() for PEER_TO_PEER
DataList ParallelInfo::exchangeSumwz(DataList &mySumwz)
{
  
#if defined(TIMING_MODE)                                                                                                              
  long long elapsed, start;                                                                                                       
  elapsed = 0;                                                                                                                    
#endif   
  
  DataList toReturn(totalNumNrns, 0.0L);
  
#if defined(TIMING_MODE)                                                                                                              
  start = rdtsc();                                                                                                                
#endif 
  
  MPI_Allreduce(&mySumwz[0], &toReturn[0], totalNumNrns, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
  
#if defined(TIMING_MODE)                                                                                                              
  elapsed += rdtsc() - start;
  Output::Out() << "Elapsed root_rcv time = " << elapsed * 1.0 / TICKS_PER_SEC << " seconds" << endl;
#endif
  
  return toReturn;
  
}

Pattern ParallelInfo::rcvZi()
{
   int  P_Tag = 0;
   MPI_Status P_Status;
#if defined(TIMING_MODE)
   start = rdtsc();
#endif

#if defined(RNG_BUCKET)
   P_RandComm->Recv(&(P_Nodes[0].P_zi[0]), totalNumNrns, MPI_INT,
                    P_ROOT_NODE_NUM, P_Tag, MPI_COMM_WORLD, &P_Status);
#else
   MPI_Recv(&(P_Nodes[0].P_zi[0]), totalNumNrns, MPI_INT, P_ROOT_NODE_NUM,
            P_Tag, MPI_COMM_WORLD, &P_Status);
#endif

#if defined(TIMING_MODE)
   elapsed = rdtsc() - start;
   Output::Out() << MSG << "Elapsed nonroot_rcv time = " 
                 << elapsed * 1.0 / TICKS_PER_SEC << " seconds" << endl;
#endif

   Pattern toReturn(totalNumNrns);
   for (unsigned int i = 0; i < totalNumNrns; i++)
      toReturn.at(i) = P_Nodes[0].P_zi[i];
   return toReturn;
}

void ParallelInfo::RootSend(const string msg)
{
   int  P_Tag = 0;
   for (unsigned int node = 1; node < P_NumNodes; ++node) {
      int msg_size = msg.size();
      MPI_Send(&(msg_size), 1, MPI_INT, node, P_Tag, MPI_COMM_WORLD);      
      char* toSend = new char[msg_size+1];
      strncpy(toSend, msg.c_str(), static_cast<unsigned int>(msg_size));
      MPI_Send(&(toSend[0]), msg_size, MPI_CHAR, node, P_Tag, MPI_COMM_WORLD);
      delete[] toSend;
   }
}

string ParallelInfo::ChildRcv()
{
   int  P_Tag = 0;
   MPI_Status P_Status;
   int msg_size = 0;
   MPI_Recv(&(msg_size), 1, MPI_INT, P_ROOT_NODE_NUM, P_Tag, 
            MPI_COMM_WORLD, &P_Status);
   char* toRcv = new char[msg_size+1];
   MPI_Recv(&(toRcv[0]), msg_size, MPI_CHAR, P_ROOT_NODE_NUM, P_Tag,
            MPI_COMM_WORLD, &P_Status);
   string toReturn = toRcv;
   delete[] toRcv;
   return toReturn;
}

void ParallelInfo::sendNodeConn(const UIVector &Shuffle, const UIVector &FanInCon)
{
   int  P_Tag = 0;
   for (unsigned int nrn = 0; nrn < totalNumNrns; nrn++) {
      const unsigned int shuffRow = Shuffle.at(nrn);
      int  NumCon = FanInCon.at(shuffRow);
      MPI_Send(&NumCon, 1, MPI_INT, P_ROOT_NODE_NUM, P_Tag, MPI_COMM_WORLD);
   }
}

void ParallelInfo::sendNrnConn(const UIVector &Shuffle, const UIVector &UnShuffle, const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix)
{
   int  P_Tag = 0;
   for (unsigned int nrn = 0; nrn < totalNumNrns; nrn++) {
      const unsigned int shuffRow = Shuffle.at(nrn);
      // send the cmatrix data
      unsigned int *tmp_cin = new unsigned int[FanInCon.at(shuffRow)];
      const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
      for (unsigned int c = 0; c < FanInCon.at(shuffRow); ++c) {
         tmp_cin[c] = UnShuffle.at(dendriticTree[c].getSrcNeuron());
      }
      MPI_Send(&tmp_cin[0], FanInCon.at(shuffRow), MPI_INT, P_ROOT_NODE_NUM, P_Tag,
                MPI_COMM_WORLD);
      delete[] tmp_cin;
   }
}

//FIXME: This needs to be tested!
void ParallelInfo::sendNrnAij(const UIVector &Shuffle, const UIVector &FanInCon, const DendriticSynapse * const * const inMatrix, const UIMatrix &FanOutCon, const AxonalSynapse * const * const * const outMatrix, const unsigned int minAxonalDelay, const unsigned int maxAxonalDelay)
{
   int  P_Tag = 0;
   for (unsigned int nrn = 0; nrn < totalNumNrns; ++nrn) {
      const unsigned int shuffRow = Shuffle.at(nrn);
      // send the cmatrix data
      unsigned int *tmp_ain = new unsigned int[FanInCon.at(shuffRow)];
      const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
      for (unsigned int axonalCol = 0; axonalCol < FanInCon[shuffRow]; ++axonalCol) {
         unsigned int inNeuron = dendriticTree[axonalCol].getSrcNeuron();
         const AxonalSynapse * const * const inAxon = outMatrix[inNeuron];
         unsigned int axonalDelay = 0; // not a valid value!
         for (unsigned int refTime = minAxonalDelay-1; refTime < maxAxonalDelay; ++refTime) {
            const AxonalSynapse * const axonalSegment = inAxon[refTime];
            for (unsigned int outNeuron = 0; outNeuron < FanOutCon[inNeuron][refTime]; ++outNeuron) {
               // Pointer comparsion, not value comparison
               if (axonalSegment[outNeuron].connectsTo(dendriticTree[axonalCol])) {
                  axonalDelay = refTime + 1;
                  break;
               }
            }
         }
         tmp_ain[axonalCol] = axonalDelay;
      }
      MPI_Send(&tmp_ain[0], FanInCon.at(shuffRow), MPI_INT, P_ROOT_NODE_NUM, P_Tag,
                MPI_COMM_WORLD);
      delete[] tmp_ain;
   }
}

void ParallelInfo::sendNrnWij(const UIVector &Shuffle, const UIVector &FanInCon,const DendriticSynapse * const * const inMatrix)
{
   int  P_Tag = 0;
   for (unsigned int nrn = 0; nrn < totalNumNrns; nrn++) {
      const unsigned int shuffRow = Shuffle.at(nrn);
      float *tmp_wij = new float[FanInCon.at(shuffRow)];
      const DendriticSynapse * const dendriticTree = inMatrix[shuffRow];
      for (unsigned int c = 0; c < FanInCon[shuffRow]; ++c)
         tmp_wij[c] = dendriticTree[c].getWeight();
      // send the wmatrix data
      MPI_Send(&tmp_wij[0], FanInCon.at(shuffRow), MPI_FLOAT,
               P_ROOT_NODE_NUM, P_Tag, MPI_COMM_WORLD);
   }
}

void ParallelInfo::sendSumwz(const DataList &toSend)
{
   int P_Tag = 0;

   // P_Info points to the info struct for the neigbor under consideration
   ParallelInfo *P_Info;

#if defined(TIMING_MODE)
   long long start, elapsed;
   start = rdtsc();
#endif

   P_Info = &P_Nodes[0];

   // send sumwz to the root node  
   for (unsigned int i = 0; i < toSend.size(); i++)
      P_Info->P_sumwz[i] = toSend.at(i);
#if defined(RNG_BUCKET)
   P_RandComm->Send(&P_Info->P_sumwz[0], totalNumNrns, MPI_DOUBLE,
                    P_ROOT_NODE_NUM, P_Tag, MPI_COMM_WORLD);
#else
   MPI_Send(&P_Info->P_sumwz[0], totalNumNrns, MPI_DOUBLE, P_ROOT_NODE_NUM,
            P_Tag, MPI_COMM_WORLD);
#endif

#if defined(TIMING_MODE)
   elapsed = rdtsc() - start;
   Output::Out() << MSG << "Elapsed nonroot_snd time = "
                 << elapsed * 1.0 / TICKS_PER_SEC << " seconds" << endl;
#endif      
}

void ParallelInfo::sendZi(const Pattern &toSend)
{
   int P_Tag = 0;

#if defined(TIMING_MODE)
   long long elapsed, start;
   elapsed = 0;
#endif

   // 10-2-2002, Replaced MPI_Send with MPI_Rsend
   //    this assumes that the comp nodes have already
   //    called MPI_Recv, which they should have.
   for (unsigned int i = 1; i < P_NumNodes; i++) {
#if defined(TIMING_MODE)
      start = rdtsc();
#endif
      int * tmpToSend = new int[totalNumNrns];
      for (unsigned int nrn = 0; nrn < totalNumNrns; nrn++)
         tmpToSend[nrn] = toSend.at(nrn);
      MPI_Rsend(&tmpToSend[0], totalNumNrns, MPI_INT, i, P_Tag, MPI_COMM_WORLD);
#if defined(TIMING_MODE)
      elapsed += rdtsc() - start;
#endif
   }

#if defined(TIMING_MODE)
   Output::Out() << "Elapsed root_snd time = " << elapsed * 1.0 / TICKS_PER_SEC
                 << " seconds" << endl;
#endif   
}

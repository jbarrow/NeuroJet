#if !defined(NEUROJET_HPP)
#define NEUROJET_HPP
///////////////////////////////////////////////////////////////////////////////
//
// NeuroJet.hpp
//
// Really Useful Network Interaction Tool (was RUNIT/PUNIT)
//
// Per Sederberg
// Matt Harrison
//
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <limits>
using std::string;
using std::fstream;

// NeuroJet header files
#if !defined(MATLAB_HPP)
#  include "Matlab.hpp"
#endif
#if !defined(NEURONTYPE_HPP)
#  include "NeuronType.hpp"
#endif
#if !defined(PROGRAM_HPP)
#   include "Program.hpp"
#endif
#if !defined(PARALLEL_HPP)
#   include "Parallel.hpp"
#endif
#if !defined(SYNAPSE_HPP)
#   include "Synapse.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////
// SYNFAILS_DEBUG_MODE Macros
//////////////////////////////////////////////////////////////////////////////
#if defined(SYNFAILS_DEBUG_MODE)
   /* total actual synaptic transmissions and action potentials */
#   define SYNFAILS_DEBUG_MODE_INIT    int  TotalSuccess = 0, TotalAPs = 0;    
#   define SYNFAILS_DEBUG_MODE_INC     ++TotalSuccess;
#   define SYNFAILS_DEBUG_MODE_OUTPUT  SynFailsOutput(TotalAPs, TotalSuccess);
inline void SynFailsOutput(int TotalAPs, int TotalSuccess);
#else
#   define SYNFAILS_DEBUG_MODE_INIT
#   define SYNFAILS_DEBUG_MODE_INC
#   define SYNFAILS_DEBUG_MODE_OUTPUT
#endif

//////////////////////////////////////////////////////////////////////////////
// RNG Variables
//////////////////////////////////////////////////////////////////////////////
#if defined(TIMING_RNG)
double rng_start, rng_elapsed;
#endif
#if defined(RNG_BUCK_TIMING)
double rng_start_buck, rng_elapsed_buck;
double rng_start_calc, rng_elapsed_calc;
#endif

#if defined(RNG_BUCK_USG)
int rng_buck_usage = 0;
int rng_buck_empty = 0;
int ttl_rng_buck_usage = 0;
int ttl_rng_buck_empty = 0;
int max_buck_qty = 0;
int max_rng_usg = 0;
#endif
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Timing Macro
//////////////////////////////////////////////////////////////////////////////
#if defined(TIMING_MODE) || defined(TIMING_MODE3) \
  || defined(TIMING_RNG) || defined(RNG_BUCK_TIMING)
#  define USESTIMING
#endif
#if defined(USESTIMING)
extern "C" long long rdtsc(void);
#endif
#define TICKS_PER_SEC 1533354000        // from /proc/cpuinfo
//////////////////////////////////////////////////////////////////////////////

// system include files
#include <fstream>
#include <iostream>
#include <iomanip>
#include <climits>
#include <cmath>
#include <cstdlib>

#include <vector>
#include <list>
using std::vector;
using std::list;

// for timing purposes
#  if defined(TIMING_MODE2)
clock_t t1 = 0, t2 = 0;
#endif

/* Byte type and some definitions */
#define ZERO       0
#define ONE       1
const unsigned int justNow = 0;
const unsigned int lastTime = 1;
const unsigned int defMaxAxonalDelay = 1;

////////////////////////////////////////////////////////////////////////////////
// Internally Regulated Variables
////////////////////////////////////////////////////////////////////////////////
// System (Neural Network) variables
unsigned int maxAxonalDelay;      // size of buffer for Fired
unsigned int minAxonalDelay;      // first place to start looking for synapses
unsigned int ni;                  // number of neurons
unsigned int NumNetworkCon;       // total number of recurrent connections

float K0Soma;
float KFBSoma;
float KFFSoma;
bool useSomaInh;
float K0Dend;
float KFBDend;
float KFFDend;
bool useDendInh;
                                
// Simulation variables
bool TrainingNetwork;           // a flag indicating that the network is being
                                // trained and not tested
int timeStep;                   // Current timestep
int TotalNumTied;               // total number of neurons tied last timestep
Pattern zi;                     // current firing state of neurons
xInput xAllZeros;               // zero vector input
float Threshold;                // subtractive form threshold

// SpikeRule Settings
int ruleSize;  // length of spikeTiming rule in timesteps = SystemVar::GetFloatVar("spikeRuleSize");
float muSpike; // synaptic modification constant for spike rule = SystemVar::GetFloatVar("muSpike");
int tZero;     // where in the spike rule post-pre = 0, = SystemVar::GetIntVar("spikeTzero");
float * rule;  // array of length ruleSize where the rule is.
string ruleFile;

// Neuron variables (of size ni)
float *Inhibition;
float *VarKConductanceArray;
DataList sumwz;                 // sum of the weights times the firing state
DataList dendExc;               // dendritic excitation
DataList somaExc;               // somatic excitation
vector<float*> dendriteQueue;   // Queue from dendrite to soma
// inhdiv = Divisive inhibitory interneurons
DataList sumwz_inhdiv;          // sum of the weights times the firing state
vector<float*> dendriteQueue_inhdiv; // Queue from dendrite to soma
// inhsub = Subtractive inhibitory interneurons
DataList sumwz_inhsub;          // sum of the weights times the firing state
vector<float*> dendriteQueue_inhsub; // Queue from dendrite to soma
float *IzhV;
float *IzhU;
UIVector FanInCon;              // the fan in connections of a neuron
UIMatrix FanOutCon;             // the fan out connections of a neuron per axonal delay

UIVectorDeque Fired;            // what neurons fired last n timesteps(indexed)
                                // n is determined by max axonal delay

DendriticSynapse **inMatrix;    // Fan-in synapses
AxonalSynapse ***outMatrix;     // Fan-out synapses (per axonal delay/segment)

unsigned int StartNeuron;       // Index of First Neuron a node is responsible for
unsigned int EndNeuron;         // Index of last Neuron a node is responsible for

#if defined(MULTIPROC)
UIVectorDeque FiredHere; // what neurons fired on this node last timestep
UIVector Shuffle;   // used so that externals are distributed randomly
UIVector UnShuffle; // used to make firing diagrams look nice 
#   if defined(CHECK_BOUNDS)
#      define SHUFFLEIFMULTIPROC(x) Shuffle.at(x)
#      define UNSHUFFLEIFMULTIPROC(x) UnShuffle.at(x)
#   else
#      define SHUFFLEIFMULTIPROC(x) Shuffle[x]
#      define UNSHUFFLEIFMULTIPROC(x) UnShuffle[x]
#   endif
#   define MULTIPROCFILESUFFIX(x) x + to_string(ParallelInfo::getRank())
#   define LOCALFIRED FiredHere
#else
#   define SHUFFLEIFMULTIPROC(x) (x)
#   define UNSHUFFLEIFMULTIPROC(x) (x)
#   define MULTIPROCFILESUFFIX(x) x
#   define LOCALFIRED Fired
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
inline double periodicFn(double phi, bool usesin = true) {
   if (usesin) return sin(phi); // Default periodicFn
   // First put phi on range [0, 2*pi]
   while (phi > 2 * PI) {
      phi -= 2 * PI; // FIXME: This needs to be optimized
   }
   while (phi < 0) {
      phi += 2 * PI; // FIXME: This needs to be optimized
   }
   const double offset = PI / 3;
   if (phi < offset) return -1.0L;
   const double maxVal = 0.2095; // Found from MATLAB
   phi -= offset;
   return (phi * gampdf(phi, 0.4, 1.8) * 2 / maxVal) - 1.0L;
}

// Internal Functions
void AllocateMemory();
vector<float> assignIzhParams(const string &IzhNeuronType);
void CalcDendriticExcitation();
void CalcDendriticToSomaInput(const xInput& curPattern, const bool isComp);
double CalcFBInternrnExcitation();
double CalcFFInternrnExcitation(const xInput &curPattern);
void CalcSomaResponse(const xInput &curPattern, DataMatrix &IzhVValues,
                      DataMatrix &IzhUValues);
void CalcSynapticActivation(const UIVectorDeque &FiredArray, 
                            const Pattern &inPattern);
void CalcSynapticActivation(const UIVectorDeque &FiredArray, 
                            const xInput &curPattern);
bool chkDataExists(const StrArg &DataName, const DataListType newDataType,
                   const string FunctionName, const CommandLine &ComL);
void CheckIzhikevich();
void createSelectArray(vector<IxSumwz> &excSort, const xInput &curPattern,
                       const int startN, const int endN);
void DeAllocateMemory();
void enqueueDendriticResponse(const DataList& dendriticResponse, const DataList& dendResp_inhdiv,
                              const DataList& dendResp_inhsub);
const NeuronType* findNeuronType(const unsigned int nrn);
void FireNonTiedNeurons(const unsigned int numLeft2Fire, const vector<IxSumwz> &excSort);
void FireSingleNeuron(const int nrn);
void FireTiedNeurons(const unsigned int numLeft2Fire, const double cutOff,
                     vector<IxSumwz> &excSort);
list<xInput> GenerateInputSequence(UIPtnSequence &Seq,
   const float inputNoise, const float exactNoise, int& SumExtFired,
   int& PatternCount);
void GetConnectivity(string filename);

inline void setSpikeRule() {
   ruleFile = SystemVar::GetStrVar("ruleFile");
   Output::Out() << "Loading " << ruleFile << "..." << std::endl;
		
   const int len = ruleFile.size();
   char thisFile[len]; 
   strcpy(thisFile,ruleFile.c_str());	

   fstream file;
   file.open(thisFile, fstream::in);
   if(! file.is_open()) {
   	CALL_ERROR << "Unable to open "<< ruleFile <<".\n"<< ERR_WHERE;
   	exit(EXIT_FAILURE);
   }
   char ch[20]; //floats are never more than 20 digits?
   float temp[50]; //create temp to store filter values max is 50?
   float x = 0.0f;
   int i = 0;	
   while (!file.eof()) {
   	file.getline(ch,20,' '); //get first number in file, space delimited
   	if(ch[0] == '\0') //if null, continue
   	   continue;
   	string str (ch);
   	if(sscanf(ch, "%f", &x) == 0) //convert the string into a float
   	   break;
   	temp[i++] = x; //store the float in the temp
   }
   file.close();

   //values are in temp, there are i of them
   // There is NO CHECK for good values here, because each element of 
   // the spikeRule could be any float, (though values >-10 and <10 
   // make the most sense. 
   ruleSize = i;
   rule = new float[i];
   bool check = false;
   for (int it = 0; it < i; it++) {
      rule[it] = temp[it];
      if(rule[it] > 1.0f) check = true;
//      Output::Out() << "rule[it] = " << rule[it] << std::endl;      	
   }

   if( check) Output::Out() << "Some values in the spikeRule are greater than 1..." << std::endl;

}

inline void eat_whitespace(std::istream &in)
{
   while (isspace(in.peek())) {
      in.get();
   }
}

template <class T> void read_without_comments(std::istream &in, T &var)
{
   eat_whitespace(in);
   while (in.peek() == '#' || in.peek() == '\n')
   {
      in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
      eat_whitespace(in);
   }
   in >> var;
}

void checkNextChar(bool& inIt, unsigned int& depthIn, const bool inString, const char nextChar, 
                   const char deeperChar, const char shallowerChar);
void PopulatePopulation(const string& globalProps, const string& neuronProps,
                        const string& synapseProps, UIMatrix& effDelays);
std::string debracket(const std::string& toDebracket, char beginToken, char endToken);
void FillFanOutMatrices();
inline void GetNullTimingData();
void getThetaSettings(const NeuronType& NeurType, int& Period, float& Amplitude,
                      float& MidPoint, float& Phase, bool& UseSin);
inline void InitCurBucketStats();
bool isNJNetworkFileType(const string& filename);
bool isNumeric(const string& toCheck);
inline bool isLocalNeuron(const unsigned int nrn);
std::map<std::string, std::string> ParseStruct(const std::string& toParse);
void Present(const xInput &curPattern, DataMatrix &IzhVValues, DataMatrix &IzhUValues,
             const bool modifyInhWeights, const bool modifyExcWeights);
void CompPresent(const xInput &curPattern, const bool isTesting);
void readDataType(const StrArg &Type, DataListType &newDataType, string &newType,
                  string &newSubType, const string FunctionName,
                  const CommandLine &ComL, const bool allowFile = false);
MatlabCommand ReadMATLABcommand(ifstream& mfile, const string& filename);
void ReadNJNetworkFile(const string& filename);
void ReadPopulationFile(const string& filename, UIMatrix& effDelays);
inline void RecordSynapticFiring(const int &neuron, const string &); // AG
void resetDendriticQueues();
void ResetSTM();
double selectCutOff(unsigned long k, unsigned long n, vector<IxSumwz> &arr);
void SetConnectivity(const int &AllowSelf = true, const char &dType =
                     'p', const float &p1 = 0.0f, const float &p2 =
                     1.0f, const float &p3 = 0.0f, const float &p4 = 1.0f);
inline void UpdateBucketStats();
void UpdateBuffers(UIPtnSequence &FiringPtns, UIPtnSequence &ExtPtns,
     DataMatrix &BusLines, DataMatrix &IntBusLines,
     DataMatrix &KWeights, DataMatrix &Inhibitions, DataMatrix &FBInternrnExcs,
     DataMatrix &FFInternrnExcs, DataList &ActVect, DataList &ThreshVect,
     const unsigned int ndx, const xInput &curPattern,
     const vector<bool> &RecordIdxList);
inline void UpdateMaxBucketStats();
void UpdateParams(const string &, const string &, const string &);
inline void UpdateWeights();
void WriteMATLABHeader(std::ofstream &MATfile, int mrows, int ncols, int namlen, 
                       bool isSparse, bool isText);
                          
// AtFunctions
void AddInterneuron(ArgListType &arg);
void Analysis(ArgListType &arg);
void AppendData(ArgListType &arg);
void CombineData(ArgListType &arg);
void Context(ArgListType &arg);
void CopyData(ArgListType &arg);
void CreateAnalysis(ArgListType &arg);
void CreateNeuronType(ArgListType &arg);
void CreateNetwork(ArgListType &arg);
void CreateSynapseType(ArgListType &arg);
void CreateVar(ArgListType &arg);
void DeleteData(ArgListType &arg);
void FileReset(ArgListType &arg);
void LoadData(ArgListType &arg);
void MakeSequence(ArgListType &arg);
void MakeRandSequence(ArgListType &arg);
void ResetFiring(ArgListType &arg);
void SaveData(ArgListType &arg);
void SaveWeights(ArgListType &arg);
void SeedRNG(ArgListType &arg);
void SetLoopVar(ArgListType &arg);
void SetStream(ArgListType &arg);
void SetVar(ArgListType &arg);
void Sim(ArgListType &arg);
void Test(ArgListType &arg);
void Train(ArgListType &arg);

// CaretFunctions
string Num2Int(ArgListType &arg);
string PatternLength(ArgListType &arg);
string SequenceLength(ArgListType &arg);
string PickSeq(ArgListType &arg);
string RandomSeed(ArgListType &arg);
string SumData(ArgListType &arg);

void InitializeProgram();
void BindUserFunctions();

// T=Pattern => Sequence, T=Vector => Matrix, T=DataList => Analysis
// T=UIVector => UIPtnSequence
template<class S, class T>
S **Convert2Mat(list<T> &Seq, int patstart,
                       int patend, int bitstart, int bitend,
                       const bool transpose = false,
                       const S &fillval = 0)
{
   int pattot = patend - (--patstart); // end - start + 1
   int bittot = bitend - (--bitstart); // end - start + 1
   int numMtxRows;
   int numMtxCols;
   if (transpose) {
       numMtxRows = bittot;
       numMtxCols = pattot;
   } else {
       numMtxRows = pattot;
       numMtxCols = bittot;
   }
   S **RetMat = new S *[numMtxRows];
   for (int fillRow = 0; fillRow < numMtxRows; fillRow++) {
      RetMat[fillRow] = new S[numMtxCols];
      for (int col = 0; col < numMtxCols; col++) {
         RetMat[fillRow][col] = fillval;
      }
   }
   typename list<T>::const_iterator it = Seq.begin();
   for (int i = 0; i < patstart; i++, it++) {}
   for (int row = 0; row < pattot; row++, it++) {
      const T temppat = *it;
      int patlen = temppat.size();
      if (patlen > bitend) {
         patlen = bitend;
      }
      int col = 0;
      for (int j = bitstart; j < patlen; j++, col++) {
         if (transpose) {
            RetMat[col][row] = static_cast<S>(temppat.at(j));
         } else {
            RetMat[row][col] = static_cast<S>(temppat.at(j));
         }
      }
   }
   return RetMat;
}

template<class T>
inline unsigned int findMaxSize(const T &dataIn, const unsigned int PriorMax = 0)
{
   typename T::const_iterator it;
   unsigned int MaxSize = PriorMax;
   for (it = dataIn.begin(); it != dataIn.end(); it++) {
      updateMax(MaxSize, static_cast<unsigned int>(it->size()));
   }
   return MaxSize;
}

template<> // Template specialization
inline unsigned int findMaxSize(const UIPtnSequence &dataIn, const unsigned int PriorMax)
{
   unsigned int MaxSize = PriorMax;
   for (UIPtnSequenceCIt it = dataIn.begin(); it != dataIn.end(); it++) {
      updateMax(MaxSize, it->back()+1); // assumes ptns in dataIn are sorted
   }
   return MaxSize;
}

template<class T>
void writeData(ostream &fout, const int width, const T &data,
               const int startTime, const int endTime, const int startN,
               const int endN)
{
   typename T::const_iterator it = data.begin();
   // increment to the start position
   for (int inc = 0; inc < startTime-1; inc++, it++) {}
   for (int i = startTime; i <= endTime; i++, it++) {
     // 0-based vs. 1-based causes - 1 below
      for (int j = startN - 1; j < endN - 1; j++) {
         fout << std::setw(width) << it->at(j);
      }
      fout << "\n";
   }
}

// User include files
#include "User.hpp"

#endif

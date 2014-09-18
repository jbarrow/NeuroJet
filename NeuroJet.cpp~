/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 3; tab-width: 3 -*- */

///////////////////////////////////////////////////////////
//
// NeuroJet
//
// Scott Witherell
// Aaron Shon
// Paul Rodriguez
// et alia
//
// July 2000
//
// merged with RUNIT 3.0 by Dave Sullivan and Joe
// June 2001
// Debugged by Dave, Feb 2002
// completely absorbed RUNIT by Ben Hocking
// June 2004
//
// NMDArise changed by Blake Thomas
// July 2009
//
// Added more Filter functionality by Blake Thomas
// December 2011
// 
////////////////////////////////////////////////////////////

//CHIP
// Two classes of new components: Ones that use variables that already exist(or fewer), and ones that use new
//   variables
// Need instructions for how to use new variables in these components
// Eventually, want to facilitate modifications for arbitrary time-scales
// Need to correct ability to stop and restart NeuroJet - currently no way to load PyrToInternrnWt values as
//   well as any other state variables
// Need test to verify that restarting NeuroJet after reloading state yields equivalent results to continuing
//   NeuroJet
// Eventually, we want to add more compartments to a neuron

//#define USESTIMING
//#define TIMING_P2P
//#define TIMING_MODE
//#define TIMING_MODE2
//#define TIMING_MODE3
//#define TIMING_RNG
//#define RNG_BUCK_TIMING
//#define RNG_BUCKET
//#define RNG_BUCK_USG
//#define PARENT_CHILD

// PEER_TO_PEER is default for MULTIPROC
#if defined(MULTIPROC) && !defined(PARENT_CHILD)
#  define PEER_TO_PEER
#endif

#include <string.h>

#if defined(TIMING_P2P)
long trials;
double total_time;
#endif

//TIMING_MODE => TIMING_MODE3
#if defined(TIMING_MODE) && !defined(TIMING_MODE3)
#  define TIMING_MODE3
#endif

//!RNG_BUCKET => !RNG_BUCK_USG
#if defined(RNG_BUCK_USG) && !defined(RNG_BUCKET)
#  undef RNG_BUCK_USG
#endif

#if !defined(NEUROJET_HPP)
#  include "NeuroJet.hpp"
#endif
#if !defined(CALC_HPP)
#  include "Calc.hpp"
#endif
#if !defined(PARSER_HPP)
#  include "Parser.hpp"
#endif
#if !defined(POPULATION_HPP)
#  include "Population.hpp"
#endif

using namespace std;

// version information
#if defined(MULTIPROC)
const string NeuroJetVersionText = "NeuroJet (multi-processor)";
#else
const string NeuroJetVersionText = "NeuroJet v1.B.??? (single-processor)";
#endif
const string NeuroJetLastUpdateText = "Last changed on Thu Mar 22 01:41:13 EST 2012";
const string NeuroJetLastUpdateAuth = "Last changed by btt2h";

#if defined(WIN32)
#  define IOS ios_base
#else
#  define IOS ios
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Definitions
////////////////////////////////////////////////////////////////////////////////

// Internal Functions
//#if defined(WIN32)
//int _tmain(int argc, char * argv[])
//#else
int main(int argc, char * argv[])
//#endif
{
   Output::setStreams(cout, cerr);
   program::initMain();
#if defined(MULTIPROC)
   ParallelInfo::staticInitialize(argc, argv, ParallelRand::RandComm);
   // Report whether we're running the parallel or serial version
   IFROOTNODE {
      Output::Out() << std::endl << NeuroJetVersionText << std::endl
         << NeuroJetLastUpdateText << std::endl << NeuroJetLastUpdateAuth
         << std::endl << std::endl;
   }
   // The next 2 barriers are just to make the output pretty
   ParallelInfo::Barrier();
   Output::Out() << MSG << " Parallel Version: Node " << ParallelInfo::getRank() 
           << " out of " << ParallelInfo::getNumNodes() << std::endl;
   if (ParallelInfo::getNumNodes() == 1) {
      CALL_ERROR << "The MPI version must run with at least 2 nodes!"
                 << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   ParallelInfo::Barrier();
#else
   Output::Out() << std::endl << NeuroJetVersionText << std::endl
         << NeuroJetLastUpdateText << std::endl << NeuroJetLastUpdateAuth
         << std::endl << std::endl;
#endif

   // Initialize
   InitializeProgram();
   
   // Check Command lines
   if (argc == 1) {
      CALL_ERROR << argv[0] << " has no default action. Try the -help option\n"
         "or give a script filename.\n\n" << ERR_WHERE;
      program::Main().PrintHelp("help");
      return 1;
   }

   if ("-help" == string(argv[1])) {
      if (argc == 2) {
         program::Main().PrintHelp();
         Output::Err() << argv[0] << " 'file'\n\twill run 'file'." << std::endl;
      } else if (argc == 3) {
         program::Main().PrintHelp(argv[2]);
      } else if (argc == 4) {
         program::Main().PrintHelp(argv[2], argv[3]);
      } else {
         CALL_ERROR << "Unrecognized help sequence... too many parameters.\n\n" << ERR_WHERE;
         program::Main().PrintHelp("help");
      }
      return 1;
   }

#if !defined(MULTIPROC) // mpirun _MIGHT_ add additional arguments
   if (argc != 2) {
      CALL_ERROR << "Too many arguments." << ERR_WHERE;
      return 1;
   }
#endif

   IFROOTNODE {
      Output::Out() << "Last compiled on: " << __DATE__ << std::endl;
   }

#if defined(MULTIPROC)
   // Set input file to a variable   
   IFROOTNODE {
      SystemVar::SetStrVar("InputFile", argv[1]);
      ParallelInfo::RootSend(SystemVar::GetStrVar("InputFile"));
   } else {
      // Depending on version of mpirun, the child node might get bad info on args
      SystemVar::SetStrVar("InputFile", ParallelInfo::ChildRcv());
   }
#else
   SystemVar::SetStrVar("InputFile", argv[1]);
#endif
   //Output::Out() << MSG << "InputFile = " << SystemVar::GetStrVar("InputFile") << std::endl;

   // **********All nodes stop here!**************

#if defined(TIMING_MODE3)
   long long start;
   long long finish;

#   if defined(MULTIPROC)
   ParallelInfo::Barrier();
#   endif
   start = rdtsc();
#endif

   // Parse the input file
   Parser::ParseScript(SystemVar::GetStrVar("InputFile"));

#if defined(TIMING_MODE3)
#   if defined(MULTIPROC)
   ParallelInfo::Barrier();
#   endif
   finish = rdtsc();
   IFROOTNODE {
      Output::Out() << "Elapsed total time = " << (finish - start) * 1.0 /
         TICKS_PER_SEC << " seconds" << std::endl;
   }
#endif

#if defined(RNG_BUCK_USG)
   Output::Out() << MSG << "Max RNG bucket contents: " << max_buck_qty << std::endl;
   Output::Out() << MSG << "Max RNG usage per cycle: " << max_rng_usg << std::endl;
   Output::Out() << MSG << "Total RNG's available in bucket: "
           << ttl_rng_buck_usage << std::endl;
   Output::Out() << MSG << "Total RNG's unavailable in bucket: "
           << ttl_rng_buck_empty << std::endl;
#endif

#if defined(RNG_BUCK_TIMING)
   Output::Out() << MSG << "Elapsed total_rng_buck time = "
           << rng_elapsed_buck / TICKS_PER_SEC << " seconds" << std::endl;
   Output::Out() << MSG << "Elapsed total_rng_calc time = "
           << rng_elapsed_calc / TICKS_PER_SEC << " seconds" << std::endl;
#endif
#if defined(TIMING_RNG)
   Output::Out() << MSG << "Elapsed total_rng time = "
           << rng_elapsed / TICKS_PER_SEC << " seconds" << std::endl;
#endif

#if defined(TIMING_MODE)
   long long time1;
   long long time2,
   long long time3;
   time1 = rdtsc();
   time2 = rdtsc();
   usleep(10000);
   time3 = rdtsc();
   Output::Out() << "Number of clock ticks since boot: " << time3 << std::endl;
   Output::Out() << "Time for a clock access: " << (time2 - time1) << " ticks, "
      << (time2 - time1) * 1000000.0 / TICKS_PER_SEC << " microseconds" << std::endl;
   Output::Out() << "OS approximation for 10 millisecond sleep: " << (time3 - time2)
      << " ticks, " << (time3 -
                        time2) * 1000.0 / TICKS_PER_SEC << " milliseconds" << std::endl;
   time2 = rdtsc();
   usleep(100000);
   time3 = rdtsc();
   Output::Out() << "OS approximation for 100 millisecond sleep: " << (time3 - time2)
      << " ticks, " << (time3 - time2) * 1000.0 / TICKS_PER_SEC
      << " milliseconds" << std::endl;
#endif

#if defined(MULTIPROC)
   MPI_Finalize();
#endif

   // Done.
   return 0;
}

inline void InitCurBucketStats()
{
#if defined(RNG_BUCK_USG)
   rng_buck_usage = 0;
   rng_buck_empty = 0;
#endif
}

void InitializeProgram()
{
   /***********************************************************
    * Must first set variables, adding them to the data lists *
    ***********************************************************
    * User Setable Variables
   */
#if defined(TIMING_RNG)
   rng_elapsed = 0;
#endif

#if defined(RNG_BUCK_TIMING)
   rng_elapsed_buck = 0;
   rng_elapsed_calc = 0;
#endif

   // Set default values
   SystemVar::AddIntVar("seed", 1);
   SystemVar::AddIntVar("ni", 1000);
   SystemVar::AddFloatVar("wStart", 0.4f);
   SystemVar::AddFloatVar("ZeroCutOff", 0.001f);
   SystemVar::AddFloatVar("Activity", 0.0f);
   SystemVar::AddFloatVar("synFailRate", 0.0f);
   SystemVar::AddFloatVar("xNoise", 0.0f);
   SystemVar::AddFloatVar("xNoiseF", 0.0f);
   SystemVar::AddFloatVar("xTestingNoise", 0.0f);
   SystemVar::AddFloatVar("xTestingNoiseF", 0.0f);
   SystemVar::AddFloatVar("yDecay", 0.0f);
   SystemVar::AddFloatVar("lambdaFB", 0.0f);
   SystemVar::AddFloatVar("lambdaFF", 0.0f);
   SystemVar::AddFloatVar("PyrToInternrnWtAdjDecay", 0.0f);
   SystemVar::AddIntVar("UseWeightedActAvg", 0);
   SystemVar::AddFloatVar("WeightedActAvgAdj", 1.0f);
   SystemVar::AddIntVar("UseSin", true);
   SystemVar::AddIntVar("Period", 9);
   SystemVar::AddFloatVar("Amplitude", 0.0f);
   SystemVar::AddFloatVar("MidPoint", 1.0f);
   SystemVar::AddFloatVar("Phase", 1.0f);
   SystemVar::AddFloatVar("theta", 0.5f);
   SystemVar::AddFloatVar("ExtExc", 0.0f);
   SystemVar::AddFloatVar("DGstrength", 0.0f);
   SystemVar::AddFloatVar("Con", 0.1f);
   SystemVar::AddIntVar("Reset", 1);
   SystemVar::AddFloatVar("ResetAct", -1.0f);
   SystemVar::AddFloatVar("mu", 0.0f);
   SystemVar::AddFloatVar("KFBDend", 0.0f);
   SystemVar::AddFloatVar("KFFDend", 0.0f);
   SystemVar::AddFloatVar("K0Dend", 0.0f);
   SystemVar::AddFloatVar("KFB", 1.0f);
   SystemVar::AddFloatVar("KFF", 0.0f);
   SystemVar::AddFloatVar("K0", 0.0f);
   SystemVar::AddFloatVar("alpha", -1.0f);
   SystemVar::AddIntVar("UseMvgAvg", false);
   SystemVar::AddIntVar("UseAltSynapse", false);

   //NMDA RISE SYSTEM VARIABLES
   SystemVar::AddIntVar("NMDArise", 0);
   SystemVar::AddStrVar("riseFile", "");
   SystemVar::AddIntVar("saveZbarArray", 0);
   SystemVar::AddFloatVar("resolution", 0.0000001f);
   SystemVar::AddFloatVar("inv", 10000000);
   SystemVar::AddIntVar("stochastic", 0);

   //Customizable Dendritic Filter Settings
   SystemVar::AddStrVar("filterFile", "");
   SystemVar::AddStrVar("InterNeuronfilterFile", "");
   SystemVar::AddIntVar("FBadjustNumToFire", 0);
   SystemVar::AddFloatVar("FBadjust", 0.08333333f);

	//boolean int spikeTimingRule
	SystemVar::AddIntVar("spikeTimingRule", 0);
   SystemVar::AddFloatVar("muSpike", 0.0f);
	SystemVar::AddIntVar("spikeTzero", 0);
	SystemVar::AddIntVar("spikeRuleSize", 0);
   SystemVar::AddStrVar("ruleFile", "");

   SystemVar::AddFloatVar("DenomMult", 1.0f);
   SystemVar::AddFloatVar("DumpConst", 0.0f);
   SystemVar::AddIntVar("DumpDendrite", 0);
   SystemVar::AddFloatVar("CAconst", 0.0f);
   SystemVar::AddFloatVar("VarKConductance", 0.0f);
   SystemVar::AddStrVar("ReadWeights", "");
   SystemVar::AddStrVar("ResetPattern", "");
   SystemVar::AddStrVar("title", "");

   SystemVar::AddStrVar("DendriteToSomaFilter", "");
   SystemVar::AddFloatVar("filterDecay", 1.0f);
   SystemVar::AddStrVar("SynapseFilter", "");
   SystemVar::AddIntVar("WtFiltIsGeneric", 0);

   SystemVar::AddFloatVar("InternrnExcDecay", 1.0f);
   SystemVar::AddIntVar("FBInternrnAxonalDelay", 1);
   SystemVar::AddIntVar("FFInternrnAxonalDelay", 1);

   // Izhikevich neuron type - automatically sets other Izhikevich variables,
   // but these can be later overridden. Types are A-T and correspond to his
   // figure 1 in several papers (e.g., IEEE NNS, 2004)
   SystemVar::AddStrVar("IzhType", "");
   // Izhikevich explicit variables - currently, they should all be set or
   // none should be set. Also, deltaT should be set.
   SystemVar::AddFloatVar("IzhvStart", -1.0f);
   SystemVar::AddFloatVar("IzhuStart", -1.0f);
   SystemVar::AddFloatVar("IzhA", -1.0f);
   SystemVar::AddFloatVar("IzhB", -1.0f);
   SystemVar::AddFloatVar("IzhC", -1.0f);
   SystemVar::AddFloatVar("IzhD", -1.0f);
   SystemVar::AddFloatVar("deltaT", -1.0f); // in ms
   // Izhikevich implicit variables - these do not have to be set if A-D are
   // set, but they will not be used if A-D are not set. NB: these values
   // differ from the 5 and 140 commonly reported by Izhikevich in his papers,
   // but these are values used for his integrator neuron
   SystemVar::AddFloatVar("IzhE", 4.1f);
   SystemVar::AddFloatVar("IzhF", 108.0f);
   SystemVar::AddFloatVar("IzhVMax", 30.0f);
   SystemVar::AddFloatVar("IzhIMult", 10.0f);
   SystemVar::AddFloatVar("IzhTimeThresh", 1.0f);
   // Do NOT track Izh buffers by default (takes more memory)
   SystemVar::AddIntVar("IzhTrackData", 0);

   // Internally regulated Variables, but outside readable
   SystemVar::AddStrVar("Version", NeuroJetVersionText, true);
   SystemVar::AddStrVar("LastUpdate", NeuroJetLastUpdateText, true);
   SystemVar::AddIntVar("TrainingCount", 0, true);
   SystemVar::AddFloatVar("AveThreshold", 0.0f, true);
   SystemVar::AddIntVar("NumTiesPicked", 0, true);
   SystemVar::AddFloatVar("AveTrainAct", 0.0f, true);
   SystemVar::AddFloatVar("AveTrainTies", 0.0f, true);
   SystemVar::AddFloatVar("AveTrainExt", 0.0f, true);
   SystemVar::AddFloatVar("AveTrainInt", 0.0f, true);
   SystemVar::AddFloatVar("AveTestAct", 0.0f, true);
   SystemVar::AddFloatVar("AveTestExt", 0.0f, true);
   SystemVar::AddFloatVar("AveTestInt", 0.0f, true);
   SystemVar::AddFloatVar("AveConLen", 0.0f, true);
   SystemVar::AddFloatVar("AveConLen0", 0.0f, true);
   SystemVar::AddFloatVar("FracUnused", 0.0f, true);
   SystemVar::AddFloatVar("FracRefired", 0.0f, true);
   SystemVar::AddIntVar("NumUnused", 0, true);
   SystemVar::AddIntVar("NumRefired", 0, true);
   SystemVar::AddFloatVar("VarConLen", 0.0f, true);
   SystemVar::AddFloatVar("VarConLen0", 0.0f, true);
   SystemVar::AddFloatVar("AveWij", 0.0f, true);
   SystemVar::AddFloatVar("AveWij0", 0.0f, true);
   SystemVar::AddFloatVar("FracZeroWij", 0.0f, true);
   SystemVar::AddFloatVar("FracConnect", 0.0f, true);
   SystemVar::AddStrVar("InputFile", "", true);

   // Internally regulated Variables
   program::Main().setNetworkCreated(false);
   maxAxonalDelay = defMaxAxonalDelay;
   minAxonalDelay = defMaxAxonalDelay;
   unsigned int oldni = ni;
   ni = SystemVar::GetIntVar("ni");
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      if ((pIt->getFirstNeuron() == 0) && (pIt->getLastNeuron() == oldni-1)) {
         pIt->setNeuronRange(0, ni-1);
      }
   }
   NumNetworkCon = 0;
   VarKConductanceArray = NULL;
   Inhibition = NULL;
   K0Dend = SystemVar::GetFloatVar("K0Dend");
   KFBDend = SystemVar::GetFloatVar("KFBDend");
   KFFDend = SystemVar::GetFloatVar("KFFDend");
   useDendInh = false;
   K0Soma = SystemVar::GetFloatVar("K0");
   KFBSoma = SystemVar::GetFloatVar("KFB");
   KFFSoma = SystemVar::GetFloatVar("KFF");
   useSomaInh = false;

   inMatrix = NULL;
   outMatrix = NULL;

   Output::setStreams(cout, cerr);

#if defined(MULTIPROC)
   // One of the nodes must be the root node.  Only the root
   // node has access to certain "global" analysis variables
   // which summarize data from all nodes.
   IFROOTNODE {
      // The root node maintains global variables for
      // average context length, average non-zero context length,
      // variance in context length, variance in non-zero
      // context length, number unused, and number refired      
      SystemVar::AddFloatVar("GlobalAveConLen", 0.0f, true);
      SystemVar::AddFloatVar("GlobalAveConLen0", 0.0f, true);
      SystemVar::AddFloatVar("GlobalVarConLen", 0.0f, true);      
      SystemVar::AddFloatVar("GlobalVarConLen0", 0.0f, true);
      SystemVar::AddIntVar("GlobalNumUnused", 0, true);
      SystemVar::AddIntVar("GlobalNumRefired", 0, true);
   }
#endif

   // Add AtFunctions
   SystemVar::AddAtFun("AddInterneuron", AddInterneuron);
   SystemVar::AddAtFun("Analysis", Analysis);
   SystemVar::AddAtFun("AppendData", AppendData);
   SystemVar::AddAtFun("CombineData", CombineData);
   SystemVar::AddAtFun("Context", Context);
   SystemVar::AddAtFun("CopyData", CopyData);
   SystemVar::AddAtFun("CreateAnalysis", CreateAnalysis);
   SystemVar::AddAtFun("CreateNeuronType", CreateNeuronType);
   SystemVar::AddAtFun("CreateNetwork", CreateNetwork);
   SystemVar::AddAtFun("CreateSynapseType", CreateSynapseType);
   SystemVar::AddAtFun("CreateVar", CreateVar);
   SystemVar::AddAtFun("DeleteData", DeleteData);
   SystemVar::AddAtFun("FileReset", FileReset);
   SystemVar::AddAtFun("LoadData", LoadData);
   SystemVar::AddAtFun("MakeRandSequence", MakeRandSequence);
   SystemVar::AddAtFun("MakeSequence", MakeSequence);
   SystemVar::AddAtFun("ResetFiring", ResetFiring);
   SystemVar::AddAtFun("SaveData", SaveData);
   SystemVar::AddAtFun("SaveWeights", SaveWeights);
   SystemVar::AddAtFun("SeedRNG", SeedRNG);
   SystemVar::AddAtFun("SetLoopVar", SetLoopVar);
   SystemVar::AddAtFun("SetStream", SetStream);
   SystemVar::AddAtFun("SetVar", SetVar);
   SystemVar::AddAtFun("Sim", Sim);
   SystemVar::AddAtFun("Test", Test);
   SystemVar::AddAtFun("Train", Train);

   // Add CaretFunctions
   SystemVar::AddCaretFun("Num2Int", Num2Int);
   SystemVar::AddCaretFun("PatternLength", PatternLength);
   SystemVar::AddCaretFun("SequenceLength", SequenceLength);
   SystemVar::AddCaretFun("PickSeq", PickSeq);
   SystemVar::AddCaretFun("RandomSeed", RandomSeed);
   SystemVar::AddCaretFun("SumData", SumData);

	// Variable Threshold
	SystemVar::AddIntVar("VariableThreshold", 0, false);

   // Load user stuff
   BindUserFunctions();
   return;
}

bool isNJNetworkFileType(const string& filename) {
   ifstream chkFile(filename.c_str());
   if (!chkFile.is_open()) {
      CALL_ERROR << "Unable to open weight file " << filename << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   string lineBuf;
   bool foundLine = false;
   while ((!foundLine) && std::getline(chkFile, lineBuf)) {
      lineBuf = ltrim(lineBuf); // Remove leading whitespace
      const string commentChars = "#%/";
      if (lineBuf.size() > 0 && commentChars.find(lineBuf[0]) == std::string::npos) {
         foundLine = true;
      }
   }
   chkFile.close();
   bool isDesiredType = false;
   if (foundLine && !isNumeric(lineBuf)) {
      if (fileExists(lineBuf)) {
         isDesiredType = true;
      } else {
         CALL_ERROR << "Unable to open population file " << lineBuf
                    << " in weight file " << filename << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
   }
   return isDesiredType;
}

bool isNumeric(const string& toCheck) {
   bool toReturn = true;
   for (unsigned int i = 0; i < toCheck.length(); ++i) {
      if (!(isdigit(toCheck[i]) || (toCheck[i] == '.')))
         toReturn = false;
   }
   return toReturn;
}

void checkNextChar(bool& inIt, unsigned int& depthIn, const bool inString, const char nextChar, 
                   const char deeperChar, const char shallowerChar) {
   if (inIt) {
      if (nextChar == deeperChar) ++depthIn;
      if (nextChar == shallowerChar) --depthIn;
      if (depthIn == 0) inIt = false;
   } else if ((depthIn == 0) && !inString) {
      if (nextChar == deeperChar) ++depthIn;
      if (depthIn > 0) inIt = true;
   }
}

void PopulatePopulation(const string& globalProps, const string& neuronProps,
                        const string& synapseProps, UIMatrix& effDelays)
{
   map<string, string> globalVars = ParseStruct(globalProps);
   // File format has all neurons as 1-based. Internally, they are 0-based.
   unsigned int firstN = from_string<unsigned int>(debracket(globalVars["first"], '{', '}')) - 1;
   unsigned int lastN = from_string<unsigned int>(debracket(globalVars["last"], '{', '}')) - 1;
   // Currently, this bracket is here to manage memory - should be its own function
   {
      map<string, string> neuronVars = ParseStruct(neuronProps);
      vector<string> faninVec = tokenize(debracket(neuronVars["fanin"], '{', '}'), ',', "");
      vector<string> typeVec = tokenize(debracket(neuronVars["type"], '{', '}'), ',', "");
      if (faninVec.size() != (lastN - firstN + 1)) {
         CALL_ERROR << "Fan-in vector of incorrect size:\n"
                    << "  Size:     " << faninVec.size() << "\n"
                    << "  Expected: " << (lastN - firstN + 1) << std::endl << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      string firstType = debracket(typeVec[0], '\'', '\'');
      if (NeuronType::Member.find(firstType) == NeuronType::Member.end()) {
         CALL_ERROR << "Unknown neuron type '" << firstType << "'." << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      for (unsigned int i = firstN; i <= lastN; ++i) {
         int shuffRow = SHUFFLEIFMULTIPROC(i);
         FanInCon[shuffRow] = from_string<unsigned int>(faninVec[i-firstN]);
         string thisType = debracket(typeVec[i-firstN], '\'', '\'');
         if (thisType != firstType) {
            CALL_ERROR << "Only one neuron type is allowed in a population." << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
      Population::addMember(Population(firstN, lastN, NeuronType::Member[firstType]));
   }
   // Currently, this bracket is here to manage memory - should be its own function
   {
      const float deltaT = SystemVar::GetFloatVar("deltaT");
      map<string, string> synapseVars = ParseStruct(synapseProps);
      vector<string> cVec = tokenize(debracket(synapseVars["c"], '{', '}'), ',', "[]");
      vector<string> wVec = tokenize(debracket(synapseVars["w"], '{', '}'), ',', "[]");
      vector<string> aVec = tokenize(debracket(synapseVars["a"], '{', '}'), ',', "[]");
      for (unsigned int i = firstN; i <= lastN; ++i) {
         int shuffRow = SHUFFLEIFMULTIPROC(i);
         // Only works if vector is space delimited
         // FIXME: Add check for commas
         vector<string> cSubVec = tokenize(debracket(cVec[i-firstN], '[', ']'), ' ', "");
         vector<string> wSubVec = tokenize(debracket(wVec[i-firstN], '[', ']'), ' ', "");
         vector<string> aSubVec = tokenize(debracket(aVec[i-firstN], '[', ']'), ' ', "");
         if (cSubVec.size() != FanInCon[i]) {
            CALL_ERROR << "Connectivity vector wrong size for neuron " << i << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         if (wSubVec.size() != FanInCon[i]) {
            CALL_ERROR << "Weight vector wrong size for neuron " << i << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         if (aSubVec.size() != FanInCon[i]) {
            CALL_ERROR << "Axonal delay vector wrong size for neuron " << i << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         unsigned int numConnForNeur = 0;
         for (unsigned int j = 0; j < FanInCon[i]; ++j) {
            if (isLocalNeuron(SHUFFLEIFMULTIPROC(from_string<unsigned int>(cSubVec[j]))))
               ++numConnForNeur;
         }
         inMatrix[shuffRow] = new DendriticSynapse[numConnForNeur];
         DendriticSynapse* dendriticTree = inMatrix[shuffRow];
         unsigned int curConnHere = 0;
         for (unsigned int j = 0; j < FanInCon[i]; ++j) {
            unsigned int afferentN = SHUFFLEIFMULTIPROC(from_string<unsigned int>(cSubVec[j])-1);
            if (isLocalNeuron(afferentN)) {
               dendriticTree[curConnHere].setSrcNeuron(afferentN);
               float affW = from_string<float>(wSubVec[j]);
               dendriticTree[curConnHere].setWeight(affW);
               unsigned int axDelay = static_cast<unsigned int>(floor((from_string<float>(aSubVec[j])/deltaT) + 0.5));
               if (axDelay < 1) {
                  if (from_string<float>(aSubVec[j]) > verySmallFloat) {
                     Output::Out() << "WARNING: An axonal delay of " << from_string<float>(aSubVec[j]) << " was encountered, which "
                                   << "is less than half the simulation time-step of " << deltaT << ". You might want to "
                                   << "use a smaller simulation time-step." << std::endl;
                  } else {
                     Output::Out() << "WARNING: An axonal delay of " << from_string<float>(aSubVec[j]) << " was encountered." << std::endl;
                  }
                  axDelay = 1;
               }
               effDelays[shuffRow].push_back(axDelay);
               if (axDelay >= FanOutCon[afferentN].size()) {
                  FanOutCon[afferentN].resize(axDelay, 0);
               }
               ++FanOutCon[afferentN][axDelay-1];
               if (axDelay < minAxonalDelay) minAxonalDelay = axDelay;
               updateMax(maxAxonalDelay, axDelay);
               ++curConnHere;
            }
         }
      }
   }
}

void DeAllocateMemory()
{
   // Allocated Generally
   delArray(VarKConductanceArray);
   delArray(Inhibition);

   FanInCon.clear();
   FanOutCon.clear();

   delMatrix(inMatrix, ni);
   delTensor(outMatrix, maxAxonalDelay, ni);

   return;
}

string debracket(const string& toDebracket, char beginToken, char endToken) {
   string toReturn = toDebracket;
   string::size_type idx = toReturn.find(beginToken);
   if (idx != string::npos) {
     toReturn.erase(0, idx+1);
   }
   idx = toReturn.find(endToken);
   if (idx != string::npos) {
     toReturn.erase(idx);
   }
   return toReturn;
}

void AllocateMemory()
{
   zi = Pattern(ni, false);
   xAllZeros.initialize(ni);
   VarKConductanceArray = new float[ni];
   IzhV = new float[ni];
   IzhU = new float[ni];
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      pIt->initInterneurons();
   }
   Inhibition = new float[ni];
#if defined(MULTIPROC)
   Shuffle = UIVector(ni);
   UnShuffle = UIVector(ni);
#endif
   Fired.push_back(UIVector(0));

   // Allocate Memory for the connections
   FanInCon.assign(ni, 0);
   FanOutCon.assign(ni, UIVector(maxAxonalDelay, 0));

   // Allocate memory for fan-in connections
   inMatrix = new DendriticSynapse *[ni];
   // Cannot allocate memory for matrix columns until we know for sure
   // how many connections there are per neuron.
   outMatrix = new AxonalSynapse **[ni];

   return;
}

vector<float> assignIzhParams(const string &IzhNeuronType)
{
   // defaults
   float a = 0.02f;
   float b = 0.2f;
   float c = -65.0f;
   float d = 6.0f;
   float e = 5.0f;
   float f = 140.0f;
   float vMax = 30.0f;
   if ((IzhNeuronType == "A") || (IzhNeuronType == "I")) {
      // use defaults
   } else if (IzhNeuronType == "B") {
      b = 0.25f;
   } else if (IzhNeuronType == "C") {
      c = -50.0f;
      d = 2.0f;
   } else if (IzhNeuronType == "D") {
      b = 0.25f;
      c = -55.0f;
      d = 0.05f;
   } else if (IzhNeuronType == "E") {
      c = -55.0f;
      d = 4.0f;
   } else if (IzhNeuronType == "F") {
      a = 0.01f;
      d = 8.0f;
   } else if ((IzhNeuronType == "G") || (IzhNeuronType == "L")) {
      b = -0.1f;
      c = -55.0f;
      e = 4.1f;
      f = 108.0f;
   } else if (IzhNeuronType == "H") {
      a = 0.2f;
      b = 0.26f;
      d = 0.0f;
   } else if (IzhNeuronType == "J") {
      a = 0.05f;
      b = 0.26f;
      c = -60.0f;
      d = 0.0f;
   } else if (IzhNeuronType == "K") {
      a = 0.1f;
      b = 0.26f;
      c = -60.0f;
      d = -1.0f;
   } else if (IzhNeuronType == "M") {
      a = 0.03f;
      b = 0.25f;
      c = -60.0f;
      d = 4.0f;
   } else if (IzhNeuronType == "N") {
      a = 0.03f;
      b = 0.25f;
      c = -52.0f;
      d = 0.0f;
   } else if (IzhNeuronType == "O") {
      a = 0.03f;
      b = 0.25f;
      c = -60.0f;
      d = 4.0f;
   } else if (IzhNeuronType == "P") {
      a = 0.1f;
      b = 0.26f;
      c = -60.0f;
      d = 0.0f;
   } else if (IzhNeuronType == "Q") {
      a = 1.0f;
      b = 0.2f;
      c = -60.0f;
      d = -21.0f;
   } else if (IzhNeuronType == "R") {
      b = 1.0f;
      c = -55.0f;
      d = 4.0f;
   } else if (IzhNeuronType == "S") {
      a = -0.02f;
      b = -1.0f;
      c = -60.0f;
      d = 8.0f;
   } else if (IzhNeuronType == "T") {
      a = -0.026f;
      b = -1.0f;
      c = -45.0f;
      d = -2.0f;
   } else {
      Output::Err() << "Unrecognized Izhikevich neuron type: '"
                    << IzhNeuronType << "'" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   float vStart, uStart;
   if (IzhNeuronType == "R") {
      vStart = -65.0f;
      uStart = -16.0f;
   } else {
      // The following calculation is the smaller of the quadratic
      // formula solutions for 0.04v^2 + e*v + 140 - b*v = 0,
      // where b*v is the steady state value for u
      vStart = static_cast<float>(12.5 * (b - e - sqrt((e-b)*(e-b) - 0.16 * f)));
      uStart = b * vStart;
   }
   vector<float> toReturn = vector<float>(9);
   toReturn[0] = a; toReturn[1] = b; toReturn[2] = c; toReturn[3] = d;
   toReturn[4] = e; toReturn[5] = f;
   toReturn[6] = vMax;
   toReturn[7] = vStart; toReturn[8] = uStart;
   return toReturn;
}

void CalcDendriticExcitation() {
   const float dMult = SystemVar::GetFloatVar("DenomMult");
   dendExc = DataList(ni, 0.0L);
   for (PopulationCIt PCIt = Population::Member.begin();
        PCIt != Population::Member.end(); ++PCIt) {
      const double FeedBackExcToInternrn = PCIt->getFeedbackInhibition();
      const double FeedFwdExcToInternrn = PCIt->getFeedforwardInhibition();
      const float BaseInhib = (KFBDend * FeedBackExcToInternrn) +
                              (KFFDend * FeedFwdExcToInternrn) + K0Dend;
      for (unsigned int i = PCIt->getFirstNeuron(); i <= PCIt->getLastNeuron(); ++i) {
         const float numerator = sumwz[i];
         if (abs(numerator) > verySmallFloat) {
            dendExc[i] = numerator;
            if (useDendInh)
               dendExc[i] /= (dMult * numerator + BaseInhib);
         }
      }
      const NeuronType* curNType = PCIt->getNeuronType();
      const int DumpDendrite = curNType->getParameter("DumpDendrite", SystemVar::GetIntVar("DumpDendrite"));
      if (DumpDendrite > 0) {
         //Reset dendrite of fired neurons
         const unsigned int filterSize = curNType->getFilterSize();
         for (unsigned int i = 0; i < Fired[justNow].size(); ++i) {
            unsigned int firedNrn = Fired[justNow][i];
            if ((PCIt->getFirstNeuron()<=firedNrn) && (firedNrn<=PCIt->getLastNeuron())) {
              dendriteQueue[firedNrn] = new float[filterSize]();
              dendriteQueue_inhdiv[firedNrn] = new float[filterSize]();
              dendriteQueue_inhsub[firedNrn] = new float[filterSize]();
            }
         }
      }
      const float filterDecay = curNType->getParameter("filterDecay", SystemVar::GetFloatVar("filterDecay"));
      if (filterDecay < 1.0f) {
         //Decay the filter of fired neurons
         const unsigned int filterSize = curNType->getFilterSize();
         for (unsigned int i = 0; i < Fired[justNow].size(); ++i) { //find all neurons that just fired
            unsigned int firedNrn = Fired[justNow][i];
            if ((PCIt->getFirstNeuron()<=firedNrn) && (firedNrn<=PCIt->getLastNeuron())) { 
              for (unsigned int timeOffset = 0; timeOffset < filterSize; ++timeOffset) { //loop through entire filter/boxcar
                dendriteQueue[firedNrn][timeOffset] = filterDecay * dendriteQueue[firedNrn][timeOffset]; //scale the excitation on the dendrite
              }
            }
         }
      }
   }
   enqueueDendriticResponse(dendExc, sumwz_inhdiv, sumwz_inhsub);
}

void CalcDendriticToSomaInput(const xInput& curPattern, const bool isComp) {
   //FLEX: Other decay options exist
   for (PopulationCIt PCIt = Population::Member.begin();
        PCIt != Population::Member.end(); ++PCIt) {
      const NeuronType* curNType = PCIt->getNeuronType();
      const float yDecay = curNType->getParameter("yDecay", SystemVar::GetFloatVar("yDecay"));
      const float DumpConst = curNType->getParameter("DumpConst", SystemVar::GetFloatVar("DumpConst"));
      if (fabs(yDecay) > verySmallFloat) {
         // Has to happen before decay to match equations correctly
         if (fabs(DumpConst) > verySmallFloat) {
            //Reset Excitation of fired neurons
            for (unsigned int i = 0; i < Fired[justNow].size(); ++i) {
               unsigned int firedNrn = Fired[justNow][i];
               if ((PCIt->getFirstNeuron()<=firedNrn) && (firedNrn<=PCIt->getLastNeuron())) {
                 somaExc[firedNrn] -= DumpConst;
               }
            }
         }
         // Decay the excitation values
         for (unsigned int i = PCIt->getFirstNeuron(); i <= PCIt->getLastNeuron(); ++i) {
            somaExc[i] *= yDecay;
         }
      } else {
         for (unsigned int i = PCIt->getFirstNeuron(); i <= PCIt->getLastNeuron(); ++i) {
            somaExc[i] = 0.0f;
         }
         if (fabs(DumpConst) > verySmallFloat) {
            //Reset Excitation of fired neurons
            for (unsigned int i = 0; i < Fired[justNow].size(); ++i) {
               unsigned int firedNrn = Fired[justNow][i];
               if ((PCIt->getFirstNeuron()<=firedNrn) && (firedNrn<=PCIt->getLastNeuron()))
                 somaExc[firedNrn] = -DumpConst;
            }
         }
      }
   }

   // Competitive networks don't use inhibition
   if (isComp) {
      for (PopulationCIt PCIt = Population::Member.begin();
           PCIt != Population::Member.end(); ++PCIt) {
         const Filter popFilter = PCIt->getNeuronType()->getFilter();
         for (unsigned int i = PCIt->getFirstNeuron(); i <= PCIt->getLastNeuron(); ++i) {
            somaExc[i] += popFilter.apply(dendriteQueue[i]) - popFilter.apply(dendriteQueue_inhsub[i]);
         }
      }
   } else {
      // Now, the root node looks at neural excitations and external input   
      // and then decides whether each neuron fires  

      for (PopulationCIt PCIt = Population::Member.begin();
           PCIt != Population::Member.end(); ++PCIt) {
         const NeuronType* curNType = PCIt->getNeuronType();
         const float DGstrength = curNType->getParameter("DGstrength",
                                               SystemVar::GetFloatVar("DGstrength"));
         const float VarKConductanceVal = curNType->getParameter("VarKConductance", 
                                               SystemVar::GetFloatVar("VarKConductance"));
         const double FeedBackExcToInternrn = PCIt->getFeedbackInhibition();
         const double FeedFwdExcToInternrn = PCIt->getFeedforwardInhibition();
         const double K0 = curNType->getParameter("K0", K0Soma);
         const double KFB = curNType->getParameter("KFB", KFBSoma);
         const double KFF = curNType->getParameter("KFF", KFFSoma);
			const double BaseInhib = K0 + (KFF * FeedFwdExcToInternrn) + (KFB * FeedBackExcToInternrn);
			//Output::Out() << "K0: " << K0 << ", KFB: " << KFB << ", newKFB " << newKFB << ", KFF: " << KFF << std::endl;
			//Output::Out() << "FF exc to I: " << FeedFwdExcToInternrn << ", FB exc to I: " << FeedBackExcToInternrn << std::endl;
			//Output::Out() << "BaseInhib: " << BaseInhib << ", Fired[jN].size(): " << lastAct << ", num2Fire: " << numToFire << std::endl;
         // This value is currently not what it claims to be (FIXME)
         Threshold = BaseInhib;
         const Filter popFilter = PCIt->getNeuronType()->getFilter();
         for (unsigned int i = PCIt->getFirstNeuron(); i <= PCIt->getLastNeuron(); ++i) {
            float numerator = popFilter.apply(dendriteQueue[i]) - 
                     popFilter.apply(dendriteQueue_inhsub[i]) + DGstrength * curPattern[i];
            somaExc[i] += numerator;
            if (useSomaInh) {
               // abh2n: This has just changed (Aug 25, 06) from defaulting to Dendritic inhibition
               Inhibition[i] = popFilter.apply(dendriteQueue_inhdiv[i]) + BaseInhib + 
                                 VarKConductanceVal * VarKConductanceArray[i];
					//Output::Out() << "Inhibition at index " << i << ": " << Inhibition[i] << std::endl; 
					//Output::Out() << "Numerator: " << numerator << std::endl;
					//Output::Out() << "BaseInhib: " << BaseInhib << std::endl;
               if (numerator + Inhibition[i] > verySmallFloat) {
                  somaExc[i] /= (numerator + Inhibition[i]);
               } else {
                  // Don't want dividing by a negative number!
                  somaExc[i] = 0;
               }
					//Output::Out() << "Soma excitation at index " << i << ": " << somaExc[i] << std::endl;
            }
         }
      }
   }   
}

/**
 * The following three methods:
 *    - initializeThreshold()
 *    - setThreshold()
 *    - updateTimeSinceFired()
 * provide a variable threshold in
 * NeuroJet. They force a neuron not to fire if it fired during
 * the previous timestep.
 */

void initializeThreshold() {
	// FIXME: Set IzhV[i] to theta instead of 0.5
	for(int i = 0; i < ni; ++i) {
		IzhU[i] = 0.5f;
		IzhV[i] = 100.0f;
	}
}

void setThreshold() {
	for(int i = 0; i < ni; ++i) {
		if(IzhV[i] > 1) {
			IzhU[i] = 0.5f;
		} else {
			IzhU[i] = 1.0f;
		}
		//Output::Out() << "Current Threshold:" << IzhU[i] << std::endl;
	}
}

void updateTimeSinceFired() {
	for(int i = 0; i < ni; ++i) {
		IzhV[i] = IzhV[i] + 1.0f;
		//Output::Out() << "Time since fired: " << IzhV[i] << std::endl;
	}
}

void CalcSomaResponse(const xInput &curPattern, DataMatrix &IzhVValues,
                      DataMatrix &IzhUValues)
{
   zi = Pattern(ni, false);
   
   Fired.pop_back();
   Fired.push_front(UIVector(0));

#if defined(MULTIPROC)
   FiredHere.pop_back();
   FiredHere.push_front(UIVector(0));
#endif

   double integratingTimeStep = SystemVar::GetFloatVar("deltaT");
   const double maxIntegrateTimeStep = SystemVar::GetFloatVar("IzhTimeThresh");
   unsigned int numIntegrates = 1;
   if (integratingTimeStep > maxIntegrateTimeStep) { // ms
      numIntegrates = iceil(integratingTimeStep/maxIntegrateTimeStep);
      integratingTimeStep /= numIntegrates;
   }
   for (unsigned int t = 0; t < numIntegrates; ++t) {
      IzhVValues.push_back(DataList(0));
      IzhUValues.push_back(DataList(0));
   }
   const bool trackIzhBuffs = (SystemVar::GetIntVar("IzhTrackData") != 0);
   for (PopulationCIt PCIt = Population::Member.begin();
         PCIt != Population::Member.end(); ++PCIt) {
      const NeuronType* nType = PCIt->getNeuronType();
      const float CAconstVal = nType->getParameter("CAconst", SystemVar::GetFloatVar("CAconst"));
      const float OneMinCAcV = 1 - CAconstVal;
      DataMatrix tmpIzhV;
      DataMatrix tmpIzhU;
      if (program::Main().GetIzhExplicitCount() || nType->useIzh()) {
         const float IzhA = nType->getParameter("IzhA", SystemVar::GetFloatVar("IzhA"));
         const float IzhB = nType->getParameter("IzhB", SystemVar::GetFloatVar("IzhB"));
         const float IzhC = nType->getParameter("IzhC", SystemVar::GetFloatVar("IzhC"));
         const float IzhD = nType->getParameter("IzhD", SystemVar::GetFloatVar("IzhD"));
         const float IzhE = nType->getParameter("IzhE", SystemVar::GetFloatVar("IzhE"));
         const float IzhF = nType->getParameter("IzhF", SystemVar::GetFloatVar("IzhF"));
         const float IzhIMult = nType->getParameter("IzhIMult", SystemVar::GetFloatVar("IzhIMult"));
         const float IzhVMax = nType->getParameter("IzhVMax", SystemVar::GetFloatVar("IzhVMax"));
         const string IzhNeuronType = nType->getParameter("IzhType", SystemVar::GetStrVar("IzhType"));
         for (unsigned int t = 0; t < numIntegrates; ++t) {
            const unsigned int popSize = PCIt->getLastNeuron() - PCIt->getFirstNeuron() + 1;
            const unsigned int offset = PCIt->getFirstNeuron();
            DataList curIzhVValues(popSize);
            DataList curIzhUValues(popSize);
            for (unsigned int nrn = PCIt->getFirstNeuron(); nrn <= PCIt->getLastNeuron(); ++nrn) {
               const bool forceExt = (t==0) && PCIt->forceExt();
               //FLEX: Allow different decay models for VarKConductance(Izhikevich?)
               float oldV = IzhV[nrn];
               float oldU = IzhU[nrn];
               if (abs(oldV - IzhVMax) < verySmallFloat) {
                  oldV = IzhC;
                  oldU += IzhD;
               }
               IzhV[nrn] = oldV + integratingTimeStep * (0.04 * oldV * oldV
                      + IzhE * oldV + IzhF - oldU + IzhIMult * somaExc[nrn]);
               if (IzhNeuronType == "R") { // accomodation (figure 1 on many Izh papers)
                  IzhU[nrn] = oldU + integratingTimeStep * IzhA * IzhB * (oldV + 65.0f);
               } else {
                  IzhU[nrn] = oldU + integratingTimeStep * IzhA *
                                                   (IzhB * oldV - oldU);
               }
               if (t == 0) {
                  VarKConductanceArray[nrn] *= OneMinCAcV;
               }
               if ((IzhV[nrn] > IzhVMax) || (forceExt && curPattern[nrn] && t == 0)) {
                  if (!zi[nrn]) {
                     VarKConductanceArray[nrn] += CAconstVal;
                     FireSingleNeuron(nrn);
                  }
                  IzhV[nrn] = IzhVMax;
               }
               if (trackIzhBuffs) {
                  // Only works if shuffling is within population
                  curIzhVValues[UNSHUFFLEIFMULTIPROC(nrn)-offset] = IzhV[nrn];
                  curIzhUValues[UNSHUFFLEIFMULTIPROC(nrn)-offset] = IzhU[nrn];
               }
            }
            if (trackIzhBuffs) {
               tmpIzhV.push_back(curIzhVValues);
               tmpIzhU.push_back(curIzhUValues);
            }
         }
         if (trackIzhBuffs) {
            // const_reverse_iterators can't use != yada.rend() (in g++ v3.3.5)
            DataMatrix::reverse_iterator tmpVIt = tmpIzhV.rbegin();
            DataMatrix::reverse_iterator VIt = IzhVValues.rbegin();
            DataMatrix::reverse_iterator tmpUIt = tmpIzhU.rbegin();
            DataMatrix::reverse_iterator UIt = IzhUValues.rbegin();
            // Add the new population Izh values to the running total
            for (; tmpVIt != tmpIzhV.rend(); ++tmpVIt, ++VIt, ++tmpUIt, ++UIt) {
               DataListCIt tmpDLVIt = tmpVIt->begin();
               DataListCIt tmpDLUIt = tmpUIt->begin();
               for (; tmpDLVIt != tmpVIt->end(); ++tmpDLVIt, ++tmpDLUIt) {
                  VIt->push_back(*tmpDLVIt);
                  UIt->push_back(*tmpDLUIt);
               }
               //VIt->insert(VIt->end(), tmpVIt->begin(), tmpVIt->end());
               //UIt->insert(UIt->end(), tmpUIt->begin(), tmpUIt->end());
            }
         }
      } else {
         const float theta = SystemVar::GetFloatVar("theta");
         const bool forceExt = PCIt->forceExt();
			//Output::Out() << "tick" << std::endl;
            for (unsigned int nrn = PCIt->getFirstNeuron(); nrn <= PCIt->getLastNeuron(); ++nrn) {
            //FLEX: Allow different decay models for VarKConductance(Izhikevich?)
            VarKConductanceArray[nrn] *= OneMinCAcV;
   
            // if recurrent OR external neuron fired 
            //FLEX: Differentiating DG vs EC could cause this to change
				//Output::Out() << "Neuron: " << nrn << " somaExc: " << somaExc[nrn] << std::endl;
				float thresh;
				if(SystemVar::GetIntVar("VariableThreshold") != 0) {
					thresh = IzhU[nrn];
				} else {
					thresh = 0.5f;
				}

            if ((somaExc[nrn] >= thresh) || (forceExt && curPattern[nrn])) {
               VarKConductanceArray[nrn] += CAconstVal;
               FireSingleNeuron(nrn);
					if(SystemVar::GetIntVar("VariableThreshold") != 0)
						IzhV[nrn] = 0.0f;
               // If fired by external neuron and excitation was sub-threshold,
               // set to threshold so that exitation won't go negative.
               if (somaExc[nrn] < thresh)
                  somaExc[nrn] = thresh;
            }
         }
      }
   }
}

inline void CalcSynapticActivation(const UIVectorDeque &FiredArray, const Pattern &inPattern) {
   CalcSynapticActivation(FiredArray, xInput(ni, inPattern));
}

void CalcSynapticActivation(const UIVectorDeque &FiredArray, const xInput &curPattern)
{
   SYNFAILS_DEBUG_MODE_INIT
   // Initialize firing time matrix for Z0 
   if (timeStep == 0) {
      for (unsigned int i = 0; i < ni; i++) {
         DendriticSynapse * dendriticTree = inMatrix[i];
         for (unsigned int c = 0; c < FanInCon[i]; ++c)
            dendriticTree[c].resetLastActivate();
      }
   }

   // reset Sumwz's to all zeroes
   sumwz = DataList(ni, 0.0f);
   sumwz_inhdiv = DataList(ni, 0.0f);
   sumwz_inhsub = DataList(ni, 0.0f);

   // RNGs were initiated during CreateNetwork
   InitCurBucketStats(); // Typically does nothing
   // For each possible time-step back
   for (unsigned int relTime = minAxonalDelay-1; relTime < maxAxonalDelay; ++relTime) {
      // for each neuron on this compute node that fired
      for (unsigned int i = 0; i < FiredArray[relTime].size(); i++) {
         const int iFire = FiredArray[relTime][i];
         AxonalSynapse * axonalSegment = outMatrix[iFire][relTime];
         // for every outgoing connection of this neuron
#if defined(CHECK_BOUNDS)
         for (unsigned int c = 0; c < FanOutCon.at(iFire).at(relTime); c++) {
#else
         for (unsigned int c = 0; c < FanOutCon[iFire][relTime]; c++) {
#endif
            // Updates sumwz and synpatic information
            axonalSegment[c].activate(sumwz, sumwz_inhdiv, sumwz_inhsub, timeStep);
            SYNFAILS_DEBUG_MODE_INC
         }
      }
   }
   UpdateMaxBucketStats(); // Typically does nothing
   SYNFAILS_DEBUG_MODE_OUTPUT
   UpdateBucketStats();  // Typically does nothing

   for (PopulationCIt PCIt = Population::Member.begin();
         PCIt != Population::Member.end(); ++PCIt) {
      const float ExtExc = PCIt->getNeuronType()->getParameter("ExtExc", SystemVar::GetFloatVar("ExtExc"));
      if (ExtExc > 0) {
         for (unsigned int nrn = PCIt->getFirstNeuron(); nrn <= PCIt->getLastNeuron(); ++nrn) {
            if (curPattern[nrn])
               sumwz[nrn] += ExtExc;
         }
      }
   }
}

bool chkDataExists(const StrArg &DataName, const DataListType newDataType,
                   const string FunctionName, const CommandLine &ComL)
{
   string dataType = SystemVar::GetVarTypeName(DataName.getValue());
   if (dataType == "unknown") return false;
   if (((dataType == "sequence") && (newDataType == DLT_sequence))
       || ((dataType == "matrix") && (newDataType == DLT_matrix))
       || ((dataType == "analysis") && (newDataType == DLT_analysis))) {
      return true;
   }
   if (dataType[0] == '@' || dataType[0] == 'a' || dataType[0] == 'i')
      dataType = "an " + dataType;
   else
      dataType = "a " + dataType;
   CALL_ERROR << "Error in " << FunctionName << " : " << DataName.getValue()
              << " already exists as " << dataType << "." << ERR_WHERE;
   ComL.DisplayHelp(Output::Err());
   exit(EXIT_FAILURE);
}

void CheckIzhikevich()
{
   int IzhExplicitCount = program::Main().GetIzhExplicitCount();
   int IzhMaxExplicitCount = program::Main().GetIzhExplicitMaxCount();
	//Output::Out() << SystemVar::GetIntVar("VariableThreshold") << std::endl;

	// If we are using a variable threshold, we want to use IzhvStart and
	// IzhuStart
   if ((0 < IzhExplicitCount) && (IzhExplicitCount < IzhMaxExplicitCount)) {
      CALL_ERROR << "Error: If any of IzhA, IzhB, IzhC, IzhD, IzhvStart, or\n"
                    "IzhuStart are set, then they must all be set."
                 << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if ((IzhExplicitCount > 0) && (fabs(SystemVar::GetFloatVar("deltaT") + 1.0f) < verySmallFloat)) {
      CALL_ERROR << "Error: If Izhikevich model is being used, then\n"
                    "deltaT must also be set." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
}

void CompPresent(const xInput &curPattern, const bool isTesting)
{
   int  numToFire = iround(SystemVar::GetFloatVar("Activity") * ni);

#if !defined(PARENT_CHLD)
#  if defined(TIMING_MODE)
   clock_t t1 = clock();
#  endif
#endif

#if !defined(PARENT_CHLD)
   // Sum presynaptic(fan-in) activity for each neuron
   CalcSynapticActivation(LOCALFIRED, curPattern);
#endif
#if defined(MULTIPROC)
#  if defined(PARENT_CHILD)
   // Receive and merge the excitation vectors from the compute nodes
   sumwz = ParallelInfo::rcvSumwz();
   sumwz_inhdiv = ParallelInfo::rcvSumwz();
   sumwz_inhsub = ParallelInfo::rcvSumwz();
#  else
   // Exchange and sum sumwz among nodes
   sumwz = ParallelInfo::exchangeSumwz(sumwz);
   sumwz_inhdiv = ParallelInfo::exchangeSumwz(sumwz_inhdiv);
   sumwz_inhsub = ParallelInfo::exchangeSumwz(sumwz_inhsub);
#  endif
#endif

   dendExc = sumwz;
   enqueueDendriticResponse(dendExc, sumwz_inhdiv, sumwz_inhsub);

   // increment the time step
   timeStep++;

   CalcDendriticToSomaInput(curPattern, true);

   // Get ready for firing
   zi = Pattern(ni, false);
   Fired.pop_back();
   Fired.push_front(UIVector(0));
#if defined(MULTIPROC)
   FiredHere.pop_back();
   FiredHere.push_front(UIVector(0));
#endif

   // Now set up arrays to select
   vector<IxSumwz> excSort;
   createSelectArray(excSort, curPattern, 0, ni-1);
   int TotalNumFired = Fired[justNow].size();

   unsigned int numLeft2Fire = numToFire - TotalNumFired;
   float cutOff = selectCutOff(numLeft2Fire, excSort.size(), excSort);
   Threshold = cutOff;

   // numLeft2Fire are the number of neurons that fire recurrently,
   // i.e., numToFire - number of externals
   FireTiedNeurons(numLeft2Fire, cutOff, excSort);

   // Fire the non-tiebreaker, non-external neurons 
   FireNonTiedNeurons(numLeft2Fire, excSort);

#if defined(PARENT_CHILD)
   // Send the firing data back to the compute nodes
   ParallelInfo::sendZi(zi);
#else
#  if defined(DEBUG)
   float TempAlpha = 0.0;
   printf("\nThreshold = %.4g\t\n", Threshold);
#  endif
   // Now every node has zi and can continue normal operation
   if (!isTesting && (SystemVar::GetFloatVar("mu") > verySmallFloat)) {
      // The PyrToInternrnWt values are irrelevant to competitive firing
      //I.e., don't call updateInternrnWeights
      UpdateWeights();
   }
#  if defined(TIMING_MODE)
   clock_t t2 = clock();
   printf(" (%.4gms) ", 1000.0 * static_cast<float>(t2 - t1) / (CLOCKS_PER_SEC));
#  endif
#endif

   return;
}

void createSelectArray(vector<IxSumwz> &excSort, const xInput &curPattern,
                       const int startN, const int endN)
{
   //FIXME: Need to figure out how to address populations and competitive firing
   const bool forceExt = ((SystemVar::GetFloatVar("ExtExc") < verySmallFloat) &&
                          (SystemVar::GetFloatVar("DGstrength") < verySmallFloat));
   for (int i = startN; i <= endN; i++) {
      if (curPattern[i] && forceExt) {
         FireSingleNeuron(i);
      } else {
         excSort.push_back(IxSumwz(i,somaExc[i]));
      }
   }
}

void enqueueDendriticResponse(const DataList& dendriticResponse, const DataList& dendResp_inhdiv,
                              const DataList& dendResp_inhsub) {
   // There are two queues - a "virtual" synaptic queue, and the dendritic
   // queue. The first represents the time course of synaptic activation, and
   // the second represents the RC filter of the dendrite.
   const unsigned int lastNrn = max(dendriticResponse.size(), dendriteQueue.size());
   for (PopulationCIt it = Population::Member.begin();
        it != Population::Member.end(); ++it) {
      const int mvSize = it->getNeuronType()->getFilterSize()-1;
      for (unsigned int i=it->getFirstNeuron(); i<max(it->getLastNeuron()+1,lastNrn); ++i) {
         memmove((dendriteQueue[i]+1), dendriteQueue[i], mvSize*sizeof(float));
         dendriteQueue[i][0] = dendriticResponse[i];
         memmove((dendriteQueue_inhdiv[i]+1), dendriteQueue_inhdiv[i], mvSize*sizeof(float));
         dendriteQueue_inhdiv[i][0] = dendResp_inhdiv[i];
         memmove((dendriteQueue_inhsub[i]+1), dendriteQueue_inhsub[i], mvSize*sizeof(float));
         dendriteQueue_inhsub[i][0] = dendResp_inhsub[i];
      }
      if (it->getLastNeuron()+1 >= lastNrn) break;
   }
}

const NeuronType* findNeuronType(const unsigned int nrn) {
   PopulationCIt PCIt = Population::Member.begin();
   for (; PCIt != Population::Member.end(); ++PCIt)
      if (PCIt->getLastNeuron() >= nrn) break;
   if (PCIt == Population::Member.end()) {
      CALL_ERROR << "Neuron " << nrn << " was not found in any population." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   return PCIt->getNeuronType();
}

void FillFanOutMatrices()
{
   // Only the neurons that this node is responsible for will have
   // any entries in the fan-out tables. I.e., the fan-out matrices
   // are essentially n x N whereas the fan-in matrices are N x
   // n. (In actuality the fan-out matrices are N x {0,N} with n rows
   // being N wide, and the N-n rows being 0 wide.) [In the previous
   // discussion, N is the total number of neurons and n is the
   // number of neurons for the node.]
   for (unsigned int row = StartNeuron; row <= EndNeuron; row++) {
      outMatrix[row] = new AxonalSynapse *[maxAxonalDelay];
      for (unsigned int refTime = minAxonalDelay-1; refTime < FanOutCon[row].size(); ++refTime) {
#if defined(CHECK_BOUNDS)
         outMatrix[row][refTime] = new AxonalSynapse[FanOutCon.at(row).at(refTime)];
#else
         outMatrix[row][refTime] = new AxonalSynapse[FanOutCon[row][refTime]];
#endif
      }
   }
}

void FireNonTiedNeurons(const unsigned int numLeft2Fire, const vector<IxSumwz> &excSort)
{
   // numLeft2Fire is a bit of a misnomer as it includes the tie-breakers that
   // have already been selected to fire, but that name would be too long
   for (unsigned int i = 0; i < numLeft2Fire; i++) {
      // Tie-breaker routine (FireTiedNeurons) sets ix to zero if they lose or win a tie 
      if (excSort[i].y > 0) {
         FireSingleNeuron(excSort[i].ix);
      }
   }
}

void FireSingleNeuron(const int nrn)
{
#if defined(CHECK_BOUNDS)
   zi.at(nrn) = true;
   Fired.at(justNow).push_back(nrn);
#else
   zi[nrn] = true;
   Fired[justNow].push_back(nrn);
#endif
#if defined(MULTIPROC)
   if (isLocalNeuron(nrn))
      FiredHere[justNow].push_back(nrn);
#endif
}

void FireTiedNeurons(const unsigned int numLeft2Fire, const double cutOff,
                     vector<IxSumwz> &excSort)
{
 
   // Assumption: numLeft2Fire < numToChooseFrom
   if (numLeft2Fire > 0 && cutOff > 0.0f) {
      // Find all tied units at the cutoff value
      UIVector TiedUnits;
      int NumTiedToFire = 0;
      for (unsigned int i = 0; i < excSort.size(); i++) {
         if (fabs(excSort[i].y - cutOff) < verySmallFloat) {
            if (i < numLeft2Fire) NumTiedToFire++;
            // We're storing an index into excSort
            TiedUnits.push_back(i);
         }
      }

      // Only need to deal with it separately if there are tied neurons
      // to the right of the partition
      int  NumTied = TiedUnits.size();
      if (NumTiedToFire < NumTied) {
         // Randomly cut out nonfiring neurons
         while (NumTiedToFire < NumTied) {
            int pickSlot = program::Main().pickTie(NumTied);
            // This is why we stored an index into excSort
            excSort.at(TiedUnits.at(pickSlot)).y = 0;
            TiedUnits.erase(TiedUnits.begin() + pickSlot);
            NumTied--;
         }

         SystemVar::IncIntVar("NumTiesPicked", NumTiedToFire);
         // Fire the tie-breaker neurons
         for (UIVectorCIt it = TiedUnits.begin(); it != TiedUnits.end(); it++) {
            FireSingleNeuron(excSort.at(*it).ix);
            excSort.at(*it).y = 0;
         }

         // Do NOT subtract the fired neurons from those left to fire
         // since we need to scan up to this number to determine which to fire
      }
   }
}

list<xInput> GenerateInputSequence(UIPtnSequence &Seq,
   const float inputNoise, const float exactNoise, int& SumExtFired,
   int& PatternCount)
{
   list<xInput> seqToReturn;
   if (Seq.size() > 0) {
      UIVector pat;
      int defPeriod = SystemVar::GetIntVar("Period");
      int Period = defPeriod;
      float defAmplitude = SystemVar::GetFloatVar("Amplitude");
      float Amplitude = defAmplitude;
      float defMidPoint = SystemVar::GetFloatVar("MidPoint");
      float MidPoint = defMidPoint;
      float defPhase = SystemVar::GetFloatVar("Phase");
      float Phase = defPhase;
      bool defUseSin = SystemVar::GetIntVar("UseSin");
      bool UseSin = defUseSin;
      for (UIPtnSequenceCIt it = Seq.begin(); it != Seq.end(); ++it) {
         pat = *it;
         xInput temp_xin(ni);
         if ((pat.size() > 0) && (pat.back() >= ni)) {
            CALL_ERROR << "Input pattern larger than network\n" << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         unsigned int max_ext = 0;
         if (inputNoise < verySmallFloat) {
            SumExtFired += pat.size();
            for (UIVectorCIt pIt = pat.begin(); pIt != pat.end(); ++pIt) {
               updateMax(max_ext, *pIt);
               temp_xin.turnOn(SHUFFLEIFMULTIPROC(*pIt));
            }
            // deactivate exact number of noisy external neurons if
            // the xNoiseF variable was set...
            if (exactNoise > verySmallFloat && max_ext > 0) {
               int  Num2Off = iround(exactNoise * temp_xin.numExternals());
               int  candidate = 0;
               for (int f = 0; f < Num2Off; f++) {
                  candidate = program::Main().getExtRandInt(0, max_ext);
                  while (temp_xin[candidate] == ZERO) {
                     candidate = program::Main().getExtRandInt(0, max_ext);
                  }
                  temp_xin.turnOff(SHUFFLEIFMULTIPROC(candidate));
               }
            }
         } else {
            // DWS - 12-6-2002
            // Here's where the "theta rhythm" gets implemented
            // Major Parameters:
            // ------------------
            // Period: the period (in timesteps) of a theta cycle
            // MidPoint: what % of neurons fire when theta = pi
            // Amplitude=max-min
            // we'll assume for now that start = 70% of range
            // --------------------------------------------------

            // first, where are we within the cycle?

            unsigned int nextFirstNeuron = 0;
            PopulationIt nextPopIt;
            if (pat.size() > 0) {
               for (nextPopIt = Population::Member.begin(); nextPopIt != Population::Member.end(); ++nextPopIt) {
                  if (nextPopIt->getLastNeuron() >= *(pat.begin())) {
                     getThetaSettings(*(nextPopIt->getNeuronType()), Period, Amplitude, MidPoint, Phase, UseSin);
                     ++nextPopIt;
                     if (nextPopIt != Population::Member.end())
                        nextFirstNeuron = nextPopIt->getFirstNeuron();
                     else
                        nextFirstNeuron = ni+1; //Don't want to hit it!
                     break;
                  }
               }
            }

            float sucRate;
            if (Amplitude < verySmallFloat) {
               sucRate = MidPoint;
            } else {
               sucRate = MidPoint + Amplitude * periodicFn(2.0f * PI * PatternCount / static_cast<double>(Period) + Phase, UseSin);
            }

            // *Activity = sucRate * (*ParamActivity);
            for (UIVectorCIt pIt = pat.begin(); pIt != pat.end(); ++pIt) {
               if (*pIt == nextFirstNeuron) {
                  Period = defPeriod; Amplitude = defAmplitude; MidPoint = defMidPoint;
                  Phase = defPhase; UseSin = defUseSin;
                  getThetaSettings(*(nextPopIt->getNeuronType()), Period, Amplitude, MidPoint, Phase, UseSin);
                  ++nextPopIt;
                  if (nextPopIt != Population::Member.end())
                     nextFirstNeuron = nextPopIt->getFirstNeuron();
                  if (Amplitude < verySmallFloat) {
                     sucRate = MidPoint;
                  } else {
                     sucRate = MidPoint + Amplitude * periodicFn(2.0f * PI * 
                                         PatternCount / static_cast<double>(Period) + Phase, UseSin);
                  }
               }
               if (program::Main().getExtBernoulli(sucRate)) {
                  ++SumExtFired;
                  temp_xin.turnOn(SHUFFLEIFMULTIPROC(*pIt));
               }
            }
         }

         seqToReturn.push_back(temp_xin);
         ++PatternCount;
      }
   }
   return seqToReturn;
}

void GetConnectivity(string filename)
// i.e., ReadWeights
{
   /////////////////////////////////////////////////
   // Read in connections and weights from file, 
   //   allocating memory for both
   /////////////////////////////////////////////////
   int  TotalNumberOfZeros = 0;
   float TotalSumOfWeights = 0.0;
   float TotalSumOfZeros = 0.0;

   unsigned int fileni;
   unsigned int tmpThrowAway;
   float moreTrash;
   bool fileHasAxonalDelays = false;   

#if defined(MULTIPROC)
   int P_NumNetworkCon = 0;
#endif
   unsigned int MaxInputs = 0;
   NumNetworkCon = 0;

   ifstream chkFile(filename.c_str());

   // Read first line (containing number of neurons)
   read_without_comments(chkFile, fileni);
   if (fileni != ni) {
      CALL_ERROR << "Number of neurons in " << filename
         << " does not agree with number given in script file!" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   // Read second (non-blank) line - contains number of fan-in connections for each neuron
   IFROOTNODE Output::Out() << "Reading in number of connections" << std::endl;
   for (unsigned int i = 0; i < ni; i++) {
      int shuffRow = SHUFFLEIFMULTIPROC(i);
      if (chkFile.eof()) {
         CALL_ERROR << "Unexpected EOF while reading " << filename
            << " during a search for connection numbers." << ERR_WHERE;
         exit(EXIT_FAILURE);      
      }
      read_without_comments(chkFile, FanInCon[shuffRow]);
      updateMax(MaxInputs, FanInCon[shuffRow]);
#if defined(MULTIPROC)
      if (i < ni)
         P_NumNetworkCon += FanInCon[shuffRow];
#else
      // For MULTIPROC, NumNetworkCon.. are updated later
      NumNetworkCon += FanInCon[shuffRow];
#endif
   }

   // Read next ni (non-blank) lines - contains pre-synaptic neurons for each neuron
   IFROOTNODE Output::Out() << "Scanning pre-synaptic neurons" << std::endl;
   for (unsigned int conRow = 0; conRow < ni; ++conRow) {
      for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol) {
         if (chkFile.eof()) {
            CALL_ERROR << "Unexpected EOF while reading " << filename
               << " during a search for pre-synaptic neurons." << ERR_WHERE;
            exit(EXIT_FAILURE);      
         }
         read_without_comments(chkFile, tmpThrowAway);
      }
   }

   // Read next ni (non-blank) lines - contains pre-synaptic weights for each neuron
   IFROOTNODE Output::Out() << "Scanning pre-synaptic neurons" << std::endl;
   for (unsigned int conRow = 0; conRow < ni; ++conRow) {
      for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol) {
         if (chkFile.eof()) {
            CALL_ERROR << "Unexpected EOF while reading " << filename
               << " during a search for pre-synaptic weights." << ERR_WHERE;
            exit(EXIT_FAILURE);      
         }
         read_without_comments(chkFile, moreTrash);
      }
   }

   unsigned int delayHere = 1;
   read_without_comments(chkFile, delayHere);
   // Look for axonal delays
   if (!chkFile.eof()) {
      IFROOTNODE Output::Out() << "Scanning pre-synaptic axonal delays" << std::endl;
      for (unsigned int conRow = 0; conRow < ni; ++conRow) {
         for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol) {
            if (chkFile.eof()) {
               CALL_ERROR << "Unexpected EOF while reading " << filename
                  << " during a search for axonal delays." << ERR_WHERE;
               exit(EXIT_FAILURE);      
            }
            if (!fileHasAxonalDelays) {
               minAxonalDelay = delayHere;
               maxAxonalDelay = delayHere;
               fileHasAxonalDelays = true;
            } else if (delayHere < minAxonalDelay) {
               minAxonalDelay = delayHere;
            } else if (maxAxonalDelay < delayHere) {
               maxAxonalDelay = delayHere;
            }
            read_without_comments(chkFile, delayHere);
         }
      }
   }

   chkFile.close();

   FanOutCon.assign(ni, UIVector(maxAxonalDelay, 0));

   ifstream inFile(filename.c_str());
   ifstream inFileDelays(filename.c_str()); // So we can compare two lines at once
   if (!inFile) {
      CALL_ERROR << "Could not open file " << filename << " for reading." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   // Re-scan the first line (number of neurons)
   read_without_comments(inFile, tmpThrowAway);
   read_without_comments(inFileDelays, tmpThrowAway);

   // Re-scan through the second (non-blank line)
   for (unsigned int i = 0; i < ni; i++) {
      read_without_comments(inFile, tmpThrowAway);
      read_without_comments(inFileDelays, tmpThrowAway);
   }

#if defined(RNG_BUCKET)
   int  rng_max_available =
      ifloor(2.0f * NumNetworkCon / ParallelInfo::getNumNodes()
         * SystemVar::GetFloatVar("Activity")) + RNG_MAX_GEN;
   IFROOTNODE Output::Out() << "rng_max_available = " << rng_max_available << std::endl;
   const int specseed = (SystemVar::GetIntVar("seed") + ParallelInfo::getRank() * 102) % 32767;
   Output::Out() << MSG << "NeuroJet node seed: " << specseed << std::endl;
   ParallelRand::RandComm.SetParams(1 - SystemVar::GetFloatVar("synFailRate"), rng_max_available, RNG_MAX_GEN);
   ParallelRand::RandComm.ResetSeed(specseed);
#endif

   // Fast forward through inFileDelays pre-synaptic neurons
   for (unsigned int conRow = 0; conRow < ni; ++conRow)
      for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol)
         read_without_comments(inFileDelays, tmpThrowAway);

   // Fast forward through inFileDelays pre-synaptic weights
   for (unsigned int conRow = 0; conRow < ni; ++conRow)
      for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol)
         read_without_comments(inFileDelays, moreTrash);

   // This is only approximately good, but if more exactness is desired, the
   // axonal delays should be specified in the file
   unsigned int numOccupiedSegments = (maxAxonalDelay - minAxonalDelay + 1);      
   unsigned int numSynapsesPerTimeDelay = MaxInputs / numOccupiedSegments;
   unsigned int remSynapsesPerTimeDelay = MaxInputs % numOccupiedSegments;

   // Read in the connections and weights
   IFROOTNODE Output::Out() << "Reading in connections" << std::endl;
   UIVector curLine(MaxInputs);
   UIVector curDelayLine(MaxInputs);
   UIVector numConn(ni);
   // Read in the connectivity matrix (cij)
   for (unsigned int conRow = 0; conRow < ni; conRow++) {
      unsigned int shuffRow = SHUFFLEIFMULTIPROC(conRow);
      unsigned int numConnForNeur = 0;
      for (unsigned int cntCol = 0; cntCol < FanInCon[shuffRow]; cntCol++) {
         unsigned int tmpNeuron;
         unsigned int tmpDelay = 0;
         read_without_comments(inFile, tmpNeuron);
         if (fileHasAxonalDelays) {
            read_without_comments(inFileDelays, tmpDelay);
            if (tmpDelay < 1) {
               CALL_ERROR << "Axonal delay from neuron " << (shuffRow+1) << " to "
                  "neuron " << tmpNeuron << " cannot be less than 1 time-step (in "
                  << filename << ")" << ERR_WHERE;
               exit(EXIT_FAILURE);
            }
         }
         tmpNeuron = SHUFFLEIFMULTIPROC(tmpNeuron);
         curLine.at(cntCol) = tmpNeuron;
         if (fileHasAxonalDelays)
            curDelayLine.at(cntCol) = tmpDelay;
         if (isLocalNeuron(tmpNeuron))
            ++numConnForNeur;
      }
      // Allocate memory for columns, now that we know how many fan-in
      // connections there are for each neuron
      numConn.at(shuffRow) = numConnForNeur;
      inMatrix[shuffRow] = new DendriticSynapse[numConnForNeur];
      DendriticSynapse * dendriticTree = inMatrix[shuffRow];
      unsigned int curConnHere = 0;
      for (unsigned int col = 0; col < FanInCon[shuffRow]; col++) {
         unsigned int tmpNeuron = curLine[col];
         if (isLocalNeuron(tmpNeuron)) {
            dendriticTree[curConnHere].setSrcNeuron(tmpNeuron);
            if (fileHasAxonalDelays) {
               ++FanOutCon[tmpNeuron][curDelayLine.at(col)-1];
            } else {
               unsigned int refTime = minAxonalDelay - 1;
               unsigned int toMake = numSynapsesPerTimeDelay;
               if (refTime-minAxonalDelay+1 < remSynapsesPerTimeDelay)
                  ++toMake;
               while (FanOutCon[tmpNeuron][refTime] > toMake && refTime < (maxAxonalDelay-1)) {
                  ++refTime;
                  if (refTime-minAxonalDelay+1 == remSynapsesPerTimeDelay)
                     --toMake;
               }
               ++FanOutCon[tmpNeuron][refTime];
            }
            // We're using the weights as a temporary place to store the index
            // into the weight file that we should be reading from when we get
            // to the weights section(wij) of the weight file.
            dendriticTree[curConnHere].setWeight(static_cast<float>(col));
            ++curConnHere;
         }
      }
   }

#if defined(MULTIPROC)
   int   P_TotalNumberOfZeros = 0;
   float P_TotalSumOfWeights = 0.0;
   float P_TotalSumOfZeros = 0.0;
#endif
   float zeroCutOff = SystemVar::GetFloatVar("ZeroCutOff");

   IFROOTNODE Output::Out() << "Reading in weights" << std::endl;
   // Read in the weight matrix (wij)
   for (unsigned int weightRow = 0; weightRow < ni; weightRow++) {
      const unsigned int shuffRow = SHUFFLEIFMULTIPROC(weightRow);
      int curConnHere = 0;
      for (unsigned int col = 0; col < FanInCon[shuffRow]; col++) {
         float tempweight;
         read_without_comments(inFile, tempweight);
         // The following if statement is always true if !defined(MULTIPROC)
         if ((numConn[shuffRow] > 0) && (fabs(col - inMatrix[shuffRow][curConnHere].getWeight()) < verySmallFloat)) {
            inMatrix[shuffRow][curConnHere].setWeight(tempweight);
            curConnHere++;
#if defined(MULTIPROC)
            ++NumNetworkCon;
#endif
            if (tempweight < zeroCutOff) {
               TotalNumberOfZeros++;
               TotalSumOfZeros += tempweight;
            } else {
               TotalSumOfWeights += tempweight;
            }
         }
#if defined(MULTIPROC)
         if (tempweight < zeroCutOff) {
            P_TotalNumberOfZeros++;
            P_TotalSumOfZeros += tempweight;
         } else {
            P_TotalSumOfWeights += tempweight;
         }
#endif
      }
      // Now set FanInCon to the LOCAL FanInCon(matches SetConnectivity)
      FanInCon[shuffRow] = curConnHere;
   }
   inFile.close();
   inFileDelays.close();

   IFROOTNODE Output::Out() << "Setting up matrices" << std::endl;
   FillFanOutMatrices();
   ifstream fanoutFile(filename.c_str());
   if (fileHasAxonalDelays) {
      IFROOTNODE Output::Out() << "Reading axonal delays" << std::endl;
      // Read first line (containing number of neurons)
      read_without_comments(fanoutFile, tmpThrowAway);
      // Read through the second (non-blank line)
      for (unsigned int i = 0; i < ni; i++) {
         read_without_comments(fanoutFile, tmpThrowAway);
      }
      // Fast forward through fanoutFile pre-synaptic neurons
      for (unsigned int conRow = 0; conRow < ni; ++conRow)
         for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol)
            read_without_comments(fanoutFile, tmpThrowAway);
      // Fast forward through fanoutFile pre-synaptic weights
      for (unsigned int conRow = 0; conRow < ni; ++conRow)
         for (unsigned int cntCol = 0; cntCol < FanInCon[SHUFFLEIFMULTIPROC(conRow)]; ++cntCol)
            read_without_comments(fanoutFile, moreTrash);
   }
   IFROOTNODE Output::Out() << "Connecting synapses..." << std::endl;
   UIMatrix ConCount(ni, UIVector(maxAxonalDelay, 0));
   SynapseType const* synType = &SynapseType::Member["default"];
   for (unsigned int faninrow = 0; faninrow < ni; faninrow++) {
      DendriticSynapse * dendriticTree = inMatrix[faninrow];
      for (unsigned int col = 0; col < FanInCon[faninrow]; col++) {
         const unsigned int fanoutrow = dendriticTree[col].getSrcNeuron();
         unsigned int refTime;
         if (fileHasAxonalDelays) {
            read_without_comments(fanoutFile, refTime);
            --refTime;
         } else {
            refTime = minAxonalDelay-1;
#if defined(CHECK_BOUNDS)
            while ((refTime < maxAxonalDelay) && 
               (ConCount.at(fanoutrow).at(refTime) >= FanOutCon.at(fanoutrow).at(refTime)))
#else
            while (ConCount[fanoutrow][refTime] >= FanOutCon[fanoutrow][refTime])
#endif
               ++refTime;
         }
         unsigned int synapseNum = ConCount[fanoutrow][refTime];
         outMatrix[fanoutrow][refTime][synapseNum].connectSynapse(faninrow, dendriticTree[col], synType);
         ++ConCount[fanoutrow][refTime];
      }
   }
   fanoutFile.close();
   IFROOTNODE Output::Out() << "Calculating averages" << std::endl;

   SystemVar::SetFloatVar("AveWij", TotalSumOfWeights /
      static_cast<float>(NumNetworkCon - TotalNumberOfZeros));
   SystemVar::SetFloatVar("AveWij0", (TotalSumOfWeights + TotalSumOfZeros) /
      static_cast<float>(NumNetworkCon));
   SystemVar::SetFloatVar("FracZeroWij", static_cast<float>(TotalNumberOfZeros)
                                 / static_cast<float>(NumNetworkCon));
#if defined(MULTIPROC)
   //FIXME: These statistics need to be calculated elsewhere as well
   SystemVar::SetFloatVar("P_AveWij", P_TotalSumOfWeights /
      static_cast<float>(P_NumNetworkCon - P_TotalNumberOfZeros));
   SystemVar::SetFloatVar("P_AveWij0", (P_TotalSumOfWeights + P_TotalSumOfZeros) /
      static_cast<float>(P_NumNetworkCon));
   SystemVar::SetFloatVar("P_FracZeroWij", static_cast<float>(P_TotalNumberOfZeros) /
      static_cast<float>(P_NumNetworkCon));
#endif

   return;
}

inline void GetNullTimingData()
{
#if defined(RNG_BUCK_TIMING)
            // For comparing against when synFailRate is defined
            rng_start_buck = rdtsc();
            rng_elapsed_buck += rdtsc() - rng_start_buck;
#endif
#if defined(TIMING_RNG)
            // For comparing against when synFailRate is defined
            rng_start = rdtsc();
            rng_elapsed += rdtsc() - rng_start;
#endif
}

void getThetaSettings(const NeuronType& NeurType, int& Period, float& Amplitude,
                      float& MidPoint, float& Phase, bool& UseSin)
{
   Period = NeurType.getParameter("Period", Period);
   Amplitude = NeurType.getParameter("Amplitude", Amplitude);
   MidPoint = NeurType.getParameter("MidPoint", MidPoint);
   Phase = NeurType.getParameter("Phase", Phase);
   UseSin = NeurType.getParameter("UseSin", static_cast<int>(UseSin));
}

void InitializeVariables()
{
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      pIt->initInterneurons();
   }

#if defined(MULTIPROC)
   // First, each node is going to need to know how many neurons it's responsible for.
   // ni is total number of neurons over all nodes
   // numNodes is the number of nodes running this job
   // we'll divide the neurons equally between nodes, as much as possible, leaving any
   // remainder to the last node:

   //Calculate which neurons the other nodes are responsible for

#  if defined(PARENT_CHILD)
   unsigned int numNodes = ParallelInfo::getNumNodes() - 1;
   const unsigned int rank = ParallelInfo::getRank() - 1;
#  else // #if defined(PEER_TO_PEER)
   unsigned int numNodes = ParallelInfo::getNumNodes();
   const unsigned int rank = ParallelInfo::getRank();
#  endif
   unsigned int BaseNeuronsPerNode = ni / numNodes;
   unsigned int RemNeuronsPerNode = ni % numNodes;
   
   //Root node will be 0

   //StartNeuron is the first neuron that this node is responsible for
   StartNeuron = rank * BaseNeuronsPerNode;

   //EndNeuron is the last neuron that this node is responsible for
   if (rank < RemNeuronsPerNode) {
      // The first RemNeuronsPerNode neurons get one extra neuron
      StartNeuron += rank;
      EndNeuron = StartNeuron + BaseNeuronsPerNode;
   } else {
      StartNeuron += RemNeuronsPerNode;
      EndNeuron = StartNeuron + BaseNeuronsPerNode - 1;
   }

#  if defined(PARENT_CHILD)
   IFROOTNODE {
      StartNeuron = 1;
      EndNeuron = 0;
      return;
#  endif
   Output::Out() << MSG << "start neuron : " << StartNeuron << std::endl;
   Output::Out() << MSG << "end neuron : " << EndNeuron << std::endl;

#else // #if !defined(MULTIPROC)
   StartNeuron = 0;
   EndNeuron = ni - 1;
#endif

}

#if defined(MULTIPROC)
inline bool isLocalNeuron(const unsigned int nrn)
{
   return ((StartNeuron <= nrn) && (nrn <= EndNeuron));
}
#else
inline bool isLocalNeuron(const unsigned int nrn) { return true; }
#endif

map<string, string> ParseStruct(const string& toParse) {
   string::size_type idx = toParse.find('(');
   string fnName;
   vector<string> argList;
   if (idx != string::npos) {
     fnName = toParse.substr(0, idx);
     argList = tokenize(toParse.substr(idx+1, toParse.size()-idx-2),',',"''\"\"[](){}");
   } else {
     fnName = toParse;
   }
   if (fnName != "struct") {
      CALL_ERROR << "In statement '" << toParse << "', found '"
                 << fnName << "' where 'struct' was expected" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   map<string, string> varList;
   for (unsigned int i = 0; i < argList.size()-1; i+=2) {
      string varName = ltrim(rtrim(argList[i]));
      varName.erase(0,1);
      varName.erase(varName.size()-1,1);
      varList[varName] = ltrim(rtrim(argList[i+1]));
   }
   return varList;
}

#if defined(SYNFAILS_DEBUG_MODE)
inline void SynFailsOutput(int TotalAPs, int TotalSuccess)
{
   for (unsigned int relTime = 0; relTime < maxAxonalDelay; ++relTime)
      for (int i = 0; i < Fired[relTime].size(); ++i)
         TotalAPs += FanOutCon[Fired[relTime][i]][relTime];
   if (TotalAPs > 0)
      printf("\nSYNFAILS: %.4g%% synaptic failures on timestep %d.\t",
              100 * (1.0f - (static_cast<float>(TotalSuccess)) /
                     (static_cast<float>(TotalAPs))), timeStep);
}
#endif   

inline void UpdateBucketStats()
{
#if defined(RNG_BUCK_USG)
   if (SystemVar::GetFloatVar("synFailRate") > 0.0f) {
      Output::Out() << MSG << "RNG's available in bucket: "
              << rng_buck_usage << std::endl;
      Output::Out() << MSG << "RNG's unavailable in bucket: "
              << rng_buck_empty << std::endl;
      ttl_rng_buck_usage += rng_buck_usage;
      ttl_rng_buck_empty += rng_buck_empty;
   }
#endif   
}

void UpdateBuffers(UIPtnSequence &FiringPtns, UIPtnSequence &ExtPtns,
     DataMatrix &BusLines, DataMatrix &IntBusLines,
     DataMatrix &KWeights, DataMatrix &Inhibitions, DataMatrix &FBInternrnExcs,
     DataMatrix &FFInternrnExcs, DataList &ActVect, DataMatrix &Thresholds,
     const unsigned int ndx, const xInput &curPattern,
     const vector<bool> &RecordIdxList)
{
   UIVector curFiring(0);    // These will be appended to (push_back)
   UIVector curExtFiring(0); // These will be appended to
   DataList curBusLines(ni);
   DataList curIntBusLines(ni);
   DataList curKWeights(ni);
   DataList curInhibition(ni);
	DataList curThresholds(ni);
   for (unsigned int i=0; i<ni; ++i) {
      if (zi[SHUFFLEIFMULTIPROC(i)]) {
         curFiring.push_back(i);
      }
      if (curPattern[SHUFFLEIFMULTIPROC(i)]) {
         curExtFiring.push_back(i);
      }
      curBusLines[UNSHUFFLEIFMULTIPROC(i)] = sumwz[i]-sumwz_inhsub[i];
      curIntBusLines[UNSHUFFLEIFMULTIPROC(i)] = somaExc[i];
      curInhibition[UNSHUFFLEIFMULTIPROC(i)] = Inhibition[i];
		curThresholds[UNSHUFFLEIFMULTIPROC(i)] = IzhU[i];
   }
   DataList FBInhibs;
   DataList FFInhibs;
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      DataList KFBWeights = pIt->getKFBWeights();
      unsigned int popSize = pIt->getLastNeuron() - pIt->getFirstNeuron() + 1;
      unsigned int offset = pIt->getFirstNeuron();
      for (unsigned int i=0; i<popSize; ++i) {
         curKWeights[UNSHUFFLEIFMULTIPROC(i+offset)] = KFBWeights[i];
      }
      FBInhibs.push_back(pIt->getFeedbackInhibition());
      FFInhibs.push_back(pIt->getFeedforwardInhibition());
   }
   if (RecordIdxList[0]) FiringPtns.push_back(curFiring);
   if (RecordIdxList[1]) ExtPtns.push_back(curExtFiring);
   if (RecordIdxList[2]) BusLines.push_back(curBusLines);
   if (RecordIdxList[3]) IntBusLines.push_back(curIntBusLines);
   if (RecordIdxList[4]) KWeights.push_back(curKWeights);
   if (RecordIdxList[5]) Inhibitions.push_back(curInhibition);
   if (RecordIdxList[6]) FBInternrnExcs.push_back(FBInhibs);
   if (RecordIdxList[7]) FFInternrnExcs.push_back(FFInhibs);
   if (RecordIdxList[8]) ActVect[ndx] = static_cast<float>(Fired[justNow].size()) / ni;
   if (RecordIdxList[9]) Thresholds.push_back(curThresholds);
}

inline void UpdateMaxBucketStats()
{
#if defined(RNG_BUCK_USG)
   if (rng_buck_usage + rng_buck_empty > max_rng_usg)
      max_rng_usg = rng_buck_usage + rng_buck_empty;
#endif
}

void UpdateParams(const string &varName, const string &varValue,
                  const string &FunctionName)
{
   if (SystemVar::IsReadOnly(varName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Cannot set read only "
                    "variable " << varName << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (!program::defaultsSet()) {
      program::setDefaults(ni);
   }
   const char varType = SystemVar::GetVarType(varName);
   if (varType == 'i') {
      int newValue = from_string<int>(varValue);
      string cfValue = to_string(newValue);
      if (cfValue != varValue) {
         CALL_ERROR << varName << " must be an integer (from -2,147,483,648 to 2,147,483,647)."
                    << ERR_WHERE;
         exit(EXIT_FAILURE);         
      }
      SystemVar::AddIntVar(varName, newValue);
      if (varName == "ni") {
         unsigned int oldni = ni;
         ni = SystemVar::GetIntVar("ni");
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            if ((pIt->getFirstNeuron() == 0) && (pIt->getLastNeuron() == oldni-1)) {
               pIt->setNeuronRange(0, ni-1);
            }
         }
      }
      if (varName == "ruleSize") {
         ruleSize = newValue;
      } 
      if (varName == "spikeTzero") {
         tZero = newValue;
      } 
      if (varName == "FBInternrnAxonalDelay") {
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
           if (!pIt->getNeuronType()->hasParameter(varName)) {
              pIt->updateFBInterneuronAxonalDelay(static_cast<unsigned int>(newValue));
           }
         }
      } else if (varName == "FFInternrnAxonalDelay") {
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
           if (!pIt->getNeuronType()->hasParameter(varName)) {
              pIt->updateFFInterneuronAxonalDelay(static_cast<unsigned int>(newValue));
           }
         }
      }
		if (varName == "FBadjustNumToFire") {
			for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
	              pIt->setFBadjustNumToFire(static_cast<int>(newValue));
         }
		}
      if (varName == "seed") {
         if (newValue < 0) {
            CALL_ERROR << varName << " must be positive."
                       << ERR_WHERE;
            exit(EXIT_FAILURE);         
         }
      }
      if (varName == "NMDArise") { 
         if ((newValue < 0) || (newValue > 19)) {
            CALL_ERROR << varName << " must be greater than -1 and less than 20"
                       << ERR_WHERE;
            exit(EXIT_FAILURE);         
         }
      }
      if (varName == "NMDArise") { 
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setNMDArise(newValue+1);
         }
      }
      if (varName == "stochastic") { 
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setStochastic(newValue);
         }
      }
      if (varName == "saveZbarArray") { 
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setSave(newValue);
         }
      }
      if (varName == "UseMvgAvg") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            const LearningRuleType oldRule = SynapseType::Member["default"].getLearningRule();
            // default rule to use is old rule, unless the old rule was to use the moving average
				const LearningRuleType defRule = (oldRule==LRT_MvgAvg) ? LRT_PostSyn : oldRule;
            const LearningRuleType learningRule = newValue ? LRT_MvgAvg : defRule;
            SynapseType::Member["default"].setLearningRule(learningRule);
         }
      } else if (varName == "UseAltSynapse") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            const LearningRuleType oldRule = SynapseType::Member["default"].getLearningRule();
            // default rule to use is old rule, unless the old rule was to use the alternate synapse
				const LearningRuleType defRule = (oldRule==LRT_PostSynB) ? LRT_PostSyn : oldRule;
            const LearningRuleType learningRule = newValue ? LRT_PostSynB : defRule;
            SynapseType::Member["default"].setLearningRule(learningRule);
         }
      } else if (varName == "UseWeightedActAvg") {
         for (PopulationIt it = Population::Member.begin();
              it != Population::Member.end(); ++it) {
            if (!it->getNeuronType()->hasParameter(varName)) {
               it->setUseWeightedActAvg(static_cast<bool>(newValue));
            }
         }
      }
   } else if (varType == 'f') {
      float newValue = from_string<float>(varValue);
      SystemVar::AddFloatVar(varName, newValue);
      if (varName == "Activity") {
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            if (!pIt->getNeuronType()->hasParameter(varName)) {
               pIt->setDesiredActivity(newValue);
            }
         }
      } else if (varName == "alpha") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setAlpha(newValue);
         }
      } else if (varName == "K0Dend") {
         K0Dend = newValue;
         useDendInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "KFBDend") {
         KFBDend = newValue;
         useDendInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "KFFDend") {
         KFFDend = newValue;
         useDendInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "K0") {
         K0Soma = newValue;
         useSomaInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "KFB") {
         KFBSoma = newValue;
         useSomaInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "KFF") {
         KFFSoma = newValue;
         useSomaInh = (fabs(newValue + 1.0f) > verySmallFloat);
      } else if (varName == "lambdaFB") {
         for (PopulationIt it = Population::Member.begin();
              it != Population::Member.end(); ++it) {
            if (!it->getNeuronType()->hasParameter(varName)) {
               it->setFBInternrnSynModRate(newValue);
            }
         }
      } else if (varName == "lambdaFF") {
         for (PopulationIt it = Population::Member.begin();
              it != Population::Member.end(); ++it) {
            if (!it->getNeuronType()->hasParameter(varName)) {
               it->setFFInternrnSynModRate(newValue);
            }
         }
      } else if (varName == "mu") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setSynModRate(newValue);
         }
      } else if (varName == "muSpike") {
         muSpike = newValue;
      } else if (varName == "resolution") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setResolution(newValue);
         }
      } else if (varName == "inv") {
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setInv(newValue);
         }
      } else if (varName == "PyrToInternrnWtAdjDecay") {
         for (PopulationIt it = Population::Member.begin();
              it != Population::Member.end(); ++it) {
            if (!it->getNeuronType()->hasParameter(varName)) {
               it->setActivityAveragingRate(newValue);
            }
         }
      } else if (varName == "FBadjust") {
			for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
	              pIt->setFBadjust(newValue);
         }
		} else if (varName == "deltaT") {
         for (NeuronTypeMapIt it = NeuronType::Member.begin();
              it != NeuronType::Member.end(); ++it) {
            it->second.convolveFilters();
         }
      } else if (varName == "filterDecay") {
         for (NeuronTypeMapIt it = NeuronType::Member.begin();
              it != NeuronType::Member.end(); ++it) {
            it->second.setFilterDecay(newValue);
         }
      } else if (varName == "InternrnExcDecay") {
         // For now, all populations have the same global interneuron decays
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            if (!pIt->getNeuronType()->hasParameter(varName)) {
               pIt->updateInterneuronDecay(static_cast<double>(newValue));
            }
         }
      } else if (varName == "synFailRate") {
#if defined(MULTIPROC)
         ParallelInfo::setRandCommSuccess(1 - newValue);
#endif
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
            SynapseType::Member["default"].setSynFailRate(newValue);
         }
      } else if (varName == "WeightedActAvgAdj") {
         for (PopulationIt it = Population::Member.begin();
              it != Population::Member.end(); ++it) {
            if (!it->getNeuronType()->hasParameter(varName)) {
               it->setWeightedActAvgAdj(newValue);
            }
         }
      } else if ((varName == "IzhA") || (varName == "IzhB")
                 || (varName == "IzhC") || (varName == "IzhD")) {
         int IzhExplicitCount = program::Main().GetIzhExplicitCount();
         if ((IzhExplicitCount < program::Main().GetIzhExplicitMaxCount())
              && program::Main().getNetworkCreated()) {
            CALL_ERROR << "Error in " << FunctionName << ": Cannot change to\n"
                          "use Izhikevich model after invoking CreateNetwork."
                       << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
   } else if (varType == 's') {
      SystemVar::AddStrVar(varName, varValue);
      if ((varName == "DendriteToSomaFilter") || (varName == "SynapseFilter") || 
	       (varName == "filterFile") || (varName == "InterNeuronfilterFile") || (varName == "ruleFile")) {
			if ( varName == "filterFile") {
			  for (NeuronTypeMapIt it = NeuronType::Member.begin();
                 it != NeuronType::Member.end(); ++it) {
               if (!it->second.hasParameter(varName)) {
                  it->second.loadCustomFilterValues();
               }
            }
			} else if (varName == "InterNeuronfilterFile") {
			   for (PopulationIt it = Population::Member.begin();
                 it != Population::Member.end(); ++it) {
               if (!it->getNeuronType()->hasParameter(varName)) {
                  it->loadCustomSynapseFilterValues();
               }
            }

			} else if (varName == "ruleFile") {
				ruleFile == varValue;
				setSpikeRule();
			} else {
			DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(varValue,
                                    FunctionName, CommandLine(FunctionName));
         if (Matrix.size() > 1) Matrix = transposeMatrix(Matrix);
         DataList filterVals = Matrix.front();
         if (varName == "DendriteToSomaFilter") {
				Output::Out() << "Creating " << varName << " of length " << filterVals.size() << std::endl;
				Output::Out() << "   Use a preload of at least " << 2*filterVals.size() << std::endl;         
            for (NeuronTypeMapIt it = NeuronType::Member.begin();
                 it != NeuronType::Member.end(); ++it) {
               if (!it->second.hasParameter(varName)) {
                  it->second.loadDTSFilterValues(filterVals);
               }
            }
         } else {
				Output::Out() << "Creating " << varName << " of length " << filterVals.size() << std::endl;
				Output::Out() << "   Use a preload of at least " << 2*filterVals.size() << std::endl;         
            for (PopulationIt it = Population::Member.begin();
                 it != Population::Member.end(); ++it) {
               if (!it->getNeuronType()->hasParameter(varName)) {
                  it->loadSynapseFilterValues(filterVals);
               }
            }
         } //second else
			} //first else
      } else if (varName == "IzhType") {
         vector<float> result = assignIzhParams(ucase(varValue));
         SystemVar::AddFloatVar("IzhA", result[0]);
         SystemVar::AddFloatVar("IzhB", result[1]);
         SystemVar::AddFloatVar("IzhC", result[2]);
         SystemVar::AddFloatVar("IzhD", result[3]);
         SystemVar::AddFloatVar("IzhE", result[4]);
         SystemVar::AddFloatVar("IzhF", result[5]);
         SystemVar::AddFloatVar("IzhVMax", result[6]);
         SystemVar::AddFloatVar("IzhvStart", result[7]);
         SystemVar::AddFloatVar("IzhuStart", result[8]);
      } else if (varName == "riseFile") { 
         if (SynapseType::Member.find("default") != SynapseType::Member.end()) {
				string str (varValue.c_str()) ;
            SynapseType::Member["default"].setRiseFile(str);
         }
      }
   } else {
      CALL_ERROR << "Error in " << FunctionName << ": " << varName
                 << " not found.\n\n" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
}

//FLEX: Many other weight update rules exist
inline void UpdateWeights() {
   for (unsigned int i = 0; i < Fired[justNow].size(); i++) { /* Postsynaptic fired now */
#if defined(CHECK_BOUNDS)
      unsigned int iFire = Fired.at(justNow).at(i);
#else
      unsigned int iFire = Fired[justNow][i];
#endif
      DendriticSynapse * dendriticTree = inMatrix[iFire];
      for (unsigned int c = 0; c < FanInCon[iFire]; ++c) {
         dendriticTree[c].updateWeight(timeStep);
      }
   }
}

inline void UpdateWeightsSpike(const UIPtnSequence& Z) {
	//Here we use a spikeRule to update the weights for an entire network at the END of a trial
	int s = Z.size(); // how long the trial was

	//initialize newZ with 0's
   int newZ[ s + 2 * ruleSize][ni];
	for (int i = 0; i < s+ 2*ruleSize; i++)
		for (int j = 0; j < ni; j++) 
			newZ[i][j] = 0;

	int t = 0;
   //now fill in "newZ" with the old Z, but pad the left and right with ruleSize 0's;
	for (UIPtnSequenceCIt it = Z.begin(); it != Z.end(); it++) {
		const UIVector pat = *it;
		for(int c = 0; c < pat.size(); c++) {
			newZ[t + ruleSize][ pat[c] ] = 1;
		}
		t++;
	}
	//Output::Out() << "s = " << s << std::endl;


   //new cycle through entire Z matrix and stop at each '1'
	float delW;
	int   pre;
   float tot;
   float weight;
   for (int time = 0; time < s+ 2*ruleSize; time++) {
		for (int nur = 0; nur < ni; nur++) {
			if( newZ[time][nur] == 1) {
				//post-syn neuron 'nur' fired at 'time' (note that newZ is padded)
				DendriticSynapse * dendTree = inMatrix[nur]; //these are all the inputs to nur
      		for (unsigned int c = 0; c < FanInCon[nur]; ++c) { //loop through each input
         		pre  = dendTree[c].getSrcNeuron();
					delW = 0.0f;
					tot  = 0.0f;
					for( int tOff = ruleSize-1; tOff >= 0; tOff--) { //calculate rule dot firing
						//tot  += newZ[time + tOff - tZero][pre];
						if( newZ[time + tOff - tZero][pre] == 1) {
							delW += newZ[time + tOff - tZero][pre] * rule[tOff];
							tot++; //get the pre that fired closest to post, starting with pre just after post
							//break;
						}
						//Output::Out() << "tOff = " << tOff << " rule[tOff] = " << rule[tOff] << std::endl; //" delW = " << delW << std::endl;
					}
					weight = dendTree[c].getWeight();
					if (tot == 0.0f) { //pre never fired, decrease weight by muSpike * W
					//dendTree[c].setWeight( weight - muSpike * weight ) ;
					//Output::Out() << "pre = " << pre << " post = " << nur << " delW = " << - (muSpike / 2) * weight << std::endl;
					} else { // pre fired, use the rule. Current rule is AVERAGE see "/ tot"
					dendTree[c].setWeight( weight + ( (delW / tot ) - weight ) * muSpike );
					//Output::Out() << "pre = " << pre << " post = " << nur << " deltaW = " << ((delW / tot + 1)/2 - weight) * muSpike << std::endl;
					}
					if(dendTree[c].getWeight() < verySmallFloat) dendTree[c].setWeight(0.0f); //get rid of negative weights
      		}
			}
		}
	}

	//output the newZ
/*	for (int i = 0; i < s+ 2*ruleSize; i++) {
		for (int j = 0; j < ni; j++) {
			Output::Out() << newZ[i][j] << " ";
		}
		Output::Out() << std::endl;
	}*/

//clean up
//delArray(newZ); doesn't work.

}

void WriteMATLABHeader(ofstream &MATfile, int mrows, int ncols, int namlen, 
                       bool isSparse, bool isText)
{
   const int one = 1;
   const unsigned char *endianesschk = (unsigned char *) &one;
   const unsigned char endianessref = 0xFF;
   int MOPT = 0;
   if (((*endianesschk) & endianessref) == 0) {
      MOPT += 1000; // big-endian
   }
   if (isText) {
      MOPT += 1;
   } else if (isSparse) {
      MOPT += 2;
   }
   MATfile.write((char *)&MOPT, 4);
   MATfile.write((char *)&mrows, 4);
   MATfile.write((char *)&ncols, 4);
   const int imagf = 0; // no imaginary numbers in NeuroJet (are there?)
   MATfile.write((char *)&imagf, 4);
   MATfile.write((char *)&namlen, 4);
}

// ArGhhh: Record the watched inputs
inline void RecordSynapticFiring(const int &iFire, const string & FileName) {
   // Find the proper time
   static float dt = 0.0f;
   if (fabs(dt) < verySmallFloat) {
     if (fabs(SystemVar::GetFloatVar("deltaT") + 1.0f) > verySmallFloat)
        dt = SystemVar::GetFloatVar("deltaT");
     else if (fabs(SystemVar::GetFloatVar("alpha") + 1.0f) > verySmallFloat)
        dt = -100.0f * log(SystemVar::GetFloatVar("alpha")) ;
     else {
        CALL_ERROR << "Time interval not set. Please set at least "
                      "one of: deltaT, alpha"<<endl << ERR_WHERE;
        exit(EXIT_FAILURE);
     }
   }
   // Output in the format of <time> (<weight> <synapse>)* -1 \n
   ofstream OutFile(FileName.c_str(),std::ios::app);
   if (OutFile == NULL) {
         CALL_ERROR << "Error in RecordSynapticFiring: Could not open file "
            << FileName << " for writing" << ERR_WHERE;
         exit(EXIT_FAILURE);
   }
   static int startTime = timeStep ;
   //Output::Out()<<"dt = "<<dt<<endl;
   OutFile<<(timeStep - startTime)*dt<<" ";

   DendriticSynapse * dendriticTree = inMatrix[iFire];
   for (unsigned int c = 0; c < FanInCon[iFire]; c++)
   {
      // This attempts to account for failure
      // If the second firing happens within NMDArise time from previous firing, 
      // and the second firing is a failure, this code will unfortunately
      // result in counting it as the proper firing.
      // This behavior is consistent with the behavior of the NeuroJet.
      DendriticSynapse synapse = dendriticTree[c];
      if ((synapse.getLastActivate() - timeStep <= static_cast<int>(synapse.getSynapseType()->getNMDArise())) && 
            (zi[synapse.getSrcNeuron()]))
         OutFile << synapse.getWeight() << " " << synapse.getSrcNeuron() << " ";
   }
   OutFile << "-1" << endl;
}

void Present(const xInput &curPattern, DataMatrix &IzhVValues, DataMatrix &IzhUValues,
             const bool modifyInhWeights, const bool modifyExcWeights)
{
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      pIt->calcNewFeedbackInhibition(Fired[justNow]);
      pIt->calcNewFeedforwardInhibition(curPattern);
   }

#   if defined(TIMING_MODE)
   clock_t t1 = clock();
#   endif

   bool calcSynAct = true;
#if defined(PARENT_CHILD)
   // exchange info
   IFROOTNODE {
      sumwz = ParallelInfo::rcvSumwz();
      sumwz_inhdiv = ParallelInfo::rcvSumwz();
      sumwz_inhsub = ParallelInfo::rcvSumwz();
      calcCynAct = false; // PARENT_CHILD code still calcs syn act for child
   }
#endif
   if (calcSynAct) {
      // Sum presynaptic(fan-in) activity for each neuron 
      CalcSynapticActivation(LOCALFIRED, curPattern);
   }

#if defined(TIMING_P2P)
   long long elapsed, start;
   elapsed = 0;
   start = rdtsc();
#endif

#if defined(PARENT_CHILD)
   // send the sumwz to the root node
   IFCHILDNODE {
     ParallelInfo::sendSumwz(sumwz);
     ParallelInfo::sendSumwz(sumwz_inhdiv);
     ParallelInfo::sendSumwz(sumwz_inhsub);
   }
#elif defined(PEER_TO_PEER)
   // Exchange and sum sumwz among nodes
   sumwz = ParallelInfo::exchangeSumwz(sumwz);
   sumwz_inhdiv = ParallelInfo::exchangeSumwz(sumwz_inhdiv);
   sumwz_inhsub = ParallelInfo::exchangeSumwz(sumwz_inhsub);
#endif

#if defined(TIMING_P2P)
   elapsed = rdtsc() - start;
   Output::Out() << "Elapsed exchange time = " << elapsed * 1.0 / TICKS_PER_SEC
                 << " seconds" << endl;
   total_time += elapsed * 1.0 / TICKS_PER_SEC;
   trials++;
#endif

   timeStep++;

   bool calcNeuronData = true;
#if defined(PARENT_CHILD)
   IFCHILDNODE {
      // receive zi from the root node
      zi = ParallelInfo::rcvZi();

      //------ figure out how many neurons fired, and how many neurons belonging to this
      // compute node fired
      Fired.pop_back();
      Fired.push_front(UIVector(0));
      FiredHere.pop_back();
      FiredHere.push_front(UIVector(0));

      //chug the zi data into fired arrays
      for (unsigned int i = 0; i < ni; i++) {
         if (zi[i]) FireSingleNeuron(i);
      }
      calcNeuronData = false;
   }
#endif
   if (calcNeuronData) {
      CalcDendriticExcitation();
      CalcDendriticToSomaInput(curPattern, false);
      CalcSomaResponse(curPattern, IzhVValues, IzhUValues);
   }

#if defined(PARENT_CHILD)
   IFROOTNODE
      ParallelInfo::sendZi(zi);
#endif

   if (modifyInhWeights) {
      for (PopulationIt pIt = Population::Member.begin();
           pIt != Population::Member.end(); ++pIt) {
         pIt->updateInternrnWeights(Fired[justNow], Fired[lastTime], curPattern);
      }
   }
   if (modifyExcWeights) {
#if defined(PARENT_CHILD)
      IFCHILDNODE // Only update weights for children
#endif
		if (SystemVar::GetFloatVar("mu") > verySmallFloat) UpdateWeights();
   }

   return;
}

void readDataType(const StrArg &Type, DataListType &newDataType, string &newType,
                  string &newSubType, const string FunctionName,
                  const CommandLine &ComL, const bool allowFile)
{
   newDataType = DLT_unknown;
   if (Type.getValue() == "seq") {
      newDataType = DLT_sequence;
      newType = "sequence";
      newSubType = "patterns";
   } else if (Type.getValue() == "mat") {
      newDataType = DLT_matrix;
      newType = "matrix";
      newSubType = "vectors";
   } else if (Type.getValue() == "ana") {
      newDataType = DLT_analysis;
      newType = "analysis";
      newSubType = "data lists";
   } else if (allowFile && Type.getValue() == "file") {
      newDataType = DLT_file;
      newType = "file";
      newSubType = "lines";
   } else {
      CALL_ERROR << "Error in " << FunctionName << " : unrecognized data type of "
                 << Type.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
}

MatlabCommand ReadMATLABcommand(ifstream& mfile, const string& popFile) {
   bool atEOC = false; // EOC = end of command, which will be either ';' or '\n'
   bool inDQ = false; // DQ = double quote
   bool inSQ = false; // SQ = single quote, but don't get confused with transpose
   bool inMath = false;      // In the middle of a math statement, '\n' won't end command
   bool inMatrix = false;    // []
   bool inParens = false;    // ()
   bool inCellArray = false; // {}
   bool ignoreNextWhitespace = true;
   const string ignoreTrigger = " ;{}[]=()";
   unsigned int depthIn = 0; // Depth of [, (, or {, but mainly (
   string wholeCmd = "";
   char prevChar = '\0';
   char nextChar;
   while ((!mfile.eof()) && (!atEOC)) {
      nextChar = mfile.get();
      if (nextChar == '%') { // MATLAB comment
         do {
            nextChar = mfile.get();
         } while (nextChar != '\n');
         nextChar = mfile.get();
      }
      atEOC = (wholeCmd.size() > 0) && (!inDQ) && (!inSQ) && (!inMath) && (!inMatrix)
        && (!inParens) && (!inCellArray) && ((nextChar == ';') || (nextChar == '\n'));
      if (!atEOC) {
         if ((nextChar == '.') && (prevChar == '.')) {
            // 2 dots in a row - let's assume 3
            nextChar = mfile.get();
            if (nextChar != '.') {
               CALL_ERROR << "Unexpected '..' in population file " << popFile << "," << ERR_WHERE << std::endl;
               exit(EXIT_FAILURE);
            }
            // Erase that previous "."
            wholeCmd = wholeCmd.erase(wholeCmd.size()-1);
            // We've now found (and discarded) "..." - we're expecting a '\n'
            do {
               nextChar = mfile.get();
            } while ((nextChar != '\n') && !mfile.eof());
         }
         if (nextChar == ';') {
            CALL_ERROR << "Unexpected ';' in population file " << popFile << "," << ERR_WHERE << std::endl;
            exit(EXIT_FAILURE);
         }
         if (isspace(nextChar)) {
            nextChar = ' ';
         } else {
            ignoreNextWhitespace = false;
         }
         if ((nextChar != ' ') || (!ignoreNextWhitespace)) {
            wholeCmd += nextChar;
         }
         if (ignoreTrigger.find(nextChar) != string::npos) {
            ignoreNextWhitespace = true;
         }
         if (inDQ) {
            if (nextChar == '"') inDQ = false;
         }
         if (inSQ) {
            if (nextChar == '\'') inSQ = false;
         }
         if (inMath) {
            // FIXME: This logic probably needs to be tightened up, but currently
            // I'm not expecting any math in this file
            if (nextChar != ' ') inMath = false;
         }
         bool inString = inDQ || inSQ;
         checkNextChar(inMatrix, depthIn, inString, nextChar, '[', ']');
         checkNextChar(inParens, depthIn, inString, nextChar, '(', ')');
         checkNextChar(inCellArray, depthIn, inString, nextChar, '{', '}');
         if (!(inString || inMatrix || inParens || inCellArray)) {
            if (nextChar == '"') inDQ = true;
            if (nextChar == '\'') inSQ = true;
            // MathChars is in Calc.hpp, but since % is comment in Matlab, disinclude
            if ((MathChars.find(nextChar) != string::npos) && (nextChar != '%')) inMath = true;
         }
         prevChar = nextChar;
      }
   }
   string::size_type index = wholeCmd.find('=');
   string LHS = "";
   string RHS = wholeCmd;
   if (index != string::npos) {
      LHS = wholeCmd.substr(0,index-1);
      RHS.erase(0, index+1);
   }
   return MatlabCommand(LHS, RHS);
}

void ReadNJNetworkFile(const string& filename) {
   ifstream idxFile(filename.c_str());
   string lineBuf;
   UIMatrix effDelays = UIMatrix(ni);
   Population::Member.clear(); // Get rid of default population
   minAxonalDelay = std::numeric_limits<unsigned int>::max();
   maxAxonalDelay = 0;
   while (std::getline(idxFile, lineBuf)) {
      lineBuf = ltrim(lineBuf); // Remove leading whitespace
      const string commentChars = "#%/";
      if (lineBuf.size() > 0 && commentChars.find(lineBuf[0]) == std::string::npos) {
         if (fileExists(lineBuf)) {
            IFROOTNODE {
               Output::Out() << "Reading Population File: " << lineBuf << "\n";
            }
            ReadPopulationFile(lineBuf, effDelays);
         } else {
            CALL_ERROR << "Unable to locate population file " << lineBuf
                       << " referenced in weight file " << filename << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
   }
   idxFile.close();
   FillFanOutMatrices();
   //FIXME: Not collecting statistics as in other create network routines
   UIMatrix ConCount(ni, UIVector(maxAxonalDelay, 0));
   for (PopulationCIt PCIt = Population::Member.begin(); PCIt != Population::Member.end(); ++PCIt) {
      const NeuronType* faninNType = PCIt->getNeuronType();
      map<string, SynapseType const*> mySynTypes = SynapseType::findPreSynapticTypes(faninNType->getName());
      for (unsigned int faninrow = PCIt->getFirstNeuron(); faninrow <= PCIt->getLastNeuron(); ++faninrow) {
         DendriticSynapse* dendriticTree = inMatrix[faninrow];
         for (unsigned int col = 0; col < FanInCon[faninrow]; ++col) {
            const unsigned int fanoutrow = dendriticTree[col].getSrcNeuron();
            const NeuronType* fanoutNType = findNeuronType(fanoutrow);
            SynapseType const* synType = (mySynTypes.find(fanoutNType->getName()) == mySynTypes.end()) ?
               &SynapseType::Member["default"] : mySynTypes[fanoutNType->getName()];
            synType = mySynTypes[fanoutNType->getName()];
            unsigned int refTime = effDelays[faninrow][col]-1;
            unsigned int synapseNum = ConCount[fanoutrow][refTime];
            outMatrix[fanoutrow][refTime][synapseNum].connectSynapse(faninrow, dendriticTree[col], 
                                                synType, fanoutNType->isExcType(), fanoutNType->isInhDivType());
            ++ConCount[fanoutrow][refTime];
         }
         effDelays[faninrow].clear(); // Need to restore memory as we use it
      }
   }
   // Check that population combination makes sense
   // First, create sorted list of population (by first neuron)
   vector<UIPair> firstLast;
   for (PopulationCIt PCIt = Population::Member.begin();
        PCIt != Population::Member.end(); ++PCIt) {
      bool inserted = false;
      for (vector<UIPair>::iterator it=firstLast.begin(); (it != firstLast.end() && !inserted); ++it) {
         if (it->first > PCIt->getFirstNeuron()) {
            firstLast.insert(it, UIPair(PCIt->getFirstNeuron(), PCIt->getLastNeuron()));
            inserted = true;
         }
      }
      if (!inserted) {
         firstLast.push_back(UIPair(PCIt->getFirstNeuron(), PCIt->getLastNeuron()));
      }
   }
   // Next, check that the first neuron of population n+1 is 1 greater than the
   // last neuron of population n
   unsigned int prevLast = 0;
   for (vector<UIPair>::iterator it=firstLast.begin(); it != firstLast.end(); ++it) {
      // prevLast is 1-based, it->first is 0-based, message is 1-based: deal with it
      if (it->first != prevLast) {
         if (it->first < prevLast) {
            CALL_ERROR << "Neurons " << (it->first+1) << " through " << prevLast
                       << " are in at least two different populations " << ERR_WHERE;
            exit(EXIT_FAILURE);
         } else {
            CALL_ERROR << "Neurons " << (prevLast+1) << " through " << it->first
                       << " are not in any populations " << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
      prevLast = it->second+1;
   }
   if (prevLast != ni) {
      CALL_ERROR << "Last neuron found was " << prevLast << ", but ni was specified as "
                 << ni << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   for (PopulationIt pIt = Population::Member.begin(); pIt != Population::Member.end(); ++pIt)
      pIt->initInterneurons();
}

void ReadPopulationFile(const string& filename, UIMatrix& effDelays) {
   // Need to be able to read commands, functions, strings, cell arrays, and matrices
   // Commands are either functions or assignments and can contain any of the other types
   // Functions can contain strings, cell arrays, and matrices
   // Cell arrays can contain strings and matrices
   //
   // In an attempt to not over-engineer this, we only accept the following statements
   // x = struct(desc, val, desc, val, ...)
   // createPopulation(x,y,z)
   ifstream popFile(filename.c_str());
   if (!popFile.is_open()) {
      CALL_ERROR << "Unable to read population file " << filename << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   map<string, string> popVars;
   while (!popFile.eof()) {
      MatlabCommand tmp = ReadMATLABcommand(popFile, filename);
      string RHS = tmp.getRHS();
      string LHS = tmp.getLHS();
      if (LHS.size() > 0) {
         if (RHS.size() == 0) {
            CALL_ERROR << LHS << " has no assignment in " << filename << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         popVars[LHS] = RHS;
      } else if (RHS.size() > 1) {
         string::size_type idx = RHS.find('(');
         string fnName;
         vector<string> argList;
         if (idx != string::npos) {
           fnName = RHS.substr(0, idx);
           argList = tokenize(RHS.substr(idx+1, RHS.size()-idx-2),',',"''\"\"[](){}");
         } else {
           fnName = RHS;
         }
         if (fnName == "createPopulation") {
            const unsigned int numArgsExpected = 3;
            if (argList.size() != numArgsExpected) {
               CALL_ERROR << "Expected 3 arguments in '" << fnName << "' and found "
                          << argList.size() << " in " << filename << ERR_WHERE;
               exit(EXIT_FAILURE);
            }
            for (unsigned int i=0; i<numArgsExpected; ++i) {
               if (popVars.find(ltrim(rtrim(argList[i]))) == popVars.end()) {
                  CALL_ERROR << "Unspecified variable '" << argList[i] << "' passed to '"
                             << fnName << "' in " << filename << ERR_WHERE;
                  exit(EXIT_FAILURE);
               } else {
                  argList[i] = popVars[ltrim(rtrim(argList[i]))];
               }
            }
            PopulatePopulation(argList[0], argList[1], argList[2], effDelays);
         } else {
            CALL_ERROR << "Encountered unknown command '" << fnName << "' in " << filename << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
   }
   popFile.close();
}

template<class T>
inline void vswap(vector<T> &arr, int index1, int index2)
{
#if defined(CHECK_BOUNDS)
   T temp = arr.at(index1);
   arr.at(index1) = arr.at(index2);
   arr.at(index2) = temp;
#else
   T temp = arr[index1];
   arr[index1] = arr[index2];
   arr[index2] = temp;
#endif
}

// This code is based on an algorithm derived from Numerical Recipes in C, 2nd Ed.,
// p. 342
double selectCutOff(unsigned long k, unsigned long n, vector<IxSumwz> &selectFrom)
{
   if (k == 0) return 0.0f;

   // selectCutOff finds the kth largest value (as opposed to the kth smallest
   // vector) in the vector selectFrom with first element 0 and last element n-1.
   // The kth largest value will reside in selectFrom[k-1], elements larger than
   // the kth value will be to the left (i.e., reside in selectFrom[0..k-2]),
   // and elements smaller will be to the right (i.e., reside in
   // selectFrom[k..n-1]). These elements will be in arbitrary order, however.
   // I.e., these elements are only approximately in reverse sorted order.
   // This algorithm therefore differs from most kth choose algorithms.

   unsigned long int active_partition_begin;
   unsigned long int active_partition_second; // a_p_begin+1 - used frequently
   unsigned long int active_partition_end;
   unsigned long int active_partition_middle;
   unsigned long int lesser_element_index;
   unsigned long int greater_element_index;
   const unsigned long int kth_element = k - 1;
   struct IxSumwz partition_element;

   active_partition_begin = 0;
   active_partition_second = 1;
   active_partition_end = n - 1;
   while (true) {
      // If the active partition contains only 1 element, return it. Or, if it
      // contains two elements, put the two elements in reverse order (if not
      // already) and return the kth (0th or 1st) element
      if (active_partition_end <= active_partition_second) {
         if ((active_partition_end == active_partition_second) &&
             (selectFrom[active_partition_begin] < selectFrom[active_partition_end])) {
            vswap(selectFrom, active_partition_begin, active_partition_end);
         }
         return selectFrom[kth_element].y;
      } else {
         // The partitioning element is the median of the begin, middle, and end
         // elements. Also, ensure that begin > second > end elements.
         active_partition_middle = (active_partition_begin + active_partition_end) / 2;
         vswap(selectFrom, active_partition_middle, active_partition_second);
         // Make sure b > e
         if (selectFrom[active_partition_begin] < selectFrom[active_partition_end]) {
            vswap(selectFrom, active_partition_begin, active_partition_end);
         }
         // Make sure s > e
         if (selectFrom[active_partition_second] < selectFrom[active_partition_end]) {
            vswap(selectFrom, active_partition_second, active_partition_end);
         } else if (selectFrom[active_partition_begin] < selectFrom[active_partition_second]) {
          // Make sure b > s (if we just swapped then this is already true)
            vswap(selectFrom, active_partition_begin, active_partition_second);
         }
         lesser_element_index = active_partition_second;
         greater_element_index = active_partition_end;
         partition_element = selectFrom[active_partition_second];
         while (true) {
            // Scan up to find element < partition_element.
            do
               lesser_element_index++;
            while (partition_element < selectFrom[lesser_element_index]); 
            // Scan down to find element > partition_element.
            do
               greater_element_index--;
            while (partition_element > selectFrom[greater_element_index]);  
            if (greater_element_index < lesser_element_index)
               break;   // Pointers crossed. Partitioning complete.
            vswap(selectFrom, lesser_element_index, greater_element_index);
         }         
         // Insert partitioning element
         selectFrom[active_partition_second] = selectFrom[greater_element_index];
         selectFrom[greater_element_index] = partition_element;
         // The active partition is the partition containing the kth element
         if (greater_element_index >= kth_element)
            active_partition_end = greater_element_index - 1;
         if (greater_element_index <= kth_element) {
            active_partition_begin = lesser_element_index;
            active_partition_second = lesser_element_index+1;
         }
      }
   }
   // Only required for compiler to not complain about no return
   return selectFrom[k].y;
}

void resetDendriticQueues() {
   for (unsigned int i=0; i<dendriteQueue.size(); ++i) delArray(dendriteQueue[i]);
   for (unsigned int i=0; i<dendriteQueue_inhdiv.size(); ++i) delArray(dendriteQueue_inhdiv[i]);
   for (unsigned int i=0; i<dendriteQueue_inhsub.size(); ++i) delArray(dendriteQueue_inhsub[i]);
   dendriteQueue = vector<float*>(ni);
   dendriteQueue_inhdiv = vector<float*>(ni);
   dendriteQueue_inhsub = vector<float*>(ni);
   for (PopulationCIt it = Population::Member.begin();
        it != Population::Member.end(); ++it) {
      const unsigned int filterSize = it->getNeuronType()->getFilterSize();
      for (unsigned int i=it->getFirstNeuron(); i<=it->getLastNeuron(); ++i) {
         dendriteQueue[i] = new float[filterSize]();
         dendriteQueue_inhdiv[i] = new float[filterSize]();
         dendriteQueue_inhsub[i] = new float[filterSize]();
      }
   }
}

void ResetSTM()
{
   // Reset the number fired to zero for all relative time offsets
   Fired.clear();
   for (unsigned int relTime = 0; relTime <= maxAxonalDelay; ++relTime) {
      Fired.push_front(UIVector(0));
   }
#if defined(MULTIPROC)
   FiredHere.clear();
   for (unsigned int relTime = 0; relTime <= maxAxonalDelay; ++relTime) {
      FiredHere.push_front(UIVector(0));
   }
#endif
   somaExc = DataList(ni, 0.0L);
   memset(VarKConductanceArray, ZERO, ni * sizeof(float));
   // Reset current timestep to Z0
   timeStep = 0;
   const float IzhvStart = SystemVar::GetFloatVar("IzhvStart");
   const float IzhuStart = SystemVar::GetFloatVar("IzhuStart");
   for (unsigned int i = 0; i < ni; i++) {
      DendriticSynapse * dendriticTree = inMatrix[i];
      for (unsigned int c = 0; c < FanInCon[i]; ++c)
         dendriticTree[c].resetLastActivate();
      if (fabs(IzhvStart+1) > verySmallFloat) {
         IzhV[i] = IzhvStart;
         IzhU[i] = IzhuStart;
      }
   }
   
   resetDendriticQueues();
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      pIt->resetInterneurons();
   }
   string FunctionName = "ResetSTM";
   static CommandLine ComL(FunctionName);

   // Set Z0 pattern: either generate a new random pattern 
   //   or get the sequence pointed to by ResetPattern.
   //   
   unsigned int lastTimeOffset = 1U;
   for (PopulationCIt it = Population::Member.begin();
        it != Population::Member.end(); ++it) {
      updateMax(lastTimeOffset, it->getNeuronType()->getFilterSize());
   }
   if (SystemVar::GetStrVar("ResetPattern") == "") {
      // Reset with random noise, forced probability of firing
      for (unsigned int timeOffset = 0; timeOffset < lastTimeOffset; ++timeOffset) {
         // Reset with Z0
         zi = Pattern(ni, false);
         Fired.pop_back();
         Fired.push_front(UIVector(0));
#if defined(MULTIPROC)
         FiredHere.pop_back();
         FiredHere.push_front(UIVector(0));
#endif
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            const unsigned int firstN = pIt->getFirstNeuron();
            const unsigned int popSize = pIt->getLastNeuron() - firstN + 1;
            const float ResetAct = pIt->getNeuronType()->getParameter("ResetAct",
                                                           SystemVar::GetFloatVar("ResetAct"));
            const unsigned int Num2Fire = iround(popSize * ResetAct);
            for (unsigned int i = 0; i < Num2Fire; ++i) {
               int j;
               do
                  j = program::Main().getResetNeuron(popSize) + firstN;
               while (zi[j]);
               FireSingleNeuron(j);
            }
         }
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            pIt->calcNewFeedbackInhibition(Fired[justNow]);
            pIt->calcNewFeedforwardInhibition(Fired[justNow]);
         }
         // Adds it to the dendritic queue
         CalcSynapticActivation(LOCALFIRED, zi);
         CalcDendriticExcitation();
      }
   } else {
      // Reset with the specified pattern from ResetPattern
      string ResetPattern = SystemVar::GetStrVar("ResetPattern");
      // Copy input information into time = 0
      UIPtnSequence Seq = SystemVar::getSequence(ResetPattern, FunctionName, ComL);
      for (UIPtnSequenceCIt it = Seq.begin(); it != Seq.end(); it++) {
         const UIVector pat = *it;
         // Fire the Z0 neurons
         zi = Pattern(ni, false);
         Fired.pop_back();
         Fired.push_front(UIVector(0));
#if defined(MULTIPROC)
         FiredHere.pop_back();
         FiredHere.push_front(UIVector(0));
#endif
         if ((pat.size() > 0) && (pat.back() >= ni)) {
            CALL_ERROR << "Error in ResetSTM: Pattern in ResetPattern contains"
                          " input outside defined region!\n" << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         for (UIVectorCIt pIt = pat.begin(); pIt != pat.end(); ++pIt) {
            FireSingleNeuron(*pIt);
         }
         for (PopulationIt pIt = Population::Member.begin();
              pIt != Population::Member.end(); ++pIt) {
            pIt->calcNewFeedbackInhibition(Fired[justNow]);
            pIt->calcNewFeedforwardInhibition(Fired[justNow]);
         }
         // Adds it to the dendritic queue
         CalcSynapticActivation(LOCALFIRED, zi);
         CalcDendriticExcitation();
      }
   }

   return;
}

void SetConnectivity(const int &AllowSelf, const char &dType,
                      const float &p1, const float &p2,
                      const float &p3, const float &p4)
{
   int  TotalNumberOfZeros = 0;
   int  OneThird = ifloor(static_cast<float>(ni) / 3);
   double TotalSumOfWeights = 0.0f;
   double TotalSumOfZeros = 0.0f;

   IFROOTNODE Output::Out() << "Setting up the connections" << flush;

   int  NumNeuronsHere = EndNeuron - StartNeuron + 1;
   const unsigned int NumCon = iround(SystemVar::GetFloatVar("Con") * NumNeuronsHere);
   NumNetworkCon = ni * NumCon;
#if defined(RNG_BUCKET)
   int  rng_max_available = iround(2 * NumCon * SystemVar::GetFloatVar("Activity"));
   Output::Out() << "rng_max_available = " << rng_max_available << std::endl;
   const int specseed = (SystemVar::GetIntVar("seed") + ParallelInfo::getRank() * 102) % 32000;
   Output::Out() << MSG << "NeuroJet node seed: " << specseed << std::endl;
   ParallelRand::RandComm.SetParams(1 - SystemVar::GetFloatVar("synFailRate"), rng_max_available, RNG_MAX_GEN);
   ParallelRand::RandComm.ResetSeed(specseed);
#endif

   for (unsigned int i = 0; i < ni; i++) {
      // Set up number of connections for each neuron
      FanInCon[i] = NumCon;
      inMatrix[i] = new DendriticSynapse[NumCon];
   }

   bool isPointDist = (dType != 'u') && (dType != 'n');
   // Set up connections
   unsigned int NumMade = 0;
   bool found = false;
   double tempweight = 0.0;
   float zeroCutOff = SystemVar::GetFloatVar("ZeroCutOff");
   for (unsigned int n = 0; n < ni; n++) {
      if (OneThird && n && !(n % OneThird)) {
         Output::Out() << "." << flush;
      }
      unsigned int numOccupiedSegments = (maxAxonalDelay - minAxonalDelay + 1);
      unsigned int numSynapsesPerTimeDelay = NumCon / numOccupiedSegments;
      unsigned int remSynapsesPerTimeDelay = NumCon % numOccupiedSegments;
      NumMade = 0;
      while (NumMade < NumCon) {
         found = false;
         const unsigned int NeuronIn
            = program::Main().getConnectNoise(StartNeuron, EndNeuron);

         // Check for self connections
         if (!AllowSelf && (n == NeuronIn)) continue;

         // Check to see if neuron is already assigned
         for (unsigned int m = 0; m < NumMade; m++) {
            if (inMatrix[n][m].getSrcNeuron() == NeuronIn) {
               found = true;
               break;
            }
         }

         if (found) continue;

         // If not found, add it to list
         inMatrix[n][NumMade].setSrcNeuron(NeuronIn);

         // Set the weight for this connection
         if (isPointDist) {
            tempweight = p1;
         } else if (dType == 'u') {
            tempweight = program::Main().getWeightUniform(p1, p2);
         } else if (dType == 'n') {
            tempweight = program::Main().getWeightNormal(p1, p2);
            while ((tempweight < p3) || (tempweight > p4)) {
               tempweight = program::Main().getWeightNormal(p1, p2);
            }
         }         
         inMatrix[n][NumMade].setWeight(static_cast<float>(tempweight));

         // Get zero weights and sums
         if (tempweight < zeroCutOff) {
            TotalNumberOfZeros++;
            TotalSumOfZeros += tempweight;
         } else {
            TotalSumOfWeights += tempweight;
         }

         ++NumMade;
         unsigned int refTime = minAxonalDelay - 1;
         unsigned int toMake = numSynapsesPerTimeDelay;
         if (refTime-minAxonalDelay+1 < remSynapsesPerTimeDelay)
            ++toMake;
         while (FanOutCon[NeuronIn][refTime] > toMake) {
            ++refTime;
            if (refTime-minAxonalDelay+1 == remSynapsesPerTimeDelay)
               --toMake;
         }
         // The number of fan-out synapses can be more (or less) than the number
         // of fan-in synapses. This causes some unusual results in the
         // distribution of the axonal delays. For all but the last axonal delay
         // (for large enough n) it is fixed fan-out. Not true for the last axonal
         // delay. However, the average value of connections is approximately the
         // same per neuron.
         if (refTime > maxAxonalDelay-1)
            refTime = maxAxonalDelay-1;
         ++FanOutCon[NeuronIn][refTime];  // increment fan out count
      }
   }

   // NOTE: If NeuroJet ever stops being fixed fan-in, MaxConPerNode must be
   //   replaced with the maximum number of per neuron inputs.
   FillFanOutMatrices();
   UIMatrix ConCount(ni, UIVector(maxAxonalDelay, 0));
   SynapseType const* synType = &SynapseType::Member["default"];
   for (unsigned int faninrow = 0; faninrow < ni; faninrow++) {
      DendriticSynapse * dendriticTree = inMatrix[faninrow];
      for (unsigned int col = 0; col < FanInCon[faninrow]; col++) {
         const unsigned int fanoutrow = dendriticTree[col].getSrcNeuron();
         unsigned int refTime = minAxonalDelay-1;
#if defined(CHECK_BOUNDS)
         while ((refTime < maxAxonalDelay) && 
            (ConCount.at(fanoutrow).at(refTime) >= FanOutCon.at(fanoutrow).at(refTime)))
#else
         while (ConCount[fanoutrow][refTime] >= FanOutCon[fanoutrow][refTime])
#endif
            ++refTime;
         unsigned int synapseNum = ConCount[fanoutrow][refTime];
         outMatrix[fanoutrow][refTime][synapseNum].connectSynapse(faninrow, dendriticTree[col], synType);
         ++ConCount[fanoutrow][refTime];
      }
   }

   SystemVar::SetFloatVar("AveWij", static_cast<float>(TotalSumOfWeights /
              (NumNetworkCon - TotalNumberOfZeros)));
   SystemVar::SetFloatVar("AveWij0", static_cast<float>(TotalSumOfWeights + TotalSumOfZeros) /
                 NumNetworkCon);
   SystemVar::SetFloatVar("FracZeroWij", static_cast<float>(TotalNumberOfZeros) /
                     static_cast<float>(NumNetworkCon));

   IFROOTNODE Output::Out() << " done." << std::endl;

   return;
}

//***************
// At Functions
//***************

void AddInterneuron(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "AddInterneuron";
   static StrArg TypeName("-neurontype", "Neuron type that this interneuron feeds into/out of", "default");
   static FlagArg IsFeedback("-feedback", "-feedforward", "Is a feedback interneuron", 1);
   static DblArg ExcDecay("-excdecay", "Decay rate of interneuron excitation", 1.0f);
   static IntArg BuffSize("-axdelay", "Time delay into interneuron", 1);
   static DblArg SynModRate("-lambda", "Synaptic modification rate", 0.0f);
   static DblArg ActAvgRate("-alpha", "Decay rate for synaptic mofidication", 0.0f);
   static DblArg Mult("-mult", "Strength this interneuron has relative to other interneurons", 1.0f);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.StrSet(1, &TypeName);
      ComL.FlagSet(1, &IsFeedback);
      ComL.DblSet(4, &ExcDecay, &SynModRate, &ActAvgRate, &Mult);
      ComL.IntSet(1, &BuffSize);
      ComL.HelpSet("@AddInterneuron() Creates/adds an interneuron to a population.\n");
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (program::Main().getNetworkCreated()) {
      CALL_ERROR << "Error: You must call @AddInterneuron prior to @CreateNetwork()." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   bool typeFound = false;
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      if (pIt->getNeuronType()->getName() == TypeName.getValue()) {
			typeFound = true;
         pIt->addInterneuron(IsFeedback.getValue(), ExcDecay.getValue(), static_cast<unsigned int>(BuffSize.getValue()),
									  SynModRate.getValue(), ActAvgRate.getValue(), Mult.getValue());
		}
	}
   if (!typeFound) {
      Output::Err() << "-neurontype '" << TypeName.getValue() << "' not found" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
}

void CreateNeuronType(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "CreateNeuronType";
   static StrArg TypeName("-name", "Name to give the neuron type");
   static StrArg ExcInhType("-excinhtype", "Type of excitation or inhibition\n"
                                           "\t\t\t Choices are exc, inhdiv, or inhsub", "exc");
   static StrArgList Params("-params", "Parameters for McP, LIF, or Izh neurons\n"
                                       "\t\t\t See NeuroJet.net Wiki for more information", true);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.StrSet(2, &TypeName, &ExcInhType);
      ComL.StrListSet(1, &Params);
      ComL.HelpSet("@CreateNeuronType() Creates a neuron type that will\n"
                    " be used during CreateNetwork.\n");
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   const string myName = TypeName.getValue();
   bool isExc = (ExcInhType.getValue() == "exc");
   bool isInhDiv = (ExcInhType.getValue() == "inhdiv");
   if (!isExc && !isInhDiv && (ExcInhType.getValue() != "inhsub")) {
      Output::Err() << "-excinhtype must be set to one of [exc, inhdiv, or inhsub]" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   NeuronType::addMember(myName, isExc, isInhDiv);
   if (Params.size() % 2) { // We got an odd # of arguments
      Output::Err() << "Odd number of arguments to Params. "
            "Number of arguments must be even as they are given as pair"
            " of parameter, value." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   NeuronType* curMember = &NeuronType::Member[myName];
   // If checking for size - 1, don't use unsigned int!
   for (unsigned int i = 0; i < Params.size(); i+=2) {
      curMember->setParameter(Params[i], Params[i+1]);
      const string varName = Params[i];
      if (varName == "DendriteToSomaFilter") {
         DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(Params[i+1],
                                       FunctionName, CommandLine(FunctionName));
         if (Matrix.size() > 1) Matrix = transposeMatrix(Matrix);
         DataList filterVals = Matrix.front();
         curMember->loadDTSFilterValues(filterVals);
      } else if (varName == "IzhType") {
         vector<float> result = assignIzhParams(ucase(Params[i+1]));
         if (!curMember->hasParameter("IzhA"))
            curMember->setParameter("IzhA", to_string(result[0]));
         if (!curMember->hasParameter("IzhB"))
            curMember->setParameter("IzhB", to_string(result[1]));
         if (!curMember->hasParameter("IzhC"))
            curMember->setParameter("IzhC", to_string(result[2]));
         if (!curMember->hasParameter("IzhD"))
            curMember->setParameter("IzhD", to_string(result[3]));
         if (!curMember->hasParameter("IzhE"))
            curMember->setParameter("IzhE", to_string(result[4]));
         if (!curMember->hasParameter("IzhF"))
            curMember->setParameter("IzhF", to_string(result[5]));
         if (!curMember->hasParameter("IzhVMax"))
            curMember->setParameter("IzhVMax", to_string(result[6]));
         if (!curMember->hasParameter("IzhvStart"))
            curMember->setParameter("IzhvStart", to_string(result[7]));
         if (!curMember->hasParameter("IzhuStart"))
            curMember->setParameter("IzhuStart", to_string(result[8]));
      }
   }
   // set ResetAct as default to Activity   
   if (fabs(curMember->getParameter("ResetAct", SystemVar::GetFloatVar("ResetAct")) + 1.0) < verySmallFloat) {
      curMember->setParameter("ResetAct",
                  to_string(curMember->getParameter("Activity", SystemVar::GetFloatVar("Activity"))));
   }
}

void CreateNetwork (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "CreateNetwork";
   // AllowSelf defaults to -noself(0)
   static FlagArg AllowSelf("-self", "-noself", "Allows self connections to exist", 0);
   static StrArg DistType("-dist",
                           "Generates weights from the given distribution"
                           "\n\t\t\t {point,uniform,normal}", "point");
   static DblArg MeanVal ("-mean",
                          "Initial value for point weights and"
                          "\n\t\t\t mean for normal weights {-1 defaults to wStart}",
                          -1.0);
   static DblArg LowVal ("-low", "Lower bound for weights", 0.0L);
   static DblArg HighVal ("-high", "Upper bound for weights", 1.0L);
   static DblArg StdVal ("-std", "Standard deviation for normal weights", 0.125L);
   static StrArg WeightFile("-connfrom",
                           "File to read connectivity information from "
                           "(overrides ReadWeights)\n",
                           "{none}");
   static IntArg MinDelay ("-mindelay", "Minimum number of timesteps to the"
                           " nearest synapse in an axon", 1);
   static IntArg MaxDelay ("-maxdelay", "Maximum number of timesteps to the"
                           " nearest synapse in an axon", 1);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      program::Main().setNetworkCreated(true);
      ComL.FlagSet(1, &AllowSelf);
      ComL.StrSet(2, &DistType, &WeightFile);
      ComL.DblSet(4, &MeanVal, &LowVal, &HighVal, &StdVal);
      ComL.IntSet(2, &MinDelay, &MaxDelay);
      ComL.HelpSet("@CreateNetwork() Allocates Memory for the network\n"
                    " and seeds the Random Number Generator.\n"
                    " Parameters describe the weight distributions.\n"
                    "@CreateNetwork() must be called before the first training or testing.\n");
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   // Seed the random number generators
   ArgListType NoArgs(0);
   SeedRNG(NoArgs);

   // Deallocate memory
   DeAllocateMemory();

   // set size variables to their current user values (used in AllocateMemory!)
   unsigned int oldni = ni;
   ni = SystemVar::GetIntVar("ni");
   for (PopulationIt pIt = Population::Member.begin();
        pIt != Population::Member.end(); ++pIt) {
      if ((pIt->getFirstNeuron() == 0) && (pIt->getLastNeuron() == oldni-1)) {
         pIt->setNeuronRange(0, ni-1);
      }
   }
   minAxonalDelay = MinDelay.getValue();
   maxAxonalDelay = MaxDelay.getValue();
   if (minAxonalDelay < 1) {
      CALL_ERROR << "Minimal axonal delay must be 1 or greater" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (maxAxonalDelay < minAxonalDelay) {
      CALL_ERROR << "Maximal axonal delay must be equal to or greater than"
                    " the minimum axonal delay" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   // set ResetAct as default to Activity   
   if (fabs(SystemVar::GetFloatVar("ResetAct") + 1.0) < verySmallFloat) {
      SystemVar::SetFloatVar("ResetAct", SystemVar::GetFloatVar("Activity"));
   }
   // set dominance of forced xNoise
   if (fabs(SystemVar::GetFloatVar("xNoiseF")) > verySmallFloat) {
      SystemVar::SetFloatVar("xNoise", 0.0f);
   }
   // Call function to allocate memory
   AllocateMemory();
#if defined(MULTIPROC)
   program::Main().buildShuffleVectors(Shuffle, UnShuffle, ni);
#endif

   InitializeVariables();
#if defined(MULTIPROC)
   ParallelInfo::AllocateArrays(ni);
   ParallelInfo::Barrier();
#endif

   CheckIzhikevich();  // Verify that variables are set somewhat consistently

   if (WeightFile.getValue() != "{none}")
      SystemVar::SetStrVar("ReadWeights", WeightFile.getValue());
   
   // Either read weights from file or set them up from parameters
   if (SystemVar::GetStrVar("ReadWeights") != "") {
      IFROOTNODE {
         Output::Out() << "Using Weight File: " << SystemVar::GetStrVar("ReadWeights") << std::endl;
      }
      const string filename = SystemVar::GetStrVar("ReadWeights");
      if (isNJNetworkFileType(filename)) {
         // Katharina Dobs' program is NJNetwork
         ReadNJNetworkFile(filename);
      } else {
         GetConnectivity(filename);
      }
   } else {
      if (fabs(MeanVal.getValue() + 1.0) < verySmallFloat) {
         MeanVal.setValue(SystemVar::GetFloatVar("wStart"));
      }
      if ((MeanVal.getValue() < 0.0f) || (MeanVal.getValue() > 1.0)) {
         CALL_ERROR << "Invalid mean: " << MeanVal.getValue() << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      if ((LowVal.getValue() < 0.0f) || (HighVal.getValue() > 1.0) || (LowVal.getValue() > HighVal.getValue())) {
         CALL_ERROR << "Invalid weight range: " << LowVal.getValue()
            << " ... " << HighVal.getValue() << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      if (DistType.getValue() == "point") {
         IFROOTNODE Output::Out() << "All weights will be: " << MeanVal.getValue() << "\n";
         SetConnectivity(AllowSelf.getValue(), 'p', static_cast<float>(MeanVal.getValue()));

      } else if (DistType.getValue() == "uniform") {
         IFROOTNODE
            Output::Out() << "Weights will be uniformly distributed on: [ "
            << LowVal.getValue() << " , " << HighVal.getValue() << " ]\n";
         SetConnectivity(AllowSelf.getValue(), 'u', static_cast<float>(LowVal.getValue()),
          static_cast<float>(HighVal.getValue()));

      } else if (DistType.getValue() == "normal") {
         IFROOTNODE
            Output::Out() << "Weights will be normally distributed on: [ "
            << LowVal.getValue() << " , " << HighVal.getValue() << " ]\n\t"
            << "with mean: " << MeanVal.getValue() << " and std. dev.: " << StdVal.getValue() << "\n";
         SetConnectivity(AllowSelf.getValue(), 'n', static_cast<float>(MeanVal.getValue()),
          static_cast<float>(StdVal.getValue()), static_cast<float>(LowVal.getValue()),
             static_cast<float>(HighVal.getValue()));
      } else {
         CALL_ERROR << "Unknown -dist parameter: " << DistType.getValue() << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
   }

   // Set some variables
   SystemVar::SetFloatVar("FracConnect", static_cast<float>(NumNetworkCon) /
                     (static_cast<float>(ni * ni)));
   SystemVar::SetIntVar("TrainingCount", 0);
   SystemVar::SetFloatVar("AveTrainAct", 0.0f);
   SystemVar::SetFloatVar("AveTrainTies", 0.0f);
   SystemVar::SetFloatVar("AveTestAct", 0.0f);
   SystemVar::SetFloatVar("AveTrainExt", 0.0f);
   SystemVar::SetFloatVar("AveTestExt", 0.0f);
   SystemVar::SetFloatVar("AveTrainInt", 0.0f);
   SystemVar::SetFloatVar("AveTestInt", 0.0f);
   SystemVar::SetFloatVar("AveConLen", 0.0f);
   SystemVar::SetFloatVar("AveConLen0", 0.0f);
   SystemVar::SetFloatVar("FracUnused", 0.0f);
   SystemVar::SetFloatVar("FracRefired", 0.0f);
   SystemVar::SetIntVar("NumUnused", 0);
   SystemVar::SetIntVar("NumRefired", 0);
   SystemVar::SetFloatVar("VarConLen", 0.0f);
   SystemVar::SetFloatVar("VarConLen0", 0.0f);

   TotalNumTied = 0;

   ResetSTM(); // Sets timeStep = 0

   return;
}

void CreateSynapseType(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "CreateSynapseType";
   static StrArg TypeName("-name", "Name to give the synapse type");
   static StrArg PreType("-pre", "Neuron type of pre-synaptic neuron");
   static StrArg PostType("-post", "Neuron type of post-synaptic neuron");
   static StrArgList Params("-params", "Synaptic parameters\n"
                                       "\t\t\t See NeuroJet.net Wiki for more information", true);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.StrSet(3, &TypeName, &PreType, &PostType);
      ComL.StrListSet(1, &Params);
      ComL.HelpSet("@CreateSynapseType() Creates a synapse type that will\n"
                    " be used during CreateNetwork.\n");
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   const string myName = TypeName.getValue();
   const string preType = PreType.getValue();
   const string postType = PostType.getValue();
   if (Params.size() % 2) { // We got an odd # of arguments
      Output::Err() << "Odd number of arguments to Params. "
            "Number of arguments must be even as they are given as pair"
            " of parameter, value." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   LearningRuleType learningRule = LRT_PostSyn;
   if (SystemVar::GetIntVar("UseMvgAvg")) learningRule = LRT_MvgAvg;
   if (SystemVar::GetIntVar("UseAltSynapse")) learningRule = LRT_PostSynB;
   float mu = SystemVar::GetFloatVar("mu");
   unsigned int NMDArise = static_cast<unsigned int>(SystemVar::GetIntVar("NMDArise"));
   float synFailRate = SystemVar::GetFloatVar("synFailRate");
   float alpha = SystemVar::GetFloatVar("alpha");
   float resolution = SystemVar::GetFloatVar("resolution");
   float inv = SystemVar::GetFloatVar("inv");
   float stochastic = SystemVar::GetIntVar("stochastic");
   float Ksyn = 1.0f;
   for (unsigned int i = 0; i < Params.size(); i+=2) {
      const string varName = Params[i];
      const string varV = Params[i+1];
      if (varName == "mu") mu = from_string<float>(varV);
      if (varName == "NMDArise") NMDArise = from_string<unsigned int>(varV);
      if (varName == "synFailRate") synFailRate = from_string<float>(varV);
      if (varName == "alpha") alpha = from_string<float>(varV);
      if (varName == "resolution") resolution = from_string<float>(varV);
      if (varName == "inv") inv = from_string<float>(varV);
      if (varName == "stochastic") stochastic = from_string<int>(varV);
      if (varName == "Ksyn") Ksyn = from_string<float>(varV);
      if ((varName == "UseMvgAvg") && from_string<int>(varV)) {
         learningRule = LRT_MvgAvg;
      } else if ((varName == "UseAltSynapse") && from_string<int>(varV)) {
         learningRule = LRT_PostSynB;
		}
   }
   SynapseType::addMember(myName, learningRule, mu, NMDArise, alpha, Ksyn, synFailRate, preType, postType, resolution, inv, stochastic);
}

void CreateVar(ArgListType &arg) //AT_FUN
{
   if (arg.size() < 1) {
      CALL_ERROR << "Error in CreateVar: expects at least one argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "CreateVar -help\n\n"
         "@CreateVar( ... ) takes a list of one or more variables,\n"
         "each followed by its new value, and creates the variables.\n"
         "Unlike most functions, CreateVar does not require any preceeding flags.\n"
         << std::endl;
      exit(EXIT_FAILURE);
   }
   if (arg.size() % 2 != 0) {
      CALL_ERROR <<
         "Error in CreateVar: You do not have and even number of arguments." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   for (ArgListTypeIt it = arg.begin(); it!= arg.end(); /* do nothing */) {
      string VarName = (it++)->first;
      string VarValue = (it++)->first;
      if (SystemVar::GetVarType(VarName) != 'u') {
         CALL_ERROR << "Error in CreateVar: " << VarName <<
            " already exists as a variable." << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      SystemVar::AddStrVar(VarName, VarValue);
   }

   return;
}

void CopyData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "CopyData";
   static int argunset = true;
   static IntArg StartPat("-Pstart", "start pattern(or vector)", 1);
   static IntArg EndPat("-Pend", "end pattern(or vector) {-1 gives end time}", -1);
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg EndNeuron("-Nend", "end neuron{-1 gives last neuron}", -1);
   static StrArg DataName("-to", "destination name");
   static StrArg DataType("-type", "destination data type {seq,mat,ana,file}", "seq");
   static StrArg OldMatName("-from", "data structure to be copied from");
   // Transpose defaults to -noT (0)
   static FlagArg Transpose("-T", "-noT", "transpose data", 0);
   // DoPad defaults to -nopad (0)
   static FlagArg DoPad ("-pad", "-nopad", "pad data with -pval", 0);
   static DblArg PadVal("-pval", "value to pad with for transpose or for -pad", 0.0f);
   static IntArg Width ("-width", "spacing for file output", 2);
   // LeftJust defaults to -right(0)
   static FlagArg LeftJust("-left", "-right", "make the file output left justified",
                            0);
   static FlagArg Noisy("-noisy", "-quiet",
                             "display information", true);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@CopyData( ... ) copies and converts one data structure to another.\n"
          "If you transpose or switch data types then CopyData automatically\n"
          "pads. The destination data structure cannot also be the copied\n"
          "data structure... You must copy to a temporary first.\n");
      ComL.IntSet(5, &StartPat, &EndPat, &StartNeuron, &EndNeuron, &Width);
      ComL.StrSet(3, &DataName, &DataType, &OldMatName);
      ComL.FlagSet(4, &Transpose, &DoPad, &LeftJust, &Noisy);
      ComL.DblSet(1, &PadVal);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   // Get the old data
   DataMatrix OldMat;
   const string dataName = OldMatName.getValue();
   int OldMatSize;
   int IsOldSeq = false;   
   const char oldVarType = isFn(dataName) ? 'f' : SystemVar::GetVarType(dataName);
   string oldType; // for messages
   if (oldVarType == 'S') {
      OldMat = UISeqToMatrix(SystemVar::getSequence(OldMatName, FunctionName, ComL));
      IsOldSeq = true;
      oldType = "sequence";
   } else if (oldVarType == 'M') {
      OldMat = SystemVar::getMatrix(OldMatName, FunctionName, ComL);
      oldType = "matrix";
   } else if (oldVarType == 'A') {
      OldMat = SystemVar::getAnalysis(OldMatName, FunctionName, ComL);
      oldType = "analysis";
   } else if (oldVarType == 'f') {
      OldMat.push_back(CalcFn(dataName));
      oldType = "function";
   } else {
      CALL_ERROR << "Error in " << FunctionName << " : Sequence "
                 << OldMatName.getValue() << " not found" << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   OldMatSize = OldMat.size();
   int startT, endT, startN, endN;
   setMatrixRange(startT, endT, startN, endN, StartPat, EndPat,
                  StartNeuron, EndNeuron, OldMat, ComL, FunctionName);

   DataListType newDataType;
   string newType; // for messages
   string newSubType; // for messages
   readDataType(DataType, newDataType, newType, newSubType, FunctionName, ComL, true);

   string nrnRange = "neurons(" + to_string(startN) + "..." + to_string(endN) + ")";
   string ptnRange = "patterns(" + to_string(startT) + "..." + to_string(endT) + ")";
   if (Noisy.getValue()) {
      Output::Out() << "Copied ";
   }

   // set up memory appropriately and copy
   float **TempMat;

   TempMat = Convert2Mat(OldMat, startT, endT, startN, endN, Transpose.getValue(), static_cast<float>(PadVal.getValue()));
   if (Noisy.getValue()) {
      if (Transpose.getValue()) {
         Output::Out() << "(transposed, pad = " << PadVal.getValue() << ") ";
      } else {
         Output::Out() << "(pad = " << PadVal.getValue() << ") ";
      }
      Output::Out() << oldType << " " << OldMatName.getValue() << " to ";
   }

   int NumPats;
   if (Transpose.getValue())
      NumPats = endN - startN + 1;
   else
      NumPats = endT - startT + 1;

   // Do Files First
   if (newDataType == DLT_file) {
      ofstream fout;
      fout.open(DataName.getValue().c_str());
      if (!fout) {
         CALL_ERROR << "Error in " << FunctionName << " unable to open file "
                    << DataName.getValue() << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      // Check for justification
      if (LeftJust.getValue()) {
         fout.setf(ios::left);
      }
      if (Transpose.getValue()) {
         StartPat.swapValue(StartNeuron);
         EndPat.swapValue(EndNeuron);
      }
      int  pNdx = 0;
      for (int i = StartPat.getValue(); i <= EndPat.getValue(); i++) {
         int  nNdx = 0;
         for (int j = StartNeuron.getValue(); j <= EndNeuron.getValue(); j++) {
            fout << std::setw(Width.getValue()) << TempMat[pNdx][nNdx];
            if (!IsOldSeq) fout << " ";
            nNdx++;
         }
         fout << "\n";
         pNdx++;
      }
      if (Noisy.getValue()) {
         Output::Out() << "file " << DataName.getValue() << "\n"
                 << "\tusing " << ptnRange << " and " << nrnRange << std::endl;
      }
      fout.close(); /* Fred Howell closed this file */
      delMatrix(TempMat, NumPats);
      return;
   }
   // Now do other copying

   bool foundData = chkDataExists(DataName, newDataType, FunctionName, ComL);
   if (Noisy.getValue()) {
      Output::Out() << (foundData ? "replace " : "create ") << newType << " "
              << DataName.getValue() << "\n"
              << "\tusing " << ptnRange << " and " << nrnRange << std::endl;
   }

   if (Transpose.getValue()) {
      startN = startT;
      endN = endT;
   }

   // NumBits relies on transpose swap already happening, if appropriate
   int  NumBits = endN - startN + 1;
   DataMatrix NewDataMatrix;
   for (int i = 0; i < NumPats; i++) {
      // The iterator constructor appears to want start to end+1, not
      // start to end. Gotta love it.
      DataList temp_dl(&TempMat[i][0],&TempMat[i][NumBits]);
      NewDataMatrix.push_back(temp_dl);
   }
   SystemVar::insertData(DataName, NewDataMatrix, newDataType);
   delMatrix(TempMat, NumPats);

   return;
}

void DeleteData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "DeleteData";
   if (arg.size() == 0) {
      CALL_ERROR << "Error in : " << FunctionName << " : expects at least one "
                    "argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "DeleteData -help\n\n@DeleteData( data1 ... dataN ) removes a "
                 "list of data structures from memory." << std::endl << std::endl;
      exit(EXIT_FAILURE);
   }

   for (ArgListTypeIt it = arg.begin(); it != arg.end(); it++) {
      const string toRemove = it->first;
      SystemVar::deleteData(toRemove);
   }

   return;
}

void LoadData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "LoadData";
   static int argunset = 1;
   static StrArg DataName("-to", "destination data name");
   static StrArg DataType("-type", "destination data type {seq,mat,ana}", "seq");
   static StrArg FileName("-from", "file name");
   static IntArg LineSize("-buf", "maximum buffer size",
                          iround(2.5 * SystemVar::GetIntVar("ni")));
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@LoadData( ... ) loads a file of numbers into memory.\n");
      ComL.StrSet(3, &DataName, &DataType, &FileName);
      ComL.IntSet(1, &LineSize);
      argunset = 0;
   }
   ComL.Process(arg, Output::Err());

   DataListType newDataType;
   string newType; // for messages
   string newSubType; // for messages
   readDataType(DataType, newDataType, newType, newSubType, FunctionName, ComL);

   if (LineSize.getValue() < 1) {
      CALL_ERROR << "Error in " << FunctionName << " : Invalid buffer size : "
         << LineSize.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   ifstream inFile(FileName.getValue().c_str(), ios::in);
   if (!inFile) {
      CALL_ERROR << "Error in " << FunctionName << " : Cannot open "
                 << FileName.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   // set up memory appropriately
   bool foundData = chkDataExists(DataName, newDataType, FunctionName, ComL);
   DataMatrix NewInMatrix; 
   string ThisFloat = "";

   // read in data
   int  PatternCount = 0;
   string lineBuf;
   while (std::getline(inFile, lineBuf)) {      
      int  TotalExtracted = lineBuf.size();
      if (TotalExtracted >= LineSize.getValue() - 1) {
         CALL_ERROR << "Warning in " << FunctionName
                    << " : maximum buffersize of " << LineSize.getValue()
                    << " may be too small for file " << FileName.getValue()
                    << ERR_WHERE;
      }
      // Load the line into the array buffers, counting the number of elements to set.
      DataList LineData;
      int  i = 0;
      if (newDataType == DLT_sequence) {
         while ((i < TotalExtracted) && (lineBuf[i] != '\0')) {
            if (isspace(lineBuf[i])) {
               ++i;
               continue;
            }
            if (lineBuf[i] == '1') {
               LineData.push_back(1.0f);
            } else if (lineBuf[i] == '0') {
               LineData.push_back(0.0f);
            } else {
               CALL_ERROR << "Error in " << FunctionName
                          << " : Unexpected character '"
                          << lineBuf[i] << "' in file " << FileName.getValue()
                          << ERR_WHERE;
               ComL.DisplayHelp(Output::Err());
               exit(EXIT_FAILURE);
            }
            ++i;
         }
      } else {
         while ((i < TotalExtracted) && (lineBuf[i] != '\0')) {
            if (isspace(lineBuf[i])) {
               ++i;
               continue;
            }
            ThisFloat = "";
            while ((i < TotalExtracted) && !isspace(lineBuf[i])) {
               ThisFloat += lineBuf[i];
               i++;
            }
            LineData.push_back(from_string<float>(ThisFloat));
         }
      }
      if (LineData.size() > 0) { // If the line was not empty
         PatternCount++;
         NewInMatrix.push_back(LineData);
      }
   }

   Output::Out() << (foundData ? "Replaced " : "Created ") << newType << " "
        << DataName.getValue() << " with " << PatternCount << " "
        << newSubType << " " << "from file " << FileName.getValue() << std::endl;
   // Add to the list
   SystemVar::insertData(DataName, NewInMatrix, newDataType);

   return;
}

void MakeRandSequence(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "MakeRandSequence";
   static int argunset = true;
   static StrArg SeqName("-name", "new sequence name");
   static IntArg SeqLen("-len", "sequence length");
   static DblArg ProbFire("-p", "probability of firing");
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg EndNeuron("-Nend", "end neuron {-1 defaults to ni}", -1);
   static IntArg StartPat("-Pstart", "start pattern", 1);
   static IntArg EndPat("-Pend", "end pattern {-1 defaults to -len}", -1);
   // ForceProb defaults to -pForce(true)
   static FlagArg ForceProb ("-pForce", "-nopForce",
                             "forces global probability of firing", true);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@MakeRandSequence( ... ) creates a sequence with sections"
                    " of random firing.\n");
      ComL.StrSet(1, &SeqName);
      ComL.IntSet(5, &SeqLen, &StartNeuron, &EndNeuron, &StartPat, &EndPat);
      ComL.DblSet(1, &ProbFire);
      ComL.FlagSet(1, &ForceProb);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (EndNeuron.getValue() == -1) {
      EndNeuron.setValue(SystemVar::GetIntVar("ni"));
   }
   if (EndPat.getValue() == -1) {
      EndPat.setValue(SeqLen.getValue());
   }
   if (StartNeuron.getValue() < 1 || StartNeuron.getValue() > EndNeuron.getValue()) {
      CALL_ERROR << "Error in " << FunctionName << " : Neuron range invalid : "
                 << StartNeuron.getValue() << " , " << EndNeuron.getValue()
                 << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (StartPat.getValue() < 1 || EndPat.getValue() > SeqLen.getValue()
       || StartPat.getValue() > EndPat.getValue()) {
      CALL_ERROR << "Error in " << FunctionName << " : Pattern range invalid : "
                 << StartPat.getValue() << " , " << EndPat.getValue()
                 << " with -len = " << SeqLen.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (ProbFire.getValue() < 0.0f || ProbFire.getValue() > 1.0) {
      CALL_ERROR << "Error in " << FunctionName
         << " : Probability of firing invalid : " << ProbFire.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   int  NumNeurons = EndNeuron.getValue() - StartNeuron.getValue() + 1;
   int  Num2Fire = iround(NumNeurons * ProbFire.getValue());
   if ((Num2Fire == NumNeurons) && ForceProb.getValue()) {
      ForceProb.setValue(false);
      ProbFire.setValue(1.0);
   }
   if ((Num2Fire == 0) && ForceProb.getValue()) {
      ForceProb.setValue(false);
      ProbFire.setValue(0.0f);
   }
   if (ForceProb.getValue()) {
      ProbFire.setValue(static_cast<float>(Num2Fire)/static_cast<float>(NumNeurons));
   }
   const double fireProb = static_cast<double>(ProbFire.getValue());
   if (fireProb < verySmallFloat) {
      EndNeuron.setValue(0);
   }

   UIPtnSequence NewSequence;
   int  PatternCount = 1;
   for (int i = 0; i < SeqLen.getValue(); i++) {
      UIVector pat(0);
      if (PatternCount >= StartPat.getValue() && PatternCount <= EndPat.getValue()) {
         if (ForceProb.getValue()) {
            for (int j = 0; j < Num2Fire; j++) {
               while (true) {
                  // StartNeuron and EndNeuron are 1-based, ndx is 0-based
                  unsigned int ndx = static_cast<unsigned int>(program::Main().getExtRandInt(StartNeuron.getValue(), EndNeuron.getValue()))-1;
                  if (pat.size() == 0) {
                     pat.push_back(ndx);
                     break;
                  } else {
                     bool found = false;
                     UIVectorIt pIt; // Need this outside the for loop
                     for (pIt = pat.begin(); pIt != pat.end(); ++pIt) {
                        if (*pIt == ndx) {found = true; break;}
                        // insert in order
                        if (*pIt > ndx) break;
                     }
                     if (!found) {
                        pat.insert(pIt, ndx);
                        break;
                     }
                  }
               }
            }
         } else {
            // StartNeuron and EndNeuron are 1-based, ndx is 0-based
            for (int j = StartNeuron.getValue()-1; j < EndNeuron.getValue(); j++) {
               if (program::Main().getExtBernoulli(fireProb))
                  pat.push_back(j);
            }
         }
      }
      NewSequence.push_back(pat);
      ++PatternCount;
   }

   // Check to see if new exists, if so, erase it
   const char dataType = SystemVar::GetVarType(SeqName);
   if (dataType != 'S' && dataType != 'u') {
      CALL_ERROR << "Error in " << FunctionName << " : " << SeqName.getValue()
         << " already exists as another data structure." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   IFROOTNODE {
      Output::Out() << (dataType == 'S' ? "Replaced" : "Created") << " Sequence " 
              << SeqName.getValue() << " with a random sequence\n"
       "\tLength = " << SeqLen.getValue() << ", Rate = " << ProbFire.getValue()
      << ", Patterns = " << StartPat.getValue() << ".." << EndPat.getValue()
      << ", Neurons = " << StartNeuron.getValue() << ".."
      << EndNeuron.getValue() << std::endl;
   }
   SystemVar::insertSequence(SeqName, NewSequence);

   return;
}

void MakeSequence(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "MakeSequence";
   static int argunset = true;
   static StrArg SeqName("-name", "new sequence name");
   static IntArg SeqLen("-len", "sequence length");
   static IntArg NumOn("-non", "number on in each pattern");
   static IntArg Overlap("-ol", "overlap between successive patterns", 0);
   static IntArg Stutter("-st", "stutter(does not affect sequence length)", 1);
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg StartPat("-Pstart", "start pattern", 1);
   static IntArg EndPat("-Pend", "end pattern {-1 defaults to -len}", -1);
   static DblArg ProbFire("-p", "probability of firing", 1.0);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@MakeSequence( ... ) Makes a simple sequence.\n");
      ComL.StrSet(1, &SeqName);
      ComL.IntSet(7, &SeqLen, &NumOn, &Overlap, &Stutter, &StartNeuron,
                   &StartPat, &EndPat);
      ComL.DblSet(1, &ProbFire);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (EndPat.getValue() == -1) {
      EndPat.setValue(SeqLen.getValue());
   }
   if (NumOn.getValue() < Overlap.getValue()) {
      CALL_ERROR << "Error in " << FunctionName << " : Invalid overlap : "
         << "NumOn must be greater than Overlap." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (StartPat.getValue() < 1 || EndPat.getValue() > SeqLen.getValue() || StartPat.getValue() > EndPat.getValue()) {
      CALL_ERROR << "Error in " << FunctionName << " : Invalid pattern range : "
         << StartPat.getValue() << " , " << EndPat.getValue() << " with -len of " << SeqLen.getValue()
         << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (StartNeuron.getValue() < 1) {
      CALL_ERROR << "Error in " << FunctionName << " : Invalid start neuron : "
         << StartNeuron.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   const double fireProb = static_cast<double>(ProbFire.getValue());
   if (fireProb < 0.0L || fireProb > 1.0L) {
      CALL_ERROR << "Error in " << FunctionName <<
         " : Invalid probability of firing : " << fireProb << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   UIPtnSequence NewSequence;
   int  PatternCount = 1;
   int  OnCount = 0;
   int  LastNeuron = 0;
   for (int i = 0; i < SeqLen.getValue(); i++) {
      UIVector pat(0);
      if (PatternCount >= StartPat.getValue() && PatternCount <= EndPat.getValue()) {
         // StartNeuron is 1-based, start and end are 0-based
         int  start = StartNeuron.getValue() + (OnCount / Stutter.getValue()) *
                      (NumOn.getValue() - Overlap.getValue()) - 1;
         int  end = start + NumOn.getValue();
         for (int j = start; j < end; ++j) {
            if ((abs(fireProb - 1.0L) < verySmallFloat)
                  || program::Main().getExtBernoulli(fireProb)) {
               pat.push_back(j);
               updateMax(LastNeuron, j);
            }
         }
         ++OnCount;
      }
      NewSequence.push_back(pat);
      ++PatternCount;
   }

   // Check to see if new exists, if so, erase it
   const char dataType = SystemVar::GetVarType(SeqName);
   if (dataType != 'S' && dataType != 'u') {
      CALL_ERROR << "Error in " << FunctionName << " : " << SeqName.getValue()
         << " already exists as another data structure." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   if (NumOn.getValue() == 0) {
      StartNeuron.setValue(0);
   }

   IFROOTNODE
      Output::Out() << (dataType == 'S' ? "Replacing" : "Creating") << " sequence '"
        << SeqName.getValue() << "' with a simple sequence:\n"
        "\tLength = " << SeqLen.getValue() << ", FireRate = " << fireProb
        << ", Patterns = " << StartPat.getValue() << ".." << EndPat.getValue()
        << ", Neurons = " << StartNeuron.getValue() << ".." << (LastNeuron+1)
        << ",\n\tNumber On = " << NumOn.getValue() << ", Overlap = "
        << Overlap.getValue() << ", Stutter = " << Stutter.getValue()
        << std::endl;
   SystemVar::insertSequence(SeqName, NewSequence);

   return;
}

void AppendData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "AppendData";
   static int argunset = true;
   static StrArg DataName("-to", "destination name");
   static StrArg Type("-type", "destination data type {seq,mat,ana}", "seq");
   static StrArgList SeqList("-from",
                              "# in list and list of data structures to be appended");
   static FlagArg Noisy("-noisy", "-quiet",
                             "display information", true);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@AppendData( ... ) combines a list of data structures sequentially\n"
          "into one. The destination data cannot be one of the appended\n"
          "data structures... You must copy to a temporary first.\n");
      ComL.StrListSet(1, &SeqList);
      ComL.StrSet(2, &DataName, &Type);
      ComL.FlagSet(1, &Noisy);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (SeqList.size() < 2) {
      CALL_ERROR << "Error in " << FunctionName
         << " : Must supply at least 2 data structures" << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   DataListType newDataType;
   string newType; // for messages
   string newSubType; // for messages
   readDataType(Type, newDataType, newType, newSubType, FunctionName, ComL);

   if (Noisy.getValue()) {
      IFROOTNODE Output::Out() << "Appending Data Structures: ";
   }
   bool foundData = chkDataExists(DataName, newDataType, FunctionName, ComL);
   int  PatternCount = 0;
   
   DataMatrix NewInMatrix;
   for (unsigned int i = 0; i < SeqList.size(); i++) {
      DataMatrix temp_mat = SystemVar::getData(SeqList[i], FunctionName, ComL);      
      for (DataMatrixCIt it = temp_mat.begin(); it != temp_mat.end(); it++) {
         PatternCount++;
         NewInMatrix.push_back(*it);
      }
      if (Noisy.getValue()) {
         IFROOTNODE Output::Out() << SeqList[i] << " ";
      }
   }
   SystemVar::insertData(DataName, NewInMatrix, newDataType);

   if (Noisy.getValue()) {
      IFROOTNODE {
         Output::Out() << "\n\t to " << (foundData ? "replace " : "create ")
                       << DataName.getValue() << " with " << PatternCount
                       << " " << newSubType << ".\n";
      }
   }

   return;
}

void CombineData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "CombineData";
   static int argunset = true;
   static StrArg DataName("-to", "destination name");
   static StrArgList SeqList("-from", "# in list and list of data structures "
                              "to be combined");
   static StrArg Type("-type", "destination data type {seq,mat,ana}", "seq");
   static StrArg Method ("-method", "method of combining", "|");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@CombineData( ... ) takes a list of data structures and combines "
          "them together\n" "element by element into a new data structure.\n"
          "The method of combining can be:\n"
          "|, ~|, &, +, -, *, /, '^',>,<,>=,<=,~=,max,min,mean\n"
          "where these operators have the same meaning as in ^Calc.\n"
          "The process of combining is to recursively combine the\n"
          "first two data structures into one and then to repeat the process.\n"
          "The data structures are combined as floating point numbers\n"
          "and then converted to the destination type. The destination\n"
          "data cannot be one of the appended data structures.\n"
          "You must copy to a temporary first.\n"
          "(The same data structure cannot be listed more than once\n"
          "in -from. You must copy each instance to a temporary.)\n");
      ComL.StrListSet(1, &SeqList);
      ComL.StrSet(3, &DataName, &Type, &Method);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (SeqList.size() < 2) {
      CALL_ERROR << "Error in " << FunctionName
         << " : Must supply at least 2 data structures" << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   IFROOTNODE Output::Out() << "Combining Data (" << Method.getValue() << ") ... " << std::flush;

   // doesn't have to be time, but this is a good way to think of it
   unsigned int maxTimeStep = 0;
   // doesn't have to be neurons, but this is a good way to think of it
   unsigned int maxNeuron = 0;
   for (unsigned int i = 0; i < SeqList.size(); i++) {
      DataMatrix tmp = SystemVar::getData(SeqList[i], FunctionName, ComL);
      updateMax(maxTimeStep, static_cast<unsigned int>(tmp.size()));
      maxNeuron = findMaxSize(tmp, maxNeuron);
   }

   DataMatrix BuildMat(maxTimeStep, DataList(maxNeuron));
   DataMatrix inputData;
   // Copy in the first pattern, necessary for AND, ==, etc.
   IFROOTNODE Output::Out() << SeqList[0] << " " << std::flush;
   inputData = SystemVar::getData(SeqList[0], FunctionName, ComL);
   DataMatrixIt BuildIt = BuildMat.begin();
   DataMatrixCIt DMCIt;
   for (DMCIt = inputData.begin(); DMCIt != inputData.end(); ++DMCIt, ++BuildIt) {
      for (unsigned int nrn = 0; nrn < DMCIt->size(); nrn++) {
         BuildIt->at(nrn) = DMCIt->at(nrn);
      }
   }

   string combineMethod = Method.getValue();
   for (unsigned int ptn = 1; ptn < SeqList.size(); ptn++) {
      IFROOTNODE Output::Out() << SeqList[ptn] << " " << std::flush;
      inputData = SystemVar::getData(SeqList[ptn], FunctionName, ComL);
     
      DMCIt = inputData.begin();
      for (BuildIt = BuildMat.begin(); BuildIt != BuildMat.end(); ++BuildIt) {
         for (unsigned int nrn = 0; nrn < maxNeuron; nrn++) {
            float newbit = BuildIt->at(nrn);
            float nextbit = 0.0f;            
            if ((DMCIt != inputData.end()) && (nrn < DMCIt->size())) {
               nextbit = DMCIt->at(nrn);
            }
            if (combineMethod == "|") {
               newbit = static_cast<float>(static_cast<bool>(fabs(newbit) > verySmallFloat) || static_cast<bool>(fabs(nextbit) > verySmallFloat));
            } else if (combineMethod == "&") {
               newbit = static_cast<float>(static_cast<bool>(fabs(newbit) > verySmallFloat) && static_cast<bool>(fabs(nextbit) > verySmallFloat));
            } else if ((combineMethod == "~|")) {
               newbit = static_cast<float>(static_cast<bool>(fabs(newbit) > verySmallFloat) != static_cast<bool>(fabs(nextbit) > verySmallFloat));
            } else if ((combineMethod == "~=")) {
               newbit = static_cast<float>(abs(newbit-nextbit)>verySmallFloat);
            } else if (combineMethod == "+") {
               newbit += nextbit;
            } else if (combineMethod == "-") {
               newbit -= nextbit;
            } else if (combineMethod == "*") {
               newbit *= nextbit;
            } else if (combineMethod == "/") {
               if (fabs(nextbit) < verySmallFloat) {
                  CALL_ERROR << "Error in " << FunctionName
                             << " Division by zero " << ERR_WHERE;
                  exit(EXIT_FAILURE);
               }
               newbit /= nextbit;
            } else if (combineMethod == "^") {
               if (fabs(newbit) < verySmallFloat) {
                  if (fabs(nextbit) < verySmallFloat) {
                     newbit = 1.0f;
                  } else if (nextbit < 0.0f) {
                     CALL_ERROR << "Error in " << FunctionName << ": 0 cannot "
                      "be raised to a negative power." << ERR_WHERE;
                     exit(EXIT_FAILURE);
                  } else {
                     newbit = 0.0f;
                  }
               } else if ((newbit < 0.0f) && (fabs(floor(nextbit) - nextbit)) > verySmallFloat) {
                  CALL_ERROR << "Error in " << FunctionName
                             << "negative numbers cannot be raised to "
                                "noninteger powers." << ERR_WHERE;
                  exit(EXIT_FAILURE);
               } else {
                  newbit = static_cast<float>(pow(newbit, nextbit));
               }
            } else if (combineMethod == ">") {
               newbit = static_cast<float>(newbit > nextbit);
            } else if (combineMethod == "<") {
               newbit = static_cast<float>(newbit < nextbit);
            } else if (combineMethod == ">=") {
               newbit = static_cast<float>(newbit >= nextbit);
            } else if (combineMethod == "<=") {
               newbit = static_cast<float>(newbit <= nextbit);
            } else if (combineMethod == "=") {
               newbit = static_cast<float>(fabs(newbit - nextbit) < verySmallFloat);
            } else if (combineMethod == "mean") {
               newbit += nextbit;
            } else if (combineMethod == "max") {
               newbit = max(nextbit, newbit);
            } else if (combineMethod == "min") {
               newbit = min(nextbit, newbit);
            } else {
               CALL_ERROR << "Error in " << FunctionName << " : unknown method "
                          << combineMethod << ERR_WHERE;
               ComL.DisplayHelp(Output::Err());
               exit(EXIT_FAILURE);
            }
            BuildIt->at(nrn) = newbit;
         }
         if (DMCIt != inputData.end()) ++DMCIt;
      }
   }

   if (combineMethod == "mean") {
      for (BuildIt = BuildMat.begin(); BuildIt != BuildMat.end(); BuildIt++) {
         for (unsigned int nrn = 0; nrn < maxNeuron; nrn++) {
            BuildIt->at(nrn) /= static_cast<float>(SeqList.size());
         }
      }
   }

   DataListType newDataType = DLT_unknown;
   string newType;
   string newSubType;
   readDataType(Type, newDataType, newType, newSubType, FunctionName, ComL);

   bool foundData = chkDataExists(DataName, newDataType, FunctionName, ComL);

   IFROOTNODE {
      Output::Out() << "\n\tto " << (foundData ? "replace " : "create ")
              << DataName.getValue() << " with " << maxTimeStep << " "
              << newSubType << "." << std::endl;      
   }
   SystemVar::insertData(DataName, BuildMat, newDataType);

   return;
}

void ResetFiring(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "ResetFiring";
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@ResetFiring() manually resets the network firing.");
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   ResetSTM();

   return;
}

void SaveData (ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "SaveData";
   static int argunset = true;
   static StrArg DataName("-from", "data structure name");
   static StrArg FileName("-to", "file name");
   // Append defaults to -overwrite (0)
   static FlagArg Append("-append", "-overwrite", "Append existing file", 0);
   // DoPad defaults to -nopad (0)
   static FlagArg DoPad ("-pad", "-nopad", "Fill in with zeros", 0);
   static FlagArg MATLABFmt ("-matlab", "-ascii", "Use the MATLAB format", 0);
#if defined(MULTIPROC)
   // NodeNum allows selection of a single node which reports data
   static IntArg NodeNum("-node", "node number for reporting(0 is root node)",
                         P_ROOT_NODE_NUM);
#endif
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@SaveData( ... ) saves data to file.\n");
      ComL.StrSet(2, &DataName, &FileName);
      ComL.FlagSet(3, &DoPad, &Append, &MATLABFmt);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

#if defined(TIMING_MODE)
   long long start,
        finish;

   Output::Out() << "Entering SaveData" << endl;

   IFROOTNODE {
      start = rdtsc();
   }
#endif

#if defined(MULTIPROC)
   // Only the root node(if all nodes are reporting) or the
   // node selected (if the NodeNum() argument was used) should
   // open the file for overwriting
   if (NodeNum.getValue() == static_cast<int>(ParallelInfo::getRank())) {
#else
   {
#endif
      ofstream outFile;
      int namlen = DataName.getValue().length();
#if defined(WIN32) || defined(NO_IOS_OPENMODE)
      ios_base::openmode oFlag = IOS::out;
#else
      std::_Ios_Openmode oFlag = IOS::out;
#endif
      if (MATLABFmt.getValue())
         oFlag |= IOS::binary;
      if (Append.getValue()) {
         oFlag |= IOS::app;
     } else {
         ofstream fset(FileName.getValue().c_str());
     }
      outFile.open(FileName.getValue().c_str(), oFlag);
      if (!outFile) {
         CALL_ERROR << "Error in " << FunctionName << " : Unable to open "
            << FileName.getValue() << " for writing" << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }

      const char dataType = SystemVar::GetVarType(DataName.getValue());

      if (dataType == 'S') {
         UIPtnSequence Seq = SystemVar::getSequence(DataName, FunctionName, ComL);
         // Output the Sequence         
         if (MATLABFmt.getValue() && DoPad.getValue()) {
            // MATLAB's .mat format is column major, 
            // whereas MATLAB's ASCII is row major
            Seq = PtnSeqToUISeq(transposeMatrix(UISeqToPtnSeq(Seq)));
         }
         int mrows = Seq.size();
         // int ncols = findMaxSize(Seq);
         int ncols = ni;
         int numPasses = 1;
         if (MATLABFmt.getValue()) {
            if (DoPad.getValue()) {
               WriteMATLABHeader(outFile, ncols, mrows, namlen, false, false);
            } else {
               int numNonZero = 0;
               for (UIPtnSequenceCIt it = Seq.begin(); it != Seq.end(); it++) {
                  numNonZero += it->size();
               }
               UIVector last = (Seq.size()==0) ? UIVector(0) : Seq.back();
               // If the last neuron of the last pattern isn't set to 1, then set it
               // to 0 for the sparse matrix format, so that when loading it into
               // MATLAB it fills out the matrix to the correct size.
               if ((last.size()==0) || (last.back()!=static_cast<unsigned int>(ncols)) && !DoPad.getValue()) {
                  ++numNonZero; // OK, so it will be zero, but give me a break
               }
               numPasses = 3; // one for each "column" (see prior column major discussion)
               // FIXME: Sparse is being set to false because true seems to corrupt
               // the file. Not a big problem, but it would be nice to fix.
               WriteMATLABHeader(outFile, numNonZero, numPasses, namlen, false, false);
            }
            outFile.write(DataName.getValue().c_str(), namlen);
         }
         for (int pass = 0; pass < numPasses; ++pass) {
            int ElementNum = 1;
            for (UIPtnSequenceCIt it = Seq.begin(); it != Seq.end(); ++it) {
               // Padding changes the output from sparse to full (and pads with
               // zeros if necessary)
               UIVector pat = *it;
               int lastFired = -1; // last Neuron we wrote out
               for (UIVectorCIt pIt = pat.begin(); pIt != pat.end(); ++pIt) {
                  // Specifying -pad will cause the matrix to be written in full
                  // matrix format
                  int curNeur = static_cast<int>(*pIt);
                  if (DoPad.getValue()) {
                     if (MATLABFmt.getValue()) {
                        double toWrite = 0.0L;
                        for (int iPad = 0; iPad < (curNeur-lastFired-1); ++iPad)
                           outFile.write((char *)&toWrite, 8);
                        toWrite = 1.0L;
                        outFile.write((char *)&toWrite, 8);
                     } else {
                        for (int iPad = 0; iPad < (curNeur-lastFired-1); ++iPad)
                           outFile << "0 ";
                        outFile << "1 ";
                     }
                  } else {
                     // This format might look weird and inefficient, but it is
                     // the sparse matrix format that MATLAB recognizes, so don't
                     // change it!
                     if (MATLABFmt.getValue()) {
                        double toWrite;
                        switch (pass) {
                           case 0:
                              toWrite = static_cast<double>(ElementNum);
                              outFile.write((char *)&toWrite, 8);
                              break;
                           case 1:
                              toWrite = static_cast<double>(curNeur+1);
                              outFile.write((char *)&toWrite, 8);
                              break;
                           case 2:
                              toWrite = static_cast<double>(1);
                              outFile.write((char *)&toWrite, 8);
                        }
                     } else {
                        outFile << ElementNum << " " << (curNeur+1) << " 1\n";
                     }
                  }
                  lastFired = curNeur;
               }
               if (DoPad.getValue()) {
                  for (int i = lastFired+1; i < ncols; ++i) {
                     if (MATLABFmt.getValue()) {
                        double toWrite = 0.0L;
                        outFile.write((char *)&toWrite, 8);
                     } else {
                        outFile << "0 ";
                     }
                  }
                  if (!MATLABFmt.getValue()) {
                     outFile << "\n";
                  }
               }
               ElementNum++;
            }
            UIVector last = (Seq.size()==0) ? UIVector(0) : Seq.back();
            // If the last neuron of the last pattern isn't set to 1, then set it
            // to 0 for the sparse matrix format, so that when loading it into
            // MATLAB it fills out the matrix to the correct size.
            const int highNlastT = (last.size() == 0) ?
                                       -1 : static_cast<int>(last.back());
            if ((highNlastT != (ncols-1)) && !DoPad.getValue()) {
               if (MATLABFmt.getValue()) {
                  double toWrite;
                  switch (pass) {
                     case 0:
                        toWrite = static_cast<double>(mrows);
                        outFile.write((char *)&toWrite, 8);
                        break;
                     case 1:
                        toWrite = static_cast<double>(ncols);
                        outFile.write((char *)&toWrite, 8);
                        break;
                     case 2:
                        toWrite = static_cast<double>(0);
                        outFile.write((char *)&toWrite, 8);
                  }
               } else {
                  outFile << mrows << " " << ncols << " 0\n";
               }
            }
         }
         Output::Out() << "Wrote sequence " << DataName.getValue() << " to file "
                 << FileName.getValue() << std::endl;
      } else if ((dataType == 'M') || (dataType == 'A')) {
         DataMatrix Matrix;
         string dataTypeName;         
         if (dataType == 'M') {
            Matrix = SystemVar::getMatrix(DataName, FunctionName, ComL);
            dataTypeName = "matrix";
         } else {
            Matrix = SystemVar::getAnalysis(DataName, FunctionName, ComL);
            dataTypeName = "analysis";
         }
         if (MATLABFmt.getValue()) {
            // MATLAB's .mat format is column major, whereas ASCII is row major
            Matrix = transposeMatrix(Matrix);
         }
         // Output the Matrix
         int mrows = Matrix.size();
         int ncols = findMaxSize(Matrix);
         if (MATLABFmt.getValue()) {
            // isSparse is set to false because I can't figure out how to make
            // sparse matrices actually work. This means that to make a full
            // matrix you should use full(spconvert(x)) where x is the matrix
            // (A true sparse matrix would just use full(x) instead.)
            WriteMATLABHeader(outFile, ncols, mrows, namlen, false, false);
            outFile.write(DataName.getValue().c_str(), namlen);
         }
         for (DataMatrixCIt it = Matrix.begin(); it != Matrix.end(); it++) {
            for (unsigned int i = 0; i < it->size(); i++) {
               if (MATLABFmt.getValue()) {
                  double toWrite = static_cast<double>(it->at(i));
                  outFile.write((char *)&toWrite, 8);
               } else {
                  outFile << it->at(i) << " ";
               }
            }
            if (DoPad.getValue()) {
               for (int i = it->size(); i < ncols; i++) {
                  if (MATLABFmt.getValue()) {
                     double toWrite = 0.0L;
                     outFile.write((char *)&toWrite, 8);
                  } else {
                     outFile << 0.0f << " ";
                  }
               }
            }
            if (!MATLABFmt.getValue())
               outFile << "\n";
         }
         Output::Out() << "Wrote " << dataTypeName << " "
                       << DataName.getValue() << " to file "
                       << FileName.getValue() << std::endl;
      } else if (dataType == 'i') {
         int intVal = SystemVar::GetIntVar(DataName.getValue());
         if (MATLABFmt.getValue()) {
            WriteMATLABHeader(outFile, 1, 1, namlen, false, false);
            outFile.write(DataName.getValue().c_str(), namlen);
            double toWrite = static_cast<double>(intVal);
            outFile.write((char *)&toWrite, 8);
         } else {
            outFile << intVal << std::endl;
         }
         Output::Out() << "Wrote integer " << DataName.getValue() << " to file "
                 << FileName.getValue() << std::endl;
      } else if (dataType == 'f') {
         float floatVal = SystemVar::GetFloatVar(DataName.getValue());
         if (MATLABFmt.getValue()) {
            WriteMATLABHeader(outFile, 1, 1, namlen, false, false);
            outFile.write(DataName.getValue().c_str(), namlen);
            double toWrite = static_cast<double>(floatVal);
            outFile.write((char *)&toWrite, 8);
         } else {
            outFile << floatVal << std::endl;
         }
         Output::Out() << "Wrote float " << DataName.getValue() << " to file "
                 << FileName.getValue() << std::endl;
      } else if (dataType == 's') {
         string strVal = SystemVar::GetStrVar(DataName.getValue());
         bool isText = !isNumeric(strVal);
         if (MATLABFmt.getValue()) {
            int ncols = 1;
            if (isText)
               ncols = strVal.length();
            WriteMATLABHeader(outFile, 1, ncols, namlen, false, isText);
            outFile.write(DataName.getValue().c_str(), namlen);
            if (isText) {
               for (int c = 0; c < ncols; ++c) {
                  double toWrite = static_cast<double>(strVal[c]);
                  outFile.write((char *)&toWrite, 8);
               }
            } else {
               double toWrite = from_string<double>(strVal);
               outFile.write((char *)&toWrite, 8);
            }
         } else {
            outFile << strVal << std::endl;
         }
         if (isText)
            Output::Out() << "Wrote string ";
         else
            Output::Out() << "Wrote number ";
         Output::Out() << DataName.getValue() << " to file "
                 << FileName.getValue() << std::endl;
      } else {
         CALL_ERROR << "Error in " << FunctionName << " : Data Structure "
            << DataName.getValue() << " not found" << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
      outFile.close();
   }

#if defined(TIMING_MODE)
   IFROOTNODE {
      finish = rdtsc();
      Output::Out() << "Elapsed saving time = " << (finish - start) * 1.0 / TICKS_PER_SEC
         << " seconds" << std::endl;
   }
#endif
   return;
}

void SaveWeights(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "SaveWeights";
   static int argunset = true;
   static int MadeMatlab = false;
   // MakeMatlab defaults to -noMFiles(0)
   static FlagArg MakeMatlab ("-MFiles", "-noMFiles",
                              "make the mfile to\n\t\t\t load in weights and "
                              "connections to Matlab\n\t\t\t only called once per script",
                              0);
   static StrArg FileName("-to", "file name", "wij.dat");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@SaveWeights( ... ) saves the weights to file.\n");
      ComL.StrSet(1, &FileName);
      ComL.FlagSet(1, &MakeMatlab);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   ofstream outFile(FileName.getValue().c_str());
   if (!outFile) {
      CALL_ERROR << "Error in " << FunctionName << " : Unable to open "
         << FileName.getValue() << " for writing" << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   // for i = 1:NumNodes, the node sends 1: how many neurons we're dealing with
   // 2: connections(one array at a time) 3: weights(one array at a time)
   // as soon as an array arrives, it gets dumped to a file
   // there will be separate files for connections and weights, and I'll change
   // the SaveWeights function such that there's an option to do it like that

#if defined(MULTIPROC)
   IFROOTNODE {
      Output::Out() << "Saving weights to file " << FileName.getValue() << std::endl;
      outFile << ni << "\n";
      outFile << "# Fan-in synaptic count per neuron:\n";

      // rows are neurons, columns are nodes
      UIMatrix nodeConn = ParallelInfo::rcvNodeConn(Shuffle, FanInCon);
      // sum over columns
      UIVector totalConn = matrixSum(nodeConn);

      for (unsigned int i = 0; i < ni; i++) {
         outFile << totalConn[i] << " ";
      }
      outFile << "\n";
      outFile << "# Fan-in synapses (pre-synaptic neuron number)\n";
      
      for (unsigned int i = 0; i < ni; i++) {
         UIVector nrnConn = ParallelInfo::rcvNrnConn(nodeConn.at(i), i, Shuffle, UnShuffle, FanInCon, inMatrix);
         for (UIVectorCIt it = nrnConn.begin(); it != nrnConn.end(); ++it) {
            outFile << *it << " ";
         }
         outFile << "\n";
      }

      outFile << "# Fan-in synaptic weights\n";
      for (unsigned int i = 0; i < ni; i++) {
         DataList nrnWij = ParallelInfo::rcvNrnWij(nodeConn.at(i), i, Shuffle, FanInCon, inMatrix);
         for (DataListCIt it = nrnWij.begin(); it != nrnWij.end(); ++it) {
            outFile << *it << " ";
         }
         outFile << "\n";
      }

      if (maxAxonalDelay > defMaxAxonalDelay) {
         outFile << "# Fan-in axonal delays\n";
         for (unsigned int i = 0; i < ni; i++) {
            UIVector nrnAij = ParallelInfo::rcvNrnAij(nodeConn.at(i), i, Shuffle, FanInCon, inMatrix, FanOutCon, outMatrix,
                               minAxonalDelay, maxAxonalDelay);
            for (UIVectorCIt it = nrnAij.begin(); it != nrnAij.end(); ++it) {
               outFile << *it << " ";
            }
            outFile << "\n";
         }
      }
   } else {        // I'm a computation node
      // Tell the root node how many connections I have for each neuron
      ParallelInfo::sendNodeConn(Shuffle, FanInCon);
      ParallelInfo::sendNrnConn(Shuffle, UnShuffle, FanInCon, inMatrix);
      ParallelInfo::sendNrnWij(Shuffle, FanInCon, inMatrix);
      ParallelInfo::sendNrnAij(Shuffle, FanInCon, inMatrix, FanOutCon, outMatrix,
                               minAxonalDelay, maxAxonalDelay);
   }
#else
   Output::Out() << "Saving weights to file " << FileName.getValue() << std::endl;
   outFile << ni << "\n";
   outFile << "# Fan-in synaptic count per neuron:\n";
   for (unsigned int connectCol = 0; connectCol < ni; ++connectCol) {
      outFile << FanInCon.at(connectCol) << " ";
   }
   outFile << "\n";
   outFile << "# Fan-in synapses (pre-synaptic neuron number)\n";
   for (unsigned int conRow = 0; conRow < ni; ++conRow) {
      for (unsigned int conCol = 0; conCol < FanInCon.at(conRow); ++conCol) {
         outFile << inMatrix[conRow][conCol].getSrcNeuron() << ' ';
      }
      outFile << "\n";
   }
   outFile << "# Fan-in synaptic weights\n";
   for (unsigned int weightRow = 0; weightRow < ni; ++weightRow) {
      DendriticSynapse * dendriticTree = inMatrix[weightRow];
      for (unsigned int weightCol = 0; weightCol < FanInCon.at(weightRow); ++weightCol) {
         outFile << dendriticTree[weightCol].getWeight() << ' ';
      }
      outFile << "\n";
   }
   if (maxAxonalDelay > defMaxAxonalDelay) {
      outFile << "# Fan-in axonal delays\n";
      // This ostensibly scales as n^3c^2, which could be bad
      for (unsigned int axonalRow = 0; axonalRow < ni; ++axonalRow) {
         DendriticSynapse * dendriticTree = inMatrix[axonalRow];
         for (unsigned int axonalCol = 0; axonalCol < FanInCon[axonalRow]; ++axonalCol) {
            unsigned int inNeuron = dendriticTree[axonalCol].getSrcNeuron();
            AxonalSynapse **inAxon = outMatrix[inNeuron];
            unsigned int axonalDelay = 0; // not a valid value!
            for (unsigned int refTime = minAxonalDelay-1; refTime < maxAxonalDelay; ++refTime) {
               AxonalSynapse * axonalSegment = inAxon[refTime];
               for (unsigned int outNeuron = 0; outNeuron < FanOutCon[inNeuron][refTime]; ++outNeuron) {
                  // Pointer comparison, not value comparison
                  if (axonalSegment[outNeuron].connectsTo(dendriticTree[axonalCol])) {
                     axonalDelay = refTime + 1;
                     break;
                  }
               }
            }
            outFile << axonalDelay << ' ';
         }
         outFile << "\n";
      }
   }
#endif
   IFROOTNODE {
      outFile.close();
      if (MakeMatlab.getValue() && !MadeMatlab) {
         MadeMatlab = true;
         Output::Out() <<
            "Creating readwij.m for loading weights and connections into Matlab." <<
            std::endl;
         ofstream wijout("readwij.m");
         if (!wijout) {
            CALL_ERROR << "Error in " << FunctionName <<
               " : Unable to open readwij.m" << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         wijout <<
            "function [FanInCon,cInMatrix,wInMatrix] = readwij(filename)\n"
            "\n"
            "% [FanInCon,cInMatrix,wInMatrix] = readwij(filename)\n"
            "%\n"
            "% gets the weight data in NeuroJet form\n"
            "%\n"
            "% FanInCon is a vector of number of connections\n"
            "% FanInCon(j) = # inputs to neuron j\n"
            "%\n"
            "% cInMatrix is a list of input indices\n"
            "% cInMatrix(j,1:FanInCon(j)) = list of indices for input\n"
            "%  neurons into neuron j\n"
            "%  NOTE: Indices start at zero in NeuroJet so cInMatrix+1\n"
            "%  gives proper indices for Matlab.\n"
            "%\n"
            "% wInMatrix is a list of weights corresponding to cInMatrix\n"
            "% wInMatrix(j,k) = weight of cInMatrix(j,k)\n"
            "%\n"
            "\n"
            "tic\n"
            "\n"
            "if (nargin == 0)\n"
            "\tfilename = 'wij.dat';\n"
            "elseif (nargin ~= 1)\n"
            "\tdisp('readwij only take one parameter');\n"
            "\treturn\n"
            "end\n"
            "\n"
            "if ((nargout < 1) | (nargout > 3))\n"
            "\tdisp('readwij only outputs 1, 2, or 3 matrices');\n"
            "\treturn\n"
            "end\n"
            "\n"
            "fid = fopen(filename,'r');\n"
            "ni = fscanf(fid,'%d',1);\n"
            "\n"
            "disp('reading in total connections...');\n"
            "FanInCon(1:ni) = fscanf(fid,'%d',ni);\n"
            "\n"
            "if (nargout > 1)\n"
            "disp('reading in connections...');\n"
            "for i = 1:ni\n"
            "\tcInMatrix(i,1:FanInCon(i)) = [fscanf(fid,'%d',FanInCon(i))]';\n"
            "end\n"
            "end\n"
            "\n"
            "if (nargout > 2)\n"
            "disp('reading in weights...');\n"
            "for i = 1:ni\n"
            "\twInMatrix(i,1:FanInCon(i)) = [fscanf(fid,'%f',FanInCon(i))]';\n"
            "end\n" "end\n" "\n" "fclose(fid);\n" "\n" "toc\n" "\n" "return\n" "\n";
         wijout.close();
      }
   }
   return;
}

void SetLoopVar(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "SetLoopVar";
   static int argunset = true;
   static StrArg VarName("-var", "variable name");
   static StrArg IterName("-loop", "loop iterator name");
   static DblArg StartVal("-start", "starting value");
   static DblArg EndVal("-end", "ending value");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet(
          "@SetLoopVar( ... ) A short cut for calculating a variable's value\n"
          "that you want to vary linearly with a loop iterator.\n"
          "The variable will begin at start and finish at end,\n"
          "changing linearly in between as the given iterator changes.\n");
      ComL.DblSet(2, &StartVal, &EndVal);
      ComL.StrSet(2, &VarName, &IterName);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   /*
    * Load the Iterator 
    */
   if (SystemVar::GetVarType(IterName.getValue()) != 'I') {
      CALL_ERROR << "Error in " << FunctionName << " : Could not find iterator "
         << IterName.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   Iterator TempIterator = SystemVar::GetIterator(IterName.getValue());
   int lastStep = TempIterator.EndVal;
   int firstStep = TempIterator.StartVal;
   int thisStep = TempIterator.CurrentVal;
   int stepSize = TempIterator.StepVal;
   int TotalSteps = iceil(static_cast<float>(abs(lastStep - firstStep) + 1) /
                          static_cast<float>(abs(stepSize)));
   int CurrentStep = 1 + TotalSteps - iceil(static_cast<float>((lastStep - thisStep) + 1) /
                                            static_cast<float>(stepSize));

   /*
    * Calculate the Current Value 
    */
   float CurrentVal;
   if (TotalSteps - 1 > 0) {
      CurrentVal = static_cast<float>(StartVal.getValue() +
                (CurrentStep - 1) * (EndVal.getValue() - StartVal.getValue()) / (TotalSteps - 1));
   } else {
      CurrentVal = static_cast<float>(StartVal.getValue());
   }

   /*
    * Load The Variable to set and do so 
    */
   UpdateParams(VarName.getValue(), to_string(CurrentVal), "SetLoopVar");
}

void SetStream(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "SetStream";
   static int argunset = true;
   static StrArg StdOutFile("-StdOut",
                             "Standard Out File(no argument ignores)", "{noarg}");
   static StrArg StdErrFile("-StdErr", "Error Out File(no argument ignores)",
                             "{noarg}");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@SetStream( ... ) redirects output to files and / or screen.\n"
          "StdOut or StdErr as filenames will redirect output to standard out\n"
          "and standard error, respectively.\n");
      ComL.StrSet(2, &StdOutFile, &StdErrFile);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   // check for errors in the arguments
   if (StdOutFile.getValue() != "{noarg}") {
      if (StdOutFile.getValue() == "StdOut") {
         Output::setStreams(cout, Output::Err());
      } else if (StdOutFile.getValue() == "StdErr") {
         Output::setStreams(cerr, Output::Err());
      } else {
         static ostream *stdOut = NULL;
         delPtr(stdOut); // if it's not NULL
         stdOut = new ofstream(StdOutFile.getValue().c_str(), ios::app);
         if (!stdOut) {
            CALL_ERROR << "Unable to redirect StdOut to " << StdOutFile.getValue() << ERR_WHERE;
            ComL.DisplayHelp(Output::Err());
            exit(EXIT_FAILURE);
         }
         Output::setStreams(*stdOut, Output::Err());
      }
   }   
   if (StdErrFile.getValue() != "{noarg}") {
      if (StdErrFile.getValue() == "StdOut") {
         Output::setStreams(Output::Out(), cout);
      } else if (StdErrFile.getValue() == "StdErr") {
         Output::setStreams(Output::Out(), cerr);
      } else {
         static ostream *stdErr;
         delPtr(stdErr); // only if it's not NULL
         stdErr = new ofstream(StdErrFile.getValue().c_str(), ios::app);
         if (!stdErr) {
            CALL_ERROR << "Unable to redirect StdErr to " << StdErrFile.getValue() << ERR_WHERE;
            ComL.DisplayHelp(Output::Err());
            exit(EXIT_FAILURE);
         }
         Output::setStreams(Output::Out(), *stdErr);
      }
   }

   return;
}

void SetVar(ArgListType &arg) //AT_FUN
{
   if (arg.size() < 1) {
      CALL_ERROR << "Error in SetVar: expects at least one argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "SetVar -help\n\n"
         "@SetVar( ... ) takes a list of one or more variables,\n"
         "each followed by its new value, and updates the value\n"         
         "Unlike most functions, SetVar does not require any preceeding flags.\n"
         << std::endl;
      exit(EXIT_FAILURE);
   }

   if (arg.size() % 2 != 0) {
      CALL_ERROR <<
         "Error in SetVar: You do not have and even number of arguments." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   map<string, string> deprecatedVars;
   deprecatedVars["KCA"] = "VarKConductance";
   deprecatedVars["KdAdjDecay"] = "PyrToInternrnWtAdjDecay";
   deprecatedVars["DASConst"] = "lambdaFB";
   deprecatedVars["wNoise"] = "synFailRate";
   deprecatedVars["Ki"] = "KFF";
   deprecatedVars["Kr"] = "KFB";
   deprecatedVars["v"] = "";
   deprecatedVars["dt"] = "";
   deprecatedVars["nmda"] = "";
   deprecatedVars["niCA1"] = "";
   deprecatedVars["PeriodCA1"] = "";
   deprecatedVars["ActivityTestCA1"] = "";
   deprecatedVars["ActivityTrainCA1"] = "";
   deprecatedVars["AmplitudeCA1"] = "";
   deprecatedVars["ConCA3CA1"] = "";
   for (ArgListTypeIt it = arg.begin(); it!= arg.end(); /* do nothing */) {
      string VarName = (it++)->first;
      if (deprecatedVars.find(VarName) != deprecatedVars.end()) {
         IFROOTNODE {
            Output::Out() << "'" << VarName << "' is deprecated.";
            if (deprecatedVars[VarName].size() > 0) {
               Output::Out() << " Use '" << deprecatedVars[VarName] << "' instead." << std::endl;
            } else {
               Output::Out() << " It has no impact on simulation behavior." << std::endl;
            }
         }
         VarName = deprecatedVars[VarName]; // Replace deprecated variable with suggested replacement
      }
      const string VarValue = (it++)->first;
      if (VarName.size() > 0)
         UpdateParams(VarName, VarValue, "SetVar");
   }

   return;
}

void Sim(ArgListType &arg) //AT_FUN
{
   // In the multi-processor version of Sim, there's no need for any data interchange;
   // All data that Sim needs is available on all nodes

   // process the function arguments
   static string FunctionName = "Sim";
   static int argunset = true;
   static StrArg FileName("-file", "output file name", "{no file}");
   static StrArg xSeqName("-x", "x data name", "TestingBuffer");
   static StrArg ySeqName("-y", "y data name", "TrainingBuffer");
   static StrArg Method ("-method", "type of similarity {cos,ham,nham}", "cos");
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg EndNeuron("-Nend", "end neuron {-1 defaults to seq size}", -1);
   // DoSummary defaults to -sum(1)
   static FlagArg DoSummary("-sum", "-nosum", "print a summary", 1);
   // DoGraph defaults to -nograph (0)
   static FlagArg DoGraph ("-graph", "-nograph", "print an ascii graph", 0);
   static IntArg Winners("-win", "number of winners to show in summary", 3);
   // DoDov defaults to -nodov(0)
   static FlagArg DoDov("-dov", "-nodov", "show degree of victory info in summary", 0);
   static IntArg xGraphLen("-xgraph",
                            "max length of x-axis in graph {-1 is seq len}", -1);
   static IntArg yGraphLen("-ygraph",
                            "max length of y-axis in graph {-1 is seq len}", -1);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@Sim( ... ) computes the similarity matrix for two sequences.\n"
          "SimBuffer is the similarity matrix. WinBuffer is the winners matrix.\n"
          "WinBuffer has a vector for each time step in the x data. Each\n"
          "of these vectors then has elements of the form:\n"
          "winner1 sim1 winner2 sim2 ... winnerN simN, where N is the number\n"
          "of winners from the command line.\n");
      //#if defined(MULTIPROC)
      //"The root node also keeps track of GlobalSimBuffer, which is an average\n"
      //"of SimBuffers from all nodes in the network.\n"
      //#endif
      ComL.StrSet(4, &FileName, &xSeqName, &ySeqName, &Method);
      ComL.IntSet(5, &StartNeuron, &EndNeuron, &Winners, &xGraphLen, &yGraphLen);
      ComL.FlagSet(3, &DoSummary, &DoDov, &DoGraph);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   int  meth = 0;
   if (Method.getValue() == "ham") {
      meth = 1;
   } else if (Method.getValue() == "nham") {
      meth = -1;
   } else if (Method.getValue() != "cos") {
      CALL_ERROR << "Error in " << FunctionName << " : Method "
         << Method.getValue() << " unknown." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   char xType = SystemVar::GetVarType(xSeqName);
   char yType = SystemVar::GetVarType(ySeqName);

   if (meth && (xType !='S' || yType != 'S')) {
      CALL_ERROR << "Error in " << FunctionName << " : Method "
                << Method.getValue() << " can only be used with Sequences." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   DataMatrix xMat = SystemVar::getData(xSeqName, FunctionName, ComL);
   int xSeqSize = xMat.size();
   int MaxSize = findMaxSize(xMat);

   DataMatrix yMat = SystemVar::getData(ySeqName, FunctionName, ComL);
   int ySeqSize = yMat.size();      
   MaxSize = findMaxSize(yMat, MaxSize);

   int startN = StartNeuron.getValue();
   int endN = EndNeuron.getValue();
   if (endN == -1) {
      endN = MaxSize;
   }
   if (startN < 1 || startN > endN || endN > MaxSize) {
      CALL_ERROR << "Error in " << FunctionName << " : Invalid neuron range : "
                 << startN << "..." << endN << " with maximum pattern size of "
                 << MaxSize << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   // Precalculate the magnitudes
   DataList xMag(xSeqSize);
   DataMatrixCIt xDMCIt = xMat.begin();
   for (int ix = 0; ix < xSeqSize; ++ix, ++xDMCIt) {
      double sum = 0.0L;
      int stopN = min(endN, static_cast<int>(xDMCIt->size()));
      for (int j = startN-1; j < stopN; ++j) {
         sum += xDMCIt->at(j) * xDMCIt->at(j);
      }
      xMag.at(ix) = static_cast<float>(sum);
   }
   DataList yMag(ySeqSize);
   DataMatrixCIt yDMCIt = yMat.begin();
   for (int iy = 0; iy < ySeqSize; ++iy, ++yDMCIt) {
      double sum = 0.0L;
      int stopN = min(endN, static_cast<int>(yDMCIt->size()));
      for (int j = startN-1; j < stopN; ++j) {
         sum += yDMCIt->at(j) * yDMCIt->at(j);
      }
      yMag.at(iy) = static_cast<float>(sum);
   }

   DataMatrix SimBuffer;

   // Perform the calculation
   yDMCIt = yMat.begin();
   double worst = 0.0L;
   for (int i = 0; i < ySeqSize; ++i, ++yDMCIt) {
      DataList temp_vect(xSeqSize);
      xDMCIt = xMat.begin();
      for (int j = 0; j < xSeqSize; ++j, ++xDMCIt) {
         double sim = 0.0f;
         if ((fabs(xMag.at(j)) < verySmallFloat) || (fabs(yMag.at(i)) < verySmallFloat)) {
            if (meth == -1) {
               sim = 1.0f;
            } else if (meth == 1) {
               sim = xMag.at(j) + yMag.at(i);
            }
         } else {
            double dot_prod = 0.0f;
            int lastN = static_cast<int>(min(xDMCIt->size(), yDMCIt->size()));
            for (int n = startN-1; n < min(endN,lastN); ++n) {
               dot_prod += xDMCIt->at(n) * yDMCIt->at(n);
            }
            sim = 0.0f;
            if (meth) {
               sim = xMag.at(j) + yMag.at(i) - 2.0f * dot_prod;
            } else {
               sim = dot_prod / (sqrt(xMag.at(j) * yMag.at(i)));
            }
            if (meth == -1) {
               sim /= (xMag.at(j) + yMag.at(i));
            }
         }
         temp_vect.at(j) = static_cast<float>(sim);
         bool worse = (meth) ? (sim > worst) : (sim < worst);
         if (((i == 0) && (j == 0)) || worse) {
            worst = sim;
         }
      }
      SimBuffer.push_back(temp_vect);
   }
   SystemVar::insertData("SimBuffer", SimBuffer, DLT_matrix);

   // Calculate the winners
   // create the matrix of winners... format: winner#1 sim#1 winner#2 sim#2 ... winner#N sim#N
   // no DOV information is given since it can be calculated

   // go ahead and get the vectors
   int  numWinners = Winners.getValue();
   if (numWinners < 1) numWinners = 1;
   // bound it by the number of Ys
   if (numWinners > ySeqSize) numWinners = ySeqSize;
   DataMatrix WinMatrix(numWinners * 2, DataList(xSeqSize));

   // Show the summary information
   Output::Out() << "Similarity between X = " << xSeqName.getValue()
                 << " and Y = " << ySeqName.getValue() << " using neurons "
                 << startN << "..." << endN << " and method "
                 << Method.getValue() << "." << endl;

   if ((ySeqSize < 2) || (xSeqSize < 1)) {
      if (DoSummary.getValue()) {
         Output::Out() << "Summary Statistics: N/A\n\n";
      }
   } else if ((ySeqSize > 1000000) || (xSeqSize > 1000000)) {
      Output::Out() << "Sequence sizes are unrealistic:" << endl;
      Output::Out() << "\tySeqSize: " << ySeqSize << endl;
      Output::Out() << "\txSeqSize: " << xSeqSize << endl;
   } else {
      // get a vector to keep the winners in - initially assume the last y wins
      vector<unsigned int> winlist;
      DataList winvalues;

      // set up the labels
      if (DoSummary.getValue()) {
         Output::Out() << "Summary Statistics: \n X " << setiosflags(ios::left);
         string dovStr = (DoDov.getValue()) ? "dov. " : "";         
         for (int i = 0; i < numWinners; ++i) {
            if (i == numWinners - 1) {
               dovStr = "";
            }
            Output::Out() << "win" << std::setw(3) << (i + 1) << " sim. " << dovStr;
         }
         Output::Out() << "\n\n" << std::resetiosflags(ios::left);
      }
      // loop through each x
      for (int i1 = 0; i1 < xSeqSize; ++i1) {
         if (DoSummary.getValue()) {
            // print out the current x
            Output::Out() << std::setw(3) << (i1 + 1) << " " << std::flush;
         }
         // get a temp to help with keeping track of winners
         winlist = vector<unsigned int>(numWinners, ySeqSize - 1);
         winvalues = DataList(numWinners, static_cast<float>(worst));
         // loop through the number of summaries
         for (int j = 0; j < numWinners; ++j) {
            // loop through the rest of the ys (begin in reverse is the end)
            DataMatrix::reverse_iterator sDMRevCIt = SimBuffer.rbegin();
            // so we always find one better
            double best = (meth) ? (worst + 1.0f) : (worst - 1.0f); 
//            for (int k = ySeqSize - 1; k >= 0; --k, --sDMRevCIt) {
            for (int k = ySeqSize - 1; k >= 0; --k, ++sDMRevCIt) {
               // if one wins, then check against past winners and update if
               // necessary
               bool better = (meth) ? (sDMRevCIt->at(i1) <= best)
                                    : (sDMRevCIt->at(i1) >= best);
               if (better) {
                  bool found = false;
                  for (int m = 0; m < j; ++m) {
                     if (winlist.at(m) == static_cast<unsigned int>(k)) {
                        found = true;
                        break;
                     }
                  }
                  // if not already in our winners list
                  if (!found) {
                     best = sDMRevCIt->at(i1);
                     winlist.at(j) = k;
                     winvalues.at(j) = static_cast<float>(best);
                  }
               }
            }
         }
         // remember the winners
         DataMatrixIt WinIt = WinMatrix.begin();
         for (int j1 = 0; j1 < numWinners; ++j1) {
            WinIt->at(i1) = static_cast<float>(winlist.at(j1) + 1);
            WinIt++;
            WinIt->at(i1) = winvalues.at(j1);
            WinIt++;
         }
         // display the winners
         if (DoSummary.getValue()) {
            for (int j = 0; j < numWinners; ++j) {
               Output::Out() << std::setw(3) << (winlist.at(j) + 1) << " "
                             << std::setiosflags(ios::fixed | ios::showpoint)
                             << std::setprecision(4) << winvalues.at(j) << " ";
               if ((j != numWinners - 1) && DoDov.getValue()) {
                  Output::Out() << std::setprecision(4)
                              << (winvalues.at(j) - winvalues.at(j + 1)) << " ";
               }
               Output::Out() << std::resetiosflags(ios::fixed | ios::showpoint);
            }
            Output::Out() << std::endl;
         }
      }
   }
   // put the matrix into the global environment
   SystemVar::insertData("WinBuffer", WinMatrix, DLT_matrix);

   // Do a graph
   if (DoGraph.getValue() && (meth != 1)) {
      Output::Out() << "\n";
      if ((yGraphLen.getValue() == -1) || (yGraphLen.getValue() > ySeqSize)) {
         yGraphLen.setValue(ySeqSize);
      }
      if ((xGraphLen.getValue() == -1) || (xGraphLen.getValue() > xSeqSize)) {
         xGraphLen.setValue(xSeqSize);
      }
      if (yGraphLen.getValue() < 1 || xGraphLen.getValue() < 1) {
         CALL_ERROR << "Error in " << FunctionName << " : Invalid ranges"
            << " for screen graph sizes of x = " << xGraphLen.getValue()
            << " and y = " << yGraphLen.getValue() << "\n\n" << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
      double xStep = static_cast<double>(xSeqSize)/static_cast<double >(xGraphLen.getValue());
      double yStep = static_cast<double>(ySeqSize)/static_cast<double>(yGraphLen.getValue());
      double BlockArea = xStep * yStep;
      double ScaleFact = 4.999999L / BlockArea;
      const char sym[5] = { char(183), '-', '+', '*', '#' };
      for (int i = 0; i < yGraphLen.getValue(); i++) {
         double yEnd = yStep * (i + 1);
         for (int j = 0; j < xGraphLen.getValue(); j++) {
            // Calculate sum
            double xEnd = xStep * (j + 1);
            double sum = 0.0L;
            double yStart = yStep * i;
            while (yStart < yEnd) {
               double xStart = xStep * j;
               double yNext = yStart + 1.0L;
               if (yNext > yEnd) {
                  yNext = yEnd;
               } else if (yNext > floor(yNext)) {
                  yNext = floor(yNext);
               }
               double yLen = yNext - yStart;
               int  yIndex = ySeqSize - ifloor(yStart) - 1;
               DataMatrixCIt sDMCIt = SimBuffer.begin();
               for (int inc = 1; inc < yIndex; ++inc, ++sDMCIt) {}
               while (xStart < xEnd) {
                  double xNext = xStart + 1.0L;
                  if (xNext > xEnd) {
                     xNext = xEnd;
                  } else if (xNext > floor(xNext)) {
                     xNext = floor(xNext);
                  }
                  double xLen = xNext - xStart;
                  int  xIndex = ifloor(xStart);
                  sum += yLen * xLen * sDMCIt->at(xIndex);
                  xStart = xNext;
               }
               yStart = yNext;
            }
            int symIdx = meth ? 4 - ifloor(sum * ScaleFact)
                              : ifloor(sum * ScaleFact);
            Output::Out() << sym[symIdx];
         }
         Output::Out() << "\n";
      }
      Output::Out() << "\n";
   }
   // OutPut the matrix
   if (FileName.getValue() != "{no file}") {
      ofstream OutFile(FileName.getValue().c_str());
      if (OutFile == NULL) {
         CALL_ERROR << "Error in " << FunctionName << " : Could not open file "
            << FileName.getValue() << " for writing" << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
      IFROOTNODE {
         for (DataMatrixCIt sDMCIt = SimBuffer.begin(); sDMCIt != SimBuffer.end(); ++sDMCIt) {
            for (int j = 0; j < xSeqSize; j++) {
               OutFile << sDMCIt->at(j) << " ";
            }
            OutFile << "\n";
         }
         Output::Out() << "Wrote Similarity Matrix to " << FileName.getValue() << "\n";
      }
   }

   return;
}

void Test(ArgListType &arg) //AT_FUN
{
#if defined(TIMING_MODE)
   long long start,
        finish;
#endif

   // process the function arguments
   static string FunctionName = "Test";
   static int argunset = true;
   static StrArg SeqName("-name", "sequence name", "{zeros}");
   static IntArg TimeSteps("-time", "number of time steps to test");
   static IntArg StartPat("-Pstart", "start pattern", 1);
   static IntArg EndPat("-Pend", "end pattern {-1 gives end of sequence}", -1);
   static IntArg StartStep("-begin", "time step of testing on which"
                            "\n\t\t\t to begin sequence presentation", 1);
   static StrArg Analysis("-analysis", "filename of analysis output", "{no analysis}");
   // NetType defaults to -nocomp(0)
   static FlagArg NetType("-comp", "-nocomp",
                           "competitive versus variable activity", 0);
   static StrArgList CellRecording("-cellrec", 
                  "Records the occurance of synaptic events to specific neuron.(with weight) Format of the parameter: # in list (2*# of neurons) and [cell#, filename]*",true);
   static StrArgList NoRecordList("-norecord",
                  "List of datatypes to not get data for. This is useful only for reducing memory usage.",true);
   static FlagArg InhLearn("-inhlearn", "-noinhlearn", "inhibitory weight modification", 0);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("@Test( ... ) runs the network for 1 trial without updating the weights.\n"
          "Sets the TestingBuffer to the output sequence.\n"
          "Sets TestingActivity to a matrix containing the activity of each timestep.\n"
          "Sets TestingThresholds to a matrix containing the threshold of each timestep.\n"
          "Sets TestingBusLines to a matrix containing buslines of each timestep.\n"
          "Sets TestingIntBusLines to a matrix containing integrated buslines of each timestep.\n"
          "Sets TestingKWeights to a matrix containing inhibitory weights of each timestep.\n"
          "Sets TestingFBInternrnExc to a vector containing the feedback\n"
            "\tinhibition for each timestep of the last training trial.\n"
          "Sets TestingFFInternrnExc to a vector containing the feedforward\n"
            "\tinhibition for each timestep of the last training trial.\n"
          "Sets TestingInhibitions to a matrix containing the inhibitions of\n"
            "\teach neuron for each timestep of the last training trial.\n"
          "Sets TestingIzhV to a matrix containing the Izhikevich v values of\n"
            "\teach neuron for each timestep of the last training trial.\n"
          "Sets TestingIzhU to a matrix containing the Izhikevich u values of\n"
            "\teach neuron for each timestep of the last training trial.\n"
          "Sets AveThreshold to the average threshold.\n"
          "Sets AveTestAct to the average activity.\n");
      ComL.StrSet(2, &SeqName, &Analysis);
      ComL.IntSet(4, &TimeSteps, &StartPat, &EndPat, &StartStep);
      ComL.FlagSet(2, &NetType, &InhLearn);
      ComL.StrListSet(2, &CellRecording, &NoRecordList);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   const int ntst = TimeSteps.getValue();
   const int StartPrompt = StartStep.getValue();
   const bool davesRule = InhLearn.getValue();
   int StartLocation = StartPat.getValue();
   int EndLocation = EndPat.getValue();

   TrainingNetwork = false;

   // Save w_iI values
   if (davesRule) {
      for (PopulationIt pIt = Population::Member.begin();
           pIt != Population::Member.end(); ++pIt) {
         pIt->saveInhState();
		}
   }

   // check to make sure that CreateNetwork was called
   if (!program::Main().getNetworkCreated()) {
      CALL_ERROR << "Error: You must call @CreateNetwork() before you can "
                 << "Train or Test." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   UIPtnSequence Seq;
   bool useSeq = false;
   if (SeqName.getValue() == "{zeros}") {
      StartLocation = 1;
      EndLocation = ntst;
   } else {
      useSeq = true;
      Seq = SystemVar::getSequence(SeqName.getValue(), FunctionName, ComL);
      if (EndLocation == -1) {
         EndLocation = Seq.size();
      }
      if ((StartLocation < 1) || (EndLocation > static_cast<int>(Seq.size()))
          || (StartLocation > EndLocation)) {
         CALL_ERROR << "Error in " << FunctionName <<
            " : invalid Start and End Times : " << StartLocation << " , " << EndLocation
            << " for sequence of length " << Seq.size() << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
   }

   const int PtnDuration = EndLocation - StartLocation + 1;
   const int EndPrompt = std::min(StartPrompt + PtnDuration - 1, ntst);
   const int PromptDuration = EndPrompt - StartPrompt + 1;

   if (ntst < 1) {
      CALL_ERROR << "Error in " << FunctionName <<
         " : invalid number of time steps : " << ntst << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if ((StartPrompt < 1) || (StartPrompt > ntst)) {
      CALL_ERROR << "Error in " << FunctionName
                 << " : invalid begining time step : " << StartPrompt
                 << " with time steps " << ntst
                 << " and prompt duration " << PromptDuration << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   int  DoAnalysis = false;
   ofstream analysisFile;

   // NetType's default property is competitive, meaning that it will
   // return true if it is set to competitive.
   bool CompTest = NetType.getValue();

   if (Analysis.getValue() != "{no analysis}") {
      string FileToOpen = MULTIPROCFILESUFFIX(Analysis.getValue());
      analysisFile.open(FileToOpen.c_str(), ios::app);
      if (!analysisFile) {
         CALL_ERROR << "Error in " << FunctionName << " : Could not open file "
            << Analysis.getValue() << " for writing." << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
      DoAnalysis = true;
   }
   // begin testing
   UIPtnSequence Testing;
   UIPtnSequence Externals;
   DataMatrix BusLines;
   DataMatrix IntBusLines;
   DataMatrix KWeights;
   DataMatrix Inhibitions;
   DataMatrix IzhVValues;
   DataMatrix IzhUValues;
   DataMatrix FBInternrnExcs;
   DataMatrix FFInternrnExcs;
   DataList TestActVect(TimeSteps.getValue());
   DataList TestThreshVect(TimeSteps.getValue());
	DataMatrix TestThresholds;

   // Copy input information into linked list array x_input
   int  SumExtFired = 0;
   int  PatternCount = 0;
   const float xNoise = SystemVar::GetFloatVar("xTestingNoise");
   const float xNoiseF = SystemVar::GetFloatVar("xTestingNoiseF");
   list<xInput> xin = GenerateInputSequence(Seq, xNoise, xNoiseF,
                                            SumExtFired, PatternCount);

   double SumAveAct = 0.0L;
   float SumThresh = 0.0f;

   IFROOTNODE {
      // Presenting Sequence
      Output::Out() << "Testing " << (CompTest ? "competitively" : "free-running")
                    << " with Sequence " << SeqName.getValue() << " (" << StartLocation
                    << ".." << EndLocation << ") for " << ntst << " rep(s) beginning at step "
                    << StartPrompt << ":" << std::endl;
#if !defined(TIMING_MODE)
      Output::Out() << "1 " << flush;
#endif
   }

#if defined(TIMING_MODE)
   start = rdtsc();
#endif

   // Check to reset the t = 0
   if (SystemVar::GetIntVar("Reset")) {
      ResetSTM();
   }
	
	if(SystemVar::GetIntVar("VariableThreshold") != 0) {
		initializeThreshold();
	}

   xInput curPattern;
   xInputListCIt ptnIt = xin.begin();
   if (StartLocation > int(xin.size())) {
      CALL_ERROR << "Error in " << FunctionName
                 << ": Start pattern is after sequence end!" << endl << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   vector<bool> RecordIdxList = vector<bool>(10, true);
   for (unsigned int iNR = 0; iNR < NoRecordList.size(); ++iNR) {
      if (strcmp(NoRecordList[iNR].c_str(),"TestingBuffer")==0) {
         RecordIdxList[0] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingExtBuffer")==0) {
         RecordIdxList[1] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingBusLines")==0) {
         RecordIdxList[2] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingIntBusLines")==0) {
         RecordIdxList[3] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingKWeights")==0) {
         RecordIdxList[4] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingInhibitions")==0) {
         RecordIdxList[5] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingFBInternrnExc")==0) {
         RecordIdxList[6] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingFFInternrnExc")==0) {
         RecordIdxList[7] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingActivity")==0) {
         RecordIdxList[8] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TestingThresholds")==0) {
         RecordIdxList[9] = false;
      }
   }
   // Advance the current input pattern to the starting location
   for (int i3 = 1; i3 < StartLocation ; i3++, ptnIt++) {}
#if !defined(TIMING_MODE)
   int  MultipleOfTen = 1;
#endif
   for (int i4 = 1; i4 <= ntst; i4++) {
		if(SystemVar::GetIntVar("VariableThreshold") != 0) {
		   updateTimeSinceFired();
			setThreshold();
		}

#if !defined(TIMING_MODE)
      IFROOTNODE {
         if (MultipleOfTen++ == 10) {
            Output::Out() << i4 << " " << flush;
            MultipleOfTen = 1;
         }
      }
#endif
      
      if ((i4 <= int(xin.size()))
          && (StartPrompt <= i4 && i4 <= EndPrompt)) {
         curPattern = *(ptnIt++);
      } else {
         curPattern = xAllZeros;
      }

      bool doCompPresent = CompTest;
#if defined(PARENT_CHILD)
      // For parent/child mode, parent figures out who fires
      IFCHILDNODE doCompPresent = false;
#endif

      if (doCompPresent) {
         CompPresent(curPattern, true);
      } else {
         Present(curPattern, IzhVValues, IzhUValues, davesRule, false);
      }

      IFROOTNODE {     
         // Do single cell recording - AG
         int  SCRArgs = CellRecording.size();
         if (SCRArgs%2) { // We got odd # of arguments
            Output::Err()<<"Odd number of arguments to Cell Recording. "
                  "Number of arguments must be even as they are given as pair"
                  " of cell,filename."<<endl;
            exit(EXIT_FAILURE);
         }
         // Do the recording
         for (int i = 0; i<SCRArgs; i+=2)
            RecordSynapticFiring(atoi(CellRecording[i].c_str()),CellRecording[i+1]);
      }

      // Update the various buffers
      UpdateBuffers(Testing, Externals, BusLines, IntBusLines, KWeights,
                    Inhibitions, FBInternrnExcs, FFInternrnExcs, TestActVect,
                    TestThresholds, i4 - 1, curPattern, RecordIdxList);
      SumAveAct += TestActVect.at(i4 - 1);
      //SumThresh += Threshold;
      if (DoAnalysis) {
         analysisFile << i4 << "\t" << TestActVect.at(i4 - 1) << "\n";
      }
   }

#if !defined(TIMING_MODE)
   IFROOTNODE {
      if (MultipleOfTen != 1) {
         Output::Out() << ntst;
      }
      Output::Out() << std::endl;
   }
#endif

   double aveTestAct = SumAveAct / static_cast<double>(ntst);
   double aveTestExt = static_cast<double>(SumExtFired) /
                            static_cast<double>(ni * ntst);

   //FIXME: This must be some kind of hack!
   if (aveTestAct < aveTestExt) {
      aveTestExt = aveTestAct;
   }

   SystemVar::SetFloatVar("AveTestAct", static_cast<float>(aveTestAct));
   SystemVar::SetFloatVar("AveTestExt", static_cast<float>(aveTestExt));
   SystemVar::SetFloatVar("AveTestInt", static_cast<float>(aveTestAct - aveTestExt));
   //SystemVar::SetFloatVar("AveThreshold", SumThresh / ntst);

   if (RecordIdxList[0]) SystemVar::insertSequence("TestingBuffer", Testing);
   if (RecordIdxList[1]) SystemVar::insertSequence("TestingExtBuffer", Externals);
   if (RecordIdxList[2]) SystemVar::insertData("TestingBusLines", BusLines, DLT_matrix);
   if (RecordIdxList[3]) SystemVar::insertData("TestingIntBusLines", IntBusLines, DLT_matrix);
   if (RecordIdxList[4]) SystemVar::insertData("TestingKWeights", KWeights, DLT_matrix);
   if (RecordIdxList[5]) SystemVar::insertData("TestingInhibitions", Inhibitions, DLT_matrix);
   if (RecordIdxList[6]) SystemVar::insertData("TestingFBInternrnExc", FBInternrnExcs, DLT_matrix);
   if (RecordIdxList[7]) SystemVar::insertData("TestingFFInternrnExc", FFInternrnExcs, DLT_matrix);
   const bool trackIzhBuffs = (SystemVar::GetIntVar("IzhTrackData") != 0);
   if (trackIzhBuffs) {
      SystemVar::insertData("TestingIzhV", IzhVValues, DLT_matrix);
      SystemVar::insertData("TestingIzhU", IzhUValues, DLT_matrix);
   }

   if (RecordIdxList[8]) {
      DataMatrix TempTAVMat;
      TempTAVMat.push_back(TestActVect);
      SystemVar::insertData("TestingActivity", TempTAVMat, DLT_matrix);
   }

   if (RecordIdxList[9]) {
      //DataMatrix TempTTMat;
      //TempTTMat.push_back(TestThreshVect);
      SystemVar::insertData("TestingThresholds", TestThresholds, DLT_matrix);
   }

   // Save w_iI values
   if (davesRule) {
      for (PopulationIt pIt = Population::Member.begin();
           pIt != Population::Member.end(); ++pIt) {
         pIt->restoreInhState();
		}
   }

#if defined(TIMING_MODE)
   finish = rdtsc();
   IFROOTNODE
      * Output::Out() << "Elapsed testing time = " << (finish - start) * 1.0 / TICKS_PER_SEC
                      << " seconds" << std::endl;
#endif

   return;
}

void Train(ArgListType &arg) //AT_FUN
{
#if defined(TIMING_MODE)
   long long start,
        finish;
#endif

   // process the function arguments
   static string FunctionName = "Train";
   static int argunset = true;
   static StrArg SeqName("-name", "sequence name");
   static IntArg Trials("-trials", "number of trials to train");
   static StrArg AnaFile("-analysis", "filename of analysis output", "{no analysis}");
   // NetType defaults to -nocomp(0)
   static FlagArg NetType("-comp", "-nocomp",
                           "competitive versus variable activity", 0);
   // DoWijAna defaults to -nowijana (0)
   static FlagArg DoWijAna ("-wijana", "-nowijana", "do weight analysis", 0);
   static StrArgList AnaCalls("-anacalls",
                   "# in list and list of analysis variables to update", true);
   static StrArgList CellRecording("-cellrec", 
                  "Records the occurance of synaptic events to specific neuron.(with weight) Format of the parameter: # in list (2*# of neurons) and [cell#, filename]*",true);
   static StrArgList NoRecordList("-norecord",
                  "List of datatypes to not get data for. This is useful only for reducing memory usage.",true);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@Train( ... ) runs the network.\n"
      "Sets the TrainingBuffer to the last output sequence.\n"
      "Sets TrainingActivity to a matrix containing the activity of each\n"
         "\ttimestep of the last training trial.\n"
      "Sets TrainingThresholds to a matrix containing the threshold of each\n"
         "\ttimestep of the last training trial.\n"
      "Sets TrainingBusLines to a matrix containing buslines of each timestep\n"
         "\tof the last training trial.\n"
      "Sets TrainingIntBusLines to a matrix containing integrated buslines of\n"
         "\teach timestep of the last training trial.\n"
      "Sets TrainingKWeights to a matrix containing inhibitory weights of\n"
         "\teach timestep of the last training trial.\n"
      "Sets TrainingInhibitions to a matrix containing the inhibitions of\n"
         "\teach neuron for each timestep of the last training trial.\n"
      "Sets TrainingFBInternrnExc to a vector containing the feedback\n"
         "\tinhibition for each timestep of the last training trial.\n"
      "Sets TrainingFFInternrnExc to a vector containing the feedforward\n"
         "\tinhibition for each timestep of the last training trial.\n"
      "Sets TrainingIzhV to a matrix containing the Izhikevich v values of\n"
         "\teach neuron for each timestep of the last training trial.\n"
      "Sets TrainingIzhU to a matrix containing the Izhikevich u values of\n"
         "\teach neuron for each timestep of the last training trial.\n"
      "Sets AveTrainAct to the average activity of the last trial.\n"
      "Sets AveTrainExt to the average external activity of the last trial.\n"
      "Sets AveTrainInt to the average internal activity of the last trial.\n"
      "Sets AveThreshold to the average threshold of the last trial.\n"
      "If Reset is nonzero, then the network is Reset before each trial.\n"
      "In this case, if ResetPattern is \"\" then Reset picks a random\n"
      "pattern with activity ResetAct, otherwise Reset uses the first\n"
      "pattern found in the sequence whose name is given by ResetPattern.\n");
      ComL.StrSet(2, &SeqName, &AnaFile);
      ComL.IntSet(1, &Trials);
      ComL.FlagSet(2, &NetType, &DoWijAna);
      ComL.StrListSet(3, &AnaCalls, &CellRecording, &NoRecordList);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   TrainingNetwork = true;

   // build the analysis list
   int  AnaNum = AnaCalls.size();
   ArgListType AnaArgs(AnaNum, ArgType("", false));
   for (int anai = 0; anai < AnaNum; anai++) {
      AnaArgs.at(anai).first = AnaCalls[anai];
   }
   // check to make sure that CreateNetwork was called
   if (!program::Main().getNetworkCreated()) {
      CALL_ERROR << "Error: You must call @CreateNetwork() before you can "
                 << "Train or Test." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   UIPtnSequence Seq = SystemVar::getSequence(SeqName.getValue(), FunctionName, ComL);
   if (Trials.getValue() < 1) {
      CALL_ERROR << "Error in " << FunctionName <<
         " : invalid number of trials : " << Trials.getValue() << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   int  DoAnalysis = false;
   ofstream analysisFile;

   IFROOTNODE {    //changed - dws - 7/17/2003
      if (AnaFile.getValue() != "{no analysis}") {
         //pfr 12/99 Concat the PE id to end of file so that each
         //    PE writes to its own file
         string FileToOpen = MULTIPROCFILESUFFIX(AnaFile.getValue());
         analysisFile.open(FileToOpen.c_str(), ios::app);
         // end pfr 12/99
         long fileSize = analysisFile.tellp();
         analysisFile.fill(' ');
         analysisFile.precision(10);
         analysisFile.setf(ios_base::showpoint, ios_base::floatfield);
         if (fileSize == 0) {
            analysisFile << std::setw(6) << "Trial ";
            analysisFile << std::setw(16) << "AveTrainAct";
            analysisFile << std::setw(14) << "AvgWij";
            analysisFile << std::setw(14) << "AvgWij0";
            analysisFile << std::setw(14) << "FracZeroWij";
            analysisFile << std::setw(20) << "SumWij";
            analysisFile << std::setw(14) << "SumOf0s";
            analysisFile << std::setw(10) << "Num0s";
            analysisFile << std::setw(20) << "sumPyrToInternrnWt";
            analysisFile << std::setw(20) << "numPyrToInternrnWt0" << std::endl;
         }

         if (!analysisFile) {
            CALL_ERROR << MSG << "Error in " << FunctionName <<
               " : Could not open file " << AnaFile.getValue() << " for writing." << ERR_WHERE;
            ComL.DisplayHelp(Output::Err());
            exit(EXIT_FAILURE);
         }
         DoAnalysis = true;
#if defined(MULTIPROC)
         DoWijAna.setValue(false); //changed - dws - 7/17/2003
#else
         DoWijAna.setValue(true);
#endif
      }
   }

   // NetType's default property is competitive, meaning that it will
   // return true if it is set to competitive.
   int  CompTrain = NetType.getValue();

   const int ntrn = Trials.getValue();
   UIPtnSequence Training;
   UIPtnSequence Externals;
   DataMatrix BusLines;
   DataMatrix IntBusLines;
   DataMatrix KWeights;
   DataMatrix Inhibitions;
   DataMatrix IzhVValues;
   DataMatrix IzhUValues;
   DataMatrix FBInternrnExcs;
   DataMatrix FFInternrnExcs;
   DataMatrix TrainThresholds;
   DataList TrainingAct;

   // Copy input information into linked list array x_input
   int  SumExtFired = 0;
   int  PatternCount = 0;

   // Presenting Sequence

   IFROOTNODE {
      if (CompTrain) {
         Output::Out() << "Training competitively with Sequence " << SeqName.getValue()
            << " for " << ntrn << " rep(s):" << std::endl;
      } else {
         Output::Out() << "Training free-running with Sequence " << SeqName.getValue()
            << " for " << ntrn << " rep(s):" << std::endl;
      }
   }

#if defined(TIMING_MODE)
   IFROOTNODE start = rdtsc();
#else
   IFROOTNODE Output::Out() << "1 " << flush;
#endif

#if !defined(TIMING_MODE)
   // temporary counter
   int  MultipleOfTen = 1;
#endif
   // for timing purposes
#  if defined(TIMING_MODE2)
   clock_t t1 = 0,
      t2 = 0;
#endif
   unsigned int ndx = 0;
   int  SumNumFired = 0;
   int  SumNumTies = 0;
   float SumThresh = 0.0f;
   SystemVar::SetIntVar("NumTiesPicked", 0);
   float xNoise = SystemVar::GetFloatVar("xNoise");
   float xNoiseF = SystemVar::GetFloatVar("xNoiseF");

   vector<bool> RecordIdxList = vector<bool>(10, true);
   for (unsigned int iNR = 0; iNR < NoRecordList.size(); ++iNR) {
      if (strcmp(NoRecordList[iNR].c_str(),"TrainingBuffer")==0) {
         RecordIdxList[0] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingExtBuffer")==0) {
         RecordIdxList[1] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingBusLines")==0) {
         RecordIdxList[2] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingIntBusLines")==0) {
         RecordIdxList[3] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingKWeights")==0) {
         RecordIdxList[4] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingInhibitions")==0) {
         RecordIdxList[5] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingFBInternrnExc")==0) {
         RecordIdxList[6] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingFFInternrnExc")==0) {
         RecordIdxList[7] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingActivity")==0) {
         RecordIdxList[8] = false;
      } else if (strcmp(NoRecordList[iNR].c_str(),"TrainingThresholds")==0) {
         RecordIdxList[9] = false;
      }
   }

#if defined(TIMING_P2P)
   trials = 0;
   total_time = 0;
#endif

   for (int i3 = 1; i3 <= ntrn; i3++) {
	   if(SystemVar::GetIntVar("VariableThreshold") != 0)
			initializeThreshold();

      list<xInput> xin = GenerateInputSequence(Seq, xNoise, xNoiseF,
                                               SumExtFired, PatternCount);
      // reset before each training trial
      if (SystemVar::GetIntVar("Reset")) {
         ResetSTM();
      }

      SystemVar::IncIntVar("TrainingCount");  // increment the training count

#if !defined(TIMING_MODE)
      IFROOTNODE {
         // mod takes a while so use a temporary counter to check for 10s
         if (MultipleOfTen++ == 10) {
            Output::Out() << i3 << " " << flush;
            MultipleOfTen = 1;
         }
      }
#endif
      SumNumFired = 0;
      SumThresh = 0.0f;
      SumNumTies = 0;      

      if (i3 == ntrn) {
         //TrainThreshVect = DataList(xin.size());
         TrainingAct = DataList(xin.size());
      }
      for (xInputListCIt ptnIt = xin.begin(); ptnIt != xin.end(); ptnIt++) {
         // For timing
         IFROOTNODE {
#  if defined(TIMING_MODE2)
            if (t1) {
               t2 = clock();
               printf(" [%.4gms] ", 1000.0 * static_cast<float>(t2 - t1) /
                                     (CLOCKS_PER_SEC));
            }
#endif
         }

#if defined(MULTIPROC)
         
#endif

         bool doCompPresent = CompTrain;
#if defined(PARENT_CHILD)
         // For parent/child mode, parent figures out who fires
         IFCHILDNODE doCompPresent = false;
#endif

			if(SystemVar::GetIntVar("VariableThreshold") != 0) {
				updateTimeSinceFired();
				setThreshold();
			}
         
			if (doCompPresent) {
            CompPresent(*ptnIt, false);
         } else {
            Present(*ptnIt, IzhVValues, IzhUValues, true, true);
         }

         // Do single cell recording - AG
         // If root or if single processor
         IFROOTNODE {     
            int  SCRArgs = CellRecording.size();
            if (SCRArgs%2) { // We got odd # of arguments
               Output::Err()<<"Odd number of arguments to Cell Recording. "
                     "Number of arguments must be even as they are given as pair"
                     " of cell,filename."<<endl;
               exit(EXIT_FAILURE);
            } else {
               // Do the recording
               for (int i = 0; i<SCRArgs; i+=2)
                  RecordSynapticFiring(atoi(CellRecording[i].c_str()),CellRecording[i+1]);
            }
         }

         // For timing
#  if defined(TIMING_MODE2)
         IFROOTNODE t1 = clock();
#endif
         // If last training cycle, then add the firing to the buffer
         if (i3 == ntrn) {
            UpdateBuffers(Training, Externals, BusLines, IntBusLines, KWeights,
                          Inhibitions, FBInternrnExcs, FFInternrnExcs, TrainingAct,
                          TrainThresholds, ndx, *ptnIt, RecordIdxList);
            ndx++;
         }
         SumThresh += Threshold;
         SumNumFired += Fired[justNow].size();
         SumNumTies += TotalNumTied;
      }
#  if defined(TIMING_MODE2)
      t1 = 0;
#endif
      SystemVar::SetFloatVar("AveThreshold", SumThresh / PatternCount);
      SystemVar::SetFloatVar("AveTrainTies", static_cast<float>(SumNumTies) /
                         static_cast<float>(PatternCount));
      float aveTrainAct = static_cast<float>(SumNumFired) /
                            static_cast<float>(ni * PatternCount);
      float aveTrainExt = static_cast<float>(SumExtFired) /
                            static_cast<float>(ni * PatternCount);

      //FIXME: This must be some kind of hack!
      if (aveTrainAct < aveTrainExt) {
         aveTrainExt = aveTrainAct;
      }

      SystemVar::SetFloatVar("AveTrainAct", aveTrainAct);
      SystemVar::SetFloatVar("AveTrainExt", aveTrainExt);
      SystemVar::SetFloatVar("AveTrainInt", aveTrainAct - aveTrainExt);
      
      if (DoWijAna.getValue()) {
         float TotalSumOfWeights = 0.0f;
         float TotalSumOfZeros = 0.0f;
         int  TotalNumberOfZeros = 0;
#if defined(MULTIPROC)
         // FIXME: Needs implementing for parallel version
         //float zeroCutOff = SystemVar::GetFloatVar("ZeroCutOff");
         //for (int j2 = 0; j2 < ni; j2++) {
         //  for (int c1 = 0; c1 < FanInCon.at(j2); c1++) {
         //      if (inMatrix[j2][c1].weight >= zeroCutOff) {
         //         wSum += inMatrix[j2][c1].getWeight();
         //      } else {
         //         wZero += inMatrix[j2][c1].getWeight();
         //         ZeroCount++;
         //      }
         //   }
         //}
#else
         float zeroCutOff = SystemVar::GetFloatVar("ZeroCutOff");
         for (unsigned int j2 = 0; j2 < ni; j2++) {
            DendriticSynapse * dendriticTree = inMatrix[j2];
            for (unsigned int c = 0; c < FanInCon.at(j2); ++c) {  
               float tmpWt;
               if ((tmpWt = dendriticTree[c].getWeight()) >= zeroCutOff) {
                  TotalSumOfWeights += tmpWt;
               } else {
                  TotalSumOfZeros += tmpWt;
                  TotalNumberOfZeros++;
               }
            }
         }
#endif

         SystemVar::SetFloatVar("AveWij", TotalSumOfWeights /
            static_cast<float>(NumNetworkCon - TotalNumberOfZeros));
         SystemVar::SetFloatVar("AveWij0", (TotalSumOfWeights + TotalSumOfZeros) /
            static_cast<float>(NumNetworkCon));
         SystemVar::SetFloatVar("FracZeroWij", static_cast<float>(TotalNumberOfZeros) /
            static_cast<float>(NumNetworkCon));

         if (DoAnalysis) {
            double sumPyrToInternrnWt = 0.0L;
            int numPyrToInternrnWt0 = 0;
            for (PopulationIt pIt = Population::Member.begin();
                 pIt != Population::Member.end(); ++pIt) {
               DataList KFBWeights = pIt->getKFBWeights();
               for (DataListCIt it = KFBWeights.begin(); it != KFBWeights.end(); ++it) {
                  sumPyrToInternrnWt += *it;
                  if (*it < verySmallFloat) ++numPyrToInternrnWt0;
               }
            }
            analysisFile << std::setw(6) << SystemVar::GetIntVar("TrainingCount")
               << std::setw(16) << showpoint << SystemVar::GetFloatVar("AveTrainAct")
               << std::setw(14) << SystemVar::GetFloatVar("AveWij") 
               << std::setw(14) << SystemVar::GetFloatVar("AveWij0")
               << std::setw(14) << SystemVar::GetFloatVar("FracZeroWij")
               << std::setw(20) << TotalSumOfWeights
               << std::setw(14) << TotalSumOfZeros
               << std::setw(10) << TotalNumberOfZeros
               << std::setw(20) << sumPyrToInternrnWt
               << std::setw(20) << numPyrToInternrnWt0 << std::endl;
         }
      }

      if (AnaNum) {
         Analysis(AnaArgs);
      }
   }


///The training trial just ended. If using the spikeTiming rule, here's where we change the weights.

   int spikeRule = SystemVar::GetIntVar("spikeTimingRule");
   if ( spikeRule == 1) {
		//use the spikeRule
 		//Output::Out() << "inside spikeRule" << std::endl;
		UpdateWeightsSpike(Training);
   }

#if !defined(TIMING_MODE)
   IFROOTNODE {
      if (MultipleOfTen != 1) {
         Output::Out() << ntrn << " ";
      }
      Output::Out() << "(" << SystemVar::GetIntVar("TrainingCount") << ")" << std::endl;
   }
#endif

   if (RecordIdxList[0]) SystemVar::insertSequence("TrainingBuffer", Training);
   if (RecordIdxList[1]) SystemVar::insertSequence("TrainingExtBuffer", Externals);
   if (RecordIdxList[2]) SystemVar::insertData("TrainingBusLines", BusLines, DLT_matrix);
   if (RecordIdxList[3]) SystemVar::insertData("TrainingIntBusLines", IntBusLines, DLT_matrix);
   if (RecordIdxList[4]) SystemVar::insertData("TrainingKWeights", KWeights, DLT_matrix);
   if (RecordIdxList[5]) SystemVar::insertData("TrainingInhibitions", Inhibitions, DLT_matrix);
   if (RecordIdxList[6]) SystemVar::insertData("TrainingFBInternrnExc", FBInternrnExcs, DLT_matrix);
   if (RecordIdxList[7]) SystemVar::insertData("TrainingFFInternrnExc", FFInternrnExcs, DLT_matrix);
   const bool trackIzhBuffs = (SystemVar::GetIntVar("IzhTrackData") != 0);
   if (trackIzhBuffs) {
      SystemVar::insertData("TrainingIzhV", IzhVValues, DLT_matrix);
      SystemVar::insertData("TrainingIzhU", IzhUValues, DLT_matrix);
   }

   if (RecordIdxList[8]) {
      DataMatrix TempTAVMat;
      TempTAVMat.push_back(TrainingAct);
      SystemVar::insertData("TrainingActivity", TempTAVMat, DLT_matrix);
   }

   if (RecordIdxList[9]) {
      //DataMatrix TempTTMat;
      //TempTTMat.push_back(TrainThreshVect);
      SystemVar::insertData("TrainingThresholds", TrainThresholds, DLT_matrix);
   }

#if defined(TIMING_MODE)
   IFROOTNODE {
      finish = rdtsc();
      Output::Out() << "Elapsed training time = " << (finish - start) * 1.0 / TICKS_PER_SEC
                    << " seconds" << std::endl;
   }
#endif

#if defined(TIMING_P2P)
   Output::Out() << "Average exchange time  = " << (total_time) / (double)trials
                 << " seconds over " << trials << " trials" << endl;
#endif

   return;
}

//******************
// Caret Functions
//******************

string PickSeq (ArgListType &arg) //CARET_FUN
{
   // process the function arguments
   static string FunctionName = "PickSeq";
   static int argunset = true;
   static StrArgList SeqList("-seq", "# of sequences and list of sequences");
   static DblArgList ProbList("-prob", "# of sequences and ratio to pick sequence");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet
         ("^PickSeq( ... ) randomly selects a sequence name(or any string)\n"
          "from a list. The ratio to pick each sequence can also be specified.\n");
      ComL.StrListSet(1, &SeqList);
      ComL.DblListSet(1, &ProbList);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (SeqList.size() < 1) {
      CALL_ERROR << "Error in " << FunctionName << " : no strings given." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (SeqList.size() == 1) {
      return SeqList[0];
   }

   if (ProbList.size() == 0) {
      return SeqList[program::Main().chooseItem(SeqList.size())];
   }
   if (ProbList.size() != SeqList.size()) {
      CALL_ERROR << "Error in " << FunctionName <<
         " : string list and prob list " << "have different sizes." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }

   double sum = 0.0L;
   for (unsigned int i = 0; i < ProbList.size(); i++) {
      sum += ProbList[i];
      if (ProbList[i] < 0.0f) {
         CALL_ERROR << "Error in " << FunctionName <<
            " : invalid probability : " << ProbList[i] << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
   }
   if (fabs(sum) < verySmallFloat) {
      return SeqList[program::Main().chooseItem(SeqList.size())];
   }
   
   double testnum = sum*program::Main().chooseWeightedItem();
   if (testnum > sum) testnum = sum; // just in case there's weirdness in floating point mult
   for (unsigned int i1 = 0; i1 < ProbList.size(); i1++) {
      testnum -= ProbList[i1];
      if (testnum <= 0) {
         return SeqList[i1];
      }
   }   
   
   CALL_ERROR << "Error in " << FunctionName << " algorithm." << ERR_WHERE;
   exit(EXIT_FAILURE);

   // never get here
   return "";
}

string Num2Int(ArgListType &arg) //CARET_FUN
{
   if (arg.size() != 1) {
      CALL_ERROR << "Error in Num2Int: expects exactly one argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "Num2Int -help\n\n"
         "^Num2Int( ... ) converts a numerical string into an integer.\n"         
         "Unlike most functions, Num2Int does not require any preceeding flags.\n"
         << std::endl;
      exit(EXIT_FAILURE);
   }

   return to_string(from_string<int>(arg.at(0).first));
}

void Context(ArgListType &arg) //AT_FUN
{
   //@Context is a root-only function
   IFROOTNODE {
      // process the function arguments
      static string FunctionName = "Context";
      static int argunset = 1;
      static IntArg StartNeuron("-Nstart", "start neuron", 1);
      static IntArg EndNeuron("-Nend", "end neuron { -1 gives last neuron}", -1);
      static IntArg StartTime("-Tstart", "start time", 1);
      static IntArg EndTime("-Tend", "end time {-1 gives end time}", -1);
      static IntArg Tolerance("-tol", "tolerance", 0);
      static StrArg OutFile("-file", "output filename", "{no file}");
      static StrArg FiringSequence("-name", "firing sequence name", "TrainingBuffer");
      // DoSummary defaults to -sum(1)
      static FlagArg DoSummary("-sum", "-nosum", "show a summary", 1);
      // DoLoop defaults to -noloop(0)
      static FlagArg DoLoop("-loop", "-noloop", "calculate for a looped sequence", 0);
      static CommandLine ComL(FunctionName);
      if (argunset) {
         ComL.HelpSet
            ("@Context( ... ) calculates the context lengths of the neurons\n"
             "in a sequence. The output can go to a file and also to the screen.\n"
             "Several variables get calculated in context:\n"
             "AveConLen gives the average context length (excluding\n"
             "unused neurons) of the last call to @Context(...).\n"
             "AveConLen0 gives the average context length (including\n"
             "unused neurons as context zero) of the last call to @Context(...).\n"
             "FracRefired gives the fraction of neurons that did not satisfy\n"
             "the definition of context in the last call to @Context(...).\n"
             "FracUnused gives the fraction of neurons that did not fire\n"
             "in the last call to @Context(...).\n"
             "NumRefired gives the number of neurons that did not satisfy\n"
             "the definition of context in the last call to @Context(...).\n"
             "NumUnused gives the number of neurons that did not fire\n"
             "in the last call to @Context(...).\n"
             "VarConLen gives the variance in context length (excluding\n"
             "unused neurons) of the last call to @Context(...).\n"
             "VarConLen0 gives the variance in context length (including\n"
             "unused neurons as context zero) of the last call to @Context(...).\n"
             "ContextBuffer is a two vector matrix that contains the context\n"
             "distribution of the last call to @Context(...). The first vector contains\n"
             "the numbers in the distribution and the second vector contains the\n"
             "ratio of the numbers to the number of neurons.\n"
             "The -loop flag will cause the context algorithm to assume a looped\n"
             "sequence.  This means that the end of the sequence is assumed to hook\n"
             "to the beginning.\n\n");
         ComL.IntSet(5, &StartNeuron, &EndNeuron, &StartTime, &EndTime, &Tolerance);
         ComL.StrSet(2, &OutFile, &FiringSequence);
         ComL.FlagSet(2, &DoSummary, &DoLoop);
         argunset = 0;
      }
      ComL.Process(arg, Output::Err());

      Sequence fSeq = UISeqToPtnSeq(SystemVar::getSequence(FiringSequence.getValue(), FunctionName, ComL));

      int startT, endT, startN, endN;
      setMatrixRange(startT, endT, startN, endN, StartTime, EndTime,
                     StartNeuron, EndNeuron, fSeq, ComL, FunctionName);

      if (Tolerance.getValue() < 0) {
         CALL_ERROR << "Error in " << FunctionName << ": Tolerance of "
                    << Tolerance.getValue() << " is invalid." << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
      }
      // Copy the sequence into a matrix
      const unsigned int TotalNeurons = endN - startN + 1;
      const unsigned int TotalTimeSteps = endT - startT + 1;

      Sequence InvSeq = transposeMatrix(SubMatrix(fSeq, startT, endT, startN, endN));

      // Run the Context algorithm
      // figure out the longest possible context
      unsigned int ctxlen = 0;
      for (SequenceCIt nrnIt = InvSeq.begin(); nrnIt != InvSeq.end(); ++nrnIt) {
         unsigned int numfires = 0;
         for (unsigned int t = 0; t < TotalTimeSteps; ++t) {
            if (nrnIt->at(t)) numfires++;
         }
         updateMax(ctxlen, numfires);
      }
      ++ctxlen;

      // get the return vectors
      DataList ctx(ctxlen, 0.0f);
      DataList ctxdist(ctxlen);

      // loop through each neuron and tally the context, recording it in outvect
      int NumRefired = 0;
      int NumUnused = 0;
      int sum = 0;
      int nrn = 0;
      for (SequenceCIt nrnIt = InvSeq.begin(); nrnIt != InvSeq.end(); ++nrnIt, ++nrn) {
         int  context;
         bool multifire;
         getContextStats(*nrnIt, TotalTimeSteps, Tolerance.getValue(),
                         DoLoop.getValue(), multifire, context);
         if (!multifire) {
            ctx.at(context) += 1.0f;
            if (!context) {
               NumUnused++;
            } else {
               sum += context;
            }
         } else {
            NumRefired++;
         }
      }

      // do the distribution
      for (unsigned int i3 = 0; i3 < ctxlen; i3++) {
         ctxdist.at(i3) = ctx.at(i3) / static_cast<float>(TotalNeurons);
      }
      if (!sum) {
         SystemVar::SetFloatVar("AveConLen", 0.0f);
         SystemVar::SetFloatVar("AveConLen0", 0.0f);
         SystemVar::SetFloatVar("VarConLen", 0.0f);
         SystemVar::SetFloatVar("VarConLen0", 0.0f);
      } else {         
         double AveConLen = static_cast<double>(sum) /
            static_cast<double>(TotalNeurons - NumUnused - NumRefired);
         double AveConLen0 = static_cast<double>(sum) /
            static_cast<double>(TotalNeurons - NumRefired);
         // calculate the variance in context length
         double VarConLen = 0.0f;
         double VarConLen0 = NumUnused * AveConLen0 * AveConLen0;
         for (unsigned int i = 1; i < ctxlen; i++) {
            const double diff = AveConLen - static_cast<double>(i);
            const double diff0 = AveConLen0 - static_cast<double>(i);
            VarConLen += ctx.at(i) * diff * diff;
            VarConLen0 += ctx.at(i) * diff0 * diff0;
         }
         // don't subtract 1 since it's a population sample
         int  denom = TotalNeurons - NumUnused - NumRefired;
         if (denom < 1) {
            VarConLen = 0.0f;
         } else {
            VarConLen /= static_cast<double>(denom);
         }
         // don't subtract 1 since it's a population sample
         denom = TotalNeurons - NumRefired;
         if (denom < 1) {
            VarConLen0 = 0.0f;
         } else {
            VarConLen0 /= static_cast<double>(denom);
         }
         SystemVar::SetFloatVar("AveConLen", static_cast<float>(AveConLen));
         SystemVar::SetFloatVar("AveConLen0", static_cast<float>(AveConLen0));
         SystemVar::SetFloatVar("VarConLen", static_cast<float>(VarConLen));
         SystemVar::SetFloatVar("VarConLen0", static_cast<float>(VarConLen0));
      }
      SystemVar::SetFloatVar("FracUnused", static_cast<float>(NumUnused) /
                         static_cast<float>(TotalNeurons));
      SystemVar::SetFloatVar("FracRefired", static_cast<float>(NumRefired) /
                          static_cast<float>(TotalNeurons));
      SystemVar::SetIntVar("NumUnused", NumUnused);
      SystemVar::SetIntVar("NumRefired", NumRefired);

      if (DoLoop.getValue()) {
         Output::Out() << "Loop ";
      }
      Output::Out() << "Context of sequence " << FiringSequence.getValue()
         << " using neurons " << startN << "..." << endN
         << " and patterns " << startT << "..." << endT
         << " with tolerance " << Tolerance.getValue() << "\n";

      // Do the summary if necessary
      if (DoSummary.getValue()) {
         // loops through distribution and displays the results on StdOut
         Output::Out() << "  size   Neurons  Proportion\n";
         for (unsigned int i = 0; i < ctxlen; i++) {
            if (fabs(ctx.at(i)) > verySmallFloat) {
               Output::Out() << std::setw(6) << i << std::setw(10)
                  << std::resetiosflags(ios::fixed | ios::showpoint)
                  << ctx.at(i) << std::setw(12)
                  << std::setiosflags(ios::fixed | ios::showpoint)
                  << std::setprecision(6) << ctxdist.at(i)
                  << std::resetiosflags(ios::fixed | ios::showpoint) << "\n";
            }
         }
         Output::Out() << std::setw(6) << "multi" << std::setw(10) << SystemVar::GetIntVar("NumRefired")
            << std::setw(12) << std::setiosflags(ios::fixed | ios::showpoint)
            << std::setprecision(6) << SystemVar::GetFloatVar("FracRefired")
            << std::resetiosflags(ios::fixed | ios::showpoint) << "\n\n";
         // displays other useful statistics
         Output::Out() << "Average Context(w/ zeros, w/o zeros): "
            << std::setiosflags(ios::fixed | ios::showpoint)
            << std::setprecision(6) << SystemVar::GetFloatVar("AveConLen0")
            << " " << std::setprecision(6) << SystemVar::GetFloatVar("AveConLen")
            << "\n\n";
         Output::Out() << "Variance Context(w/ zeros, w/o zeros): "
            << std::setiosflags(ios::fixed | ios::showpoint)
            << std::setprecision(6) << SystemVar::GetFloatVar("VarConLen0")
            << " " << std::setprecision(6) << SystemVar::GetFloatVar("VarConLen")
            << "\n" << endl;;
      }

      // Output to file
      if (OutFile.getValue() != "{no file}") {
         ofstream ctxout(OutFile.getValue().c_str());
         if (!ctxout) {
            CALL_ERROR << "Error in " << FunctionName << ": Unable to open "
                       << OutFile.getValue() << ERR_WHERE;
            ComL.DisplayHelp(Output::Err());
            exit(EXIT_FAILURE);
         }
         for (unsigned int i = 0; i < ctxlen; i++) {
            ctxout << ctx.at(i) << "\t" << ctxdist.at(i) << "\n";
         }
         Output::Out() << "Wrote Context data to " << OutFile.getValue() << "\n";
      }
      // Save the vector to the ContextBuffer
      DataMatrix ContextMat;
      ContextMat.push_back(ctx);
      ContextMat.push_back(ctxdist);
      SystemVar::insertData("ContextBuffer", ContextMat, DLT_matrix);
   }               // end nonroot-exclusion clause
   return;
}

void FileReset(ArgListType &arg) //AT_FUN
{
// Only the root node should be involved in resetting files
   IFROOTNODE {
      if (arg.size() == 0) {
         CALL_ERROR << "Error in FileReset... no arguments given" << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      if (arg.at(0).first == "-help") {
         Output::Err() << "FileReset -help\n\n"
                    "@FileReset( file1 file2 ... fileN ) deletes the given files."
                 << std::endl << std::endl;
         exit(EXIT_FAILURE);
      }      
      for (ArgListTypeIt it = arg.begin(); it != arg.end(); it++) {
         ofstream fset(it->first.c_str());
      }
   }
#if defined(MULTIPROC)
   // We still need to barrier since we can make no guarantees about
   // when this function was called for every node, e.g. consider the
   // following:
   //  Node 0 is root node, Node 1 is a non-root node running the
   //  same script.  We have the following snippet of script code:
   //
   //  @FileReset(file1);
   //  @SaveData(-from seq1 -to file1);
   //
   //  If Node 1 executes these two lines, THEN Node 0 executes the
   //  call to @FileReset, we're screwed.
   ParallelInfo::Barrier();
   #endif
   return;
}

string RandomSeed (ArgListType &arg) //CARET_FUN
{
   if (arg.size() > 0) {
      if (arg.at(0).first == "-help") {
         Output::Err() << "RandomSeed -help\n\n"
            "^RandomSeed() returns a random integer from 0 to 32767." << std::endl << std::endl;
         exit(EXIT_FAILURE);
      }
      CALL_ERROR << "Error in RandomSeed: no arguments expected." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }

   return Calc::RandomUserSeed();
}

void SeedRNG(ArgListType &arg) //AT_FUN
{
   if (arg.size() > 0) {
      if (arg.at(0).first == "-help") {
         Output::Err() << "SeedRNG -help\n\n"
            "@SeedRNG() resets the random number generators with\n"
            "current value of seed." << std::endl << std::endl;
         exit(EXIT_FAILURE);
      }
      CALL_ERROR << "Error in SeedRNG: no arguments expected." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   program::Main().setAllSeeds();
}

void CreateAnalysis(ArgListType &arg) //AT_FUN
{
   int num = arg.size();
   if (num == 0) {
      CALL_ERROR << "Error in CreateAnalysis: multiple arguments expected." << ERR_WHERE;
      arg.at(0).first = "-help";
   }
   if (arg.at(num - 1).first != "]") {
      CALL_ERROR << "Error in CreateAnalysis: ] expected here -> "
                 << arg.at(num - 1).first << ERR_WHERE;
      arg.at(0).first = "-help";
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "CreateAnalysis -help\n"
         "\n"
         "@CreateAnalysis(...) creates a running list of variables' over time.\n"
         "The syntax is:\n"
         "AnaName1 [ var1 ... varS ] ... AnaNameN [ var1 ... varT ]\n"
         "which creates N different analysis structures that can be referred to\n"
         "as AnaName1, ..., AnaNameN, respectively. Each of these structures\n"
         "has a list of variable names(in brackets) assigned to it. The brackets\n"
         "are part of the syntax. See @Analysis(...) for information about how to\n"
         "use analysis structures.\n" << std::endl << std::endl;
      exit(EXIT_FAILURE);
   }

   StrList CurrentList;
   DataMatrix CurrentAna;
   bool listCreated = false;
   string listName = "";
   for (int i = 0; i < num; i++) {
      string curArg = arg.at(i).first;
      if (curArg == "[") {
         CALL_ERROR << "Error in CreateAnalysis: Unexpected [ " << ERR_WHERE;
         arg.at(0).first = "-help";
         CreateAnalysis(arg);
      } else if (curArg == "]") {
         if (!listCreated) {
            CALL_ERROR << "Error in CreateAnalysis: Unexpected ] " << ERR_WHERE;
            arg.at(0).first = "-help";
            CreateAnalysis(arg);
         }
         SystemVar::insertAnalysis(listName, CurrentAna, CurrentList);
         CurrentList = StrList();
         CurrentAna = DataMatrix();
         listCreated = false;
      } else if (listCreated) {
         CurrentList.push_back(curArg);
         CurrentAna.push_back(DataList(0));
      } else {
         string &nextArg = arg.at(i + 1).first;
         if (nextArg != "[") {
            if (nextArg[0] == '[') {
               nextArg.erase(nextArg.begin());
               --i; // next arg is NOT [
            } else {
               CALL_ERROR << "Error in CreateAnalysis: [ expected here -> "
                          << nextArg << ERR_WHERE;
               arg.at(0).first = "-help";
               CreateAnalysis(arg);
            }
         }
         if (SystemVar::GetVarType(curArg) != 'u') {
            CALL_ERROR << "Error in CreateAnalysis: " << curArg
                       << "already exists as a data structure." << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         listCreated = true;
         listName = curArg;
         ++i; // Next token is [
      }
   }
   return;
}

void UpdateAnalysis( string name )
{
   string FunctionName = "Analysis";
   CommandLine ComL(FunctionName);

   StrList AnaName;
   DataMatrix &AnaList =
      SystemVar::getAnalysis(name, AnaName, FunctionName, ComL);
   StrListCIt nameIt = AnaName.begin();   // doesn't change here
   DataMatrixIt dataIt = AnaList.begin(); // does change here
   for (; nameIt != AnaName.end(); nameIt++, dataIt++) {
      if (dataIt == AnaList.end()) {
         CALL_ERROR << "Error in Analysis Algorithm!!" << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      char varType = SystemVar::GetVarType(*nameIt);
      if (varType == 'i') {
         dataIt->push_back(static_cast<float>(SystemVar::GetIntVar(*nameIt)));
      } else if (varType == 'f') {
         dataIt->push_back(SystemVar::GetFloatVar(*nameIt));
      } else if (varType == 's') {
         dataIt->push_back(from_string<float>(SystemVar::GetStrVar(*nameIt)));
      } else {
         CALL_ERROR << "Analysis did not find " << *nameIt << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
   }
}

void Analysis(ArgListType &arg) //AT_FUN
{
   IFROOTNODE {
      if (arg.size() == 0) {
         CALL_ERROR << "Error in Analysis: multiple arguments expected." << ERR_WHERE;
         arg.push_back(ArgType("-help", false));
      }
      if (arg.at(0).first == "-help") {
         Output::Err() << "Analysis -help\n\n"
            "@Analysis(...) updates a running list of variables' over time.\n"
            "The syntax is:\n"
            "AnaName1 ... AnaNameN\n"
            "where each AnaName is the name of an analysis structure created\n"
            "with @CreateAnalysis(...).\n" << std::endl;
         exit(EXIT_FAILURE);
      }
   }               // end nonroot-exclusion

   for (ArgListTypeIt it = arg.begin(); it != arg.end(); it++) {
      UpdateAnalysis(it->first);
   }
   return;
}

string SumData (ArgListType &arg) //CARET_FUN
{
   // process the function arguments
   static string FunctionName = "SumData";
   static int argunset = true;
   static IntArg FirstPat("-pat", "first pattern(or vector) to sum", 1);
   static IntArg LastPat("-patend", "last pattern{-1 equals -pat}",-1);
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg EndNeuron("-Nend", "end neuron{-1 gives last neuron}", -1);
   static StrArg SeqName("-from", "data structure to be summed");
   static DblArg PadVal("-pval", "value to pad with", 0.0f);
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("^SumData( ... ) sums elements of a vector.\n");
      ComL.IntSet(4, &FirstPat, &LastPat, &StartNeuron, &EndNeuron);
      ComL.StrSet(1, &SeqName);
      ComL.DblSet(1, &PadVal);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   DataMatrix Mat = SystemVar::getData(SeqName, FunctionName, ComL);
   const int MatSize = Mat.size();
   if ((FirstPat.getValue() > MatSize) || (FirstPat.getValue() < 1)) {
      CALL_ERROR << "Error in " << FunctionName << ": pattern " << FirstPat.getValue()
         << " does not exist in " << SeqName.getValue() << "." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }   
   if (LastPat.getValue() == -1) LastPat.setValue(FirstPat.getValue());
   if ((LastPat.getValue() > MatSize) || (LastPat.getValue() < 1)) {
      CALL_ERROR << "Error in " << FunctionName << ": pattern " << LastPat.getValue()
         << " does not exist in " << SeqName.getValue() << "." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   DataMatrixCIt it = Mat.begin();
   for (int inc = 1; inc < FirstPat.getValue(); ++inc, ++it) {}
   int endN = EndNeuron.getValue();      
   int startN = StartNeuron.getValue();
   double sum = 0.0L;
   for (int t = FirstPat.getValue(); t <= LastPat.getValue(); ++t, ++it) {
      const int vectlen = it->size();
      if (EndNeuron.getValue() == -1) endN = vectlen;
      if (startN < 1) {         
         CALL_ERROR << "Error in " << FunctionName << " : Invalid element range : "
                    << startN << " , " << endN << " in Matrix " << SeqName.getValue() 
                    << "." << ERR_WHERE;
         ComL.DisplayHelp(Output::Err());
         exit(EXIT_FAILURE);
      }
      for (int j = startN; j <= endN; j++) {
         if (j > vectlen) {
            sum += PadVal.getValue();
         } else {
            sum += it->at(j - 1);
         }
      }
   }
   return to_string(sum);
}

string PatternLength(ArgListType &arg) //CARET_FUN
{
   // process the function arguments
   static string FunctionName = "PatternLength";
   static int argunset = true;
   static IntArg Pat("-pat", "pattern(or vector) to find the length of", 1);
   static StrArg SeqName("-from", "data structure to use");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("^PatternLength( ... ) returns the length of a pattern.\n");
      ComL.IntSet(1, &Pat);
      ComL.StrSet(1, &SeqName);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   DataMatrix Mat = SystemVar::getData(SeqName.getValue(), FunctionName, ComL);
   DataMatrixCIt it = Mat.begin();
   const int MatSize = Mat.size();
   if ((Pat.getValue() > MatSize) || (Pat.getValue() < 1)) {
      CALL_ERROR << "Error in " << FunctionName
                 << ": pattern " << Pat.getValue() << " does not exist in Matrix "
                 << SeqName.getValue() << "." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   for (int inc = 1; inc < Pat.getValue(); ++inc, ++it) {}
   return to_string(it->size());
}

string SequenceLength(ArgListType &arg) //CARET_FUN
{
   // process the function arguments
   static string FunctionName = "SequenceLength";
   static int argunset = true;
   static StrArg SeqName("-from", "data structure to use");
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("^SequenceLength( ... ) returns the duration of a sequence.\n");
      ComL.StrSet(1, &SeqName);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   DataMatrix Mat = SystemVar::getData(SeqName.getValue(), FunctionName, ComL);
   return to_string(Mat.size());
}

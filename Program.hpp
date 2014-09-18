// Program.hpp
//
// Must be separate from Program.cpp

#if !defined(PROGRAM_HPP)
#   define PROGRAM_HPP

#  if !defined(BINDLIST_HPP)
#    include "BindList.hpp"
#  endif
#  if !defined(CALC_HPP)
#    include "Calc.hpp"
#  endif
#  if !defined(FILTER_HPP)
#    include "Filter.hpp"
#  endif
#  if !defined(INTERNEURON_HPP)
#    include "Interneuron.hpp"
#  endif
#  if !defined(PARALLELRAND_HPP)
#    include "ParallelRand.hpp"
#  endif
#  if defined(MULTIPROC) && !defined(POPULATION_HPP)
#    include "Population.hpp"
#  endif
#  if !defined(SYSTEMVAR_HPP)
#    include "SystemVar.hpp"
#  endif

#  include <map>
#  include <iostream>
#  include <fstream>
#  include <deque>
#  include <list>
using std::map;
//using std::ifstream;
using std::ifstream;
using std::deque;
using std::list;

/**************************************************************/
/* program class prototype */
/**************************************************************/
class program {
 public:
    program();
   ~program() {};
   static bool defaultsSet() { return areDefaultsSet; }
   int GetIzhExplicitCount() const; // returns the number of set explicit Izhikevich variables
   int GetIzhExplicitMaxCount() const { return 6; }; // returns maximum possible ""
      //TODO: Get rid of magic number above

   void PrintHelp(string str = "", string func = "") const;

   inline bool getNetworkCreated() const { return networkCreated; }
   inline void setNetworkCreated(const bool nwCreated) { networkCreated = nwCreated; }

#   if defined(MULTIPROC)
   // Each node gets a seed based on its number
   // before we diverge te seeds, need to set up the shufflers
   inline void buildShuffleVectors(vector<unsigned int> &Shuffle,
                    vector<unsigned int> &UnShuffle, unsigned int numNeurons) {
      chkNoiseInit();
      // create the shuffler
      for (unsigned int i = 0; i < numNeurons; i++) Shuffle.at(i) = i;
      // (abh2n) I'm not sure if we need to keep the shuffle within population, 
      // but until I'm sure, it seems like a sensible precaution
      //FIXME: Be sure
      for (PopulationCIt PCIt = Population::Member.begin();
         PCIt != Population::Member.end(); ++PCIt) {
         unsigned int lastNeuron = PCIt->getLastNeuron();
         for (unsigned int i = PCIt->getFirstNeuron(); i <= lastNeuron; ++i) {
            std::swap(Shuffle.at(i), Shuffle.at(ShuffleNoise.RandInt(i,lastNeuron)));
         }
      }
      // create the unshuffler
      for (unsigned int i = 0; i < numNeurons; i++) UnShuffle.at(Shuffle.at(i)) = i;
   }
#   endif
   inline void chkNoiseInit() {
      if (!isNoiseInit) setAllSeeds();
   }
   inline unsigned int chooseItem(const unsigned int listSize) {
      chkNoiseInit();
      return PickNoise.RandInt(0, listSize - 1);
   }
   inline double chooseWeightedItem() {
      chkNoiseInit();
      return PickNoise.Uniform(0.0L, 1.0L);
   }
   inline unsigned int getConnectNoise(const unsigned int start, const unsigned end) {
      chkNoiseInit();
      return ConnectNoise.RandInt(start, end);
   }
   inline unsigned int getExtRandInt(const unsigned int start, const unsigned end) {
      chkNoiseInit();
      return ExternalNoise.RandInt(start, end);
   }
   inline bool getExtBernoulli(const double p) {
      chkNoiseInit();
      return ExternalNoise.Bernoulli(p);
   }
   inline unsigned int getResetNeuron(const unsigned int numNeurons) {
      chkNoiseInit();
      return ResetNoise.RandInt(0, numNeurons - 1);
   }
   inline double getWeightNormal(const double mu, const double sigma) {
      chkNoiseInit();
      return WeightNoise.Normal(mu, sigma);
   }
   inline double getWeightUniform(const double low, const double high) {
      chkNoiseInit();
      return WeightNoise.Uniform(low, high);
   }
   inline unsigned int pickTie(const unsigned int numTies) {
      chkNoiseInit();
      return TieBreakNoise.RandInt(0, numTies - 1);
   }
   void setAllSeeds();
   static void setDefaults(unsigned int ni);

   inline static program& Main() { return *mainPgm; }
   inline static void initMain() { mainPgm = new program(); }
private:
   bool networkCreated;          // a flag indicating network creation
   static program *mainPgm;

   static bool areDefaultsSet;
   bool isNoiseInit;
   Noise ConnectNoise;             // rng for connectivity generation
   Noise ExternalNoise;            // rng for noise in the externals
   Noise PickNoise;                // rng for choosing between sequences
   Noise ResetNoise;               // rng for creating random Z0
#   if defined(MULTIPROC)
   Noise ShuffleNoise;             // rng for shuffling neurons
#   endif
   Noise TieBreakNoise;            // rng for tie breaking
   Noise WeightNoise;              // rng for generating weights
   // Synapse.hpp also has SynNoise

   void LoadLists();
};

/**************************************************************/
/* AtFunction Prototypes */
/**************************************************************/
string GetArg(string to_find, ArgListType &arg);
void Echo(ArgListType &arg);
void Print(ArgListType &arg);
void PrintVar(ArgListType &arg);
#if !defined(WIN32)
void System(ArgListType &arg);
#endif

#endif

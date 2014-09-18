/***************************************************************************
 *            Interneuron.hpp
 *
 *  Mon Mar 28 10:33:47 2005
 *  Copyright  2005  Levy Lab
 ****************************************************************************/

#if !defined(INTERNEURON_HPP)
#define INTERNEURON_HPP

#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <string.h>

using std::ifstream;
using std::endl;

#if !defined(ARGFUNCTS_HPP)
#   include "ArgFuncts.hpp"
#endif
#if !defined(DATATYPES_HPP)
#   include "DataTypes.hpp"
#endif
#if !defined(OUTPUT_HPP)
#   include "Output.hpp"
#endif
#if !defined(FILTER_HPP)
#   include "Filter.hpp"
#endif
#include <string.h>
#include <iostream>
#include <fstream>

class Interneuron
{
public:
   Interneuron(const float excitationDecay = 1.0f, const unsigned int buffSize = 1);
   Interneuron(const Interneuron& inter);
   ~Interneuron() { delArray(synapticQueue); }
   Interneuron& operator=(const Interneuron& i);
   float calcExcitation(const float axonalExcitation);
   float calcExcitation(const UIVector& afferentFirings,
        const unsigned int firstN, const unsigned int lastN);
   float calcExcitation(const xInput& afferentFirings,
        const unsigned int firstN, const unsigned int lastN);
   void copy(const Interneuron& i);
   void loadCustomSynapseFilterValues();

   inline void  setFBadjustNumToFire(int toSet) { m_FBadjustNumToFire = toSet; }
   inline void  setFBadjust(float toSet) { m_FBadjust = toSet; }
   inline int   getFBadjustNumToFire() { return m_FBadjustNumToFire; }
   inline float getFBadjust()          { return m_FBadjust; }

   inline void enqueueSynapticActivation(const float synAct) {
      memmove((synapticQueue+1), synapticQueue, (synapticFilter.size()-1)*sizeof(float));
      synapticQueue[0] = synAct;
   }
   inline float getExcitation() const { return m_internalExcitation; }
   inline const DataList getInternrnWeights() const { return m_PyrToInternrnWt; }
   inline float getMult() const { return m_mult; }
   inline void loadSynapseFilterValues(const DataList &filterVals) {
      synapticFilter.setFilter(filterVals);
      delArray(synapticQueue);
      synapticQueue = new float[synapticFilter.size()];
      for (unsigned int i=0; i<synapticFilter.size(); ++i) {
         synapticQueue[i] = 0.0f;
      }
   }

   inline void reset() {
      setMaxTimeOffset(m_axonalBuffSize);
      delArray(synapticQueue);
      m_internalExcitation = -1.0f;
      synapticQueue = new float[synapticFilter.size()]();
      for (unsigned int i=0; i<synapticFilter.size(); ++i) {
         synapticQueue[i] = 0.0f;
      }
   }
   inline void setActivityAveragingRate(const double actAvgRate) {
      m_actAvgRate = actAvgRate;
   }
   inline void setDecayRate(const float newDecay) { m_excitationDecay = newDecay; };
   inline void setDesiredActivity(const double desiredActivity) {
      m_desiredActivity = desiredActivity;
   }
   inline void setMult(const double mult) { m_mult = mult; }
   // change the axonal buffer size
   void setMaxTimeOffset(const unsigned int buffSize);
   inline void setNumWeights(const unsigned int numWeights, const unsigned int firstN) {
      m_PyrToInternrnWt.assign(numWeights, 1.0L);
      m_firstNeuron = firstN;
   };
   inline void setUseWeightedActAvg(const bool useWeightsForActivity) {
      m_useWeightsForActivity = useWeightsForActivity;
   };
   inline void setWeightedActAvgAdj(const float WeightedActAvgAdj) {
      m_WeightedActAvgAdj = WeightedActAvgAdj;
   }
   inline void setSynModRate(const double synModRate) {
      m_SynModRate = synModRate;
   };
   void updateInternrnWeights(const UIVector &JustFired, const UIVector &toModify,
                              const unsigned int firstN, const unsigned int lastN);
   std::string exportInterneuron() const {
      // FIXME, eventually will have all information here
      string toReturn = ((char *) &m_internalExcitation);
      return toReturn;
   };
   void importInterneuron(std::string& toImport) {
      m_internalExcitation = m_internalExcitation; //FIXME, to implement
   };
private:
   Filter synapticFilter;
   float* synapticQueue;
   // The axonal buffer could be considered to be coming into the interneuron,
   // leaving the interneuron, or a combination of both
   std::queue<double> m_axonalBuffer;
   // Pyramidal-to-Interneuron weights
   DataList m_PyrToInternrnWt;
   // m_excitationDecay is the amount the existing exciation is reduced by for
   // the next excitation calculation. 1.0 = no memory in the interneuron.
   // This value is relative to the size of the time-step. If you want an e-fold
   // decay after d ms, and your time-step size is s ms, then excitationDecay
   // should be set to 1-e^{-s/d}. The correctness of this can be validated by
   // considering the cases where d->0 (decay = 1.0), and d->inf (decay = 0.0).
   float m_excitationDecay;
   float m_internalExcitation;
   // m_axonalBuffSize is the maximum size we will allow m_axonalBuffer to grow to
   unsigned int m_axonalBuffSize;
   // Moving average of the activity deviation
   unsigned int m_firstNeuron; // 0-based
   bool m_useWeightsForActivity;
   float m_WeightedActAvgAdj;
   double m_activityDeviation;
   double m_desiredActivity;
   double m_SynModRate;
   double m_actAvgRate;
   double m_mult;
   int m_FBadjustNumToFire;
   float m_FBadjust;
};

typedef std::vector<Interneuron> InterneuronVec;
typedef InterneuronVec::iterator InterneuronVecIt;
typedef InterneuronVec::const_iterator InterneuronVecCIt;

#endif // INTERNEURON_HPP

#if !defined(POPULATION_HPP)
#  define POPULATION_HPP

#  if !defined(DATATYPES_HPP)
#    include "DataTypes.hpp"
#  endif
#  if !defined(INTERNEURON_HPP)
#    include "Interneuron.hpp"
#  endif
#  if !defined(NEURONTYPE_HPP)
#    include "NeuronType.hpp"
#  endif
#  include <vector>
#  include <string>

class Population {
public:
   Population(unsigned int f, unsigned int l, NeuronType& nType):
      m_firstNeuron(f), m_lastNeuron(l), m_neuronType(&nType) {
      m_feedbackInterneurons.push_back(*(new Interneuron()));
      m_feedforwardInterneurons.push_back(*(new Interneuron()));
   };
   //////////////////////////////////////////////
   // Begin functions relating to Interneurons //
   //////////////////////////////////////////////
   void addInterneuron(const bool isFeedback, const double excitationDecay = 1.0f, const unsigned int buffSize = 1,
                       const double synModRate = 0.0f, const double actAvgRate = 0.0f, const double mult = 1.0f) {
       Interneuron toAdd(excitationDecay, buffSize);
       toAdd.setSynModRate(synModRate);
       toAdd.setActivityAveragingRate(actAvgRate);
       toAdd.setMult(mult);
       if (isFeedback) {
          m_feedbackInterneurons.push_back(toAdd);
       } else {
          m_feedforwardInterneurons.push_back(toAdd);
       }
   }
   void calcNewFeedbackInhibition(const UIVector& afferentFirings) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->calcExcitation(afferentFirings, m_firstNeuron, m_lastNeuron);
      }
   }
   void calcNewFeedforwardInhibition(const UIVector& afferentFirings) {
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->calcExcitation(afferentFirings, m_firstNeuron, m_lastNeuron);
      }
   }
   void calcNewFeedforwardInhibition(const xInput& afferentFirings) {
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->calcExcitation(afferentFirings, m_firstNeuron, m_lastNeuron);
      }
   }
   bool forceExt() const { return m_neuronType->forceExt(); }
   float getFeedbackInhibition() const;
   float getFeedforwardInhibition() const;
   unsigned int getFirstNeuron() const { return m_firstNeuron; }
   DataList getKFBWeights() {
      //TODO: append to one big list?
      return m_feedbackInterneurons[0].getInternrnWeights();
   }
   unsigned int getLastNeuron() const {return m_lastNeuron;}
   NeuronType* getNeuronType() const {return m_neuronType;}
   void initInterneurons();
   void loadSynapseFilterValues(const DataList &filterVals) {
      if (SystemVar::GetIntVar("WtFiltIsGeneric")) {
         m_neuronType->loadSynapseFilterValues(filterVals);
      }
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->loadSynapseFilterValues(filterVals);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->loadSynapseFilterValues(filterVals);
      }
   }
   void loadCustomSynapseFilterValues() {
//      if (SystemVar::GetIntVar("WtFiltIsGeneric")) {
//         m_neuronType->loadSynapseFilterValues(); //THIS WILL ALWAYS THROW AN ERROR
//      }
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->loadCustomSynapseFilterValues();
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->loadCustomSynapseFilterValues();
      }
   }
   void resetInterneurons() {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->reset();
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->reset();
      }
   }
   void restoreInhState() {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->copy(m_feedbackInterneurons_dup.back());
         m_feedbackInterneurons_dup.pop_back();
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->copy(m_feedforwardInterneurons_dup.back());
         m_feedforwardInterneurons_dup.pop_back();
      }
   }
   void saveInhState() {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         m_feedbackInterneurons_dup.insert(m_feedbackInterneurons_dup.begin(),*it);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         m_feedforwardInterneurons_dup.insert(m_feedforwardInterneurons_dup.begin(),*it);
      }
   }
   void setActivityAveragingRate(float actAvgRate) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setActivityAveragingRate(actAvgRate);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setActivityAveragingRate(actAvgRate);
      }
   }
   void setDesiredActivity(const float act) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setDesiredActivity(act);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setDesiredActivity(act);
      }
   }
   void setUseWeightedActAvg(const bool useWeightsForActivity) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setUseWeightedActAvg(useWeightsForActivity);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setUseWeightedActAvg(useWeightsForActivity);
      }
   }
   void setWeightedActAvgAdj(const float WeightedActAvgAdj) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setWeightedActAvgAdj(WeightedActAvgAdj);
      }
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setWeightedActAvgAdj(WeightedActAvgAdj);
      }
   }
   void setFBInternrnSynModRate(const float synModRate) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setSynModRate(synModRate);
      }
   }
   void setFFInternrnSynModRate(const float synModRate) {
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setSynModRate(synModRate);
      }
   }
   void setNeuronRange(unsigned int f, unsigned int l) {
      m_firstNeuron = f; m_lastNeuron = l;
   }
   void updateFBInterneuronAxonalDelay(unsigned int axonalBuffLen);
   void updateFFInterneuronAxonalDelay(unsigned int axonalBuffLen);
   inline void setFBadjust(float toSet) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
           it != m_feedbackInterneurons.end(); ++it) {
         it->setFBadjust(toSet);
      }
   }
   inline void setFBadjustNumToFire(int toSet) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
           it != m_feedbackInterneurons.end(); ++it) {
         it->setFBadjustNumToFire(toSet);
      }
   }
   void updateInterneuronDecay(double newDecayRate) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->setDecayRate(newDecayRate);
      }
      /*
	for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->setDecayRate(newDecayRate);
        }
      */
   }
   void updateInternrnWeights(const UIVector &JustFired,
         const UIVector &oldFired, const xInput &curPattern) {
      for (InterneuronVecIt it = m_feedbackInterneurons.begin();
            it != m_feedbackInterneurons.end(); ++it) {
         it->updateInternrnWeights(JustFired, oldFired, m_firstNeuron, m_lastNeuron);
      }
      UIVector curPtn = xInputToUIPtn(curPattern);
      for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
            it != m_feedforwardInterneurons.end(); ++it) {
         it->updateInternrnWeights(JustFired, curPtn, m_firstNeuron, m_lastNeuron);
      }
   }
   ////////////////////
   // Static members //
   ////////////////////
   static void addMember(Population toAdd) {
      Member.push_back(toAdd);
   }
   static unsigned int getPopulationSize() {
      return Member.size();
   }
   static std::vector<Population> Member;
private:
   // m_firstNeuron & m_lastNeuron are 0-based
   unsigned int m_firstNeuron;
   unsigned int m_lastNeuron;
   InterneuronVec m_feedbackInterneurons;
   InterneuronVec m_feedforwardInterneurons;
   InterneuronVec m_feedbackInterneurons_dup;
   InterneuronVec m_feedforwardInterneurons_dup;
   NeuronType* m_neuronType;
};

typedef std::vector<Population>::const_iterator PopulationCIt;
typedef std::vector<Population>::iterator PopulationIt;

#endif // POPULATION_HPP

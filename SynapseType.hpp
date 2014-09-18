#if !defined(SYNAPSETYPE_HPP)
#  define SYNAPSETYPE_HPP

#  if !defined(FILTER_HPP)
#    include "Filter.hpp"
#  endif
#  include <map>
#  include <string>

enum LearningRuleType { LRT_PostSyn, LRT_MvgAvg,  LRT_PostSynB }; //MEGHAN: added new learning rule type

class SynapseType {
public:
   SynapseType(): m_learningRule(LRT_PostSyn), m_mu(0.0f), m_NMDArise(1), m_alpha(0.0f), m_Ksyn(1.0f),
                  m_synSuccRate(1.0f), m_preNeurType(""), m_postNeurType(""), m_resolution(0.0000001), m_inv(10000000), m_stochastic(0) {};
   SynapseType(const SynapseType& s);
   SynapseType(const LearningRuleType learningRule, const float mu, const unsigned int NMDArise,
               const float alpha, const float Ksyn, const float synFailRate,
	       const std::string& preNeurType, const std::string& postNeurType, const float resolution, const float inv, const int stochastic);
   SynapseType& operator=(const SynapseType& s);
   const static unsigned int MAX_TIME_STEP = 500;
   const static unsigned int MAX_NMDA = 21;
   const static unsigned int MAX_RES = 1001;
   float alphaRiseArray[MAX_RES][MAX_NMDA];
   float alphaFallArray[MAX_TIME_STEP];
   float getAlpha() const { return m_alpha; }
   float getResolution() const { return m_resolution; }
   float getInv() const { return m_inv; }
   int   getStochastic() const { return m_stochastic; }
   void setResolution(float toSet) { m_resolution = toSet; }
   void setInv(float toSet)  { m_inv = toSet; }
   int  setStochastic(int toSet) {m_stochastic = toSet; }
   LearningRuleType getLearningRule() const { return m_learningRule; }
   Filter* getFilter() { return &m_synapticFilter; }
   float getKsyn() const { return m_Ksyn; }
   string getRiseFile() const { return m_riseFile; } 
   unsigned int getNMDArise() const { return m_NMDArise; }
   std::string getPreNeurType() const { return m_preNeurType; }
   std::string getPostNeurType() const { return m_postNeurType; }
   float getSynModRate() const { return m_mu; }
   float getSynSuccRate() const { return m_synSuccRate; }
   void setAlpha(const float alpha) { m_alpha = alpha; setAlphaDecay(); }
   void setFilter(const DataList &filterVals) { m_synapticFilter.setFilter(filterVals); }
   void setKsyn(const float Ksyn) { m_Ksyn = Ksyn; }
   void setRiseFile(const string file) { m_riseFile = file; setAlphaDecay(); }
   void setLearningRule(const LearningRuleType learningRule) { m_learningRule=learningRule; }
   void setNMDArise(const unsigned int NMDArise) { m_NMDArise = NMDArise; setAlphaDecay(); }
   void setSynModRate(const float mu) { m_mu=mu; }
   void setSynFailRate(const float synFailRate) { m_synSuccRate = 1 - synFailRate; }
   void setSave(const int saveIt) {if (saveIt == 1) saveZbar();	}

   ////////////////////
   // Static members //
   ////////////////////
   // Using a map here is fine, since this public method will only be accessed
   // once per neuron type - while creating the network. From then on, each
   // neuron type will have a copy of the SynapseType referenced here.
   static std::map<std::string, SynapseType> Member;
   static void addMember(const std::string& name, const LearningRuleType learningRule,
	const float mu, const unsigned int NMDArise, const float alpha, const float Ksyn,
	const float synFailRate, const std::string& preType, const std::string& postType, const float resolution, const float inv, const int stochastic); 
   static std::map<std::string, SynapseType const*>
      findPreSynapticTypes(const std::string& preNeurType);
private:
   LearningRuleType m_learningRule;
   float m_mu;
   unsigned int m_NMDArise;
   float m_alpha;
   float m_Ksyn;
   float m_synSuccRate;
   float m_resolution;
   float m_inv;
   int m_stochastic;
   string m_riseFile;
   std::string m_preNeurType;
   std::string m_postNeurType;
   Filter m_synapticFilter;
   void setAlphaDecay();
   void saveZbar();
};

typedef std::map<std::string, SynapseType>::const_iterator SynapseTypeMapCIt;
typedef std::map<std::string, SynapseType>::iterator SynapseTypeMapIt;

#endif

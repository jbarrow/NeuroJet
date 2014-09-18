#if !defined(POPULATION_HPP)
#  include "Population.hpp"
#endif

#if !defined(SYSTEMVAR_HPP)
#  include "SystemVar.hpp"
#endif

////////////////////////
// Static definitions //
////////////////////////

std::vector<Population> Population::Member;

////////////////////////////
// Non-static definitions //
////////////////////////////

//////////////////////////////////////////////
// Begin functions relating to Interneurons //
//////////////////////////////////////////////
void Population::initInterneurons()
{
   float act = m_neuronType->getParameter("Activity", SystemVar::GetFloatVar("Activity"));
   const unsigned int popSize = m_lastNeuron - m_firstNeuron + 1;
   for (InterneuronVecIt it = m_feedbackInterneurons.begin();
         it != m_feedbackInterneurons.end(); ++it) {
      it->setNumWeights(popSize, m_firstNeuron);
   }
   for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
         it != m_feedforwardInterneurons.end(); ++it) {
      it->setNumWeights(popSize, m_firstNeuron);
   }
   setDesiredActivity(act);
   float lambda = m_neuronType->getParameter("lambdaFB", SystemVar::GetFloatVar("lambdaFB"));
   setFBInternrnSynModRate(lambda);
   lambda = m_neuronType->getParameter("lambdaFF", SystemVar::GetFloatVar("lambdaFF"));
   setFFInternrnSynModRate(lambda);
   float actAvgRate = m_neuronType->getParameter("PyrToInternrnWtAdjDecay", 
                              SystemVar::GetFloatVar("PyrToInternrnWtAdjDecay"));
   setActivityAveragingRate(actAvgRate);
   unsigned int axDelay = m_neuronType->getParameter("FBInternrnAxonalDelay",
                                               SystemVar::GetIntVar("FBInternrnAxonalDelay"));
   updateFBInterneuronAxonalDelay(axDelay);
   axDelay = m_neuronType->getParameter("FFInternrnAxonalDelay",
                                               SystemVar::GetIntVar("FFInternrnAxonalDelay"));
   updateFFInterneuronAxonalDelay(axDelay);
   const float decay = m_neuronType->getParameter("InternrnExcDecay", SystemVar::GetFloatVar("InternrnExcDecay"));
   updateInterneuronDecay(static_cast<double>(decay));
}

float Population::getFeedbackInhibition() const {
   float num = 0.0f;
   float denom = 0.0f;
   for (InterneuronVecCIt cit = m_feedbackInterneurons.begin();
         cit != m_feedbackInterneurons.end(); ++cit) {
      num += cit->getMult() * cit->getExcitation();
      denom += cit->getMult();
   }
   return num/denom;
}

float Population::getFeedforwardInhibition() const {
   float num = 0.0f;
   float denom = 0.0f;
   for (InterneuronVecCIt cit = m_feedforwardInterneurons.begin();
         cit != m_feedforwardInterneurons.end(); ++cit) {
      num += cit->getMult() * cit->getExcitation();
      denom += cit->getMult();
   }
   return num/denom;
}

void Population::updateFBInterneuronAxonalDelay(unsigned int axonalBuffLen) {
   for (InterneuronVecIt it = m_feedbackInterneurons.begin();
         it != m_feedbackInterneurons.end(); ++it) {
      it->setMaxTimeOffset(axonalBuffLen);
   }
}

void Population::updateFFInterneuronAxonalDelay(unsigned int axonalBuffLen) {
   for (InterneuronVecIt it = m_feedforwardInterneurons.begin();
         it != m_feedforwardInterneurons.end(); ++it) {
      it->setMaxTimeOffset(axonalBuffLen);
   }
}

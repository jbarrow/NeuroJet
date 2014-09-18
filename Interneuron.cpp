/***************************************************************************
 *            Interneuron.cpp
 *
 *  Mon Mar 28 11:43:43 2005
 *  Copyright  2005  Levy Lab
 ****************************************************************************/

#if !defined(INTERNEURON_HPP)
#  include "Interneuron.hpp"
#endif
#if !defined(ARGFUNCTS_HPP)
#  include "ArgFuncts.hpp"
#endif
#if !defined(PARSER_HPP)
#  include "Parser.hpp"
#endif
#include <string.h>
#include <iostream>
#include <fstream>
using std::fstream;
using std::vector;

Interneuron::Interneuron(const float excitationDecay, const unsigned int buffSize):
   synapticQueue(NULL), m_excitationDecay(excitationDecay), m_internalExcitation(-1.0f), 
   m_axonalBuffSize(buffSize), m_firstNeuron(0), m_useWeightsForActivity(false), 
   m_WeightedActAvgAdj(1.0f), m_activityDeviation(0.0f), m_SynModRate(0.0f),
   m_actAvgRate(0.0f), m_mult(1.0f), m_FBadjustNumToFire(0), m_FBadjust(1.0f/12.0f) {
   // For now, Filter defaults to size 1
   synapticQueue = new float[synapticFilter.size()];
   for (unsigned int j=0; j<synapticFilter.size(); ++j) {
      synapticQueue[j] = 0.0f;
   }
};

Interneuron::Interneuron(const Interneuron& i): synapticFilter(i.synapticFilter), synapticQueue(NULL),
   m_axonalBuffer(i.m_axonalBuffer), m_PyrToInternrnWt(i.m_PyrToInternrnWt), m_excitationDecay(i.m_excitationDecay),
   m_internalExcitation(i.m_internalExcitation), m_axonalBuffSize(i.m_axonalBuffSize),
   m_firstNeuron(i.m_firstNeuron), m_useWeightsForActivity(i.m_useWeightsForActivity),
   m_WeightedActAvgAdj(i.m_WeightedActAvgAdj), m_activityDeviation(i.m_activityDeviation),
   m_SynModRate(i.m_SynModRate), m_actAvgRate(i.m_actAvgRate), m_mult(i.m_mult), 
   m_FBadjustNumToFire(i.m_FBadjustNumToFire), m_FBadjust(i.m_FBadjust) {
   synapticQueue = new float[synapticFilter.size()];
   for (unsigned int j=0; j<synapticFilter.size(); ++j) {
      synapticQueue[j] = i.synapticQueue[j];
   }
}

Interneuron& Interneuron::operator=(const Interneuron& i) {
   if (this != &i) {  // make sure not same object
      this->copy(i);
   }
   return *this;    // Return ref for multiple assignment
}

float Interneuron::calcExcitation(const float axonalExcitation)
{
   m_axonalBuffer.push(axonalExcitation);
   if (m_axonalBuffer.size() > m_axonalBuffSize) m_axonalBuffer.pop();
   float curExcitation = m_axonalBuffer.front(); // arriving at the synapse
   enqueueSynapticActivation(curExcitation);
   float synResponse = synapticFilter.apply(synapticQueue);
   // m_internalExcitation is initialized to -1 so that we can recognize
   // the first time this is called after a reset (or first time ever)
   m_internalExcitation = (m_internalExcitation < 0.0f) ? synResponse :
     (1.0f - m_excitationDecay) * m_internalExcitation + m_excitationDecay * synResponse;
   return (m_mult * m_internalExcitation);
}

float Interneuron::calcExcitation(const UIVector& afferentFirings,
   const unsigned int firstN, const unsigned int lastN)
{ //this is where the FB inhibition gets calculated
// this method was changed to use the new FB interneuron rule. 
// the signal is modified by (numFired/desired) ^ (1/12)
// a network with a filter that used to die slowly will now stay alive 
   double axonalExcitation = 0.0f;
   for (unsigned int i = 0; i < afferentFirings.size(); ++i) {
      if ((afferentFirings[i] >= firstN)
            && (afferentFirings[i] <= lastN))
         axonalExcitation += m_PyrToInternrnWt[afferentFirings[i]-m_firstNeuron];
   }
   if( m_FBadjustNumToFire == 1) {
     float numToFire = iround(SystemVar::GetFloatVar("Activity") * SystemVar::GetIntVar("ni"));
     axonalExcitation *= pow( afferentFirings.size() / numToFire, m_FBadjust);
   }
//   Output::Out() << "A. afferentFirings.size(): " << afferentFirings.size() << " lastN-firstN+1: " << lastN-firstN+1 << std::endl;
   return calcExcitation(axonalExcitation);
}

float Interneuron::calcExcitation(const xInput& afferentFirings,
   const unsigned int firstN, const unsigned int lastN)
{ //this is where the FF inhibition gets calculated
   int numFired = 0;
   double axonalExcitation = 0.0f;
   for (unsigned int i = firstN; i <= lastN; ++i) {
      if (afferentFirings[i] == 1) {
         axonalExcitation += m_PyrToInternrnWt[i-m_firstNeuron];
	 numFired++;
      }
   }
   m_internalExcitation = axonalExcitation;
   return axonalExcitation;
   //return calcExcitation(axonalExcitation);
}

void Interneuron::copy(const Interneuron& i) {
   synapticFilter = i.synapticFilter;
   m_axonalBuffer = i.m_axonalBuffer;
   m_PyrToInternrnWt = i.m_PyrToInternrnWt;
   m_excitationDecay = i.m_excitationDecay;
   m_internalExcitation = i.m_internalExcitation;
   m_axonalBuffSize = i.m_axonalBuffSize;
   m_firstNeuron = i.m_firstNeuron;
   m_useWeightsForActivity = i.m_useWeightsForActivity;
   m_WeightedActAvgAdj = i.m_WeightedActAvgAdj;
   m_activityDeviation = i.m_activityDeviation;
   m_SynModRate = i.m_SynModRate;
   m_actAvgRate = i.m_actAvgRate;
   m_mult = i.m_mult;
   m_FBadjustNumToFire = i.m_FBadjustNumToFire;
   m_FBadjust = i.m_FBadjust;
   delArray(synapticQueue);
   synapticQueue = new float[synapticFilter.size()];
   for (unsigned int j=0; j<synapticFilter.size(); ++j) {
      synapticQueue[j] = i.synapticQueue[j];
   }
}

void Interneuron::setMaxTimeOffset(const unsigned int buffSize)
{
   m_axonalBuffSize = buffSize;
   while (m_axonalBuffer.size() < m_axonalBuffSize) m_axonalBuffer.push(0.0f);
   while (m_axonalBuffer.size() > m_axonalBuffSize) m_axonalBuffer.pop();
}

void Interneuron::loadCustomSynapseFilterValues() {

   Output::Out() << "Loading " << SystemVar::GetStrVar("InterNeuronfilterFile") << "..." <<endl;

   const int len = SystemVar::GetStrVar("InterNeuronfilterFile").size();
   char filterFile[len]; 
   strcpy(filterFile,SystemVar::GetStrVar("InterNeuronfilterFile").c_str());	
		
   fstream file;
   file.open(filterFile, fstream::in);
   if(! file.is_open())
   	{
   	CALL_ERROR << "Unable to open "<< filterFile <<".\n"<< ERR_WHERE;
   	exit(EXIT_FAILURE);
   	}
   char ch[20]; //floats are never more than 20 digits?
   float temp[50]; //create temp to store filter values max is 50?
   float x = 0.0f;
   int i = 0;	
   while (!file.eof())
   {
   	file.getline(ch,20,' '); //get first number in file, space delimited
   	if(ch[0] == '\0') //if null, continue
   	   continue;
   	string str (ch);
   	if(sscanf(ch, "%f", &x) == 0) //convert the string into a float
   	   break;
   	temp[i++] = x; //store the float in the temp
   }
   file.close();

   //Check for a filter that makes no sense here. (negative values, etc.)
   for (int j = 0; j < i; j++)
	if( temp[j] <= 0.0f ) //Zero or negative value in the filter!
		{
		Output::Out() << j+1 << "th value in the filter is " << temp[j] << "\n Values must be greater than 0.0f" << std::endl;
		Output::Out() << "Changing " << j+1 << "th value in the filter to 0.0f" << std::endl;
		temp[j] = 0.0f;
		};

   float sum = 0.0f;
   for (int j = 0; j < i; j++)
	sum += temp[j];

   //Check to see if filter sums to 0
   if ( sum == 0.0f)
	{
	Output::Out() << "Sum equals 0.0f, changing filter to equal values..." << std::endl;
	for (int j = 0; j < i; j++) temp[j] = 1; //copy from temp to filter
	sum = i;
	}

   if ( sum != 1.0f)
	Output::Out() << "Sum does not equal 1.0f, normalizing..." << std::endl;
   //NORMALIZE FILTER INCASE SUM != 1

   float filter[i]; //make the filter the correct length
   for (int j = 0; j < i; j++) filter[j] = temp[j] / sum; //copy from temp to filter

   Output::Out() << "Creating a Interneuron filter of length " << i
   		 << "\n Use a preload of at least " << i+1 << std::endl; 

      synapticFilter.setFilter(filter, i);
      delArray(synapticQueue);
      synapticQueue = new float[synapticFilter.size()];
      for (unsigned int i=0; i<synapticFilter.size(); ++i) {
         synapticQueue[i] = 0.0f;
      }
}

void Interneuron::updateInternrnWeights(const UIVector &JustFired, const UIVector &toModify,
                        const unsigned int firstN, const unsigned int lastN) {
   // toModify determines which neuron's efferent weights are modified
   // JustFired is used to determine the actual activity
   // So, for feedforward interneurons, toModify corresponds to just external inputs,
   // but JustFired is all neurons
   if (m_SynModRate > verySmallFloat) {
      // "Dave's rule" for inhibition adjustment(Pyramidal-to-Interneuron
      // Synaptic Modification)
      //FLEX: Adjustment could be based off of previous activity instead of current
      // activity, as well as other modifications
      double actualAct = 0.0f;
      for (unsigned int i = 0; i < JustFired.size(); ++i) {
         unsigned int iFired = JustFired[i];
         if ((iFired >= firstN) && (iFired <= lastN)) {
            actualAct += (m_useWeightsForActivity ? m_PyrToInternrnWt[iFired] : 1.0L);
         }
      }
      if (m_useWeightsForActivity) {
         double totalWeights = 0.0f;
         for (unsigned int nrn = firstN; nrn <= lastN; ++nrn) {
            totalWeights += m_PyrToInternrnWt[nrn];
         }
         actualAct /= totalWeights;
      } else {
         actualAct /= (lastN - firstN + 1);
      }
      
      m_activityDeviation = (m_actAvgRate * m_activityDeviation)
         + (1 - m_actAvgRate) * (actualAct * m_WeightedActAvgAdj - m_desiredActivity);

      for (unsigned int i = 0; i < toModify.size(); i++) {
         unsigned int iFired = toModify[i];
         m_PyrToInternrnWt[iFired] += m_SynModRate * m_activityDeviation;
         if (m_PyrToInternrnWt[iFired] < 0)
            m_PyrToInternrnWt[iFired] = 0;
      }
   }
}

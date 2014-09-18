#if !defined(NEURONTYPE_HPP)
#  include "NeuronType.hpp"
#endif

#if !defined(CALC_HPP)
#  include "Calc.hpp"
#endif
#if !defined(OUTPUT_HPP)
#  include "Output.hpp"
#endif
#if !defined(SYSTEMVAR_HPP)
#  include "SystemVar.hpp"
#endif

#include <string.h>
#include <iostream>
#include <fstream>
using std::fstream;

std::map<std::string, NeuronType> NeuronType::Member;

NeuronType::NeuronType(const NeuronType& n): m_dendriteToSomaFilter(n.m_dendriteToSomaFilter),
   m_filterDecay(n.m_filterDecay), m_convolvedFilter(n.m_convolvedFilter), m_synapseType(n.m_synapseType),
   m_parameter(n.m_parameter), m_isExc(n.m_isExc), m_isInhDiv(n.m_isInhDiv), m_name(n.m_name) {
}

NeuronType& NeuronType::operator=(const NeuronType& n) {
   if (this != &n) {  // make sure not same object
      m_dendriteToSomaFilter = n.m_dendriteToSomaFilter;
      m_filterDecay = n.m_filterDecay;
      m_convolvedFilter = n.m_convolvedFilter;
      m_synapseType = n.m_synapseType;
      m_parameter = n.m_parameter;
      m_isExc = n.m_isExc;
      m_isInhDiv = n.m_isInhDiv;
      m_name = n.m_name;
   }
   return *this;    // Return ref for multiple assignment
}

void NeuronType::addMember(const std::string& name, bool isExc, bool isInhDiv) {
   if (Member.find(name) == Member.end()) {
      Member[name] = NeuronType(isExc, isInhDiv, name);
   } else {
      CALL_ERROR << "NeuronType " << name << " already exists" << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
}

void NeuronType::convolveFilters() {
   const float deltaT = SystemVar::GetFloatVar("deltaT");
   Filter* synapticFilter = m_synapseType.getFilter();
   float* vals = synapticFilter->getFilter();
   if (deltaT > verySmallFloat) {
      if ((m_dendriteToSomaFilter.size() > 1) && (synapticFilter->size() > 1)) {
         unsigned int convSize = m_dendriteToSomaFilter.size() + synapticFilter->size() - 1;
         m_convolvedFilter.setFilter(Calc::convolve(
                                         m_dendriteToSomaFilter.getFilter(), m_dendriteToSomaFilter.size(),
                                         synapticFilter->getFilter(), synapticFilter->size(), deltaT), convSize);
      } else if (m_dendriteToSomaFilter.size() > 1) {
         m_convolvedFilter.setFilter(m_dendriteToSomaFilter.getFilter(), m_dendriteToSomaFilter.size());
      } else {
         m_convolvedFilter.setFilter(synapticFilter->getFilter(), synapticFilter->size());
      }
   }
}

void NeuronType::loadCustomFilterValues() {

   Output::Out() << "Loading " << SystemVar::GetStrVar("filterFile") << "..." <<endl;

   const int len = SystemVar::GetStrVar("filterFile").size();
   char filterFile[len]; 
   strcpy(filterFile,SystemVar::GetStrVar("filterFile").c_str());	
		
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

   Output::Out() << "Creating a filter of length " << i
   		 << "\n Use a preload of at least " << i+1 << std::endl; 

   m_convolvedFilter.setFilter(filter, i);
}

bool NeuronType::forceExt() const {
   return ((getParameter("ExtExc", SystemVar::GetFloatVar("ExtExc")) < verySmallFloat) &&
          (getParameter("DGstrength", SystemVar::GetFloatVar("DGstrength")) < verySmallFloat));
}

bool NeuronType::useIzh() const {
   return (hasParameter("IzhA"));
}

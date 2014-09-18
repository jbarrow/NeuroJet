/***************************************************************************
*            Synapse.hpp
*
*  Mon Apr 18 16:50:23 2005
*  Copyright  2005
*
* Adam Christian Edited in Jan 26 2012 under ATC
****************************************************************************/

#if !defined(SYNAPSE_HPP)
#  define SYNAPSE_HPP
#  if !defined(ARGFUNCTS_HPP)
#    include "ArgFuncts.hpp"
#  endif
#  if !defined(NOISE_HPP)
#    include "Noise.hpp"
#  endif
#  if !defined(SYNAPSETYPE_HPP)
#    include "SynapseType.hpp"
#  endif
#  if defined(RNG_BUCKET) && !defined(PARALLELRAND_HPP)
#    include "ParallelRand.hpp"
#  endif
#  include <vector>
#  include <math.h> //ATC 1-26-2012 - Added because of the floor() command
#  include <stdlib.h> //ATC 1-26-2012 - Added to give the srand() seed for the random number generator
#  include <iostream> //ATC 1-26-2012
class AxonalSynapse;

// DendriticSynapse = Synapse on dendritic tree
class DendriticSynapse 
{
	public:   
	inline DendriticSynapse():
	m_isExc(true), m_isInhDiv(false), srcNeuron(0), destNeuron(0), weight(0.0f),
	lastActivate(NEVER_ACTIVATED), mvgAvg(0.0f), m_synType(NULL) {riseUntil = 0;};
	// activate happens prior to ++timeStep
	inline void activate(DataList &bus, DataList &bus_inhdiv, DataList &bus_inhsub, const int timeStep) 
	{
	 // throw the coin
	 // Whatever you do, don't do this:
	 // const SynapseType mySynType = *m_synType;
#if defined(RNG_BUCKET)
	bool result = ParallelRand::RandComm.RandBernoulli();
#else             // not RNG_BUCKET
                  // relies on programmer to invoke chkNoiseInit in code before use
	bool result = SynNoise.Bernoulli(m_synType->getSynSuccRate());
#endif            // RNG_BUCKET
	if (result) 
		{
		const bool useMvgAvg = (m_synType->getLearningRule() == LRT_MvgAvg);
		const unsigned int NMDArise = m_synType->getNMDArise();
		const int timeDiff = timeStep - lastActivate;
		if (useMvgAvg) 
		{
			if (timeDiff < NMDArise)
			{
			// Zeroth element is 0, first element is 1, 2nd element is alpha, 
			// nth element is alpha^(n-1)
			mvgAvg = (timeStep - lastActivate) < static_cast<int>(SynapseType::MAX_TIME_STEP) ?
			m_synType->alphaRiseArray[oldZbar][timeDiff+1] * mvgAvg + (1-m_synType->getAlpha()) :
			1-m_synType->getAlpha();
			}
			else
			{
			mvgAvg = (timeStep - lastActivate) < static_cast<int>(SynapseType::MAX_TIME_STEP) ?
			m_synType->alphaFallArray[timeDiff+1-NMDArise] * mvgAvg + (1-m_synType->getAlpha()) :
			1-m_synType->getAlpha();
			}
		}
		// Calculates net excitation
		//FLEX: Could add AMPA-like biology/dendritic capacitance(with lookup table)
		if (m_isExc)
		 bus[destNeuron] += m_synType->getKsyn() * weight;
		else if (m_isInhDiv)
		 bus_inhdiv[destNeuron] += m_synType->getKsyn() * weight;
		else 
		 bus_inhsub[destNeuron] += m_synType->getKsyn() * weight;
		//FLEX: Could add more biology to this functionality(see NMDArise.ps for
		// an explanation)
		// NEVER_ACTIVATED is set so that if lastActivate == NEVER_ACTIVATED,
		// then timeStep - lastActivate >= MAX_TIME_STEP (because timeStep >= 0)
		//NOT USING THIS ANYMORE:	// lastActivate = ((NMDArise<=1) || (timeDiff>=static_cast<int>(SynapseType::MAX_TIME_STEP)) || useMvgAvg) 
						// ? timeStep : timeStep - iround(m_synType->alphaArray[timeDiff] * NMDArise);
		if(lastActivate == NEVER_ACTIVATED)
			{
			oldZbar = 0;
			riseUntil = 0;			
			}
		else if (timeDiff < NMDArise)	//Activation during a Rise
			{
			if(timeStep < riseUntil)
				{//previously had fired on a rise, i.e. at least the second fire on this rise
				riseUntil = timeStep + NMDArise - 1;
				}
			else
				{//first fire on this rise			
				riseUntil = timeStep + NMDArise - 1;	
				riseActivate = lastActivate;			
				}
			}
		else				//Activation during a Fall
			{
			oldZbar = static_cast<int>(1000.0f * m_synType->alphaFallArray[timeDiff-NMDArise]);
			riseUntil = 0;
			}
		prevLastActivate = lastActivate;
		lastActivate = timeStep;
		if (oldZbar < 0) oldZbar = 0;
		else if (oldZbar >= 1000) oldZbar = 1000;
		}
	}

	inline void connectNeuron(unsigned int dstNeuron, SynapseType const* synType, bool isExc, bool isInhDiv) 
	{
	destNeuron = dstNeuron;
	m_isExc = isExc;
	m_isInhDiv = isInhDiv;
	m_synType = synType;
	}

	inline unsigned int getSrcNeuron() const { return srcNeuron; }
	inline void setSrcNeuron(const unsigned int toSet) { srcNeuron = toSet; }
	inline float getWeight() const { return weight; }
	inline void setWeight(const float toSet) { weight = toSet; }
	inline void updateWeight(int timeStep) 
	{
	 // Whatever you do, don't do this:
	 // const SynapseType mySynType = *m_synType;
	 // NEVER_ACTIVATED is set so that if lastActivate == NEVER_ACTIVATED,
	 // then timeStep - lastActivate >= MAX_TIME_STEP (because timeStep >= 0)
	int stochastic = m_synType->getStochastic();
	float resolution = m_synType->getResolution(); //.0000001; //ATC 1-26-2012 - Added to give the resolution of the weight change
	float inv = m_synType->getInv(); //ATC 1-26-2012 - Added to make the math easier to visualize during flooring
	float floored; //ATC 1-26-2012 - The floored weight change
	float nonfloored; //ATC 1-26-2012 - The nonfloored weight change
	const float synModRate = m_synType->getSynModRate();
	const int timeDiff = timeStep - lastActivate;
	const unsigned int NMDArise = m_synType->getNMDArise();
	const int fallDiff = timeDiff - NMDArise;	
	if ((timeDiff) >= static_cast<int>(SynapseType::MAX_TIME_STEP)) 
		{	//Never_Activated
			nonfloored = -synModRate * weight; //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 1: " << nonfloored << floored << std::endl;
			
			//weight -= synModRate * weight; ////ATC 1-26-2012 - Original weight change
		}
	else if (timeStep <= riseUntil) //Synapse last activated during a rise. Use the multiple fire rule.
		{
		float Z;
		if (timeStep - riseActivate > NMDArise) //zBar is saturated at maximum
			Z = 1;
		else Z = m_synType->alphaRiseArray[oldZbar][timeStep-riseActivate]; //zBar is still rising from the first fire
		
		if (m_synType->getLearningRule() == LRT_MvgAvg) 
			{
			// For moving averager, it only gets updated when synapse is
			// activated. Need to do calculations since that's happened (all zeros)
			nonfloored = (synModRate * (Z * mvgAvg - weight)); //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 2: " << nonfloored << floored << std::endl;
			//weight += static_cast<float>(synModRate * (Z * mvgAvg - weight)); //ATC 1-26-2012 - Original weight change
			}
		else
			{
			nonfloored = (synModRate * (Z - weight)); //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 3: " << nonfloored << floored << std::endl;
			//weight += static_cast<float>(synModRate * (Z - weight)); //ATC 1-26-2012 - Original weight change
			}
		}

	else if (timeDiff < NMDArise) //zBar is Still Rising, Use Rise Table
		{
		if (m_synType->getLearningRule() == LRT_MvgAvg) 
			{
			// For moving averager, it only gets updated when synapse is
			// activated. Need to do calculations since that's happened (all zeros)
			nonfloored = (synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] * mvgAvg - weight)); //ATC 1-26-2012 -
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 4: " << nonfloored << floored << std::endl;
			//weight += static_cast<float>(synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] * mvgAvg - weight)); //ATC 1-26-2012 - Original weight change
			}
		else if (m_synType->getLearningRule() == LRT_PostSyn)
			{
			nonfloored = (synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] - weight)); //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 5: " << nonfloored << floored << std::endl;
			//weight += static_cast<float>(synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] - weight)); //ATC 1-26-2012 - Original weight change
			}
		else
			{
			//Post Synaptic Learning Rule B
			//use previous last activation time until peak 
			//then decay according to new last activation time
			//(if (timeStep-lastActivate) < NMDArise)
			if( !((m_synType->alphaRiseArray[oldZbar][timeStep-lastActivate]) > 0))
				{
				nonfloored = (synModRate * (m_synType->alphaRiseArray[oldZbar][timeStep-prevLastActivate] - weight)); //ATC 1-26-2012
				floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
				//std::cout << "Here 6: " << nonfloored << floored << std::endl;
				//weight += static_cast<float>(synModRate * (m_synType->alphaRiseArray[oldZbar][timeStep-prevLastActivate] - weight)); //ATC 1-26-2012 - Original weight change
				}
			else
				{
				nonfloored = (synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] - weight)); //ATC 1-26-2012
				floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
				//std::cout << "Here 7: " << nonfloored << floored << std::endl;
				//weight += static_cast<float>(synModRate * (m_synType->alphaRiseArray[oldZbar][timeDiff] - weight)); //ATC 1-26-2012 - Original weight change
				}
			}
		}

	else //zBar is falling, Use Fall Table
		{
		if (m_synType->getLearningRule() == LRT_MvgAvg) 
			{
			// For moving averager, it only gets updated when synapse is
			// activated. Need to do calculations since that's happened (all zeros)
			nonfloored = (synModRate * (m_synType->alphaFallArray[fallDiff] * mvgAvg - weight)); //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 8: " << nonfloored << floored << std::endl;
			//weight += static_cast<float>(synModRate * (m_synType->alphaFallArray[fallDiff] * mvgAvg - weight)); //ATC 1-26-2012 - Original weight change
			}
		else if (m_synType->getLearningRule() == LRT_PostSyn)
			{
			nonfloored = (synModRate * (m_synType->alphaFallArray[fallDiff] - weight)); //ATC 1-26-2012
			//std::cout << synModRate << ", " << m_synType->alphaFallArray[fallDiff] << ", " << weight << std::endl;
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 9: NF: " << nonfloored << ", F: " << floored << std::endl;
			//weight += static_cast<float>(synModRate * (m_synType->alphaFallArray[fallDiff] - weight)); //ATC 1-26-2012 - Original weight change
			}
		else 
			{
			//Post Synaptic Learning Rule B
			//use previous last activation time until peak 
			//then decay according to new last activation time
			//(if (timeStep-lastActivate) < NMDArise)
			nonfloored = (synModRate * (m_synType->alphaFallArray[fallDiff] - weight)); //ATC 1-26-2012
			floored = floor(nonfloored*inv)/inv; //ATC 1-26-2012
			//std::cout << "Here 10: " << nonfloored << floored << std::endl; 
			//weight += static_cast<float>(synModRate * (m_synType->alphaFallArray[fallDiff] - weight));  //ATC 1-26-2012 - Original weight change
			}
		}
		//ATC 1-26-2012 - Added block to give the weight change a stochastic granularity
		if(stochastic == 1 ) {
			//Output::Out() << "\ninv = " << inv << "   res = " << resolution << " product = " << inv*resolution << std::endl;
			float random = rand() % 10000;
			random = random/10000;
			weight += floored;
			float difference = (nonfloored - floored) / resolution; //this gives us p for the bernoulli coin flip.			
			//if(difference < 0){
			//	std::cout << "Difference and Random: " << difference << ", " << random << std::endl; 
			//	std::cout << "Floored and Nonfloored: " << floored << ", " << nonfloored <<std::endl;
			//}
			if(difference > random){
				weight += resolution;
			}
			//ATC 1-26-2012 - End of block to give weight change a stochastic granularity
		} else {
			weight += nonfloored;
		}
	}

	inline int getLastActivate() const { return lastActivate; }
	inline SynapseType const* getSynapseType() const { return m_synType; }
	inline void setLastActivate(const int toSet) 
	{
	prevLastActivate = lastActivate;
	lastActivate = toSet; 
	}
	inline void resetLastActivate() 
	{ 
	prevLastActivate = lastActivate;
	lastActivate = NEVER_ACTIVATED; 
	}
	inline void setOldZbar(const float toSet) {oldZbar = static_cast<int>(toSet*1000);}
	static Noise SynNoise;                 // rng for syn failure

private:
	bool m_isExc;
	bool m_isInhDiv;
	unsigned int srcNeuron;
	unsigned int destNeuron;
	float weight;
	int oldZbar;
	int riseUntil;
	int riseActivate;
	int lastActivate; // actual last activation time
	int prevLastActivate; // MEGHAN: adjusted activation time before last
	float mvgAvg;
	SynapseType const* m_synType;
	// Static variables
	// NEVER_ACTIVATED is set so that if lastActivate == NEVER_ACTIVATED,
	// then timeStep - lastActivate >= MAX_TIME_STEP (because timeStep >= 0)
	const static int NEVER_ACTIVATED = -1 - static_cast<int>(SynapseType::MAX_TIME_STEP);
};

typedef DendriticSynapse * Dendrite;
typedef DendriticSynapse const * const DendriteConst;

// AxonalSynapse = Synapse on axon
class AxonalSynapse 
{
public:
   inline AxonalSynapse(): synapse(NULL) {};
   inline void connectSynapse(unsigned int destNeuron, DendriticSynapse &dendritic,
                        SynapseType const* synType, bool isExc=true, bool isInhDiv=false) 
{
      synapse = &dendritic;
      synapse->connectNeuron(destNeuron, synType, isExc, isInhDiv);
}
   inline bool connectsTo(const DendriticSynapse &dendritic) const 
{
      return synapse == &dendritic;
}
   inline void activate(DataList &bus, DataList &bus_inhdiv, DataList &bus_inhsub, const int timeStep) 
{
      synapse->activate(bus, bus_inhdiv, bus_inhsub, timeStep);
}
   inline float getWeight() const { return synapse->getWeight(); }
   inline void setWeight(const float toSet) { synapse->setWeight(toSet); }
   inline int getLastActivate() const { return synapse->getLastActivate(); }
   inline void setLastActivate(const int toSet) { synapse->setLastActivate(toSet); }
private:
   DendriticSynapse *synapse; // Points to corresponding DendriticSynapse
};

typedef AxonalSynapse * AxonalSegment;
typedef AxonalSegment * Axon;
typedef AxonalSegment const * const AxonConst;

#endif //SYNAPSE_HPP

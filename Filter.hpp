#if !defined(FILTER_HPP)
#  define FILTER_HPP

#  include <vector>
#  if !defined(ARGFUNCTS_HPP)
#    include "ArgFuncts.hpp"
#  endif
#  if !defined(DATATYPES_HPP)
#    include "DataTypes.hpp"
#  endif

class Filter {
public:
   // initialize to filter of size 1 and value 1
   Filter(): m_size(1) {
     m_filterValues = new float[m_size];
     m_filterValues[0] = 1.0f;
   }
   Filter(const Filter& f): m_size(f.m_size) {
      m_filterValues = new float[m_size];
      for (unsigned int i=0; i<m_size; ++i) {
         m_filterValues[i] = f.m_filterValues[i];
      }
   }
   ~Filter() {
      delArray(m_filterValues);
   }
   Filter& operator=(const Filter& f) {
      if (this != &f) {   // make sure not same object
         delArray(m_filterValues);
         m_size = f.m_size;
         m_filterValues = new float[m_size];
         for (unsigned int i=0; i<m_size; ++i) {
            m_filterValues[i] = f.m_filterValues[i];
         }
      }
      return *this;        // Return ref for multiple assignment
   }
   // historicalData has most recent data up front (push_front(), pop_back())
   inline float apply(const float* historicalData) const {
      float toReturn = 0.0f;
      for (unsigned int timeOffset = 0; timeOffset < m_size; ++timeOffset) {
         toReturn += m_filterValues[timeOffset] * historicalData[timeOffset];
      }
      return toReturn;
   }
   float* getFilter() const { return m_filterValues; };
   void setFilter(const float* filterVals, unsigned int filterSize) {
//     Output::Out() << "in setFilter: " << std::endl;
     delArray(m_filterValues);
     m_size = filterSize;
     m_filterValues = new float[m_size];
     for (unsigned int i=0; i<m_size; ++i) {
       m_filterValues[i] = filterVals[i];
//       Output::Out() << filterVals[i] << " ";
     }
//     Output::Out() << std::endl;
   }
   void setFilter(const DataList &filterVals) {
//     Output::Out() << "in setFilter: " << std::endl;
     delArray(m_filterValues);
     m_size = filterVals.size();
     m_filterValues = new float[m_size];
     for (unsigned int i=0; i<m_size; ++i) {
       m_filterValues[i] = filterVals[i];
//	Output::Out() << filterVals[i] << " ";
     }
//	Output::Out() << std::endl;
   }
   unsigned int size() const { return m_size; }
private:
   float* m_filterValues;
   unsigned int m_size;
};

#endif

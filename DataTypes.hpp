#if !defined(DATATYPES_HPP)
#   define DATATYPES_HPP
///////////////////////////////////////////////////////////////////////////////
// System includes and defines
///////////////////////////////////////////////////////////////////////////////

#   include <climits>
#   include <list>
#   include <vector>
#   include <deque>
#   include <map>
#   include <string>
#   include <cmath>

enum DataListType { DLT_unknown, DLT_sequence, DLT_matrix, DLT_analysis, DLT_file };
const float verySmallFloat = 0.00000001f;
const float veryCloseToOne = 0.99999999f;

template<class T>
inline void updateMax(T &max, const T chk) {
   if (chk > max)
      max = chk;
}

typedef std::pair<unsigned int, unsigned int> UIPair;

#if defined(MEMORY)
#   define BOOL(X) X
   typedef std::vector<bool> Pattern;
#else
#   define BOOL(X) (X != 0)
   typedef std::vector<char> Pattern;
#endif
typedef std::vector<float> DataList;

typedef Pattern::iterator PatternIt;
typedef Pattern::const_iterator PatternCIt;
typedef DataList::iterator DataListIt;
typedef DataList::const_iterator DataListCIt;

// time x neuron
typedef std::list<Pattern> Sequence;
typedef std::list<DataList> DataMatrix;
typedef std::list<std::string> StrList;

typedef Sequence::iterator SequenceIt;
typedef Sequence::const_iterator SequenceCIt;
typedef DataMatrix::iterator DataMatrixIt;
typedef DataMatrix::const_iterator DataMatrixCIt;
typedef StrList::iterator StrListIt;
typedef StrList::const_iterator StrListCIt;

typedef std::vector<unsigned int> UIVector;
typedef UIVector::iterator UIVectorIt;
typedef UIVector::const_iterator UIVectorCIt;

typedef std::vector<UIVector> UIMatrix;
typedef std::list<UIVector> UIPtnSequence;
typedef UIMatrix::iterator UIMatrixIt;
typedef UIMatrix::const_iterator UIMatrixCIt;
typedef std::deque<UIVector> UIVectorDeque;
typedef UIPtnSequence::iterator UIPtnSequenceIt;
typedef UIPtnSequence::const_iterator UIPtnSequenceCIt;

typedef std::pair<std::string, bool> ArgType;
typedef std::vector<ArgType> ArgListType;
typedef ArgListType::iterator ArgListTypeIt;
typedef ArgListType::const_iterator ArgListTypeCIt;
inline ArgListType StrToArgListType(std::string fromString) {
   return ArgListType(1, ArgType(fromString, false));
}

typedef void (*AT_FUN)(ArgListType &arg);
typedef std::string (*CARET_FUN)(ArgListType &arg);

/* Struct definition for competitive */
struct IxSumwz {
   IxSumwz(): ix(0), y(0) {};
   IxSumwz(int index, double val): ix(index), y(val) {};
   int ix;
   double y;
};
inline bool operator < (const IxSumwz &a, const IxSumwz &b) { return a.y < b.y; };
inline bool operator > (const IxSumwz &a, const IxSumwz &b) { return a.y > b.y; };

/* Formerly the contents of xInput.hpp: */
class xInput {
private:
   Pattern extPtn;
   long numOn;
public:
   inline xInput(): numOn(0) {};
   inline xInput(const int numNeurons): extPtn(Pattern(numNeurons, false)),
        numOn(0) {};
   inline xInput(const int numNeurons, const Pattern &inPtn):
                 extPtn(inPtn), numOn(0) {
      for (int nrn=0; nrn<numNeurons; nrn++) {
#if defined(CHECK_BOUNDS)
         if (inPtn.at(nrn)) ++numOn;
#else
         if (inPtn[nrn]) ++numOn;
#endif
      }
   };
   inline void initialize(const int numNeurons) {
      numOn = 0;
      extPtn = Pattern(numNeurons, false);
   }
#if defined(CHECK_BOUNDS)
   inline bool at(const int &index) const { return BOOL(extPtn.at(index)); };
   inline bool operator [] (const int &index) { return BOOL(extPtn.at(index)); };
   inline bool operator [] (const int &index) const { return BOOL(extPtn.at(index)); };
#else
   inline bool at(const int &index) const { return BOOL(extPtn[index]); };
   inline bool operator [] (const int &index) { return BOOL(extPtn[index]); };
   inline bool operator [] (const int &index) const { return BOOL(extPtn[index]); };
#endif
   inline void turnOn(const int index) {
#if defined(CHECK_BOUNDS)
      extPtn.at(index) = true;
#else
      extPtn[index] = true;
#endif
      ++numOn;
   }
   inline void turnOff(const int index) {
#if defined(CHECK_BOUNDS)
      extPtn.at(index) = false;
#else
      extPtn[index] = false;
#endif
      --numOn;
   }
   inline long numExternals() const { return numOn; };
   inline long size() const { return extPtn.size(); };
};

typedef std::list<xInput> xInputList;
typedef xInputList::const_iterator xInputListCIt;
typedef xInputList::iterator xInputListIt;

inline DataList UIPtnToDataList(const UIVector &ptnIn) {
   // assumes ptnIn is sorted
   unsigned int numNeurons = (ptnIn.size()==0) ? 0 : (ptnIn.back()+1);
   DataList toReturn(numNeurons);
   for (UIVectorCIt PCIt = ptnIn.begin(); PCIt != ptnIn.end(); PCIt++)
      toReturn[*PCIt]=1.0f;
   return toReturn;
}

inline UIVector DataListToUIPtn(const DataList &DLIn) {
   unsigned int ptnSize = DLIn.size();
   UIVector toReturn(0);
   for (unsigned int i=0; i<ptnSize; ++i)
      if (fabs(DLIn[i]) > verySmallFloat)
         toReturn.push_back(i);
   return toReturn;
}

inline UIVector xInputToUIPtn(const xInput &xIn) {
   unsigned int ptnSize = xIn.size();
   UIVector toReturn(0);
   for (unsigned int i=0; i<ptnSize; ++i)
      if (xIn[i]) 
         toReturn.push_back(i);
   return toReturn;
}

inline DataMatrix UISeqToMatrix(const UIPtnSequence &seqIn) {
   unsigned int mtxSize = seqIn.size();
   DataMatrix toReturn(mtxSize);
   DataMatrixIt DMIt = toReturn.begin();
   // (Willow) This is where the problem is
   for (UIPtnSequenceCIt SeqConstIterator = seqIn.begin();
      SeqConstIterator != seqIn.end(); SeqConstIterator++, DMIt++)
      *DMIt = UIPtnToDataList(*SeqConstIterator);
   return toReturn;
}

inline UIPtnSequence MatrixToUISeq(const DataMatrix &mtxIn) {
   unsigned int seqSize = mtxIn.size();
   UIPtnSequence toReturn(seqSize);
   UIPtnSequenceIt SIt = toReturn.begin();
   for (DataMatrixCIt DMCIt = mtxIn.begin(); DMCIt != mtxIn.end(); DMCIt++, SIt++)
      *SIt = DataListToUIPtn(*DMCIt);
   return toReturn;
}

// Converts from a list of "neuron numbers" to a boolean vector (Pattern)
inline Pattern UIVectorToPtn(const UIVector &v, unsigned int numNeurons=0) {
   if ((numNeurons == 0) && (v.size() > 0)) numNeurons = v.back()+1; // assumes v is sorted
   Pattern toReturn(numNeurons, false);
   for (UIVectorCIt it = v.begin(); it != v.end(); ++it) {
      toReturn[*it] = true;
   }
   return toReturn;
}

// Vice-versa of the above
inline UIVector PtnToUIVector(const Pattern &p) {
   UIVector toReturn(0);
   for (unsigned int i=0; i < p.size(); ++i) {
      if (p[i]) toReturn.push_back(i);
   }
   return toReturn;
}

// Converts from a list of a list of "neuron numbers" to a list
// of boolean vectors (Patterns)
inline Sequence UISeqToPtnSeq(const UIPtnSequence &s, unsigned int numNeurons=0) {
   Sequence toReturn(0);
   for (UIPtnSequenceCIt it = s.begin(); it != s.end(); ++it) {
      toReturn.push_back(UIVectorToPtn(*it, numNeurons));
   }
   return toReturn;
}

// Vice-versa of the above
inline UIPtnSequence PtnSeqToUISeq(const Sequence &s) {
   UIPtnSequence toReturn(0);
   for (SequenceCIt it = s.begin(); it != s.end(); ++it) {
      toReturn.push_back(PtnToUIVector(*it));
   }
   return toReturn;
}

#endif

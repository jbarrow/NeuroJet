///////////////////////////////////////////////////////////////////////////////
// ArgFuncts.hpp
//
// prototypes and definitions for C++ function argument processing,
//
// Matt Harrison
//
// Last Update: 6/02/98
//
///////////////////////////////////////////////////////////////////////////////
//
// High Level Command Line Processing
//
// class IntArg(const string flag, const string help, int default)
// class IntArg(const string flag, const string help)
// class DblArg(const string flag, const string help, double default)
// class DblArg(const string flag, const string help)
// class StrArg(const string flag, const string help, string default)
// class StrArg(const string flag, const string help)
// class FlagArg(const string onflag, const string offflag, const string help, int default)
// class FlagArg(const string onflag, const string offflag, const string help)
//
// class CommandLine(string FunctionName)
// CommandLine.IntSet(int numints, IntArg * arg1, ...)
// CommandLine.DblSet(int numdbls, DblArg * arg1, ...)
// CommandLine.StrSet(int numstrs, StrArg * arg1, ...)
// CommandLine.FlagSet(int numflags, FlagArg * arg1, ...)
// CommandLine.HelpSet(string helpstr);
// CommandLine.Process(int argc, const string argv[], ostream& errorout)
// CommandLine.Process(int argc, const string argv[], ostream& errorout, ostream& callout)
// CommandLine.DisplayHelp(ostream &helpout)
//
// The IntArg, DblArg, and StrArg classes allow you to define integers,
// doubles, and strings for commandline processing. The first parameter
// is the flag that is searched for in the command line. The second is
// the help message that should be printed when -help is invoked. The
// third is the value that the variable should default to. If no default
// is given then the program will exit with an error if the indicated flag
// is not found in the command line.
// The FlagStr class allows you to define flags for commandline processing.
// The first parameter is the on flag, the second is the off flag, and the
// third is the help. If the onflag is found in the commandline then the
// variable takes a value of 1. If the offflag is found then it takes
// a value of zero. If neither are found then it becomes the default{0,1}.
// If both are found then it becomes the opposite of the default. If no
// default is given then exactly one of the flags(on or off) must be
// present in the commandline or the program exits with an error.
//
// The CommandLine class processing the command lines. After all of the
// Arg types are declared, then they must be passed (as pointers) to the
// CommandLine variable using the Set functions.
//
// Finally, the Process function is given argc and argv and the command line
// is processed. If -help is encountered, then the help commands are
// printed and the program exits. Any unrecognized arguments are reported.
// The complete function call with all defaults inserted is echoed to the
// screen. The values of the variables can be accessed and manipulated
// with the() operator.
//
//
///////////////////////////////////////////////////////////////////////////////
//
// Lower Level CommandLine Processing Functions
//
// int findarg(string str, int argc, string argv[])
//
// findarg returns the index for which argv[index] = str
// if no such index exists, then it returns argc
//
// examples:
// if (findarg("-help",argc,argv) == argc) cerr << "Sorry, No Help." << endl;
// int numfiles = findarg("-nf",argc,argv);
//
///////////////////////////////////////////////////////////////////////////////
//
// Numerical string processing commands and basic string processing commands
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(ARGFUNCTS_HPP)
#define ARGFUNCTS_HPP

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using std::cerr;
using std::endl;
using std::list;
using std::map;
using std::ostream;
using std::pair;
using std::setw;
using std::string;
using std::vector;

#if !defined(DATATYPES_HPP)
#   include "DataTypes.hpp"
#endif

#if !defined(OUTPUT_HPP)
#   include "Output.hpp"
#endif

////////////////////////////////////////////////////////////////////////////////
// Some comparison functions
////////////////////////////////////////////////////////////////////////////////

using std::min;
using std::max;

////////////////////////////////////////////////////////////////////////////////
// Some matrix/vector functions (see also setMatrixRange, which depends on IntArg)
////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T vectorSum(const vector<T> &toSum)
{
   T toReturn = static_cast<T>(0);
   for (typename vector<T>::const_iterator it = toSum.begin(); it != toSum.end(); it++) {
      toReturn += *it;
   }
   return toReturn;
}

// sums over columns (into rows)
template<class T>
inline vector<T> matrixSum(const vector<vector<T> > &toSum)
{
   vector<T> toReturn(toSum.size());
   for (unsigned int i = 0; i < toSum.size(); i++) {
#if defined(CHECK_BOUNDS)
      toReturn.at(i) = vectorSum(toSum.at(i));
#else
      toReturn[i] = vectorSum(toSum[i]);
#endif   
   }
   return toReturn;
}

inline double matrixMean(const DataMatrix &toAvg)
{
   double Accum = 0.0L;
   double Denom = 0.0L;
   for (DataMatrixCIt it = toAvg.begin(); it != toAvg.end(); it++) {
      for (unsigned int i = 0; i < it->size(); i++) {
#if defined(CHECK_BOUNDS)
         Accum += it->at(i);
#else
         Accum += (*it)[i];
#endif
         ++Denom;
      }
   }
   return Accum / Denom;   
}

inline double matrixMoment(const DataMatrix &Matrix, const float avg, const int moment)
{
   double Accum = 0.0L;
   double Denom = 0.0L;
   for (DataMatrixCIt it = Matrix.begin(); it != Matrix.end(); it++) {
      for (unsigned int i = 0; i < it->size(); i++) {
         const float diff = (it->at(i) - avg);
         Accum += pow(diff, moment);
         ++Denom;
      }
   }
   return Accum / Denom;   
}

inline double matrixAvgSS(const DataMatrix &Matrix)
{
   double Accum = 0.0L;
   double Denom = 0.0L;
   for (DataMatrixCIt it = Matrix.begin(); it != Matrix.end(); it++) {
      for (unsigned int i = 0; i < it->size(); i++) {
         Accum += (it->at(i)) * (it->at(i));
         ++Denom;
      }
   }
   return Accum / Denom;   
}

inline double matrixVar(const DataMatrix &Matrix)
{
   const double avg = matrixMean(Matrix);
   return matrixAvgSS(Matrix) - avg * avg;
}

inline double matrixSkew(const DataMatrix &Matrix)
{
   const double avg = matrixMean(Matrix);
   const double var = matrixVar(Matrix);
   const double moment3 = matrixMoment(Matrix, avg, 3);
   return moment3 / pow(var,1.5);
}

inline double matrixKurt(const DataMatrix &Matrix)
{
   const double avg = matrixMean(Matrix);
   const double var = matrixVar(Matrix);
   const double moment4 = matrixMoment(Matrix, avg, 4);
   // The modern definition of kurtosis includes the "-3" term
   // Thus, a normal distribution has zero kurtosis
   return moment4 / (var*var) - 3;
}

// works on vector<vector<foo> >, list<list<foo> >, list<vector<foo> >, etc.
// but slower than necessary
template<class T>
inline T transposeMatrixSlow(const T &toTranspose)
{
   unsigned int oldRowSize = toTranspose.size();
   unsigned int oldColSize = findMaxSize(toTranspose);
   typename T::const_iterator itSrcRow;
   typename T::value_type::const_iterator itSrcCol;
   typename T::iterator itDstRow;
   typename T::value_type::iterator itDstCol;
   typename T::value_type emptySubType(oldRowSize);
   T toReturn(oldColSize, emptySubType);   
   itDstRow = toReturn.begin();
   unsigned int dstColCnt = 0;
   for (itSrcRow = toTranspose.begin(); itSrcRow != toTranspose.end(); ++itSrcRow, ++dstColCnt) {
      itDstRow = toReturn.begin();
      for (itSrcCol = itSrcRow->begin(); itSrcCol != itSrcRow->end(); ++itSrcCol, ++itDstRow) {
         itDstCol = itDstRow->begin();
         for (unsigned int inc = 0; inc < dstColCnt; ++inc, ++itDstCol) {}
         *itDstCol = *itSrcCol;
      }
   }
   return toReturn;
}

template<class T>
inline list<vector<T> > transposeMatrix(const list<vector<T> > &toTranspose)
{
   unsigned int oldRowSize = toTranspose.size();
   unsigned int oldColSize = findMaxSize(toTranspose);
   typename list<vector<T> >::const_iterator itSrcRow;
   typename vector<T>::const_iterator itSrcCol;
   typename list<vector<T> >::iterator itDstRow;
   list<vector<T> > toReturn(oldColSize, vector<T>(oldRowSize, static_cast<T>(0)));
   itDstRow = toReturn.begin();
   unsigned int dstColCnt = 0;
   for (itSrcRow = toTranspose.begin(); itSrcRow != toTranspose.end(); ++itSrcRow, ++dstColCnt) {
      itDstRow = toReturn.begin();
      for (itSrcCol = itSrcRow->begin(); itSrcCol != itSrcRow->end(); ++itSrcCol, ++itDstRow) {
#if defined(CHECK_BOUNDS)
         itDstRow->at(dstColCnt) = *itSrcCol;
#else
         (*itDstRow)[dstColCnt] = *itSrcCol;
#endif
      }
   }
   return toReturn;
}

// first & last are 1-based, VecIn & toReturn are 0-based
template<class T>
inline vector<T> SubVector(const vector<T> &VecIn, unsigned int first, unsigned int last) {
   vector<T> toReturn(last - first + 1, static_cast<T>(0));
   for (unsigned int nrn = first-1; nrn < std::min(last, static_cast<unsigned int>(VecIn.size())); ++nrn)
#if defined(CHECK_BOUNDS)
      toReturn.at(nrn-first+1) = VecIn.at(nrn);
#else
      toReturn[nrn-first+1] = VecIn[nrn];
#endif
   return toReturn;
}

// first & last are 1-based, mtxIn & toReturn are 0-based
template<class T>
inline list<vector<T> > SubMatrix(const list<vector<T> > &mtxIn,
                                  unsigned int firstT, unsigned int lastT,
                                  unsigned int firstN, unsigned int lastN)
{
   unsigned int numRows = lastT - firstT + 1;
   unsigned int numCols = lastN - firstN + 1;
   list<vector<T> > toReturn(numRows, vector<T>(numCols, static_cast<T>(0)));
   typename list<vector<T> >::const_iterator MCIt = mtxIn.begin();
   for (unsigned int inc = 1; inc < firstT; ++inc, ++MCIt) {}
   typename list<vector<T> >::iterator MIt = toReturn.begin();
   for (unsigned int i = firstT-1; i < lastT; ++i, ++MCIt, ++MIt) {
      *MIt = SubVector(*MCIt, firstN, lastN);
   }
   return toReturn;
}

////////////////////////////////////////////////////////////////////////////////
// Some integer functions
////////////////////////////////////////////////////////////////////////////////

template<class T>
inline int ifloor(const T data_in)
{ return static_cast<int>(data_in); }

template<class T>
inline int iround(const T data_in)
{ return static_cast<int>(data_in + 0.5); }

template<class T>
inline int iceil(const T data_in)
{ return static_cast<int>(ceil(data_in)); }

////////////////////////////////////////////////////////////////////////////////
// Some string manipulation functions
////////////////////////////////////////////////////////////////////////////////
// Modified from code at http://www.adp-gmbh.ch/cpp/config_file.html
inline std::string ltrim(std::string const& source, char const* delims = " \t\r\n") {
  std::string result(source);
  std::string::size_type index = result.find_first_not_of(delims);
  if (index != std::string::npos)
    result.erase(0, index);
  else
    result.erase();
  return result;
}

// Modified from code at http://www.adp-gmbh.ch/cpp/config_file.html
inline std::string rtrim(std::string const& source, char const* delims = " \t\r\n") {
  std::string result(source);
  std::string::size_type index = result.find_last_not_of(delims);
  if (index != std::string::npos)
    result.erase(++index);
  return result;
}

template <class T>
inline std::string to_bytes(const T data_in)
{
    std::string toReturn(sizeof(T),'\0');
    char* toCopy = (char *) &data_in;
    for (int i=0; i<sizeof(T); ++i)
        toReturn[i] = toCopy[i];
    return toReturn;
}

template <class T>
inline T from_bytes(const std::string& data_in)
{
    const char* toCopy = data_in.data();
    return (const T &) *toCopy;
}

inline std::string ucase(const std::string &s)
{
   std::string toReturn = "";
   for (unsigned int i=0; i<s.length(); ++i)
      toReturn += toupper(s[i]);
   return toReturn;
}

template<class T>
inline std::string to_string(const T data_in)
{
   std::ostringstream os;
   os << std::setprecision(20) << data_in;
   return os.str();
}

template<class T>
T from_string(const std::string &data_in)
{
   if (data_in[0] == '.')
      return from_string<T>("0" + data_in);
   std::istringstream is(data_in);
   T data_out;
   is >> data_out;
   return data_out;
}

// tokenize separates the string s using token, except where token is inside of
// elements specified by groups. For example, if groups is ""()[], then the
// token will be ignored when it is inside quotes, parentheses, or square
// brackets.
inline vector<string> tokenize(const std::string &s, const char token,
                               const std::string &groups)
{
   vector<string> toReturn;
   const unsigned int numGrps = groups.size() / 2;
   vector<int> grpCnt = vector<int>(numGrps, 0);
   string curToken = "";
   for (unsigned int i = 0; i < s.size(); ++i) {
      char curChar = s[i];
      bool inGrp = false;
      for (unsigned int j = 0; j < numGrps; ++j) {
         if (groups[j*2] == groups[j*2+1]) {
            if (curChar == groups[j*2]) {
               if (grpCnt[j] > 0)
                  --grpCnt[j];
               else
                  ++grpCnt[j];
            }
         } else {
            if (curChar == groups[j*2])
               ++grpCnt[j];
            else if (curChar == groups[j*2+1])
               --grpCnt[j];
         }
         if (grpCnt[j] > 0) {
            inGrp = true;
            break;
         } else if (grpCnt[j] < 0) {
            CALL_ERROR << "Mismatching " << groups[j*2] << groups[j*2+1] << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
      if ((curChar == token) && !inGrp) {
         toReturn.push_back(curToken);
         curToken = "";
      } else {
         curToken += curChar;
      }
   }
   if (curToken.size() > 0)
      toReturn.push_back(curToken);
   return toReturn;
}

////////////////////////////////////////////////////////////////////////////////
// Some data deallocation functions
////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void delPtr(T &ptr)
{
   if (ptr) {
      delete ptr;
      ptr = NULL;
   }
}

template<class T>
inline void delArray(T &ptr)
{
   if (ptr) {
      delete[] ptr;
      ptr = NULL;
   }
}

template<class T>
inline void delMatrix(T &ptr, int height)
{
   if (ptr) {
      for (int i=0; i<height; i++)
         delArray(ptr[i]);
      delete[] ptr;
      ptr = NULL;
   }
}

template<class T>
inline void delTensor(T &ptr, int otherDim, int height)
{
   if (ptr) {
      for (int i=0; i<otherDim; i++)
         delMatrix(ptr[i], height);
      delete[] ptr;
      ptr = NULL;
   }
}

inline void memcheck(const void * test, const string str = "Out of memory");

////////////////////////////////////////////////////////////////////////////////
// Some common TArg routines
////////////////////////////////////////////////////////////////////////////////
// Evidently, this isn't built into vector
ArgListTypeIt find(ArgListType &argList, const ArgType toFind);

template<class T>
inline bool setTValue(ArgListType &argv, const string Flag, T &Value,
                      const bool IsDeault, const T Deault)
{
   // Find an argument that matches the flag, but that hasn't already been found
   ArgListTypeIt thing1 = find(argv, ArgType(Flag, false));
   if (thing1 != argv.end()) {
      // Check out its corresponding value
      ArgListTypeIt thing2 = thing1 + 1;
      // If we're not at the end and not already marked as being found
      if ((thing2 != argv.end()) && (!thing2->second)) {
         // Mark it and its value as being accounted for
         thing1->second = true;
         thing2->second = true;
         Value = from_string<T>(thing2->first);
         return true;
      }
   }
   if (!IsDeault) return false;
   Value = Deault;
   return true;
}

template<class T>
string setListValue(ArgListType &argv, const string Flag,
                 vector<T> &Value, const bool IsDeault, const vector<T> Deault)
{
   // Find an argument that matches the flag, but that hasn't already been found
   ArgListTypeIt flagpos = find(argv, ArgType(Flag, false));
   ArgListTypeIt assignpos = flagpos;
   // Move to the value corresponding to the flag
   if (assignpos != argv.end()) assignpos++;
   if (assignpos == argv.end() && !IsDeault) {
      return "Unable to find required flag " + Flag;
   }
   if (assignpos == argv.end()) {
      // Mark the flag accounted for if it was given with no arguments and
      // default is allowed
      if (flagpos != argv.end()) flagpos->second = true;
      Value = Deault;
   } else {
      int numvals = from_string<int>(assignpos->first);
      if (numvals < 0) {
         return "Bad number of arguments following " + Flag;
      }
      Value = vector<T>(numvals);
      // Mark the flag accounted for
      flagpos->second = true;
      // Mark the # accounted for
      assignpos->second = true;
      assignpos++; // Move to the first item in the list (after the #)
      for (int j = 0; j < numvals; j++, assignpos++) {
         // If the argument is already accounted for
         if (assignpos->second) {
            return "Unable to find " + to_string(numvals) + " arguments after " + Flag;
         }
         // Mark the value accounted for
         assignpos->second = true;
#if defined(CHECK_BOUNDS)
         Value.at(j) = from_string<T>(assignpos->first);
#else
         Value[j] = from_string<T>(assignpos->first);
#endif
      }
   }
   return ""; // No errMsg
}

template<class T>
inline void printTHelp(ostream &os, const string &Flag, const string &Help,
                       const bool IsDeault, const T &Deault)
{
   int w = 10;
   string s = " : ";
   os << std::setw(w) << Flag << s << std::setw(w)
      << (IsDeault ? to_string<T>(Deault) : "{required}")
      << s << Help << "\n";
}

template<class T>
void printListHelp(ostream &os, const string &Flag, const string &Help,
                   const bool IsDeault, const vector<T> &Deault)
{
   int w = 10;
   string s = " : ";
   os << std::setw(w - 3) << Flag << " # " << s << std::setw(w);
   if (IsDeault) {
      string nada = "{0 items}";
      os << (Deault.size() ? to_string<T>(Deault.at(0)) : nada) << s;
      for (unsigned int i = 1; i < Deault.size(); i++)
         os << "\n" << std::setw(w) << " " << s << std::setw(w) << Deault[i] << s;
   } else {
      os << "{required}" << s;
   }
   os << Help << "\n";
}

///////////////////////////////////////////////////////////////////////////////
class IntArg {
 public:
   IntArg(const string flag, const string help, const int deault):
       Value(deault), Flag(flag), Help(help), Deault(deault), IsDeault(true) {
   };
   IntArg(const string flag, const string help):
      Value(0), Flag(flag), Help(help), Deault(0), IsDeault(false) {
   };
   inline string getFlag() const { return Flag; };
   inline int getValue() const { return Value; };
   void printHelp(ostream &os) const {
      printTHelp(os, Flag, Help, IsDeault, Deault);
   };
   inline bool setValue(const int newValue) {Value = newValue; return true;};
   inline void swapValue(IntArg &OtherArg) {std::swap(Value, OtherArg.Value);};
   bool setValue(ArgListType &argv) {
      return setTValue(argv, Flag, Value, IsDeault, Deault);
   };
 private: 
   int Value;
   string Flag;
   string Help;
   int Deault;
   bool IsDeault;
};

inline ostream& operator << (ostream &os, const IntArg &x)
{
   return (os << x.getFlag() << " " << x.getValue());
}

class IntArgList {
 public:
   IntArgList(const string flag, const string help):
      Flag(flag), Help(help), IsDeault(false), errMsg("") {};

   inline string getErr() const { return errMsg; };
   inline string getFlag() const { return Flag; };
   inline int getValue(const int i) const { return Value.at(i); };
   void printHelp(ostream &os) const {
      printListHelp(os, Flag, Help, IsDeault, Deault);
   };
   bool setValue(ArgListType &argv) {
      errMsg = setListValue(argv, Flag, Value, IsDeault, Deault);
      return errMsg.size() == 0;
   };

   inline unsigned int size() const {return Value.size();};
   inline int &operator[] (const int &index) {return Value.at(index);};

 private:
   string Flag;
   string Help;
   bool IsDeault;
   string errMsg;
   vector<int> Value;
   vector<int> Deault;
};

inline ostream& operator << (ostream &os, const IntArgList &x)
{
   int numvals = x.size();
   os << x.getFlag() << " " << numvals << " ";
   for (int i = 0; i < numvals; i++)
      os << x.getValue(i) << " ";
   return os;
}

class DblArg {
 public:
   DblArg(const string flag, const string help, const double &deault): 
       Value(deault), Flag(flag), Help(help), Deault(deault), IsDeault(true) {
   };
   DblArg(const string flag, const string help): Value(0.0), Flag(flag),
       Help(help), Deault(0.0), IsDeault(false) {
   };
   inline string getFlag() const { return Flag; };
   inline double getValue() const { return Value; };
   void printHelp(ostream &os) const {
      printTHelp(os, Flag, Help, IsDeault, Deault);
   };
   inline bool setValue(const double newValue) {Value = newValue; return true;};
   bool setValue(ArgListType &argv) {
      return setTValue(argv, Flag, Value, IsDeault, Deault);
   };

 private:
   double Value;
   string Flag;
   string Help;
   double Deault;
   bool IsDeault;
};

inline ostream& operator << (ostream &os, const DblArg &x)
{
   return (os << x.getFlag() << " " << x.getValue());
}

class DblArgList {
 public:
   DblArgList(const string flag, const string help):
           Flag(flag), Help(help), IsDeault(false), errMsg("") {};

   inline string getErr() const { return errMsg; };
   inline string getFlag() const { return Flag; };
   inline double getValue(const int i) const { return Value.at(i); };
   void printHelp(ostream &os) const {
      printListHelp(os, Flag, Help, IsDeault, Deault);
   };
   bool setValue(ArgListType &argv) {
      errMsg = setListValue(argv, Flag, Value, IsDeault, Deault);
      return errMsg.size() == 0;
   };

   inline unsigned int size() const { return Value.size(); };
   inline double &operator[] (const int &index) { return Value[index]; };

 protected:
   string Flag;
   string Help;
   bool IsDeault;
   string errMsg;
   vector<double> Value;
   vector<double> Deault;
};

inline ostream& operator << (ostream &os, const DblArgList &x)
{
   int numvals = x.size();
   os << x.getFlag() << " " << numvals << " ";
   for (int i = 0; i < numvals; i++)
      os << x.getValue(i) << " ";
   return os;
}

class StrArg {
 public:
   StrArg(const string flag, const string help, string deault):
       Value(""), Flag(flag), Help(help), Deault(deault), IsDeault(true) {
   };
   StrArg(const string flag, const string help):
       Value(""), Flag(flag), Help(help), Deault(""), IsDeault(false) {
   };
   inline string getFlag() const { return Flag; };
   inline string getValue() const { return Value; };
   void printHelp(ostream &os) const {
      printTHelp(os, Flag, Help, IsDeault, Deault);
   };
   inline bool setValue(const string newValue) {Value = newValue; return true;};
   bool setValue(ArgListType &argv) {
      return setTValue(argv, Flag, Value, IsDeault, Deault);
   }

 private:
   string Value;
   string Flag;
   string Help;
   string Deault;
   bool IsDeault;
};

inline ostream& operator << (ostream &os, const StrArg &x)
{
   return (os << x.getFlag() << " " << x.getValue());
}

class StrArgList {
 public:
   StrArgList(const string flag, const string help, const bool isOptional = false):
      Flag(flag), Help(help), IsOptional(isOptional), errMsg("") {};

   inline string getErr() const { return errMsg; };
   inline string getFlag() const { return Flag; };
   inline string getValue(const int i) const { return Value.at(i); };
   void printHelp(ostream &os) const {
      printListHelp(os, Flag, Help, IsOptional, Deault);
   };
   bool setValue(ArgListType &argv) {
      errMsg = setListValue(argv, Flag, Value, IsOptional, Deault);
      return errMsg.size() == 0;
   };

   inline unsigned int size() const {return Value.size();};
   inline string operator[] (const int &index) { return Value.at(index); };

 private:
   string Flag;
   string Help;
   bool IsOptional;
   string errMsg;
   vector<string> Value;
   vector<string> Deault;
};

inline ostream& operator << (ostream &os, const StrArgList &x)
{
   int numvals = x.size();
   os << x.getFlag() << " " << numvals << " ";
   for (int i = 0; i < numvals; i++)
      os << x.getValue(i) << " ";
   return os;
}

class FlagArg {
 public:
   FlagArg(const string onflag, const string offflag, string help, const bool &deault):
       Value(deault), OnFlag(onflag), OffFlag(offflag), Help(help),
       Deault(deault), IsDeault(true) {
   };
   FlagArg(const string onflag, const string offflag, string help):
       Value(false), OnFlag(onflag), OffFlag(offflag), Help(help),
       Deault(false), IsDeault(false) {
   };
   inline string getFlag() const { return (Value ? OnFlag : OffFlag); };
   inline bool getValue() const { return Value; };
   void printHelp(ostream &os) const;
   inline bool setValue(const bool newValue) { Value = newValue; return true; };
   bool setValue(ArgListType &argv);

 protected:
   bool Value;
   string OnFlag;
   string OffFlag;
   string Help;
   bool Deault;
   bool IsDeault;
};

class CommandLine {
 public:
   CommandLine(const std::string funcname): FuncName(funcname), HelpString("") {};
   ~CommandLine() {};

   void IntSet(int number, ...);
   void DblSet(int number, ...);
   void StrSet(int number, ...);
   void FlagSet(int number, ...);
   void IntListSet(int number, ...);
   void DblListSet(int number, ...);
   void StrListSet(int number, ...);

   inline void Process(ArgListType &argv, std::ostream & eout) {
      Process(argv, eout, cerr, false);
   };
   inline void Process(ArgListType &argv, std::ostream & eout,
                       std::ostream & sout) {
      Process(argv, eout, sout, true);
   };

   inline void HelpSet(const std::string str) {
      HelpString = str;
   };
   void DisplayHelp(std::ostream & eout) const;

 private:

   void Process(ArgListType &argv, std::ostream & eout, std::ostream & sout,
                const bool dolog);

   vector<IntArg *> IntList;
   vector<DblArg *> DblList;
   vector<StrArg *> StrList;
   vector<FlagArg *> FlagList;
   vector<IntArgList *> IntListList;
   vector<DblArgList *> DblListList;
   vector<StrArgList *> StrListList;

   string FuncName;
   string HelpString;
};

inline ostream& operator << (ostream &os, const FlagArg &x)
{
   return (os << x.getFlag());
}

template<class T>
inline void setMatrixRange(int &startT, int &endT, int &startN, int &endN,
                           const IntArg &StartTime, const IntArg &EndTime,
                           const IntArg &StartNeuron, const IntArg &EndNeuron,
                           const list<vector<T> > cfMtx, const CommandLine &ComL,
                           const string &FunctionName)
{
   startN = StartNeuron.getValue();
   endN = EndNeuron.getValue();
   startT = StartTime.getValue();
   endT = EndTime.getValue();

   const int lastN = findMaxSize(cfMtx);
   const int lastT = cfMtx.size();

   if (endN == -1) {         
      endN = lastN;
   }
   if (endT == -1) {
      endT = lastT;
   }
   // startN = endN + 1 implies an empty range - this is allowed
   if (endN > lastN || startN < 1 || startN > (endN+1)) {
      CALL_ERROR << "Error in " << FunctionName << ": Neuron range of "
                 << startN << "..." << endN << " in a sequence with " << lastN
                 << " neurons is invalid." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (endT < 1 || endT > lastT || startT < 1 || startT > endT) {
      CALL_ERROR << "Error in " << FunctionName << ": Time range of "
                 << startT << "..." << endT << " is invalid." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
}

#endif

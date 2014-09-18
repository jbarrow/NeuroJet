#if !defined(SYSTEMVAR_HPP)
#  define SYSTEMVAR_HPP

#  if !defined(ARGFUNCTS_HPP)
#    include "ArgFuncts.hpp"
#  endif
#  if !defined(BINDLIST_HPP)
#    include "BindList.hpp"
#  endif
#  if !defined(DATATYPES_HPP)
#    include "DataTypes.hpp"
#  endif
#  if !defined(SIMSTATE_HPP)
#    include "SimState.hpp"
#  endif

struct Iterator {
   int CurrentVal;
   int StartVal;
   int EndVal;
   int StepVal;
};

template <class T> struct SystemData {
   T Data;
   bool IsReadOnly;
};

typedef SystemData<int> SysIntData;
typedef map<std::string, SysIntData> SysMapIntData;
typedef SystemData<float> SysFloatData;
typedef map<std::string, SysFloatData> SysMapFloatData;
typedef SystemData<std::string> SysStrData;
typedef map<std::string, SysStrData> SysMapStrData;
typedef SystemData<AT_FUN> SysAtData;
typedef map<std::string, SysAtData> SysMapAtData;
typedef SystemData<CARET_FUN> SysCaretData;
typedef map<std::string, SysCaretData> SysMapCaretData;
typedef SystemData<Iterator> SysItData;
typedef map<std::string, SysItData> SysMapItData;

class SystemVar {
public:
   static void AddAtFun(string, AT_FUN);
   static void AddCaretFun(string, CARET_FUN);
   static void AddIntVar(string, int, const bool &ReadOnly = false);
   static void AddFloatVar(string, float, const bool &ReadOnly = false);
   static void AddStrVar(string, string, const bool &ReadOnly = false);

   inline static void ClearIntVar() {
      IntVar.clear();
   };
   inline static void ClearFloatVar(){
      FloatVar.clear();
   };
   inline static void ClearStrVar(){
      StrVar.clear();
   };
   inline static void ClearAllVars(){
      ClearIntVar();
      ClearFloatVar();
      ClearStrVar();
   };

   static void deleteData(const std::string &toDelete);

   static DataMatrix getAnalysis(const std::string &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL);
   inline static DataMatrix getAnalysis(const StrArg &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL) {
      return getAnalysis(SeqName.getValue(), FunctionName, ComL);
   }
   static DataMatrix & getAnalysis(const std::string &SeqName, StrList &anaDesc, 
                   const std::string &FunctionName, const CommandLine &ComL);
   static void getAnalysis(const std::string &SeqName, DataMatrix &anaList,
                   StrList &anaDesc, const std::string &FunctionName,
                   const CommandLine &ComL);

   inline static AT_FUN GetAtFun(const std::string &s) {
      return AtFunList.find(s)->second.Data;
   };

   inline static CARET_FUN GetCaretFun(const std::string &s) {
      return CaretFunList.find(s)->second.Data;
   };

   static DataMatrix getData(const std::string &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL);
   inline static DataMatrix getData(const StrArg &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL) {
      return getData(SeqName.getValue(), FunctionName, ComL);
   }

   inline static int GetIntVar(const std::string &s) {
      return IntVar.find(s)->second.Data;
   };

   inline static Iterator GetIterator(const std::string &s) {
      return IteratorList.find(s)->second.Data;
   };

   inline static float GetFloatVar(const std::string &s) {
      return FloatVar.find(s)->second.Data;
   };

   static DataMatrix getMatrix(const std::string &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL);
   inline static DataMatrix getMatrix(const StrArg &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL) {
      return getMatrix(SeqName.getValue(), FunctionName, ComL);
   }
   inline static DataMatrix getMatrixOrAnalysis(const std::string &varName,
                   const std::string &FunctionName, const CommandLine &ComL) {
      const char varType = GetVarType(varName);
      if (varType == 'M')
         return getMatrix(varName, FunctionName, ComL);
      return getAnalysis(varName, FunctionName, ComL);
   }

   static UIPtnSequence getSequence(const std::string &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL);
   inline static UIPtnSequence getSequence(const StrArg &SeqName,
                   const std::string &FunctionName, const CommandLine &ComL) {
      return getSequence(SeqName.getValue(), FunctionName, ComL);
   }

   inline static string GetStrVar(const std::string &s) {
      return StrVar.find(s)->second.Data;
   };

   inline static char GetVarType(const StrArg &s) {
      return GetVarType(s.getValue());
   };
   static char GetVarType(const std::string &s);
   static string GetVarTypeName(const std::string &s);

   static void IncIntVar(std::string varName, int incAmt = 1) {
      IntVar[varName].Data += incAmt;
   }

   static void insertAnalysis(const std::string &insertName,
                   const DataMatrix &insertData, const StrList &insertDesc);

   static void insertData(const std::string &insertName,
                   const DataMatrix &toInsert, const DataListType insertType);
   inline static void insertData(const StrArg &insertName,
                   const DataMatrix &toInsert, const DataListType insertType) {
      insertData(insertName.getValue(), toInsert, insertType);
   }

   static void insertSequence(const std::string &seqName, const UIPtnSequence &toInsert);
   inline static void insertSequence(const StrArg &seqName, const UIPtnSequence &toInsert) {
      insertSequence(seqName.getValue(), toInsert);
   }

   static bool IsReadOnly(const string &s);

   static void OutputAtFuns();
   static void OutputCaretFuns();
   static void OutputFloatVars();
   static void OutputIntVars();
   static void OutputStrVars();

   static void SetIntVar(string, int);
   inline static void SetIterator(const string &VarName, const Iterator &iter) {
      IteratorList[VarName].Data = iter;
   }
   static void SetFloatVar(string, float);
   static void SetStrVar(string, string);

private:
   static SysMapIntData IntVar;
   static SysMapFloatData FloatVar;
   static SysMapStrData StrVar;
   static BindList<UIPtnSequence> SequenceList;   // list of sequences
   static BindList<DataMatrix> MatrixList;   // list of matrixes
   static BindList<DataMatrix> AnalysisList; // list of data anlayses
   static BindList<StrList> AnalysisNames;   // names of analyses
   static BindList<SimState> SimStates; // list of saved simulation states

   /* Function lists */
   static SysMapAtData AtFunList;
   static SysMapCaretData CaretFunList;

   /* Iterator list */
   static SysMapItData IteratorList;
};

#endif

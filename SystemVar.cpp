#if !defined(SYSTEMVAR_HPP)
#  include "SystemVar.hpp"
#endif

#if !defined(OUTPUT_HPP)
#  include "Output.hpp"
#endif

using std::string;

SysMapIntData SystemVar::IntVar;
SysMapFloatData SystemVar::FloatVar;
SysMapStrData SystemVar::StrVar;
BindList<UIPtnSequence> SystemVar::SequenceList;   // list of sequences
BindList<DataMatrix> SystemVar::MatrixList;   // list of matrixes
BindList<DataMatrix> SystemVar::AnalysisList; // list of data anlayses
BindList<StrList> SystemVar::AnalysisNames;   // names of analyses

/* Function lists */
SysMapAtData SystemVar::AtFunList;
SysMapCaretData SystemVar::CaretFunList;

/* Iterator list */
SysMapItData SystemVar::IteratorList;

void SystemVar::AddAtFun(string name, AT_FUN fun)
{
   AtFunList[name].Data = fun;
   return;
}

void SystemVar::AddCaretFun(string name, CARET_FUN fun)
{
   CaretFunList[name].Data = fun;
   return;
}

void SystemVar::AddIntVar(string name, int var, const bool &ReadOnly)
{
   IntVar[name].IsReadOnly = ReadOnly;
   IntVar[name].Data = var;
   return;
}

void SystemVar::AddFloatVar(string name, float var, const bool &ReadOnly)
{
   FloatVar[name].IsReadOnly = ReadOnly;
   FloatVar[name].Data = var;
   return;
}

void SystemVar::AddStrVar(string name, string var, const bool &ReadOnly)
{
   StrVar[name].IsReadOnly = ReadOnly;
   StrVar[name].Data = var;
   return;
}

void SystemVar::deleteData(const string &toDelete)
{
   const string dataType = GetVarTypeName(toDelete);
   if (dataType == "sequence") {
      SequenceList.remove(toDelete);
   } else if (dataType == "integer") {
      IntVar.erase(toDelete);
   } else if (dataType == "float") {
      FloatVar.erase(toDelete);
   } else if (dataType == "string") {
      StrVar.erase(toDelete);
   } else if (dataType == "matrix") {
      MatrixList.remove(toDelete);
   } else if (dataType == "analysis") {
      AnalysisList.remove(toDelete);
      AnalysisNames.remove(toDelete);
   } else {
      CALL_ERROR << "Warning: Unable to find " << toDelete << " for deletion." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
}

DataMatrix SystemVar::getAnalysis(const string &SeqName, const string &FunctionName,
                              const CommandLine &ComL)
{
   if (!AnalysisList.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Could not find Analysis "
                 << SeqName << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   return AnalysisList.GetEntry(SeqName);
}

DataMatrix & SystemVar::getAnalysis(const string &SeqName, StrList &anaDesc, 
                          const string &FunctionName, const CommandLine &ComL)
{
   if (!AnalysisList.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": " << SeqName
                 << " was not created with CreateAnalysis" << endl << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (!AnalysisNames.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Could not find Analysis Description "
                 << SeqName << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }   
   anaDesc = AnalysisNames.GetEntry(SeqName);
   return AnalysisList.GetEntry(SeqName);
}

void SystemVar::getAnalysis(const string &SeqName, DataMatrix &anaList,
                          StrList &anaDesc, const string &FunctionName,
                          const CommandLine &ComL)
{
   if (!AnalysisList.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": " << SeqName
                 << " was not created with CreateAnalysis" << endl << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   if (!AnalysisNames.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Could not find Analysis Description "
                 << SeqName << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }   
   anaList = AnalysisList.GetEntry(SeqName);
   anaDesc = AnalysisNames.GetEntry(SeqName);
}

DataMatrix SystemVar::getData(const string &SeqName, const string &FunctionName,
                            const CommandLine &ComL)
{
   const char dataType = GetVarType(SeqName);
   if (dataType == 'S')
      return UISeqToMatrix(SequenceList.GetEntry(SeqName));
   else if (dataType == 'M')
      return MatrixList.GetEntry(SeqName);
   else if (dataType == 'A')
      return AnalysisList.GetEntry(SeqName);
   CALL_ERROR << "Error in " << FunctionName << ": Could not find data "
              << SeqName << ERR_WHERE;
   ComL.DisplayHelp(Output::Err());
   exit(EXIT_FAILURE);   
}

DataMatrix SystemVar::getMatrix(const string &SeqName, const string &FunctionName,
                              const CommandLine &ComL)
{
   if (!MatrixList.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Could not find Matrix "
                 << SeqName << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   return MatrixList.GetEntry(SeqName);
}

UIPtnSequence SystemVar::getSequence(const string &SeqName, const string &FunctionName,
                                     const CommandLine &ComL)
{
   if (!SequenceList.exists(SeqName)) {
      CALL_ERROR << "Error in " << FunctionName << ": Could not find Sequence "
                 << SeqName << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   //Willow code
   //UIPtnSequence test = SequenceList.GetEntry(SeqName);
   //return test;
   return SequenceList.GetEntry(SeqName);
}

char SystemVar::GetVarType(const string &name)
{
   if (IntVar.find(name) != IntVar.end())
      return 'i';
   else if (FloatVar.find(name) != FloatVar.end())
      return 'f';
   else if (StrVar.find(name) != StrVar.end())
      return 's';
   else if (AtFunList.find(name) != AtFunList.end())
      return '@';
   else if (CaretFunList.find(name) != CaretFunList.end())
      return '^';
   else if (IteratorList.find(name) != IteratorList.end())
      return 'I';
   else if (SequenceList.exists(name))
      return 'S';
   else if (MatrixList.exists(name))
      return 'M';
   else if (AnalysisList.exists(name))
      return 'A';
   else
      return 'u';
}

string SystemVar::GetVarTypeName(const string &name)
{
   if (IntVar.find(name) != IntVar.end())
      return "integer";
   else if (FloatVar.find(name) != FloatVar.end())
      return "float";
   else if (StrVar.find(name) != StrVar.end())
      return "string";
   else if (AtFunList.find(name) != AtFunList.end())
      return "@ function";
   else if (CaretFunList.find(name) != CaretFunList.end())
      return "^ function";
   else if (IteratorList.find(name) != IteratorList.end())
      return "iterator";
   else if (SequenceList.exists(name))
      return "sequence";
   else if (MatrixList.exists(name))
      return "matrix";
   else if (AnalysisList.exists(name))
      return "analysis";
   else
      return "unknown";
}

void SystemVar::insertAnalysis(const string &insertName, const DataMatrix &insertData,
                             const StrList &insertDesc)
{
   AnalysisList.insert(insertName, insertData);
   AnalysisNames.insert(insertName, insertDesc);
}

void SystemVar::insertData(const string &insertName, const DataMatrix &toInsert,
                         const DataListType insertType)
{
   if (insertType == DLT_sequence)
      SequenceList.insert(insertName, MatrixToUISeq(toInsert));
   else if (insertType == DLT_matrix)
      MatrixList.insert(insertName, toInsert);
   else
      AnalysisList.insert(insertName, toInsert);
}

void SystemVar::insertSequence(const string &seqName, const UIPtnSequence &toInsert)
{
   SequenceList.insert(seqName, toInsert);
}

bool SystemVar::IsReadOnly(const string &varName)
{
   if (IntVar.find(varName) != IntVar.end()) {
      return IntVar.find(varName)->second.IsReadOnly;
   } else if (FloatVar.find(varName) != FloatVar.end()) {      
      return FloatVar.find(varName)->second.IsReadOnly;
   } else if (StrVar.find(varName) != StrVar.end()) {      
      return StrVar.find(varName)->second.IsReadOnly;
   } else {
      return false;
   }
}

void SystemVar::OutputAtFuns()
{
   SysMapAtData::const_iterator AtIt;
   for (AtIt = AtFunList.begin(); AtIt != AtFunList.end(); AtIt++) {
      Output::Err() << " " << AtIt->first << std::endl;
   }
}

void SystemVar::OutputCaretFuns()
{
   SysMapCaretData::const_iterator CaretIt;
   for (CaretIt = CaretFunList.begin(); CaretIt != CaretFunList.end(); CaretIt++) {
      Output::Err() << " " << CaretIt->first << std::endl;
   }
}

void SystemVar::OutputFloatVars()
{
   SysMapFloatData::const_iterator it;
   for (it = FloatVar.begin(); it != FloatVar.end(); it++) {
      Output::Err() << " " << it->first << std::endl;
   }
}

void SystemVar::OutputIntVars()
{
   SysMapIntData::const_iterator it;
   for (it = IntVar.begin(); it != IntVar.end(); it++) {
      Output::Err() << " " << it->first << std::endl;
   }
}

void SystemVar::OutputStrVars()
{
   SysMapStrData::const_iterator it;
   for (it = StrVar.begin(); it != StrVar.end(); it++) {
      Output::Err() << " " << it->first << std::endl;
   }
}

void SystemVar::SetIntVar(string varName, int varValue)
{
   IntVar[varName].Data = varValue;
}

void SystemVar::SetFloatVar(string varName, float varValue)
{
   FloatVar[varName].Data = varValue;
}

void SystemVar::SetStrVar(string varName, string varValue)
{
   StrVar[varName].Data = varValue;
}

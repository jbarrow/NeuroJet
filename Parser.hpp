#if !defined(PARSER_HPP)
#  define PARSER_HPP

#  include <string>
#  if !defined(CALC_HPP)
#    include "Calc.hpp"
#  endif
#  if !defined(SYSTEMVAR_HPP)
#    include "SystemVar.hpp"
#  endif

class Parser {
public:
   static void ParseScript(const std::string &filename);
   static void ParseLine(const std::string lineIn, StackNode* &CurrentNode,
                         unsigned long CurrentLine);
   static StackNode *GetArgs(const StackNode * const, ArgListType &);
private:
   static StackNode *CallAtFunction(StackNode * const);
   static StackNode *CallCaretFunction(StackNode * const, string&);
   static StackNode *DoConditional(StackNode * const);
   static StackNode *DoLoop(StackNode * const);
   static void FillStack(const std::string& filename);
   static StackNode *GetLoopVal(const StackNode * const, string&);
   static StackNode *ParseStack(StackNode * const);
   static void PrePostErrCheck(const bool ErrCondition, const std::string &pre,
                               const std::string &post);
   static StackNode *SkipStatement(StackNode * const);
};

const std::string fnName = "^Fn(";
inline bool isFn(const std::string toChk) {
   return (toChk.compare(0,fnName.size(),fnName) == 0);
}

// For using Calc from elsewhere
double CalcFromStr(const std::string &toCalc);

DataList CalcFn(const std::string &calcStmt);

#endif


#if !defined(SYMBOLS_HPP)
  #define SYMBOLS_HPP

#  if !defined(ARGFUNCTS_HPP)
#    include "ArgFuncts.hpp"
#  endif

const int NumGrammarSymbols = 13;
const char StartNodeSymbol = '{';
const char EndNodeSymbol = '}';
const char CommentSymbol = '#';
const char AtSymbol = '@';
const char CaretSymbol = '^';
const char LoopSymbol = '%';
const char LoopIndexSymbol = '$';
const char MacroDefSymbol = ']';
const char StartParameterSymbol = '(';
const char EndParameterSymbol = ')';
const char QuoteSymbol = '\"';
const char JoinSymbol = ':';
const char ConditionalSymbol = '?';

const string GrammarSymbolList = " {}#@^%$]()\":?";

inline int GrammarSymbol(const char &c) {
   return (GrammarSymbolList.find(c) == string::npos) ? 0 : GrammarSymbolList.find(c);
}

const string SymbolHelp[NumGrammarSymbols + 1] = {
   "Invalid Symbol.",
   "StartNodeSymbol... Used for enclosing commands after a loop.",
   "EndNodeSymbol... Used for enclosing commands after a loop.",
   "CommentSymbol... Used for beginning a comment line that\n\t   is ignored by the program.",
   "AtSymbol... Used for indicating that the next function\n\t   is a void function. ",
   "CaretSymbol... Used for indicating that the next function\n\t   returns a string. Alone it stands for ^GetVar.",
   "LoopSymbol... Used for beginning a loop function.\n\t   The only loop function is( .... ).",
   "LoopIndexSymbol... Used for accessing the value of a loop\n\t   iterator.",
   "MacroDefSymbol... Under Construction... ",
   "StartParameterSymbol... Used for enclosing the parameters\n\t   of the previous function.",
   "EndParameterSymbol... Used for enclosing the parametes of\n\t   the previous function.",
   "QuoteSymbol... Used around strings to keep the program\n\t   from parsing them.",
   "JoinSymbol... Used for concatenating strings into one.",
   "ConditionalSymbol... Used before a conditional function:\n\t   If or While. \n\n"
   "Conditional If -help\n\n"
   "?If(...) { ... } evaluates a numerical expression and executes\n"
   "the subsequent braced statement if true.\n"
   "The syntax is ?If( expression ) { .... }\n"
   "Expression can be number or a ^Calc expression.\n\n\n"
   "Conditional While -help\n\n"
   "?While(...) { ... } evaluates a numerical expression and executes\n"
   "the subsequent braced statement while it remains true.\n"
   "The syntax is ?While( expression ) { .... }\n"
   "Expression can be number or a ^Calc expression.\n"
};

#endif

#if !defined(CALC_HPP)
#  include "Calc.hpp"
#endif

/**************************************************************/
/* CaretFunction Definitions */
/**************************************************************/

string CalcCaret(ArgListType &arg) //CARET_FUN
{
   if (arg.at(0).first == "-help") {
      Output::Err() << "Calc -help\n\n"
         "^Calc(...) is a calculator. Although the syntax is rigid,\n"
         "Calc is quite powerful.\n"
         "^Calc evaluates all [..],(..),{..} first and then evaluates from left to right.\n"
         "The following standard operators are recognized:\n"
         "+,-,*,/,^,%,!,~,=,~=,<>,&,|,~|,>,<,>=,<=\n"
         "Note that '{','}','(',')','%','^' must be in a quoted string to use.\n"
         "Note that the left to right precedence makes brackets necessary.\n"
         "For example: 3 + 4! = [3+4]! = 7!, not 3+[4!].\n\n"
         "The following functions are recognized:\n"
         "abs[x]\t\t=\tabs(x)\t\t=\tabsolute value of x\n"
         "acos[x]\t\t=\tarccos(x)\t=\tarc cosine of x\n"
         "asin[x]\t\t=\tarcsin(x)\t=\tarc sine of x\n"
         "atan[x]\t\t=\tarctan(x)\t=\tarc tangent of x\n"
         "ceil[x]\t\t=\tceil(x)\t\t=\tleast integer >= x\n"
         "cos[x]\t\t=\tcos(x)\t\t=\tcosine of x\n"
         "cosh[x]\t\t=\tcosh(x)\t\t=\thyperbolic cosine of x\n"
         "exists[x]\t=\texists(x)\t=\twhether or not structure/file x exists\n"
         "exp[x]\t\t=\te^x\t\t=\te raised to the x power\n"
         "fact[x]\t\t=\tx!\t\t=\tx factorial (actually gamma(x+1))\n"
         "gamma[x]\t=\tgamma(x)\t=\tthe gamma function evaluated at x\n"
         "gampdf[x,a,b]\t=\tgampdf(x,a,b)\t=\tthe gampdf fn from MATLAB\n"
         "kurt[sequence]\t=\t\t\t\tkurtosis of sequence\n"
         "log[x]\t\t=\tln(x)\t\t=\tnatural log of x\n"
         "log10[x]\t=\tlog_10(x)\t=\tlog base 10 of x\n"
         "max[x1,...,xN]\t=\tmax{x1,...,xN}\t=\tmaximum of the values x1,...,xN\n"
         "mean[sequence]\t=\tmean(sequence)\t=\tmean value of sequence\n"
         "min[x1,...,xN]\t=\tmin{x1,...,xN}\t=\tminimum of the values x1,...,xN\n"
         "mod[x,y]\t=\tx mod y\t\t=\tremainder of x/y\n"
         "normal[mu,sig]\t=\tnormal(mu,sig)\t=\trandom normal float\n"
         "pow[x,y]\t=\tx^y\t\t=\tx raised to the y power\n"
         "randint[x,y]\t=\trandint(m,n)\t=\trandom integer in [m,n]\n"
         "round[x]\t=\trint(x)\t\t=\tx rounded to nearest integer\n"
         "sin[x]\t\t=\tsin(x)\t\t=\tsine of x\n"
         "sinh[x]\t\t=\tsinh(x)\t\t=\thyperbolic sine of x\n"
         "skew[sequence]\t=\t\t\t\tskewness of sequence\n"
         "sqrt[x]\t\t=\tsqrt(x)\t\t=\tsquare root of x\n"
         "strcmp[x,y]\t=\tstrcmp(x,y)\t=\tperforms C's strcmp on x amd y\n"
         "tan[x]\t\t=\ttan(x)\t\t=\ttangent of x\n"
         "tanh[x]\t\t=\ttanh(x)\t\t=\thyperbolic tangent of x\n"
         "uniform[x,y]\t=\tuniform(x,y)\t=\trandom float in [x,y]\n"
         "var[sequence]\t=\tvar(sequence)\t=\tvariance of sequence\n" << std::endl;
      exit(EXIT_FAILURE);
   }

   CalcNode *CurrentNode = new CalcNode("[");
   CalcNode *TopNode = CurrentNode;
   BuildCalcNodeList(CurrentNode, arg);
   CalcNode *EndNode = CurrentNode;

   // Parse for unary +,- operators and for errors
   CurrentNode = TopNode;
   CalcNode *NextNode;
   CalcNode *PrevNode;

   while ((NextNode = CurrentNode->next) != NULL) {
      PrevNode = CurrentNode->prev;
      // ~
      if (CurrentNode->str == "~") {
         errChkNot(NextNode->str);
         // '~~' = ''
         if (NextNode->str == "~") {
            // CurrentNode becomes NextNode, NextNode becomes NextNode->next
            removeCurNode(CurrentNode, NextNode);
            removeCurNode(CurrentNode, NextNode);
            CurrentNode = PrevNode;
            continue;
         }
      }
      // +
      if (CurrentNode->str == "+") {
         errChkPlus(NextNode->str);
         // '++' = '+', '+-' = '-'
         if ((NextNode->str == "-") || (NextNode->str == "+")) {
            removeCurNode(CurrentNode, NextNode);
            continue;
         }
         // '*+', '/+', '[+',',+','%+','^+' = '*','/','[',',','%','^'
         if (isUnaryCompatible(PrevNode->str)) {
            removeCurNode(CurrentNode, NextNode);
            CurrentNode = CurrentNode->prev;
            continue;
         }
      }
      // -
      if (CurrentNode->str == "-") {
         errChkMinus(NextNode->str);
         // '-+' = '-'
         if (NextNode->str == "+") {
            removeNextNode(CurrentNode, NextNode);
            continue;
         }
         // '--' = '+'
         if (NextNode->str == "-") {
            removeNextNode(CurrentNode, NextNode);
            CurrentNode->str = "+";
            continue;
         }
         // '*-', '[-',',-' = '*-1*', '[-1*',',-1*'
         if (isUnaryCompatible(PrevNode->str)) {
            // check for numbers in the next spot and negate them
            string nextstr = NextNode->str;
            if (nextstr[0] == '.') nextstr == "0" + nextstr;
            if (isdigit(nextstr[0])) {
               removeCurNode(CurrentNode, NextNode);
               CurrentNode = PrevNode;
               NextNode = CurrentNode->next;
               NextNode->str = "-" + nextstr;
               continue;
            } else {
               errChkMinusUnary(PrevNode->str, NextNode->str);
            }            
            CurrentNode->str = "-1";

            if (PrevNode->str == "/") {
               CurrentNode->next = new CalcNode(CurrentNode, NextNode, "/");
            } else {
               CurrentNode->next = new CalcNode(CurrentNode, NextNode, "*");
            }
            NextNode->prev = CurrentNode->next;
            continue;
         }
      }      
      // [, *, /, %, ^, et al
      errChkGeneric(CurrentNode->str, NextNode->str);
      // ]
      if (CurrentNode->str == "]") {
         if (PrevNode->str == ",") {
            CalcErr(",] undefined");
         }
         if (NextNode->str == "[") {
            CurrentNode->next = new CalcNode(CurrentNode, NextNode, "*");
            NextNode->prev = CurrentNode->next;
            continue;
         }
      }
      CurrentNode = CurrentNode->next;
   }

   // Perform all the Calculations
   double FinalVal = Calc::Calculate(TopNode, EndNode);

   delete TopNode; // Cascading delete

   return to_string(FinalVal);
}

string GetVar(ArgListType &arg) //CARET_FUN0
{
   if (arg.size() != 1) {
      CALL_ERROR << "Error in GetVar: expects exactly one argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "GetVar -help\n\n"
         "^GetVar( ... ) takes a single variable name and returns its value.\n"
         "Unlike most functions, GetVar does not require any preceeding flags.\n"
         << "^( ... ) is a shortcut for ^GetVar( ... ).\n" << std::endl;
      exit(EXIT_FAILURE);
   }
   string varName = arg.at(0).first;
   char varType = SystemVar::GetVarType(varName);

   if (varType == 'i') {
      return to_string(SystemVar::GetIntVar(varName));
   } else if (varType == 'f') {
      return to_string(SystemVar::GetFloatVar(varName));
   } else if (varType == 's') {
      return SystemVar::GetStrVar(varName);
   } else if (varType == 'I') {
      return to_string(SystemVar::GetIterator(varName).CurrentVal);
   } else {
      CALL_ERROR << "GetVar did not find " << varName << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   // never get here
   return "";
}

/**************************************************************/
/* CondFunction Definitions */
/**************************************************************/

string If(ArgListType &arg) //CARET_FUN
{
   unsigned int num = arg.size();
   if (num < 1) {
      CALL_ERROR << "Error in If: At least one argument expected." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "If -help\n\n"         
         "^If(...) evaluates a numerical expression and returns a true string\n"
         "or a false string.\n"
         "The syntax is ^If( expression 'then' string1 'else' string2 )\n"
         "Expression can be number or a ^Calc expression.\n"
         "'then' and 'else' default to 1 and 0 respectively." << std::endl;
      exit(EXIT_FAILURE);
   }
   string TempResult = "";
   unsigned int i;
   for (i = 0; i < num; i++) {
      if (arg[i].first == "then") {
         if (num != (i + 2) && num != (i + 4)) {
            CALL_ERROR << "Error in If: 1 or 3 arguments expected after 'then'."
                       << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
         ArgListType beginArg = arg;
         beginArg.resize(i);
         TempResult = CalcCaret(beginArg);
         break;
      }
   }
   bool isTrue = (fabs(from_string<float>(TempResult)) > 0.0f);
   if (i == num) {
      // No "then" was found
      TempResult = CalcCaret(arg);
      return isTrue ? "1" : "0";
   }
   if (i == (num - 2)) {
      return isTrue ? arg.at(i + 1).first : "0";
   }
   if (arg.at(i + 2).first != "else") {
      CALL_ERROR << "Error in If: 'else' expected here -> "
                 << arg.at(i + 2).first << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   return isTrue ? arg.at(i + 1).first : arg.at(i + 3).first;
}

void BuildCalcNodeList(CalcNode * &CurrentNode, const ArgListType &arg)
{
   int  NumBrace = 0;

   for (ArgListTypeCIt it = arg.begin(); it != arg.end(); it++) {
      string curArg = it->first;
      unsigned int j = 0;
      while (j < curArg.size()) {
         char ch = curArg[j];
         char ch2 = curArg[j+1];
         // Ignore that darned space
         if (isspace(ch)) {
            j++;
            continue;
         }
         // Get new node and advance
         CurrentNode->next = new CalcNode(CurrentNode);
         CurrentNode = CurrentNode->next;

         // Search the cases
         if ((ch == '[') || (ch == '{') || (ch == '(')) {
            CurrentNode->str = "[";
            ++NumBrace;
         } else if ((ch == ']') || (ch == '}') || (ch == ')')) {
            CurrentNode->str = "]";
            --NumBrace;
         } else if ((ch == '<' && ch2 == '=') || (ch == '=' && ch2 == '<')) {
            CurrentNode->str = "<=";
            ++j;
         } else if ((ch == '>' && ch2 == '=') || (ch == '=' && ch2 == '>')) {
            CurrentNode->str = ">=";
            j++;
         } else if ((ch == '~' && ch2 == '=') || (ch == '<' && ch2 == '>')) {
            CurrentNode->str = "~=";
            j++;
         } else if (ch == '~' && ch2 == '|') {
            CurrentNode->str = "~|";
            j++;
            } else if (MathChars.find(ch) != string::npos) {
            CurrentNode->str = ch;
         } else {
            string TempStr = "";
            while ((j < curArg.size()) &&
                    ((((ch=='-') || (ch=='+')) && (j > 1) && (curArg[j-1] == 'e'))
                      || (TokenChars.find(ch) == string::npos))) {
               TempStr += ch;
               j++;

               ch = curArg[j];
            }
            if (TempStr != "") {
               CurrentNode->str = TempStr;
            }
            j--; // so that token character can be handled next time
         }
         j++;
      }
   }

   // Check the number of braces
   if (NumBrace != 0) {
      CalcErr("unmatching braces (number of braces = " + to_string(NumBrace) + ")");
   }
   // Get new node and advance
   CurrentNode->next = new CalcNode(CurrentNode);
   CurrentNode = CurrentNode->next;
   CurrentNode->str = "]";
}

// no return
void CalcErr(const string msg)
{
   CALL_ERROR << "Error in Calc: " << msg << ERR_WHERE;
   exit(EXIT_FAILURE);
}

double Calc::Calculate(const CalcNode * StartNode, CalcNode * &EndNode)
{
   static string FunctionName = "Calculate";
   static CommandLine ComL(FunctionName);

   if ((StartNode->str == "[") || (StartNode->str == ",")) {
      double CurrentArg = Calculate(StartNode->next, EndNode);
      while ((StartNode = EndNode) != NULL) {

         if ((StartNode->str == "]") || (StartNode->str == ",")) {
            EndNode = EndNode->next;
            break;
         }

         if (StartNode->next == NULL) {
            CalcErr("unexpected end of equation");
         }

         if (StartNode->str == "+") {
            CurrentArg += Calculate(StartNode->next, EndNode);
            continue;
         }

         if (StartNode->str == "-") {
            CurrentArg -= Calculate(StartNode->next, EndNode);
            continue;
         }

         if (StartNode->str == "*") {
            CurrentArg *= Calculate(StartNode->next, EndNode);
            continue;
         }

         if (StartNode->str == "/") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            if (fabs(tempdbl) < verySmallFloat) {
               CalcErr("division by zero");
            }
            CurrentArg /= tempdbl;
            continue;
         }

         if (StartNode->str == "^") {
            double ArgTwo = Calculate(StartNode->next, EndNode);
            if (fabs(CurrentArg) < verySmallFloat) {
               if (fabs(ArgTwo) < verySmallFloat) {
                  CurrentArg = 1.0L;
                  continue;
               }
               if (ArgTwo < 0.0L) {
                  CalcErr("0 cannot be raised to a negative power");
               }
            }
            if ((CurrentArg < 0.0L) && (fabs(floor(ArgTwo) - ArgTwo) > verySmallFloat)) {
               CalcErr("negative numbers cannot be raised to noninteger powers");
            }
            CurrentArg = pow(CurrentArg, ArgTwo);
            continue;
         }

         if (StartNode->str == "%") {
            CurrentArg = fmod(CurrentArg, Calculate(StartNode->next, EndNode));
            continue;
         }

         if (StartNode->str == "!") {
            CurrentArg = myGamma(CurrentArg + 1.0);
            EndNode = EndNode->next;
            continue;
         }

         if (StartNode->str == "&") {
            const double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (static_cast<bool>(fabs(CurrentArg) > verySmallFloat) && static_cast<bool>(fabs(tempdbl) > verySmallFloat)) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "|") {
            const double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (static_cast<bool>(fabs(CurrentArg) > verySmallFloat) || static_cast<bool>(fabs(tempdbl) > verySmallFloat)) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "~|") {
            const double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (static_cast<bool>(fabs(CurrentArg) > verySmallFloat) != static_cast<bool>(fabs(tempdbl) > verySmallFloat)) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "=") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (fabs(CurrentArg-tempdbl) < verySmallFloat) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "~=") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (fabs(CurrentArg-tempdbl) > verySmallFloat) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == ">") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (CurrentArg > tempdbl) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "<") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (CurrentArg < tempdbl) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == "<=") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (CurrentArg <= tempdbl) ? 1.0L : 0.0L;
            continue;
         }

         if (StartNode->str == ">=") {
            double tempdbl = Calculate(StartNode->next, EndNode);
            CurrentArg = (CurrentArg >= tempdbl) ? 1.0L : 0.0L;
            continue;
         }

         CalcErr(StartNode->str + " unexpected");
      }

      return CurrentArg;
   }
   // Check and perform for ~
   if (StartNode->str == "~") {
      return fabs(Calculate(StartNode->next, EndNode)) > verySmallFloat ? 0.0L : 1.0L;
   }
   // Check and Peform if Function
   if (isalpha(StartNode->str[0])) {
      if (StartNode->next->str != "[") {
         CalcErr("bracket expected after " + StartNode->str);
      }
      // Do Single Arg Functions

      if (StartNode->str == "abs") {
         return fabs(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "acos") {
         return acos(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "asin") {
         return asin(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "atan") {
         return atan(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "ceil") {
         return ceil(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "cos") {
         return cos(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "strcmp") {
         CalcNode * curNode = StartNode->next->next;
         string ArgOne = curNode->str;
         curNode = curNode->next->next;
         string ArgTwo = curNode->str;
         EndNode = curNode->next->next;
         return strcmp(ArgOne.c_str(), ArgTwo.c_str());
      }

      if (StartNode->str == "cosh") {
         return cosh(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "exp") {
         return exp(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "exists") {
         return (fileExists(StartNode->next->next->str) ||
            (SystemVar::GetVarType(StartNode->next->next->str) != 'u')) ? 1.0L : 0.0L;
      }

      if (StartNode->str == "fact") {
         return myGamma(Calculate(StartNode->next, EndNode) + 1.0f);
      }

      if (StartNode->str == "gamma") {
         return myGamma(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "floor") {
         return floor(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "log") {
         return log(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "log10") {
         return log10(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "round") {
         return floor(Calculate(StartNode->next, EndNode) + 0.5);
      }

      if (StartNode->str == "sin") {
         return sin(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "sinh") {
         return sinh(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "sqrt") {
         return pow(Calculate(StartNode->next, EndNode), 0.5);
      }

      if (StartNode->str == "tan") {
         return tan(Calculate(StartNode->next, EndNode));
      }

      if (StartNode->str == "tanh") {
         return tanh(Calculate(StartNode->next, EndNode));
      }

      // Two Arg Functions
      if (StartNode->str == "mod") {
         double ArgOne, ArgTwo;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo);
         return fmod(ArgOne, ArgTwo);
      }

      if (StartNode->str == "normal") {
         double ArgOne, ArgTwo;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo);
         if (fabs(ArgTwo) < verySmallFloat) return ArgOne;

         if (ArgTwo < 0.0L) {
            CalcErr("for normal[x,y], y must be >= 0");
         }

         return getUserNormal(ArgOne, ArgTwo);
      }

      if (StartNode->str == "pow") {
         double ArgOne, ArgTwo;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo);
         if (fabs(ArgOne) < verySmallFloat) {
            if (fabs(ArgTwo) < verySmallFloat) return 1.0L;

            if (ArgTwo < 0.0L) {
               CalcErr("0 cannot be raised to a negative power");
            }
         }
         if ((ArgOne < 0.0L) && (fabs(floor(ArgTwo) - ArgTwo) > verySmallFloat)) {
            CalcErr("negative numbers cannot be raised to noninteger powers");
         }
         return pow(ArgOne, ArgTwo);
      }

      if (StartNode->str == "randint") {
         double ArgOne, ArgTwo;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo);
         if (fabs(ArgOne - ArgTwo) < verySmallFloat) return ArgOne;
         if (ArgOne > ArgTwo) std::swap(ArgOne, ArgTwo);

         return static_cast<double>(getUserRandInt(iround(ArgOne), iround(ArgTwo)));
      }

      if (StartNode->str == "uniform") {
         double ArgOne, ArgTwo;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo);
         if (fabs(ArgOne - ArgTwo) < verySmallFloat) return ArgOne;
         if (ArgOne > ArgTwo) std::swap(ArgOne, ArgTwo);
         return getUserUniform(ArgOne, ArgTwo);
      }

      // Three Arg Function(s)
      if (StartNode->str == "gampdf") {
         double ArgOne, ArgTwo, ArgThree;
         getCalcArgs(StartNode, EndNode, ArgOne, ArgTwo, ArgThree);
         return gampdf(ArgOne, ArgTwo, ArgThree);
      }

      // Multi Arg Functions
      if ((StartNode->str == "max") || (StartNode->str == "min")) {
         string calcType = StartNode->str; // Save it
         StartNode = StartNode->next;
         DataList Args;
         while (true) {
            Args.push_back(static_cast<float>(Calculate(StartNode, EndNode)));
            if (EndNode->prev->str == "]") {
               return (calcType == "max") ? max(Args) : min(Args);
            }
            if (EndNode->prev->str != ",") {
               CalcErr(StartNode->str + " takes multiple arguments: "
                                    ", expected here -> " + EndNode->prev->str);
            }
            StartNode = EndNode->prev;
         }
      }

      // Vector Functions
      if (StartNode->str == "mean") {
         string varName = StartNode->next->next->str;
         char varType = SystemVar::GetVarType(varName);
         if ((varType == 'M') || (varType == 'A')) {
            DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(varName, FunctionName, ComL);
            return matrixMean(Matrix);
         } else {
            CalcErr(StartNode->str + " currently does not work on " + varName);
         }
      }

      if (StartNode->str == "var") {
         string varName = StartNode->next->next->str;
         char varType = SystemVar::GetVarType(varName);
         if ((varType == 'M') || (varType == 'A')) {
            DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(varName, FunctionName, ComL);
            return matrixVar(Matrix);
         } else {
            CalcErr(StartNode->str + " currently does not work on " + varName);
         }
      }

      if (StartNode->str == "skew") {
         string varName = StartNode->next->next->str;
         char varType = SystemVar::GetVarType(varName);
         if ((varType == 'M') || (varType == 'A')) {
            DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(varName, FunctionName, ComL);
            return matrixSkew(Matrix);
         } else {
            CalcErr(StartNode->str + " currently does not work on " + varName);
         }
      }

      if (StartNode->str == "kurt") {
         string varName = StartNode->next->next->str;
         char varType = SystemVar::GetVarType(varName);
         if ((varType == 'M') || (varType == 'A')) {
            DataMatrix Matrix = SystemVar::getMatrixOrAnalysis(varName, FunctionName, ComL);
            return matrixKurt(Matrix);
         } else {
            CalcErr(StartNode->str + " currently does not work on " + varName);
         }
      }

      CalcErr("unknown function " + StartNode->str + " - possibly missing ^()");
   }
   // It should be a number
   EndNode = StartNode->next;
   return from_string<double>(StartNode->str);
}

inline void errChkGeneric(const string curStr, const string nextStr) {
   if (nextStr == "^") return;
   bool NeedsExamining = (curStr == "[" || curStr == "~");
   if (curStr != "," && curStr != "!") {
      for (int i=0; MathStrings[i] != "\\"; i++) {
         if (NeedsExamining) break;
         if (curStr == MathStrings[i])
            NeedsExamining = true;
      }
   }
   if (NeedsExamining) {
      // '*]', '**', '*/', '/]', '/*', '//','*,','/,' are errors
      bool undefCombo = (nextStr == "]");
      for (int j=0; MathStrings[j] != "\\"; j++) {
         if (undefCombo) break;
         if (nextStr == MathStrings[j])
            undefCombo = true;
      }
      if (undefCombo) CalcErr(curStr + nextStr + " undefined");
   }
}

void errChkMinus(const string nextStr) {
   // '-]', '-*', '-/','-,','-!','-%','-^' are errors
   bool undefCombo = (nextStr == "]");
   for (int i=0; MathStrings[i] != "\\"; i++) {
      if (nextStr == MathStrings[i]) {
         undefCombo = true;
      }
   }
   if (undefCombo) CalcErr("-" + nextStr + " undefined");
}

void errChkMinusUnary(const string prevStr, const string nextStr) {
   bool inSubsetPrior = (prevStr == "~");
   // ignore the first three members of MathStrings ("*", "/", ",")
   // we allow "*-1", "/-1", ",-1"
   for (int j=3; MathStrings[j] != "\\"; j++)
      if (prevStr == MathStrings[j])
         inSubsetPrior = true;
   if (inSubsetPrior) {
      CalcErr("brackets are needed for this use of the unary minus: "
              + prevStr + "-" + nextStr);
   }
}

void errChkNot(const string nextStr) {
   bool undefCombo = (nextStr == "]");
   for (int i=0; MathStrings[i] != "\\"; i++) {
      if (nextStr == MathStrings[i]) {
         undefCombo = true;
         break;
      }
   }
   if (undefCombo) CalcErr("~" + nextStr + " undefined");
}

void errChkPlus(const string nextStr) {
   bool undefCombo = (nextStr == "]");
   for (int i=0; MathStrings[i] != "\\"; i++) {
      if (nextStr == MathStrings[i]) {
         undefCombo = true;
         break;
      }
   }
   if (undefCombo) CalcErr("+" + nextStr + " undefined");
}

// gamma function
// myGamma uses the algorithm found in the Matlab file gamma.m
// returns the value of the gamma function evaluated at Ex
double myGamma(double Ex)
{
   const double Ep[] = {-1.71618513886549492533811e+0,
      2.47656508055759199108314e+1, -3.79804256470945635097577e+2,
      6.29331155312818442661052e+2, 8.66966202790413211295064e+2,
      -3.14512729688483675254357e+4, -3.61444134186911729807069e+4,
      6.64561438202405440627855e+4};
   const double Eq[] = {-3.08402300119738975254353e+1,
      3.15350626979604161529144e+2, -1.01515636749021914166146e+3,
      -3.10777167157231109440444e+3, 2.25381184209801510330112e+4,
      4.75584627752788110767815e+3, -1.34659959864969306392456e+5,
      -1.15132259675553483497211e+5};
   const double Ec[] = {-1.910444077728e-03, 8.4171387781295e-04,
      -5.952379913043012e-04, 7.93650793500350248e-04,
      -2.777777777777681622553e-03, 8.333333333333333331554247e-02,
      5.7083835261e-03};
   // intialize variables
   double Eres = 0.0; double Exn = 0.0; double Efact = 0.0;
   // catch negative Ex
   int Enegcheck = 0;
   if (Ex <= 0) {
      Enegcheck = 1;
      const double Ey = -1.0 * Ex; const double Ey1 = floor(Ey);
      Eres = Ey - Ey1;
      Efact = -1.0 * (PI / sin(PI * Eres)) * (1.0 - 2.0 * fmod (Ey1, 2.0));
      Ex = Ey + 1.0;
   }
   // Ex is now positive.
   // Map Ex in interval [0,1] to [1,2]
   int E01check = 0; double Ex1 = 0.0;
   if (Ex < 1) {
      E01check = 1; Ex1 = Ex; Ex += 1.0;
   }
   // Map Ex in interval [1,12] to [1,2]
   int  E112check = 0;
   if (Ex < 12) {
      E112check = 1; Exn = floor(Ex) - 1.0; Ex -= Exn;
   }
   // Evaluate approximation for 1 < Ex < 2
   if (E112check) {
      const double Ez = Ex - 1.0;
      double Exnum = 0.0; double Exden = 1.0;
      for (int Ei = 0; Ei < 8; Ei++) {
         Exnum = (Exnum + Ep[Ei]) * Ez;
         Exden = (Exden * Ez) + Eq[Ei];
      }
      Eres = (Exnum / Exden) + 1.0;
   }
   // Adjust result for case 0.0 < Ex < 1.0
   if (E01check) Eres = Eres / Ex1;
   // Adjust result for case 2.0 < Ex < 12.0
   const double Eintxn = Exn + std::numeric_limits<float>::min();
   if (fabs(Exn) > verySmallFloat) {
      for (int Ej = 1; Ej <= Eintxn; Ej++) {
         Eres *= Ex; Ex += 1.0;
      }
   }
   // compute for Ex >= 12
   if (Ex >= 12) {
      const double Eyy = Ex; const double Eysq = Eyy * Eyy;
      double Esum = Ec[6];
      for (int Ek = 0; Ek < 6; Ek++) {
         Esum = Esum / Eysq + Ec[Ek];
      }
      const double Espi = 0.9189385332046727417803297;
      Esum = (Esum / Eyy) - Eyy + Espi;
      Esum += (Eyy - 0.5) * log(Eyy);
      Eres = exp(Esum);
   }
   // Final adjustments.
   if (Enegcheck) Eres = Efact / Eres;
   // return the result
   return Eres;
}

// gampdf function
// gampdf is modified from the algorithm found in the Matlab file gampdf.m
// returns the value of the gampdf function evaluated at x, with parameters a
// and b. a & b correspond to r and 1/lambda in some formulations
double gampdf(const double x, const double a, const double b)
{
   if (x < verySmallFloat) return 0.0L;
   const double z = x / b;
   const double u = (a - 1) * log(z) - z - log(myGamma(a));
   return exp(u) / b;
}

void Calc::getCalcArgs(const CalcNode * StartNode, CalcNode * &EndNode,
                       double &ArgOne, double &ArgTwo)
{
   const string fn = StartNode->str;
   ArgOne = Calculate(StartNode->next, EndNode);
   if (EndNode->prev->str != ",") {
      CalcErr(fn + " takes two arguments: ',' expected here -> "
                 + EndNode->prev->str);
   }
   ArgTwo = Calculate(EndNode->prev, EndNode);
   if (EndNode->prev->str != "]") {
      CalcErr(fn + " takes only two arguments: ']' expected here -> "
                 + EndNode->prev->str);
   }
}

void Calc::getCalcArgs(const CalcNode * StartNode, CalcNode * &EndNode,
                       double &ArgOne, double &ArgTwo, double &ArgThree)
{
   const string fn = StartNode->str;
   ArgOne = Calculate(StartNode->next, EndNode);
   if (EndNode->prev->str != ",") {
      CalcErr(fn + " takes three arguments: ',' expected here -> "
                 + EndNode->prev->str);
   }
   ArgTwo = Calculate(EndNode->prev, EndNode);
   if (EndNode->prev->str != ",") {
      CalcErr(fn + " takes three arguments: ',' expected here -> "
                 + EndNode->prev->str);
   }
   ArgThree = Calculate(EndNode->prev, EndNode);
   if (EndNode->prev->str != "]") {
      CalcErr(fn + " takes only three arguments: ']' expected here -> "
                 + EndNode->prev->str);
   }
}

inline bool isUnaryCompatible(const string prevStr) {
   if ((prevStr == "[") || (prevStr == "~")) return true;
   if (prevStr != "!") // '!+' should be treated differently
      for (int i=0; MathStrings[i] != "\\"; i++)
         if (prevStr == MathStrings[i]) return true;
   return false;
}

void removeCurNode(CalcNode * &CurrentNode, CalcNode * &NextNode)
{
   NextNode->prev = CurrentNode->prev;
   CurrentNode->prev->next = NextNode;
   CurrentNode->next = NULL;
   delete CurrentNode;
   CurrentNode = NextNode;
   NextNode = NextNode->next;
}

void removeNextNode(CalcNode * &CurrentNode, CalcNode * &NextNode)
{
   CurrentNode->next = NextNode->next;
   NextNode->next->prev = CurrentNode;
   NextNode->next = NULL;
   delete NextNode;
   NextNode = CurrentNode->next;
}

double min(DataList arg)
{
   if (!arg.size()) CalcErr("min takes at least one argument");
   double retval = arg.at(0);
   for (unsigned int i = 1; i < arg.size(); i++) {
      if (arg.at(i) < retval) retval = arg.at(i);
   }
   return retval;
}

double max(DataList arg)
{
   if (!arg.size()) CalcErr("max takes at least one argument");
   double retval = arg.at(0);
   for (unsigned int i = 1; i < arg.size(); i++) {
      if (arg.at(i) > retval) retval = arg.at(i);
   }
   return retval;
}

Noise Calc::UserNoise; // random number generator for user specified rngs
bool Calc::isNoiseInit = false;

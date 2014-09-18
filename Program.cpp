/**************************************************************/
/* Include Files */
/**************************************************************/
#  if !defined(PROGRAM_HPP)
#    include "Program.hpp"
#  endif
#  if !defined(NEURONTYPE_HPP)
#    include "NeuronType.hpp"
#  endif
#  if !defined(PARALLEL_HPP)
#    include "Parallel.hpp"
#  endif
#  if !defined(POPULATION_HPP)
#    include "Population.hpp"
#  endif
#  if !defined(SYMBOLS_HPP)
#    include "Symbols.hpp"
#  endif
#  if !defined(SYNAPSE_HPP)
#    include "Synapse.hpp"
#  endif
#  include <iostream>
#  include <fstream>
#  include <cmath>
#  include <limits>
#  include <string>
using namespace std;

/**************************************************************/
/**************************************************************/
/* program class function definitions */
/**************************************************************/
/**************************************************************/
bool program::areDefaultsSet = false;
program* program::mainPgm = NULL;

program::program()
{
   LoadLists();
   return;
}

void program::LoadLists()
{
   // Load the AtFunctions 
   SystemVar::AddAtFun("Echo", Echo);
   SystemVar::AddAtFun("Print", Print);
   SystemVar::AddAtFun("PrintVar", PrintVar);
#   if !defined(WIN32)
   SystemVar::AddAtFun("System", System);
#   endif
   // Load the CaretFunctions 
   Calc::initCalc();

   return;
}

int program::GetIzhExplicitCount() const
{
   int IzhExplicitCount = 0;
   if (fabs(SystemVar::GetFloatVar("IzhA") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   if (fabs(SystemVar::GetFloatVar("IzhB") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   if (fabs(SystemVar::GetFloatVar("IzhC") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   if (fabs(SystemVar::GetFloatVar("IzhD") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   if (fabs(SystemVar::GetFloatVar("IzhvStart") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   if (fabs(SystemVar::GetFloatVar("IzhuStart") + 1.0f) > verySmallFloat) IzhExplicitCount++;
   return IzhExplicitCount;
}

void program::PrintHelp(string str, string funct) const {
   int gothelp = false;
   ArgListType argHelp(1);
   argHelp.at(0) = ArgType("-help", false);

   if (((str[0] == '@') || (str[0] == '^')) && str.length() != 1)
   {
      if (funct == "")
      {
         funct = str.substr(1, str.length() -1 );
         str = str[0];
      }
      else
      {
         Output::Err() << "conflict - did you want help on " << str << " or " << str[0] << funct <<"?\n\n";
         PrintHelp("help");
         exit(EXIT_FAILURE);
      }
   }
   if ((str == "@") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl << "@ Functions:" << std::endl;
         SystemVar::OutputAtFuns();
      } else {
         AT_FUN helpfun = SystemVar::GetAtFun(funct);
         if (helpfun == NULL) {
            Output::Err() << funct << " is not an @ function.\n\n";
            PrintHelp("@");
            PrintHelp("help");
            exit(EXIT_FAILURE);
         }
         helpfun(argHelp);
      }
      gothelp = true;
   }

   if ((str == "^") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl << "^ Functions:" << std::endl;
         SystemVar::OutputCaretFuns();
      } else {
         CARET_FUN helpfun = SystemVar::GetCaretFun(funct);
         if (helpfun == NULL) {
            Output::Err() << funct << " is not a ^ function.\n\n";
            PrintHelp("^");
            PrintHelp("help");
            exit(EXIT_FAILURE);
         }
         helpfun(argHelp);
      }
      gothelp = true;
   }

   if ((str == "int") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl << "int variables:" << std::endl;
         SystemVar::OutputIntVars();
      } else {
         if (SystemVar::GetVarType(funct) == 'i') {
            Output::Err() << funct << " : Sorry, no help currently available.\n";
         } else {
            Output::Err() << funct << " is not an int variable.\n\n";
            PrintHelp("int");
            PrintHelp("help");
            exit(EXIT_FAILURE);
         }
      }
      gothelp = true;
   }

   if ((str == "float") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl << "float variables:" << std::endl;
         SystemVar::OutputFloatVars();
      } else {
         if (SystemVar::GetVarType(funct) == 'f') {
            Output::Err() << funct << " : Sorry, no help currently available.\n";
         } else {
            Output::Err() << funct << " is not a float variable.\n\n";
            PrintHelp("float");
            PrintHelp("help");
            exit(EXIT_FAILURE);
         }
      }
      gothelp = true;
   }

   if ((str == "string") || (str == "")) {
      if (funct == "") {         
         Output::Err() << std::endl << "string variables:" << std::endl;
         SystemVar::OutputStrVars();
      } else {
         if (SystemVar::GetVarType(funct) == 's') {
            Output::Err() << funct << " : Sorry, no help currently available.\n";
         } else {
            Output::Err() << funct << " is not a string variable.\n\n";
            PrintHelp("string");
            PrintHelp("help");
            exit(EXIT_FAILURE);
         }
      }
      gothelp = true;
   }

   if ((str == "symbol") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl << "grammar symbols:" << std::endl;
         for (int i = 1; i <= NumGrammarSymbols; i++) {
            Output::Err() << " " << GrammarSymbolList[i] << '\t' << SymbolHelp[i] << std::endl;
         }
      } else {
         if ((funct.size() == 1) && GrammarSymbol(funct[0])) {
            Output::Err() << std::endl << funct[0] << '\t'
                    << SymbolHelp[GrammarSymbol(funct[0])] << std::endl;
         } else {
            Output::Err() << funct << " is not a grammar symbol.\n\n";
            PrintHelp("symbol");
            PrintHelp("help");
         }
      }
      gothelp = true;
   }

   if ((str == "help") || (str == "")) {
      if (funct == "") {
         Output::Err() << std::endl;
         Output::Err() << "\nUse -help @ to get a list of @ functions"
            "\nUse -help ^ to get a list of ^ functions"
            "\nUse -help int to get a list of int variable"
            "\nUse -help float to get a list of float variables"
            "\nUse -help string to get a list of string variables"
            "\nUse -help symbol to get a list of grammar symbols"
            "\nUse -help help to get a list of help commands"
            "\nUse -help @ FUNCTION to get the help for an @ function"
            "\nUse -help ^ FUNCTION to get the help for a ^ function"
            "\nUse -help int VARIABLE to get the help for an int variable"
            "\nUse -help float VARIABLE to get the help for a float variable"
            "\nUse -help string VARIABLE to get the help for a string variable"
            "\nUse -help symbol SYMBOL to get the help for a grammar symbol"
            "\n"
            "\nNeuroJet sends help information to standard error. To redirect"
            "\nstandard error to standard out and then pipe to a command"
            "\nuse '2>&1 |' in ksh and '|&' in csh or tcsh. To send to a"
            "\nfile replace the '|' with a '>' in the above commands.";
      } else {
         CALL_ERROR << str << " " << funct << " is an invalid help sequence.\n\n" << ERR_WHERE;
         PrintHelp("help");
         exit(EXIT_FAILURE);
      }
      gothelp = true;
   }

   if (!gothelp) {
      CALL_ERROR << str << " " << funct << " is an invalid help sequence.\n\n" << ERR_WHERE;
      PrintHelp("help");
      exit(EXIT_FAILURE);
   }
   Output::Err() << std::endl << std::endl;

   return;
}

void program::setAllSeeds() {
   // First seed the rngs that must match from node to node
   const int tmpseed = SystemVar::GetIntVar("seed");
   ExternalNoise.Reset(tmpseed);
   PickNoise.Reset(tmpseed);
   ResetNoise.Reset(tmpseed);
   WeightNoise.Reset(tmpseed);
#if defined(MULTIPROC)
   ShuffleNoise.Reset(tmpseed);
#endif
   TieBreakNoise.Reset(tmpseed);
   Calc::ResetUserNoise(tmpseed);
   // Then seed the rngs that must differ from node to node
#if defined(MULTIPROC)
   // here, I initialize the node-specific random number generator
   // the magic #, 102, is totally arbitrary -- so if you can think of a better way to space the seeds, go for it
   const int specseed = (tmpseed + ParallelInfo::getRank() * 102) % 32000;
   Output::Out() << MSG << "NeuroJet node seed: " << specseed << std::endl;
   DendriticSynapse::SynNoise.Reset(specseed);
   ConnectNoise.Reset(specseed);
   ParallelInfo::resetRandComm(specseed);
#else
   DendriticSynapse::SynNoise.Reset(tmpseed);
   ConnectNoise.Reset(tmpseed);
#endif
   isNoiseInit = true;
}

void program::setDefaults(unsigned int ni) {
   areDefaultsSet = true;
   NeuronType::addMember("default", true, false);
   LearningRuleType learningRule = LRT_PostSyn;
   if (SystemVar::GetIntVar("UseMvgAvg")) learningRule = LRT_MvgAvg;
   const float mu = SystemVar::GetFloatVar("mu");
   const unsigned int NMDArise = static_cast<unsigned int>(SystemVar::GetIntVar("NMDArise"));
   const float synFailRate = SystemVar::GetFloatVar("synFailRate");
   const float alpha = SystemVar::GetFloatVar("alpha");
   const float resolution = SystemVar::GetFloatVar("resolution");
   const float inv = SystemVar::GetFloatVar("inv");
   const int stochastic = SystemVar::GetIntVar("stochastic");
   const float Ksyn = 1.0f;
   SynapseType::addMember("default", learningRule, mu, NMDArise, alpha, Ksyn, synFailRate, "default", "default", resolution, inv, stochastic);
   Population::addMember(Population(0, ni-1, NeuronType::Member["default"]));
   float f = Population::Member[0].getFeedbackInhibition();
   f = f *2;
}

/**************************************************************/
/* AtFunction Definitions */
/**************************************************************/

void Echo(ArgListType &arg) //AT_FUN
{
   IFROOTNODE {
      if ((arg.size() > 0) && (arg.at(0).first == "-help")) {
         Output::Err() << "Echo -help\n\n"
            "@Echo( ... ) takes a list of one or more strings and displays\n"
            "them to StdOut separated by spaces and terminated with a new line.\n"
            "Unlike most functions, Echo does not require any preceeding flags.\n"
            << std::endl;
         exit(EXIT_FAILURE);
      }
      Print(arg);
      Output::Out() << std::endl;
   }
   return;
}

void Print(ArgListType &arg) //AT_FUN
{
   IFROOTNODE {
      if ((arg.size() > 0) && (arg.at(0).first == "-help")) {
         Output::Err() << "Print -help\n\n"
            "@Print( ... ) takes a list of one or more strings and displays\n"
            "them to StdOut. Unlike most functions, Print does not require\n"
            "any preceeding flags.\n" << std::endl;
         exit(EXIT_FAILURE);
      }
      for (ArgListTypeIt it = arg.begin(); it != arg.end(); it++) {
         Output::Out() << it->first;
      }
   }
   return;
}

void PrintVar(ArgListType &arg) //AT_FUN
{
   if (arg.size() < 1) {
      CALL_ERROR << "Error in PrintVar: expects at least one argument." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "PrintVar -help\n\n"
         "@PrintVar( ... ) takes a list of one or more variables and prints their\n"
         "values on StdOut in the form 'VarName = VarValue'.\n" <<
         "Unlike most functions, PrintVar does not require any preceeding flags.\n"
         << std::endl;
      exit(EXIT_FAILURE);
   }
   IFROOTNODE {
      for (ArgListTypeIt it = arg.begin(); it != arg.end(); it++) {
         string curArg = it->first;
         Output::Out() << MSG << curArg << " = ";
         char varType = SystemVar::GetVarType(curArg);
         if (varType == 'i') {
            Output::Out() << SystemVar::GetIntVar(curArg) << std::endl;
         } else if (varType == 'f') {
            Output::Out() << setprecision(6) << SystemVar::GetFloatVar(curArg) << std::endl;
         } else if (varType == 's') {
            Output::Out() << SystemVar::GetStrVar(curArg) << std::endl;
         } else if (varType == 'I') {
            Output::Out() << SystemVar::GetIterator(curArg).CurrentVal << std::endl;
         } else {
            CALL_ERROR << "Error in PrintVar: " << curArg << " not found. \n\n" << ERR_WHERE;
            exit(EXIT_FAILURE);
         }
      }
   }
   return;
}

#  if !defined(WIN32)
void System(ArgListType &arg) //AT_FUN
{
   if (arg.size() != 1) {
      CALL_ERROR << "Error in System: Can call with only one argument.\n" <<
          "Try putting quotes around your arguments." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (arg.at(0).first == "-help") {
      Output::Err() << "System -help\n\n"
         "@System( ... ) passes a single argument to be executed outside of the program.\n"
         "Unlike most functions, System does not require any preceeding flags.\n"
         << std::endl;
      exit(EXIT_FAILURE);
   }
   system(arg.at(0).first.c_str());
}
#  endif

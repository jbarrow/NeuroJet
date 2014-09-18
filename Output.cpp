#if !defined(OUTPUT_HPP)
#   include "Output.hpp"
#endif

#if !defined(DATATYPES_HPP)
#   include "DataTypes.hpp"
#endif

string Output::CurrentUser = "";
ostream *Output::StdOut = NULL;
ostream *Output::StdErr = NULL;

ScriptState * ScriptState::ms_curScriptState = NULL;

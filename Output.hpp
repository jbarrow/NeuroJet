// Output.hpp
//

#if !defined(OUTPUT_HPP)
#   define OUTPUT_HPP

#   if !defined(BORLAND)
#      if !defined(WIN32)
#         define GOOD_COMPILER
#      endif
#   endif

#define CALL_ERROR Output::Err() << "ERROR\n\n"
#define ERR_WHERE ScriptState::getLocation() << std::endl << std::endl

#   include <string>
#   include <ostream>
#   include <stdio.h>

using std::ostream;
using std::string;

////////////////////////////////////////////////////////////////////////////////
// Some script parsing functions
////////////////////////////////////////////////////////////////////////////////

class StackNode {
private:
public:
   StackNode(): str(""), line(0), next(NULL) {};
   StackNode(string curData, unsigned int curLine) :
      str(curData), line(curLine), next(NULL) {};
   inline ~StackNode() { if (next) delete next; };
   inline StackNode * add(string newStr, unsigned long newLine) {
      if (str != "") {
         // Assign new StackNode to next and return it
         return (next = new StackNode(newStr, newLine));
      }
      str = newStr; line = newLine;
      return this;
   }
   string str;
   unsigned int line;
   StackNode *next;
};

class ScriptState {
public:
   inline ScriptState(string name):
      m_curLine(0), m_scriptName(name), m_parentState(ms_curScriptState),
      m_StackTop(NULL) {
         ms_curScriptState = this;
         m_StackTop = new StackNode;
   };
   inline ~ScriptState() {
      ms_curScriptState = m_parentState;
      if (m_StackTop)
         delete m_StackTop;
   };
   inline static unsigned int getCurLine() {
      return ms_curScriptState->m_curLine;
   };
   inline static string getLocation() {
      if (ms_curScriptState) return ms_curScriptState->desc();
      return "";
   };
   inline static StackNode * getStackTop() {return ms_curScriptState->m_StackTop;}
   inline static void setCurLine(unsigned int curLine) {
      ms_curScriptState->m_curLine = curLine;
   };
private:
   inline string desc() const {      
      string toReturn = "";
      if (m_curLine > 0) {
         char buf[20];
         sprintf(buf, "%d", m_curLine);
         toReturn = " in " + m_scriptName + " at line number " + string(buf);
      }
      if (m_parentState)
         toReturn += "\n\t" + m_parentState->desc();
      return toReturn;
   }
   unsigned int m_curLine;
   const string m_scriptName;
   ScriptState * const m_parentState;
   StackNode * m_StackTop;
   static ScriptState * ms_curScriptState;
};

class Output {
private:
   static ostream *StdOut;
   static ostream *StdErr;
   static string CurrentUser;
public:
   static inline void setStreams(ostream &out, ostream &err) {
      StdOut = &out; StdErr = &err;
   }
   static inline ostream& Out() { return *StdOut; }
   static inline ostream& Err() { return *StdErr; }
};

#endif // OUTPUT_HPP

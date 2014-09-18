#if !defined(MATLAB_HPP)
#  define MATLAB_HPP

#  include <string>

class MatlabCommand {
public:
   MatlabCommand(std::string LHS, std::string RHS):
      m_LHS(LHS), m_RHS(RHS) {};
   std::string getLHS() const {return m_LHS;};
   std::string getRHS() const {return m_RHS;};
private:
   // LHS is left hand side of (first) equal sign
   // RHS is right hand side of equal sign
   // If there is no equal sign, then only the RHS exists
   std::string m_LHS;
   std::string m_RHS;
};
#endif

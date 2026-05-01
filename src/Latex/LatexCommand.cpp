#include "LatexCommand.hpp"
ClassImp(CAP::LatexCommand);

namespace CAP
{
  LatexCommand::LatexCommand()
  :
  LatexElement()
  { }

  LatexCommand::LatexCommand(const LatexCommand & src)
  :
  LatexElement(src),
  _comm(src._comm)
  { }

  LatexCommand  & LatexCommand::operator=(const LatexCommand & rhs)
  {
  //  if (rhs!=*this)
  //    {
  LatexElement::operator=(rhs);
  _comm = rhs._comm;
  //    }
  return *this;
  }

  void LatexCommand::write(std::ofstream & out)
  {
  const String & key  = keyword();
  const String & str  = command();
  out << "\\newcommand{" << key << "}{" << str << "}" << endl;
  }

} // namespace CAP


#include "LatexSubsection.hpp"
ClassImp(CAP::LatexSubsection);

namespace CAP
{
  LatexSubsection::LatexSubsection()
  :
  LatexElement()
  { }

  LatexSubsection::LatexSubsection(const LatexSubsection & src)
  :
  LatexElement(src)
  { }

  LatexSubsection  & LatexSubsection::operator=(const LatexSubsection & rhs)
  {
//  if (rhs!=*this)
//    {
    LatexElement::operator=(rhs);
//    }
  return *this;
  }

  void LatexSubsection::writeHeader(std::ofstream & out)
  {
  skipLines(out,2);
  if (isExcluded())
    out << "\\subsection*{" << name() << "}" << endl;
  else
    out << "\\subsection{" << name() << "}" << endl;
  if (label().Length()>0)
    out << "\\label{sec:" << label() << "}" << endl;
  skipLines(out,1);
  }

} // namespace CAP

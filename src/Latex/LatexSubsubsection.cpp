#include "LatexSubsubsection.hpp"
ClassImp(CAP::LatexSubsubsection);
ClassImp(CAP::LatexParagraph);

namespace CAP
{
  LatexSubsubsection::LatexSubsubsection()
  :
  LatexElement()
  { }

  LatexSubsubsection::LatexSubsubsection(const LatexSubsubsection & src)
  :
  LatexElement(src)
  { }

  LatexSubsubsection  & LatexSubsubsection::operator=(const LatexSubsubsection & rhs)
  {
//  if (rhs!=*this)
//    {
    LatexElement::operator=(rhs);
//    }
  return *this;
  }

  void LatexSubsubsection::writeHeader(std::ofstream & out)
  {
  skipLines(out,2);
  if (isExcluded())
    out << "\\subsubsection*{" << name() << "}" << endl;
  else
    out << "\\subsubsection{" << name() << "}" << endl;

  if (label().Length()>0)
    out << "\\label{sec:" << label() << "}" << endl;
  skipLines(out,1);
  }

} // namespace CAP

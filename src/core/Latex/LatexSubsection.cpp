#include "LatexSubsection.hpp"
//ClassImp(CAP::LatexSubsection);

namespace CAP
{
  LatexSubsection::LatexSubsection()
  :
  LatexSection()
  { }

  LatexSubsection::LatexSubsection(const LatexSubsection & src)
  :
  LatexSection(src)
  { }

  LatexSubsection  & LatexSubsection::operator=(const LatexSubsection & rhs)
  {
  if (this!=&rhs)
    {
    LatexSection::operator=(rhs);
    }
  return *this;
  }

  void LatexSubsection::writeHeader(std::ofstream & out)
  {
  skipLines(out,2);
  if (isExcluded())
    out << "\\subsection*{" << name() << "}" << endl;
  else
    out << "\\subsection{" << name() << "}" << endl;
  if (label().length()>0)
    out << "\\label{sec:" << label() << "}" << endl;
  skipLines(out,1);
  }

} // namespace CAP

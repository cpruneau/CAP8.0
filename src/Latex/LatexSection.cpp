#include "LatexSection.hpp"
ClassImp(CAP::LatexSection);

namespace CAP
{
  LatexSection::LatexSection()
  :
  LatexElement()
  { }

  LatexSection::LatexSection(const LatexSection & src)
  :
  LatexElement(src)
  { }

  LatexSection  & LatexSection::operator=(const LatexSection & rhs)
  {
//  if (rhs!=*this)
//    {
    LatexElement::operator=(rhs);
//    }
  return *this;
  }

  void LatexSection::writeHeader(std::ofstream & out)
  {
  skipLines(out,2);
  if (isExcluded())
    out << "\\section*{" << name() << "}" << endl;
  else
    out << "\\section{" << name() << "}" << endl;
  if (label().Length()>0)
    out << "\\label{sec:" << label() << "}" << endl;
  skipLines(out,1);
  }

} // namespace CAP


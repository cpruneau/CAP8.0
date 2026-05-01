#include "LatexParagraph.hpp"
ClassImp(CAP::LatexParagraph);

namespace CAP
{
  LatexParagraph::LatexParagraph()
  :
  LatexElement()
  { }

  LatexParagraph::LatexParagraph(const LatexParagraph & src)
  :
  LatexElement(src)
  { }

  LatexParagraph  & LatexParagraph::operator=(const LatexParagraph & rhs)
  {
  //  if (rhs!=*this)
  //    {
  LatexElement::operator=(rhs);
  //    }
  return *this;
  }

  void LatexParagraph::writeHeader(std::ofstream & out)
  {
  skipLines(out,2);
  if (isExcluded())
    out << "\\paragraph*{" << name() << "}" << endl;
  else
    out << "\\paragraph{" << name() << "}" << endl;

  if (label().Length()>0)
    out << "\\label{sec:" << label() << "}" << endl;
  skipLines(out,1);
  }

} // namespace CAP

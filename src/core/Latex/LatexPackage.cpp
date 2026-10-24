#include "LatexPackage.hpp"

namespace CAP
{
  LatexPackage::LatexPackage()
  :
  LatexElement()
  { }

  LatexPackage::LatexPackage(const LatexPackage & src)
  :
  LatexElement(src)
  { }

  LatexPackage  & LatexPackage::operator=(const LatexPackage & rhs)
  {
  //  if (rhs!=*this)
  //    {
  LatexElement::operator=(rhs);
  //    }
  return *this;
  }

  void LatexPackage::write(std::ofstream & out)
  {
  if (option().length()>0)
    out << "\\usepackage[" << option() << "]{" << name() << "}" <<  endl;
  else
    out << "\\usepackage{" << name() << "}"<< endl;
  }
}


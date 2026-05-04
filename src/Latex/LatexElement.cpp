#include "LatexElement.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::LatexElement);

namespace CAP
{

  void LatexElement::skipLines(std::ofstream & out, int n)
  {
  for (int k=0; k<n; k++) out << endl;
  }

  LatexElement::LatexElement()
  :
  _name("NoName"),
  _label(),
  _excluded(false),
  _children(),
  _parent(nullptr)
  { }

  LatexElement::LatexElement(const LatexElement & src)
  :
  _name(src._name),
  _label(src._label),
  _excluded(src._excluded),
  _children(src._children),
  _parent(src._parent)
  { }

  LatexElement  & LatexElement::operator=(const LatexElement & rhs)
  {
  //if (rhs != *this)
  //{
  _name      = rhs._name;
  _label     = rhs._label;
  _excluded  = rhs._excluded;
  _children  = rhs._children;
  _parent    = rhs._parent;
  // }
  return *this;
  }

  void LatexElement::reset()
  {
  _name      = "NoName";
  _label     = "";
  _excluded  = false;
  _parent    = nullptr;
  _children.clear();
  }

  void LatexElement::addChild(LatexElement * node)
  {
  node->setParent(this);
  _children.push_back(node);
  }

  void LatexElement::write(std::ofstream & out)
  {
  writeHeader(out);
  writeContent(out);
  writeTrailer(out);
  }

  void LatexElement::writeContent(std::ofstream & out)
  {
  for (auto child : _children) child->write(out);
  }

  void LatexElement::writeHeader(std::ofstream & out)
  {
  skipLines(out,1);
  }

  void LatexElement::writeTrailer(std::ofstream & out)
  {
  skipLines(out,1);
  }


} // namespace CAP

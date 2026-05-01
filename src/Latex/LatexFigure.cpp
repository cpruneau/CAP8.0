#include "LatexFigure.hpp"

ClassImp(CAP::LatexFigure);

namespace CAP
{
  LatexFigure::LatexFigure()
  :
  LatexElement(),
  _caption(),
  _scale(0.75),
  _height(2),
  _width(0),
  _trimLeft(0),
  _trimBottom(0),
  _trimRight(0),
  _trimTop(0)
  { }

  LatexFigure::LatexFigure(const LatexFigure & src)
  :
  LatexElement(src),
  _caption(src._caption),
  _scale(src._scale),
  _height(src._height),
  _width(src._width),
  _trimLeft(src._trimLeft),
  _trimBottom(src._trimBottom),
  _trimRight(src._trimRight),
  _trimTop(src._trimTop)
  { }

  LatexFigure  & LatexFigure::operator=(const LatexFigure & rhs)
  {
  //  if (rhs!=*this)
  //    {
  LatexElement::operator=(rhs);
  _caption    = rhs._caption;
  _scale      = rhs._scale;
  _height     = rhs._height;
  _width      = rhs._width;
  _trimLeft   = rhs._trimLeft;
  _trimBottom = rhs._trimBottom;
  _trimRight  = rhs._trimRight;
  _trimTop    = rhs._trimTop;
  //    }
  return *this;
  }

  void LatexFigure::writeHeader(std::ofstream & out)
  {
  out << "\\begin{figure}" << endl;
  }

  void LatexFigure::writeContent(std::ofstream & out)
  {
  String s(""); s += _scale;
  String h(""); h += _height;
  out << "\\includegraphics[scale=\"" << s << ",height=\"" << h << "in\"]{" << fileName() << "}" << endl;
  out << "\\caption{" << caption() << "}" << endl;
  out << "\\label{fig:" << label() << "}" << endl;
  }

  void LatexFigure::writeTrailer(std::ofstream & out)
  {
  out << "\\end{figure}" << endl;
  }

} //namespace CAP

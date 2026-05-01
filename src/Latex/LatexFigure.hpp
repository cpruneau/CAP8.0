#ifndef CAP__LatexFigure
#define CAP__LatexFigure
#include "LatexElement.hpp"

namespace CAP
{
  class LatexFigure : public LatexElement
  {
  public:

  LatexFigure();
  LatexFigure(const LatexFigure & src);
  LatexFigure  & operator=(const LatexFigure & rhs);
  virtual ~LatexFigure() {  }
  virtual void writeHeader(std::ofstream & out);
  virtual void writeContent(std::ofstream & out);
  virtual void writeTrailer(std::ofstream & out);

  void setFileName(const String & fileName) { _name = fileName; }
  void setCaption(const String & caption)   { _caption = caption; }
  void setScale(float v)  { _scale = v; }
  void setHeight(float v) { _height = v; }
  void setWidth(float v) { _width = v; }
  void setTrim(int left, int bottom, int right, int top);

  const String & fileName() const { return _name; }
  const String & caption() const  { return _caption;  }
  float scale() const { return _scale; }
  float height() const { return _height; }
  float width() const { return _width; }
  int   trimLeft()   const { return _trimLeft; }
  int   trimBottom() const { return _trimBottom; }
  int   trimRight()  const { return _trimRight; }
  int   trimTop()    const { return _trimTop; }

  protected:

  String _caption;
  float  _scale;
  float  _height;
  float  _width;
  int _trimLeft;
  int _trimBottom;
  int _trimRight;
  int _trimTop;

  ClassDef(LatexFigure,0)
  };

} // namespace CAP

#endif // !CAP__LatexFigure

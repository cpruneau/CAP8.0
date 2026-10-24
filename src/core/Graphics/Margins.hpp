#ifndef CAP__Margins
#define CAP__Margins
#include "TObject.h"
#include "Aliases.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Class encapsulates the notion of margin used in plotting graphs or producing documents
  //! Units are in fraction of the document size.
  //!
  class Margins
  {
  public:

  Margins();
  Margins(float left, float bottom, float right, float top);
  Margins(const Margins & src);
  Margins  & operator=(const Margins & rhs);
  virtual ~Margins() {  }

  void set(float left, float bottom, float right, float top)
  {
  _left   = left;
  _bottom = bottom;
  _right  = right;
  _top    = top;
  }

  void setLeft(float left)      { _left   = left;  }
  void setBottom(float bottom)  { _bottom = bottom;  }
  void setRight(float right)    { _right  = right;  }
  void setTop(float top)        { _top    = top;  }

  float left()   const { return _left;}
  float bottom() const { return _bottom;}
  float right()  const { return _right;}
  float top()    const { return _top;}

  static void setDefaultLeft(float v)   { _defaultLeft   = v;}
  static void setDefaultBottom(float v) { _defaultBottom = v;}
  static void setDefaultRight(float v)  { _defaultRight  = v;}
  static void setDefaultTop(float v)    { _defaultTop    = v;}

  static float defaultLeft()   { return _defaultLeft;}
  static float defaultBottom() { return _defaultBottom;}
  static float defaultRight()  { return _defaultRight;}
  static float defaultTop()    { return _defaultTop;}

  protected:

  float _left;
  float _bottom;
  float _right;
  float _top;

  static float _defaultLeft;
  static float _defaultBottom;
  static float _defaultRight;
  static float _defaultTop;

  ClassDef(Margins,0)
  };


  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Margins

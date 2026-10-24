#ifndef CAP__Line
#define CAP__Line
#include <vector>
#include "String.hpp"
#include "Drawable2D.hpp"
#include "LineAttributes.hpp"
#include "Point2D.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //! Class encapsulates the notion of line. The line can be a single stragiht segment or
  //! consist of n-1 segments defined by n points
  //!
  class Line : public Drawable2D
  {
  public:

  Line();
  Line(const Point2D & point1, const Point2D & point2);
  Line(const std::vector<Point2D> & points);
  Line(const Line & src);
  Line  & operator=(const Line & rhs);
  virtual ~Line() { }
  virtual void reset();

  void setLineAttributes(const LineAttributes & lineAttributes) { _lineAttributes = lineAttributes; }
  LineAttributes & lineAttributes() { return _lineAttributes; }
  const LineAttributes & lineAttributes() const { return _lineAttributes; }

  //! Distance from p to the segment p1-p2, minus half the line width.
  //! All arguments must be in the same units (e.g. pixels); returns 9999 when p is
  //! outside the box surrounding the segment (with a 2-unit tolerance).
  static float distanceToLine(const Point2D & p, const Point2D & p1, const Point2D & p2, float lineWidth);

  protected:

  LineAttributes _lineAttributes;

  ClassDef(Line,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Line

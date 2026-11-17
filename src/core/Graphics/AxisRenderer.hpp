#ifndef DATUM__AxisRenderer
#define DATUM__AxisRenderer
#include "Renderer.hpp"

namespace Datum
{
  namespace GRAPHICS
  {

  //!
  //! Draws an Axis: axis line, major and minor ticks, tick labels and title.
  //!
  class AxisRenderer : public Renderer
  {
  public:

  AxisRenderer();
  AxisRenderer(const AxisRenderer & src);
  AxisRenderer  & operator=(const AxisRenderer & rhs);
  virtual ~AxisRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const AxisRenderer * defaultRenderer();

  };

  } // namespace GRAPHICS
} // namespace Datum

#endif // !DATUM__AxisRenderer

#ifndef CAP__AxisRenderer
#define CAP__AxisRenderer
#include "Renderer.hpp"

namespace CAP
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
} // namespace CAP

#endif // !CAP__AxisRenderer

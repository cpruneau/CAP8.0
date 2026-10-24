#ifndef CAP__LineRenderer
#define CAP__LineRenderer
#include "Renderer.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Draws a Line as an open polyline through its points.
  //!
  class LineRenderer : public Renderer
  {
  public:

  LineRenderer();
  LineRenderer(const LineRenderer & src);
  LineRenderer  & operator=(const LineRenderer & rhs);
  virtual ~LineRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const LineRenderer * defaultRenderer();

  ClassDef(LineRenderer,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__LineRenderer

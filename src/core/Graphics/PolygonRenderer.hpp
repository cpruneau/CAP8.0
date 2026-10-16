#ifndef CAP__PolygonRenderer
#define CAP__PolygonRenderer
#include "Renderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Draws a Polygon (fill and outline) and its text, if any, at its center.
  //!
  class PolygonRenderer : public Renderer
  {
  public:

  PolygonRenderer();
  PolygonRenderer(const PolygonRenderer & src);
  PolygonRenderer  & operator=(const PolygonRenderer & rhs);
  virtual ~PolygonRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const PolygonRenderer * defaultRenderer();

  ClassDef(PolygonRenderer,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PolygonRenderer

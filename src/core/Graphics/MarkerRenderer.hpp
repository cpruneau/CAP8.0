#ifndef CAP__MarkerRenderer
#define CAP__MarkerRenderer
#include "Renderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Draws a Marker at its position, or one marker per data point of a MarkerVector.
  //!
  class MarkerRenderer : public Renderer
  {
  public:

  MarkerRenderer();
  MarkerRenderer(const MarkerRenderer & src);
  MarkerRenderer  & operator=(const MarkerRenderer & rhs);
  virtual ~MarkerRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const MarkerRenderer * defaultRenderer();

  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__MarkerRenderer

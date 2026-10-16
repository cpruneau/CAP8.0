#include "MarkerRenderer.hpp"
#include "Marker.hpp"
#include "MarkerVector.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::MarkerRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  MarkerRenderer::MarkerRenderer()
  :
  Renderer()
  { }

  MarkerRenderer::MarkerRenderer(const MarkerRenderer & src)
  :
  Renderer(src)
  { }

  MarkerRenderer  & MarkerRenderer::operator=(const MarkerRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const MarkerRenderer * MarkerRenderer::defaultRenderer()
  {
  static const MarkerRenderer renderer;
  return &renderer;
  }

  void MarkerRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  if (const Marker * marker = dynamic_cast<const Marker*>(&drawable))
    {
    painter.drawMarker(marker->position(), marker->markerAttributes());
    return;
    }
  if (const MarkerVector * markers = dynamic_cast<const MarkerVector*>(&drawable))
    {
    const Data2D * data = markers->data();
    if (!data) return;
    for (unsigned int k=0; k<data->size(); k++)
      {
      DataPoint2D p = data->point(k);
      painter.drawMarker(markers->toCanvas(p.x(), p.y()), markers->markerAttributes());
      }
    }
  }

  } // namespace GRAPHICS
} // namespace CAP

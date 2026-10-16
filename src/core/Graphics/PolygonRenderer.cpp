#include "PolygonRenderer.hpp"
#include "Polygon.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::PolygonRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  PolygonRenderer::PolygonRenderer()
  :
  Renderer()
  { }

  PolygonRenderer::PolygonRenderer(const PolygonRenderer & src)
  :
  Renderer(src)
  { }

  PolygonRenderer  & PolygonRenderer::operator=(const PolygonRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const PolygonRenderer * PolygonRenderer::defaultRenderer()
  {
  static const PolygonRenderer renderer;
  return &renderer;
  }

  void PolygonRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  const Polygon * polygon = dynamic_cast<const Polygon*>(&drawable);
  if (!polygon) return;
  std::vector<Point2D> vertices = polygon->vertices();
  if (vertices.size() < 3) return;
  painter.drawPolygon(vertices, polygon->fillAttributes(), polygon->lineAttributes());
  if (!polygon->text().isNull())
    painter.drawText(polygon->center(), polygon->text(), polygon->textAttributes(), polygon->angle());
  }

  } // namespace GRAPHICS
} // namespace CAP

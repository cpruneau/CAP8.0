#include "LineRenderer.hpp"
#include "Line.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::LineRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  LineRenderer::LineRenderer()
  :
  Renderer()
  { }

  LineRenderer::LineRenderer(const LineRenderer & src)
  :
  Renderer(src)
  { }

  LineRenderer  & LineRenderer::operator=(const LineRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const LineRenderer * LineRenderer::defaultRenderer()
  {
  static const LineRenderer renderer;
  return &renderer;
  }

  void LineRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  const Line * line = dynamic_cast<const Line*>(&drawable);
  if (!line || line->points().size() < 2) return;
  painter.drawLine(line->points(), line->lineAttributes());
  }

  } // namespace GRAPHICS
} // namespace CAP

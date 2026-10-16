#include "GraphRenderer.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::GraphRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  GraphRenderer::GraphRenderer()
  :
  Renderer()
  { }

  GraphRenderer::GraphRenderer(const GraphRenderer & src)
  :
  Renderer(src)
  { }

  GraphRenderer  & GraphRenderer::operator=(const GraphRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const GraphRenderer * GraphRenderer::defaultRenderer()
  {
  static const GraphRenderer renderer;
  return &renderer;
  }

  void GraphRenderer::draw(const Drawable2D & /*drawable*/, Painter & /*painter*/) const
  {  }

  } // namespace GRAPHICS
} // namespace CAP

#include "TextRenderer.hpp"
#include "Painter.hpp"


namespace CAP
{
  namespace GRAPHICS
  {

  TextRenderer::TextRenderer()
  :
  Renderer()
  { }

  TextRenderer::TextRenderer(const TextRenderer & src)
  :
  Renderer(src)
  { }

  TextRenderer  & TextRenderer::operator=(const TextRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const TextRenderer * TextRenderer::defaultRenderer()
  {
  static const TextRenderer renderer;
  return &renderer;
  }

  void TextRenderer::draw(const Drawable2D & /*drawable*/, Painter & /*painter*/) const
  {  }

  } // namespace GRAPHICS
} // namespace CAP

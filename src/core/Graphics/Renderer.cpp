#include "Renderer.hpp"


namespace CAP
{
  namespace GRAPHICS
  {

  Renderer::Renderer()
  { }

  Renderer::Renderer(const Renderer & /*src*/)
  { }

  Renderer  & Renderer::operator=(const Renderer & /*rhs*/)
  {
  return *this;
  }

  void Renderer::draw(const Drawable2D & /*drawable*/, Painter & /*painter*/) const
  {  }

  void Renderer::initialize()
  {  }

  void Renderer::reset()
  {  }

  } // namespace GRAPHICS
} // namespace CAP

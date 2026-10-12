#ifndef CAP__TextRenderer
#define CAP__TextRenderer
#include "Renderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Placeholder for a future Text drawable; draws nothing for now.
  //!
  class TextRenderer : public Renderer
  {
  public:

  TextRenderer();
  TextRenderer(const TextRenderer & src);
  TextRenderer  & operator=(const TextRenderer & rhs);
  virtual ~TextRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const TextRenderer * defaultRenderer();

  ClassDef(TextRenderer,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__TextRenderer

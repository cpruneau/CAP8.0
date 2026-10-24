#ifndef CAP__Renderer
#define CAP__Renderer
#include "TObject.h"
#include "Aliases.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Drawable2D;
  class Painter;

  //!
  //! Base class of all renderers. A renderer knows how to draw one kind of Drawable2D
  //! by issuing primitive calls (lines, polygons, markers, text) to a Painter.
  //! Renderers are stateless: one shared instance per type (defaultRenderer()) is enough.
  //!
  class Renderer
  {
  public:

  Renderer();
  Renderer(const Renderer & src);
  Renderer  & operator=(const Renderer & rhs);
  virtual ~Renderer() { }

  //! Draw drawable on painter. The base implementation draws nothing.
  virtual void draw(const Drawable2D & drawable, Painter & painter) const;
  virtual void initialize();
  virtual void reset();

  ClassDef(Renderer,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Renderer

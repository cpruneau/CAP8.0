#ifndef CAP__GraphRenderer
#define CAP__GraphRenderer
#include "Renderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Placeholder for Graph2D; draws nothing for now (Graph2D is not in the build yet).
  //!
  class GraphRenderer : public Renderer
  {
  public:

  GraphRenderer();
  GraphRenderer(const GraphRenderer & src);
  GraphRenderer  & operator=(const GraphRenderer & rhs);
  virtual ~GraphRenderer() { }

  virtual void draw(const Drawable2D & drawable, Painter & painter) const;

  //! Shared instance used by default by the drawables this renderer handles.
  static const GraphRenderer * defaultRenderer();

  ClassDef(GraphRenderer,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__GraphRenderer

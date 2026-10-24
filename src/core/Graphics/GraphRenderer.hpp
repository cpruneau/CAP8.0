#ifndef CAP__GraphRenderer
#define CAP__GraphRenderer
#include "Renderer.hpp"
#include "Graph.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Draws a Graph: frame, functions and data points (clipped to the frame), color maps, axes (linear or log), color
  //! scale, title, legend; or the 3D view of its maps (see Graph::is3D(), draw3D()).
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

  protected:

  //!
  //! @brief Draw the 3D view of the maps of a graph (see Projection3D): the floor and back walls of the box, the
  //! surfaces, meshes and lego boxes of the items (and scattered points), drawn from the farthest (painter's
  //! algorithm, only the faces turned to the eye), and the x, y and z axes on the front edges of the floor and the
  //! leftmost vertical edge. @param graph graph @param painter painter @param view ranges and scales
  //! @param textSize automatic size of the labels (fraction of the height) @param cellWidth width of the cell
  //! @param cellHeight height of the cell (fractions of the canvas)
  //!
  void draw3D(const Graph & graph, Painter & painter, const Graph::View & view, float textSize, float cellWidth, float cellHeight) const;

  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__GraphRenderer

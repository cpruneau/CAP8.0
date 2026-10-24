/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file LatexTextures.hpp
//! @brief LatexTextures: OpenGL textures of the texts typeset by LaTeX, for ImGuiPainter.
#ifndef CAP__GUI__LatexTextures
#define CAP__GUI__LatexTextures
#include <cstdint>
#include <map>
#include <utility>

struct NSVGrasterizer;

namespace CAP
{
  namespace GRAPHICS { struct LatexImage; }

  namespace GUI
  {

  //!
  //! @brief OpenGL textures of the texts typeset by LaTeX (see GRAPHICS::LatexRenderer), drawn by ImGuiPainter.
  //!
  //! The SVG paths of an image are rasterized by nanosvg (external/nanosvg) at the size of the text in pixels of the
  //! screen, into a white texture whose alpha is the coverage of the glyphs: ImGui tints it with the color of the
  //! text. Textures are kept by image and size (sizes in steps of half a pixel); those not drawn for a while are
  //! deleted by collect(), called once per frame. All the functions must be called with the OpenGL context of the
  //! interface current (in the thread of the interface).
  //!
  class LatexTextures
  {
  public:

  //! @brief A texture of a text: the ink box of the image, with a margin of kPadding pixels.
  struct Texture
  {
    unsigned int  id     = 0;   //!< OpenGL texture name
    int           width  = 0;   //!< width in pixels
    int           height = 0;   //!< height in pixels
    float         scale  = 1;   //!< pixels per SVG unit of the image
    int           lastFrame = 0;   //!< last frame in which it was drawn
  };

  static constexpr int kPadding = 1;   //!< margin around the ink (pixels)

  //! @brief The textures of the interface. @return textures
  static LatexTextures & instance();

  //! @brief Constructor (no texture yet).
  LatexTextures();
  //! @brief Destructor (the textures must have been deleted by clear(), while the OpenGL context existed).
  ~LatexTextures();

  //!
  //! @brief The texture of an image for a text whose font size is pixelSize pixels, made if needed.
  //! @param image image typeset by LaTeX (ok) @param pixelSize font size in pixels of the screen
  //! @param frame current frame (ImGui::GetFrameCount()) @return texture, or nullptr if it cannot be made
  //!
  const Texture * texture(const GRAPHICS::LatexImage & image, float pixelSize, int frame);
  //! @brief Delete the textures not drawn in the last frames, beyond a few hundred. @param frame current frame
  void collect(int frame);
  //! @brief Delete all the textures (before the OpenGL context is destroyed).
  void clear();
  //! @brief Number of textures. @return number
  int size() const { return int(_textures.size()); }

  protected:

  std::map<std::pair<const GRAPHICS::LatexImage*,int>,Texture> _textures;   //!< textures by image and half pixels
  NSVGrasterizer * _rasterizer;   //!< nanosvg rasterizer (made when first needed)
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__LatexTextures

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file LatexRenderer.hpp
//! @brief LatexRenderer: texts typeset by the LaTeX installation of the system, as SVG paths.
#ifndef CAP__LatexRenderer
#define CAP__LatexRenderer
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Aliases.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Drawable2D;

  //!
  //! @brief A text typeset by LaTeX: the glyphs as SVG paths, and the size of the box.
  //!
  //! The text is typeset once, at 10 TeX points (kFontSize in SVG units, i.e. big points); it is vector, so painters
  //! scale it to any size: by size/kFontSize for a text of size (the font size) size. In SVG units, the origin of the
  //! paths is on the baseline, at the left of the text, y down.
  //!
  struct LatexImage
  {
    static constexpr float kFontSize = 10.0f*72.0f/72.27f;   //!< size of the font of the typeset text (SVG units)

    bool        ok = false;   //!< the text was typeset (otherwise, see error)
    String      error;        //!< first error reported by LaTeX (when ok is false)
    std::string svg;          //!< SVG elements of the glyphs (paths and rules), without the svg element
    float       x      = 0;   //!< left edge of the ink (SVG units, from the origin)
    float       width  = 0;   //!< width of the ink (SVG units)
    float       height = 0;   //!< height of the ink above the baseline (SVG units)
    float       depth  = 0;   //!< depth of the ink below the baseline (SVG units)
  };

  //!
  //! @brief Texts typeset by the LaTeX installation of the system (latex and dvisvgm), for the painters.
  //!
  //! A text containing '$' or '\' is typeset by LaTeX, in text mode: write the math between $...$, e.g.
  //! "$p_{\mathrm{T}}$ (GeV/$c$)". Other texts are drawn by the painters with their fonts (kind()).
  //!
  //! TeX is a program, not a library: each typesetting runs latex then dvisvgm on a small document, in a temporary
  //! directory (no shell escape, with a time limit), for about half a second. The results are therefore cached: in
  //! memory, and on disk (cacheDirectory()), so that a text is typeset only once, ever. prepare() typesets many texts
  //! in a single run (e.g. all the labels of a canvas, see prepare(const Drawable2D&,float,float)).
  //!
  //! Without LaTeX (available() false), or when disabled, image() returns no image and the painters draw the text
  //! with their fonts (plainText()); a text that LaTeX cannot typeset gives an image with its error.
  //!
  //! The tools are looked for in $CAP_LATEX_BIN, in the PATH, then in the usual TeX installations
  //! (/Library/TeX/texbin, /usr/local/texlive/<year>/bin/<platform>, /usr/bin). The disk cache is
  //! $CAP_LATEX_CACHE, otherwise ~/.cache/CAP/latex.
  //!
  //! @code
  //!   auto & latex = CAP::GRAPHICS::LatexRenderer::instance();
  //!   const CAP::GRAPHICS::LatexImage * image = latex.image("$\\Delta\\rho_{2}$");
  //!   if (image && image->ok) ... image->svg, image->width ...
  //! @endcode
  //!
  class LatexRenderer
  {
  public:

  //! @brief How a text is drawn.
  enum TextKind { kPlain, kLatex, kRoot };

  //! @brief The renderer shared by all the painters. @return renderer
  static LatexRenderer & instance();

  //! @brief Renderer with the tools found on the system and the default cache directory.
  LatexRenderer();
  //! @brief Destructor.
  virtual ~LatexRenderer() { }

  //! @brief How a text is drawn: kLatex if it contains '$' or '\', kRoot if it contains '#' (ROOT syntax, drawn as
  //! plain text for now), kPlain otherwise. @param text text @return kind
  static TextKind kind(const String & text);
  //! @brief Text drawn with a font when LaTeX is not used: without its '$'. @param text text @return text
  static String plainText(const String & text);

  //! @name Configuration
  //! @{

  //! @brief Whether latex and dvisvgm were found. @return true if found
  bool available() const { return !_latex.empty() && !_dvisvgm.empty(); }
  //! @brief Whether LaTeX is used (and available). @return true if used
  bool enabled() const { return _enabled && available(); }
  //! @brief Use LaTeX or not. @param enabled use it
  void setEnabled(bool enabled) { _enabled = enabled; }
  //! @brief Directory of latex and dvisvgm. @return directory, or "" if not found
  String binDirectory() const { return _binDirectory; }
  //! @brief Use latex and dvisvgm from this directory. @param directory directory @return true if both are there
  bool setBinDirectory(const String & directory);
  //! @brief Preamble of the documents (packages, macros). @return preamble
  const String & preamble() const { return _preamble; }
  //! @brief Set the preamble of the documents (the cached images belong to a preamble). @param preamble LaTeX code
  void setPreamble(const String & preamble);
  //! @brief Directory of the disk cache. @return directory, or "" for no disk cache
  const String & cacheDirectory() const { return _cacheDirectory; }
  //! @brief Set the directory of the disk cache (created when needed). @param directory directory, or "" for none
  void setCacheDirectory(const String & directory) { _cacheDirectory = directory; }
  //! @brief Time limit of a LaTeX run. @param seconds limit
  void setTimeLimit(double seconds) { _timeLimit = seconds; }

  //! @}

  //! @name Typesetting
  //! @{

  //!
  //! @brief The image of a text, typeset now if it is not cached.
  //! @param text text
  //! @return image (with ok false and the error if LaTeX failed), or nullptr if the text is not typeset by LaTeX
  //! (kind() not kLatex, or LaTeX not enabled). The image stays valid until clearMemoryCache().
  //!
  const LatexImage * image(const String & text);
  //! @brief The image of a text if it is cached (in memory or on disk), without typesetting it. @param text text
  //! @return image, or nullptr
  const LatexImage * cached(const String & text);
  //! @brief Typeset in one LaTeX run the texts that are not cached yet (texts not typeset by LaTeX are ignored).
  //! @param texts texts
  void prepare(const std::vector<String> & texts);
  //!
  //! @brief Typeset in one run all the texts a drawable (e.g. a canvas) draws on a painter of the given size.
  //! @param drawable drawable @param width width of the painter @param height height of the painter
  //!
  void prepare(const Drawable2D & drawable, float width, float height);
  //! @brief Forget the images held in memory (the disk cache is kept).
  void clearMemoryCache();
  //! @brief Number of LaTeX runs made so far. @return number
  int nRuns() const { return _nRuns; }

  //! @}

  //! @name Pieces (public for the tests)
  //! @{

  //! @brief The LaTeX document typesetting texts, one page each; text k is alone on line firstLine()+3k (its line
  //! breaks replaced by spaces). @param preamble preamble @param texts texts @return document
  static std::string document(const String & preamble, const std::vector<String> & texts);
  //! @brief Line of the document holding the first text. @param preamble preamble @return line number (from 1)
  static int firstLine(const String & preamble);
  //! @brief Errors found without running LaTeX: unbalanced braces or '$'. @param text text @return error, or ""
  static String check(const String & text);
  //! @brief Read an SVG file written by dvisvgm (--no-fonts=1 --exact-bbox). @param svg contents @param image image
  //! @return true if it could be read
  static bool parseSvg(const std::string & svg, LatexImage & image);
  //! @brief Key of a text in the caches (64-bit FNV-1a of the preamble and the text). @param preamble preamble
  //! @param text text @return key
  static std::uint64_t key(const String & preamble, const String & text);

  //! @}

  protected:

  //! @brief Typeset texts (none cached) in one run, and cache the images. @param texts texts
  void typeset(const std::vector<String> & texts);
  //! @brief Run a program and wait for it, killing it after the time limit. @param arguments program and arguments
  //! @return true if it ran and exited with status 0
  bool run(const std::vector<std::string> & arguments) const;
  //! @brief Path of the cached SVG of a key. @param key key @return path, or "" without disk cache
  String cachePath(std::uint64_t key) const;
  //! @brief Look for latex and dvisvgm.
  void findTools();

  bool        _enabled;          //!< use LaTeX
  String      _binDirectory;     //!< directory of the tools
  std::string _latex;            //!< path of latex, or empty
  std::string _dvisvgm;          //!< path of dvisvgm, or empty
  String      _preamble;         //!< preamble of the documents
  String      _cacheDirectory;   //!< directory of the disk cache, or empty
  double      _timeLimit;        //!< time limit of a run (seconds)
  int         _nRuns;            //!< LaTeX runs made so far
  std::unordered_map<std::uint64_t,LatexImage> _images;   //!< images in memory, by key
  std::recursive_mutex _mutex;   //!< guards the images (the renderer may be used from several threads)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__LatexRenderer

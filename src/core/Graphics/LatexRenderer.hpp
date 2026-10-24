/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <string>
#include <unordered_map>
#include <vector>
#include "String.hpp"

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
  //! "$p_{\mathrm{T}}$ (GeV/$c$)". A text in the syntax of ROOT's TLatex (with '#', "_{" or "^{", e.g.
  //! "#Delta#varphi", "p_{T} (GeV/c)") is translated to LaTeX first (fromRoot()). Other texts are drawn by the painters
  //! with their fonts (kind()).
  //!
  //! TeX is a program, not a library: each typesetting runs latex then dvisvgm on a small document, in a temporary
  //! directory (no shell escape, with a time limit), for about half a second. The results are therefore cached: in
  //! memory, and on disk (cacheDirectory()), so that a text is typeset only once, ever. prepare() typesets many texts
  //! in a single run (e.g. all the labels of a canvas, see prepare(const Drawable2D&,float,float)).
  //!
  //! A graphical interface must not wait for LaTeX: imageLater() returns the image if it is cached, otherwise queues
  //! the text and returns nullptr (draw plainText() meanwhile). A background thread typesets the queued texts in one
  //! run, then calls the ready callback (e.g. to wake up the event loop of the interface).
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
  //! @brief Destructor: stops the background thread (and a LaTeX run in progress).
  virtual ~LatexRenderer();

  //! @brief How a text is drawn: kLatex if it contains '$' or '\', kRoot (ROOT syntax) if it contains '#', "_{" or
  //! "^{", kPlain otherwise. @param text text @return kind
  static TextKind kind(const String & text);
  //!
  //! @brief LaTeX (text mode) of a text in the syntax of ROOT's TLatex. As in ROOT, letters are upright unless in
  //! \#it{}, and scripts are text: "p_{T,1}" gives "p$_{\text{T,1}}$", "#Delta#varphi" gives "$\Delta\varphi$".
  //! Translated: _{} and ^{} (also on a single character), the Greek letters, the usual symbols and arrows (\#pm,
  //! \#times, \#leq, \#infty, \#rightarrow, \#int, \#sum, ...), \#frac{}{}, \#sqrt{} and \#sqrt[]{}, the accents (\#bar, \#hat,
  //! \#tilde, \#vec, \#dot, \#ddot, \#check, \#acute, \#grave), \#it{}, \#bf{}, \#splitline{}{}, \#left and \#right, \#{ and \#};
  //! \#color[]{}, \#font[]{}, \#kern[]{}, \#lower[]{} and \#scale[]{} give their text only. An unknown command is shown
  //! as written.
  //! @param text text in ROOT syntax @return LaTeX
  //!
  static String fromRoot(const String & text);
  //! @brief LaTeX typeset for a text: the text itself (kLatex), its translation (kRoot, if ROOT syntax is
  //! translated), or "" if it is drawn with a font. @param text text @return LaTeX, or ""
  String source(const String & text) const;
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
  //! (otherwise the tools are unchanged)
  bool setBinDirectory(const String & directory);
  //! @brief Look for latex and dvisvgm in the usual places (see the class description). @return true if found
  bool findTools();
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
  //! @brief Whether texts in ROOT syntax are translated and typeset by LaTeX. @return true if so
  bool rootSyntax() const { return _rootSyntax; }
  //! @brief Translate texts in ROOT syntax and typeset them by LaTeX, or draw them with a font. @param translate yes or no
  void setRootSyntax(bool translate) { _rootSyntax = translate; }

  //! @}

  //! @name Typesetting
  //! @{

  //!
  //! @brief The image of a text, typeset now if it is not cached.
  //! @param text text
  //! @return image (with ok false and the error if LaTeX failed), or nullptr if the text is not typeset by LaTeX
  //! (source() empty, or LaTeX not enabled). The image stays valid until clearMemoryCache().
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
  //! @brief Forget the images held in memory (the disk cache is kept). The images given before are deleted: only
  //! for programs that hold none (e.g. tests).
  void clearMemoryCache();
  //! @brief Number of LaTeX runs made so far. @return number
  int nRuns() const { return _nRuns; }

  //! @}

  //! @name Typesetting in the background
  //! @{

  //!
  //! @brief The image of a text if it is cached (in memory or on disk); otherwise the text is queued for the
  //! background thread, and the image will be there after the ready callback.
  //! @param text text
  //! @return image (possibly with an error), or nullptr if not ready yet or not typeset by LaTeX
  //!
  const LatexImage * imageLater(const String & text);
  //! @brief Whether texts are queued or being typeset in the background. @return true if so
  bool busy();
  //! @brief Function called by the background thread after it typeset texts (from that thread; keep it short and
  //! thread-safe, e.g. glfwPostEmptyEvent). @param callback function, or nullptr for none
  void setReadyCallback(std::function<void()> callback);
  //! @brief Time the background thread waits after the first queued text, to typeset all the texts of a frame
  //! together. @param seconds delay
  void setBatchDelay(double seconds) { _batchDelay = seconds; }

  //! @}

  //! @name Pieces (public for the tests)
  //! @{

  //! @brief The LaTeX document typesetting texts, one page each (amsmath is always loaded, for the translations of
  //! texts in ROOT syntax); text k is alone on line firstLine()+3k (its line
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
  //! @brief Loop of the background thread: typeset the queued texts, in batches.
  void work();

  bool        _enabled;          //!< use LaTeX
  std::atomic<bool> _rootSyntax; //!< translate the texts in ROOT syntax
  String      _binDirectory;     //!< directory of the tools
  std::string _latex;            //!< path of latex, or empty
  std::string _dvisvgm;          //!< path of dvisvgm, or empty
  String      _preamble;         //!< preamble of the documents
  String      _cacheDirectory;   //!< directory of the disk cache, or empty
  double      _timeLimit;        //!< time limit of a run (seconds)
  std::atomic<int> _nRuns;       //!< LaTeX runs made so far
  std::unordered_map<std::uint64_t,LatexImage> _images;   //!< images in memory, by key (never replaced once there)
  std::recursive_mutex _mutex;   //!< guards the images (the renderer may be used from several threads)

  std::mutex                 _queueMutex;       //!< guards the queue, the queued keys and the callback
  std::condition_variable    _queueCondition;   //!< wakes up the background thread
  std::vector<std::pair<std::uint64_t,String>> _queue;   //!< texts waiting for the background thread, with their keys
  std::unordered_set<std::uint64_t> _queued;    //!< keys of the texts queued or being typeset
  std::function<void()>      _ready;            //!< called after the background thread typeset texts
  std::thread                _worker;           //!< background thread (started by the first queued text)
  std::atomic<bool>          _stopping;         //!< the renderer is being destroyed
  double                     _batchDelay;       //!< wait after the first queued text (seconds)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__LatexRenderer

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
//! @file LatexRenderer.cpp
//! @brief Implementation of LatexRenderer.
#include "LatexRenderer.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <map>
#include <signal.h>
#include <spawn.h>
#include <sstream>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include "Drawable2D.hpp"
#include "FileSystem.hpp"
#include "Painter.hpp"
#include "TextAttributes.hpp"

extern char ** environ;   //!< environment of the process (passed to latex and dvisvgm)

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  //! Painter that draws nothing and records the texts (for prepare(const Drawable2D&,float,float)).
  class TextCollector : public Painter
  {
  public:
  TextCollector(float width, float height) : _width(width), _height(height) { }
  float width()  const override { return _width;  }
  float height() const override { return _height; }
  void drawLine(const std::vector<Point2D> &, const LineAttributes &) override { }
  void drawPolygon(const std::vector<Point2D> &, const FillAttributes &, const LineAttributes &) override { }
  void drawMarker(const Point2D &, const MarkerAttributes &) override { }
  void drawText(const Point2D &, const String & text, const TextAttributes &, float) override { texts.push_back(text); }
  float textWidth(const String & text, const TextAttributes & attributes) const override { return plainTextWidth(text, attributes); }
  std::vector<String> texts;   // texts drawn
  private:
  float _width, _height;
  };

  // name of the SVG file of a page written by dvisvgm (-o p%6p.svg)
  String pageName(int page)
  {
  char name[32];
  std::snprintf(name, sizeof(name), "p%06d.svg", page);
  return String(name);
  }

  bool readFile(const String & path, std::string & contents)
  {
  std::ifstream in(path.data(), std::ios::binary);
  if (!in) return false;
  std::ostringstream buffer;
  buffer << in.rdbuf();
  contents = buffer.str();
  return true;
  }

  // value of the attribute name='...' (or "...") of the first element tag, from start
  bool attribute(const std::string & svg, const char * name, std::string & value)
  {
  std::string key = std::string(" ") + name + "=";
  std::size_t at = svg.find(key);
  if (at == std::string::npos) return false;
  at += key.size();
  if (at >= svg.size()) return false;
  char quote = svg[at];
  std::size_t end = svg.find(quote, at+1);
  if (end == std::string::npos) return false;
  value = svg.substr(at+1, end-at-1);
  return true;
  }

  // ---------------------------------------------------------------- ROOT syntax

  // Commands of ROOT's TLatex typeset in math mode, without argument: Greek letters, symbols, arrows
  const std::map<std::string,std::string> & rootSymbols()
  {
  static const std::map<std::string,std::string> symbols = []
    {
    std::map<std::string,std::string> m;
    for (const char * g : {"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa", "lambda",
                           "mu", "nu", "xi", "pi", "rho", "sigma", "tau", "upsilon", "phi", "chi", "psi", "omega",
                           "varepsilon", "vartheta", "varphi", "varsigma",
                           "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", "Phi", "Psi", "Omega",
                           "pm", "mp", "times", "div", "cdot", "bullet", "circ", "leq", "geq", "neq", "approx", "equiv",
                           "propto", "sim", "simeq", "cong", "ll", "gg", "infty", "partial", "nabla", "in", "notin",
                           "subset", "supset", "subseteq", "supseteq", "cap", "cup", "wedge", "vee", "forall", "exists",
                           "hbar", "ell", "Re", "Im", "aleph", "wp", "perp", "parallel", "angle", "prime", "surd",
                           "oplus", "otimes", "oslash", "ominus", "odot", "dagger", "ddagger",
                           "leftarrow", "rightarrow", "Leftarrow", "Rightarrow", "leftrightarrow", "Leftrightarrow",
                           "uparrow", "downarrow", "Uparrow", "Downarrow", "sum", "int", "prod", "oint", "ldots", "cdots",
                           "clubsuit", "diamondsuit", "heartsuit", "spadesuit"})
      m[g] = std::string("\\") + g;
    const char * capitals[][2] = {{"Alpha", "A"}, {"Beta", "B"}, {"Epsilon", "E"}, {"Zeta", "Z"}, {"Eta", "H"}, {"Iota", "I"},
                                  {"Kappa", "K"}, {"Mu", "M"}, {"Nu", "N"}, {"Omicron", "O"}, {"Rho", "P"}, {"Tau", "T"},
                                  {"Chi", "X"}};
    for (const auto & c : capitals) m[c[0]] = std::string("\\mathrm{") + c[1] + "}";
    m["omicron"] = "o";            m["varomega"] = "\\varpi";   m["varUpsilon"] = "\\Upsilon";
    m["upoint"]  = "\\cdot";       m["3dots"]    = "\\ldots";   m["lt"] = "<";   m["gt"] = ">";
    return m;
    }();
  return symbols;
  }

  //!
  //! Translation of a text in ROOT syntax to LaTeX in text mode (see LatexRenderer::fromRoot()). Consecutive math
  //! pieces are merged into one $...$ (two adjacent $ would start display math).
  //!
  class RootTranslator
  {
  public:
  explicit RootTranslator(const std::string & text) : _s(text), _i(0) { }
  std::string translate() { return text(false); }

  private:
  struct Output
  {
    std::string s;
    bool math = false;   // s ends with a math piece
    void add(const std::string & t) { s += t; if (!t.empty()) math = false; }
    void addMath(const std::string & m) { if (math) s.pop_back(); else s += '$'; s += m + '$'; math = true; }
  };

  const std::string & _s;
  std::size_t _i;

  bool more() const { return _i < _s.size(); }

  // text up to the end, or up to the '}' closing a group (consumed) if nested
  std::string text(bool nested)
  {
  Output out;
  while (more()) if (!token(out, nested)) break;
  return out.s;
  }

  // argument of a command or a script: a group {...}, or a single character or command
  std::string argument()
  {
  if (!more()) return "";
  if (_s[_i] == '{') { _i++; return text(true); }
  Output out;
  token(out, false);
  return out.s;
  }

  // text in a math argument
  static std::string asText(const std::string & t) { return "\\text{" + t + "}"; }

  // [...] after a command (e.g. #color[2]): its contents, or "" if there is none
  std::string bracket()
  {
  if (!more() || _s[_i] != '[') return "";
  std::size_t end = _s.find(']', _i);
  if (end == std::string::npos) end = _s.size();
  std::string contents = _s.substr(_i+1, end-_i-1);
  _i = end < _s.size() ? end+1 : end;
  return contents;
  }

  // one piece of text; false at the '}' closing a nested group
  bool token(Output & out, bool nested)
  {
  char c = _s[_i];
  if (c == '}')
    {
    _i++;
    if (nested) return false;
    out.add("\\}");                                   // unmatched
    return true;
    }
  if (c == '{') { _i++; out.add("{" + text(true) + "}"); return true; }
  if (c == '_' || c == '^')
    {
    _i++;
    std::string m = std::string(1, c) + "{" + asText(argument()) + "}";
    if (more() && (_s[_i] == '_' || _s[_i] == '^') && _s[_i] != c)   // both scripts: stacked
      {
      char d = _s[_i++];
      m += std::string(1, d) + "{" + asText(argument()) + "}";
      }
    out.addMath(m);
    return true;
    }
  if (c == '#') { _i++; command(out); return true; }
  _i++;
  switch (c)
    {
    case '%':  out.add("\\%"); break;
    case '&':  out.add("\\&"); break;
    case '$':  out.add("\\$"); break;
    case '~':  out.add("\\textasciitilde{}"); break;
    case '\\': out.add("\\textbackslash{}"); break;
    case '<': case '>': case '|': out.addMath(std::string(1, c)); break;   // not in the text fonts of TeX
    default:   out.add(std::string(1, c));
    }
  return true;
  }

  void command(Output & out)
  {
  if (!more()) { out.add("\\#"); return; }
  char c = _s[_i];
  if (c == '{' || c == '}' || c == '#') { _i++; out.add(std::string("\\") + c); return; }
  if (c == '[' || c == ']')             { _i++; out.add(std::string(1, c)); return; }
  if (c == '|' && _i+1 < _s.size() && _s[_i+1] == '|') { _i += 2; out.addMath("\\|"); return; }
  std::string name;
  while (more() && std::isalnum((unsigned char)_s[_i])) name += _s[_i++];
  if (name.empty()) { out.add("\\#"); return; }
  const auto & symbols = rootSymbols();
  auto symbol = symbols.find(name);
  if (symbol != symbols.end()) { out.addMath(symbol->second); return; }
  if (name == "frac")
    {
    std::string a = argument(), b = argument();
    out.addMath("\\frac{" + asText(a) + "}{" + asText(b) + "}");
    }
  else if (name == "sqrt")
    {
    std::string n = bracket();
    std::string a = argument();
    out.addMath("\\sqrt" + (n.empty() ? std::string() : "[" + asText(RootTranslator(n).translate()) + "]") + "{" + asText(a) + "}");
    }
  else if (name == "bar" || name == "hat" || name == "tilde" || name == "vec" || name == "dot" || name == "ddot"
           || name == "check" || name == "acute" || name == "grave")
    out.addMath("\\" + name + "{" + asText(argument()) + "}");
  else if (name == "it")        out.add("\\textit{" + argument() + "}");
  else if (name == "bf")        out.add("\\textbf{" + argument() + "}");
  else if (name == "splitline")
    {
    std::string a = argument(), b = argument();
    out.add("\\shortstack[l]{" + a + "\\\\" + b + "}");
    }
  else if (name == "color" || name == "font" || name == "kern" || name == "lower" || name == "scale")
    {
    bracket();
    out.add("{" + argument() + "}");
    }
  else if (name == "left" || name == "right") { }     // the delimiter that follows is drawn as is
  else if (name == "AA")          out.add("\\AA{}");
  else if (name == "aa")          out.add("\\aa{}");
  else if (name == "ocopyright")  out.add("\\copyright{}");
  else if (name == "oright")      out.add("\\textregistered{}");
  else if (name == "trademark")   out.add("\\texttrademark{}");
  else if (name == "doublequote") out.add("\"");
  else                            out.add("\\#" + name);   // unknown: shown as written
  }
  };
  } // namespace

  LatexRenderer & LatexRenderer::instance()
  {
  static LatexRenderer renderer;
  return renderer;
  }

  LatexRenderer::LatexRenderer()
  :
  _enabled(true),
  _rootSyntax(true),
  _binDirectory(),
  _latex(),
  _dvisvgm(),
  _preamble("\\usepackage{amsmath,amssymb}"),
  _cacheDirectory(),
  _timeLimit(10.0),
  _nRuns(0),
  _images(),
  _mutex(),
  _queueMutex(),
  _queueCondition(),
  _queue(),
  _queued(),
  _ready(),
  _worker(),
  _stopping(false),
  _batchDelay(0.01)
  {
  findTools();
  const char * cache = std::getenv("CAP_LATEX_CACHE");
  if (cache)                         _cacheDirectory = FileSystem::expand(cache);
  else if (!FileSystem::home().isNull()) _cacheDirectory = FileSystem::join(FileSystem::home(), ".cache/CAP/latex");
  }

  LatexRenderer::~LatexRenderer()
  {
  {
  std::lock_guard<std::mutex> lock(_queueMutex);
  _stopping = true;
  _ready = nullptr;
  }
  _queueCondition.notify_all();
  if (_worker.joinable()) _worker.join();
  }

  // ---------------------------------------------------------------- texts

  LatexRenderer::TextKind LatexRenderer::kind(const String & text)
  {
  if (text.contains('$') || text.contains('\\')) return kLatex;
  if (text.contains('#') || text.contains("_{") || text.contains("^{")) return kRoot;
  return kPlain;
  }

  String LatexRenderer::fromRoot(const String & text)
  {
  return String(RootTranslator(text.str()).translate());
  }

  String LatexRenderer::source(const String & text) const
  {
  switch (kind(text))
    {
    case kLatex: return text;
    case kRoot:  return _rootSyntax ? fromRoot(text) : String();
    default:     return String();
    }
  }

  String LatexRenderer::plainText(const String & text)
  {
  if (!text.contains('$')) return text;
  std::string s;
  for (const char * c = text.data(); *c; c++) if (*c != '$') s += *c;
  return String(s);
  }

  String LatexRenderer::check(const String & text)
  {
  int depth = 0, dollars = 0;
  const char * s = text.data();
  for (int k=0; s[k]; k++)
    {
    if (s[k] == '\\')       { if (s[k+1]) k++; continue; }   // \{, \}, \$, \\ and commands
    if (s[k] == '%')        break;                          // comment: the rest of the line is ignored
    if (s[k] == '{')        depth++;
    else if (s[k] == '}') { if (--depth < 0) return "Unbalanced braces: '}' without '{'"; }
    else if (s[k] == '$')   dollars++;
    }
  if (depth > 0)     return "Unbalanced braces: '{' without '}'";
  if (dollars%2 != 0) return "Unbalanced '$': math mode is not closed";
  return "";
  }

  std::string LatexRenderer::document(const String & preamble, const std::vector<String> & texts)
  {
  std::string d = "\\documentclass[10pt]{article}\n\\usepackage{amsmath}\n";
  d += preamble.str();
  d += "\n\\usepackage[active,tightpage]{preview}\n\\begin{document}\n";
  for (const auto & text : texts)
    {
    std::string line = text.str();
    for (auto & c : line) if (c == '\n' || c == '\r') c = ' ';
    d += "\\begin{preview}\n" + line + "\n\\end{preview}\n";
    }
  d += "\\end{document}\n";
  return d;
  }

  int LatexRenderer::firstLine(const String & preamble)
  {
  return 7 + preamble.countChar('\n');   // documentclass, amsmath, preamble, preview, document, begin{preview}
  }

  std::uint64_t LatexRenderer::key(const String & preamble, const String & text)
  {
  std::uint64_t h = 14695981039346656037ULL;
  auto add = [&h](const char * s, std::size_t n) { for (std::size_t k=0; k<n; k++) { h ^= std::uint8_t(s[k]); h *= 1099511628211ULL; } };
  add("CAP-latex-1", 12);                 // version of the format of the images (with its terminating 0)
  add(preamble.data(), std::size_t(preamble.length())+1);
  add(text.data(), std::size_t(text.length()));
  return h;
  }

  bool LatexRenderer::parseSvg(const std::string & svg, LatexImage & image)
  {
  std::size_t start = svg.find("<svg");
  if (start == std::string::npos) return false;
  std::size_t open = svg.find('>', start);
  std::size_t close = svg.rfind("</svg>");
  if (open == std::string::npos || close == std::string::npos || close < open) return false;
  std::string viewBox;
  if (!attribute(svg.substr(start, open-start), "viewBox", viewBox)) return false;
  float x, y, w, h;
  if (std::sscanf(viewBox.c_str(), "%f %f %f %f", &x, &y, &w, &h) != 4) return false;
  image.x = x;
  image.width = w;
  image.height = -y;
  image.depth = y + h;
  // the elements of the page, without their ids (several images go into one document)
  std::string body = svg.substr(open+1, close-open-1);
  std::string clean;
  clean.reserve(body.size());
  for (std::size_t k=0; k<body.size(); )
    {
    if (body.compare(k, 5, " id='") == 0 || body.compare(k, 5, " id=\"") == 0)
      {
      std::size_t end = body.find(body[k+4], k+5);
      if (end != std::string::npos) { k = end+1; continue; }
      }
    clean += body[k++];
    }
  image.svg = clean;
  image.ok = true;
  image.error = "";
  return true;
  }

  // ---------------------------------------------------------------- configuration

  bool LatexRenderer::findTools()
  {
  std::vector<String> candidates;
  const char * bin = std::getenv("CAP_LATEX_BIN");
  if (bin) candidates.push_back(FileSystem::expand(bin));
  const char * path = std::getenv("PATH");
  if (path)
    {
    std::stringstream directories(path);
    std::string directory;
    while (std::getline(directories, directory, ':')) if (!directory.empty()) candidates.push_back(directory);
    }
  candidates.push_back("/Library/TeX/texbin");
  std::vector<String> texlive = FileSystem::glob("/usr/local/texlive/*/bin/*");
  for (auto k=texlive.rbegin(); k!=texlive.rend(); ++k) candidates.push_back(*k);   // newest first
  candidates.push_back("/usr/bin");
  for (const auto & directory : candidates) if (setBinDirectory(directory)) return true;
  return false;
  }

  bool LatexRenderer::setBinDirectory(const String & directory)
  {
  String latex = FileSystem::join(directory, "latex"), dvisvgm = FileSystem::join(directory, "dvisvgm");
  if (!FileSystem::isFile(latex) || !FileSystem::isExecutable(latex) || !FileSystem::isFile(dvisvgm) || !FileSystem::isExecutable(dvisvgm))
    return false;
  _binDirectory = directory;
  _latex = latex.str();
  _dvisvgm = dvisvgm.str();
  return true;
  }

  void LatexRenderer::setPreamble(const String & preamble)
  {
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  _preamble = preamble;
  }

  String LatexRenderer::cachePath(std::uint64_t key) const
  {
  if (_cacheDirectory.isNull()) return "";
  char name[32];
  std::snprintf(name, sizeof(name), "%016llx.svg", (unsigned long long)key);
  return FileSystem::join(_cacheDirectory, name);
  }

  // ---------------------------------------------------------------- images

  const LatexImage * LatexRenderer::cached(const String & text)
  {
  String latex = source(text);
  if (latex.isNull() || !enabled()) return nullptr;
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  std::uint64_t k = key(_preamble, latex);
  auto found = _images.find(k);
  if (found != _images.end()) return &found->second;
  String path = cachePath(k);
  std::string svg;
  LatexImage image;
  if (path.isNull() || !readFile(path, svg) || !parseSvg(svg, image)) return nullptr;
  return &_images.emplace(k, image).first->second;
  }

  const LatexImage * LatexRenderer::image(const String & text)
  {
  String latex = source(text);
  if (latex.isNull() || !enabled()) return nullptr;
  const LatexImage * image = cached(latex);
  if (image) return image;
  typeset({latex});
  return cached(latex);
  }

  void LatexRenderer::prepare(const std::vector<String> & texts)
  {
  if (!enabled()) return;
  std::vector<String> missing;
  for (const auto & text : texts)
    {
    String latex = source(text);
    if (latex.isNull() || cached(latex)) continue;
    bool seen = false;
    for (const auto & m : missing) if (m.equalTo(latex)) { seen = true; break; }
    if (!seen) missing.push_back(latex);
    }
  if (!missing.empty()) typeset(missing);
  }

  void LatexRenderer::prepare(const Drawable2D & drawable, float width, float height)
  {
  if (!enabled()) return;
  TextCollector collector(width, height);
  drawable.draw(collector);
  prepare(collector.texts);
  }

  void LatexRenderer::clearMemoryCache()
  {
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  _images.clear();
  }

  bool LatexRenderer::run(const std::vector<std::string> & arguments) const
  {
  std::vector<char *> argv;
  for (const auto & a : arguments) argv.push_back(const_cast<char *>(a.c_str()));
  argv.push_back(nullptr);
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_addopen(&actions, 0, "/dev/null", O_RDONLY, 0);
  posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", O_WRONLY, 0);
  posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0);
  pid_t pid;
  int status = posix_spawn(&pid, argv[0], &actions, nullptr, argv.data(), environ);
  posix_spawn_file_actions_destroy(&actions);
  if (status != 0) return false;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(_timeLimit);
  while (true)
    {
    pid_t done = waitpid(pid, &status, WNOHANG);
    if (done == pid) return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    if (done < 0) return false;
    if (std::chrono::steady_clock::now() > deadline || _stopping)
      {
      kill(pid, SIGKILL);
      waitpid(pid, &status, 0);
      return false;
      }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  void LatexRenderer::typeset(const std::vector<String> & texts)
  {
  if (_stopping) return;
  String preamble;
  {
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  preamble = _preamble;
  }
  std::vector<LatexImage> images(texts.size());
  std::vector<String> valid;          // texts sent to LaTeX
  std::vector<std::size_t> index;     // their index in texts
  for (std::size_t k=0; k<texts.size(); k++)
    {
    String error = check(texts[k]);
    if (error.isNull()) { valid.push_back(texts[k]); index.push_back(k); }
    else images[k].error = error;
    }

  if (!valid.empty())
    {
    char pattern[] = "/tmp/CapLatex.XXXXXX";
    const char * tmp = std::getenv("TMPDIR");
    std::string templ = std::string(tmp && *tmp ? tmp : "/tmp") + "/CapLatex.XXXXXX";
    std::vector<char> buffer(templ.begin(), templ.end());
    buffer.push_back(0);
    char * directory = mkdtemp(buffer.data());
    if (!directory) directory = mkdtemp(pattern);
    if (!directory)
      {
      for (auto k : index) images[k].error = "Cannot create a temporary directory for LaTeX";
      }
    else
      {
      String dir(directory);
      String tex = FileSystem::join(dir, "labels.tex");
      {
      std::ofstream out(tex.data());
      out << document(preamble, valid);
      }
      run({_latex, "-interaction=nonstopmode", "-no-shell-escape", "-output-directory=" + dir.str(), tex.str()});
      _nRuns++;
      // errors of the log: "! message" then "l.<line> ..." (the line gives the text)
      std::vector<String> errors(valid.size());
      std::string log;
      bool otherError = false, preambleError = false;
      String firstMessage;
      if (readFile(FileSystem::join(dir, "labels.log"), log))
        {
        std::istringstream lines(log);
        std::string line, message;
        int first = firstLine(preamble);
        while (std::getline(lines, line))
          {
          if (line.compare(0, 2, "! ") == 0)
            {
            if (!message.empty()) otherError = true;
            message = line.substr(2);
            if (firstMessage.isNull()) firstMessage = message;
            continue;
            }
          int number;
          if (!message.empty() && line.compare(0, 2, "l.") == 0 && std::sscanf(line.c_str()+2, "%d", &number) == 1)
            {
            int k = (number - first)/3;
            if (k >= 0 && k < int(valid.size()) && (number - first)%3 == 0) { if (errors[k].isNull()) errors[k] = message; }
            else if (number < first) preambleError = true;
            else otherError = true;
            message.clear();
            }
          }
        if (!message.empty()) otherError = true;
        }
      String dvi = FileSystem::join(dir, "labels.dvi");
      int nPages = 0;
      if (FileSystem::isFile(dvi))
        {
        // pages named with 6 digits: with %p, dvisvgm pads the numbers to the number of digits of the page count
        run({_dvisvgm, "--no-fonts=1", "--exact-bbox", "--page=1-", "-o", FileSystem::join(dir, "p%6p.svg").str(), dvi.str()});
        while (FileSystem::isFile(FileSystem::join(dir, pageName(nPages+1)))) nPages++;
        }
      if ((nPages != int(valid.size()) || otherError) && valid.size() > 1 && !preambleError)
        {
        // the pages cannot be matched to the texts: typeset them one by one
        try { FileSystem::removeTree(dir); } catch (...) { }
        for (const auto & text : valid) typeset({text});
        return;
        }
      for (std::size_t j=0; j<valid.size(); j++)
        {
        LatexImage & image = images[index[j]];
        std::string svg;
        if (!errors[j].isNull())              image.error = errors[j];
        else if (preambleError)               image.error = "Error in the preamble: " + firstMessage.str();
        else if (otherError)                  image.error = firstMessage.isNull() ? String("LaTeX error") : firstMessage;
        else if (int(j) >= nPages)            image.error = FileSystem::isFile(dvi) ? "dvisvgm failed" : "LaTeX produced no output";
        else if (!readFile(FileSystem::join(dir, pageName(int(j)+1)), svg) || !parseSvg(svg, image))
          image.error = "Cannot read the SVG written by dvisvgm";
        if (image.ok)
          {
          String path = cachePath(key(preamble, valid[j]));
          if (!path.isNull())
            {
            try
              {
              FileSystem::createDirectory(_cacheDirectory);
              String temporary(path.str() + ".part");
              { std::ofstream out(temporary.data(), std::ios::binary); out << svg; }
              FileSystem::rename(temporary, path);
              }
            catch (...) { }                     // no disk cache: the image stays in memory
            }
          }
        }
      try { FileSystem::removeTree(dir); } catch (...) { }
      }
    }

  std::lock_guard<std::recursive_mutex> lock(_mutex);
  if (!preamble.equalTo(_preamble)) return;   // the preamble changed meanwhile: these images are stale
  for (std::size_t k=0; k<texts.size(); k++) _images.emplace(key(preamble, texts[k]), images[k]);
  }

  // ---------------------------------------------------------------- background

  const LatexImage * LatexRenderer::imageLater(const String & text)
  {
  String latex = source(text);
  if (latex.isNull() || !enabled()) return nullptr;
  std::uint64_t k;
  {
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  k = key(_preamble, latex);
  auto found = _images.find(k);
  if (found != _images.end()) return &found->second;
  }
  {
  std::lock_guard<std::mutex> lock(_queueMutex);
  if (_queued.count(k)) return nullptr;                  // already on its way
  }
  const LatexImage * image = cached(latex);             // on disk
  if (image) return image;
  {
  std::lock_guard<std::mutex> lock(_queueMutex);
  if (_stopping || !_queued.insert(k).second) return nullptr;
  _queue.emplace_back(k, latex);
  if (!_worker.joinable()) _worker = std::thread(&LatexRenderer::work, this);
  }
  _queueCondition.notify_one();
  return nullptr;
  }

  bool LatexRenderer::busy()
  {
  std::lock_guard<std::mutex> lock(_queueMutex);
  return !_queued.empty();
  }

  void LatexRenderer::setReadyCallback(std::function<void()> callback)
  {
  std::lock_guard<std::mutex> lock(_queueMutex);
  _ready = callback;
  }

  void LatexRenderer::work()
  {
  std::unique_lock<std::mutex> lock(_queueMutex);
  while (true)
    {
    _queueCondition.wait(lock, [this] { return _stopping || !_queue.empty(); });
    if (_stopping) return;
    // let the frame that queued the first text queue the others, then typeset them all at once
    _queueCondition.wait_for(lock, std::chrono::duration<double>(_batchDelay), [this] { return bool(_stopping); });
    if (_stopping) return;
    std::vector<std::pair<std::uint64_t,String>> queued;
    queued.swap(_queue);
    lock.unlock();
    String preamble;
    {
    std::lock_guard<std::recursive_mutex> images(_mutex);
    preamble = _preamble;
    }
    std::vector<String> missing;
    for (const auto & entry : queued)
      {
      std::lock_guard<std::recursive_mutex> images(_mutex);
      if (!_images.count(key(preamble, entry.second))) missing.push_back(entry.second);
      }
    if (!missing.empty()) typeset(missing);
    lock.lock();
    for (const auto & entry : queued) _queued.erase(entry.first);   // the keys they were queued with
    if (_ready && !_stopping) _ready();
    }
  }

  } // namespace GRAPHICS
} // namespace CAP

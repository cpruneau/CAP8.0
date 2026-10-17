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
#include <cstring>
#include <fcntl.h>
#include <fstream>
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
  } // namespace

  LatexRenderer & LatexRenderer::instance()
  {
  static LatexRenderer renderer;
  return renderer;
  }

  LatexRenderer::LatexRenderer()
  :
  _enabled(true),
  _binDirectory(),
  _latex(),
  _dvisvgm(),
  _preamble("\\usepackage{amsmath,amssymb}"),
  _cacheDirectory(),
  _timeLimit(10.0),
  _nRuns(0),
  _images(),
  _mutex()
  {
  findTools();
  const char * cache = std::getenv("CAP_LATEX_CACHE");
  if (cache)                         _cacheDirectory = FileSystem::expand(cache);
  else if (!FileSystem::home().isNull()) _cacheDirectory = FileSystem::join(FileSystem::home(), ".cache/CAP/latex");
  }

  // ---------------------------------------------------------------- texts

  LatexRenderer::TextKind LatexRenderer::kind(const String & text)
  {
  if (text.contains('$') || text.contains('\\')) return kLatex;
  if (text.contains('#')) return kRoot;
  return kPlain;
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
  std::string d = "\\documentclass[10pt]{article}\n";
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
  return 6 + preamble.countChar('\n');   // documentclass, preamble, preview, document, begin{preview}
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

  void LatexRenderer::findTools()
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
  for (const auto & directory : candidates) if (setBinDirectory(directory)) return;
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
  if (kind(text) != kLatex || !enabled()) return nullptr;
  std::lock_guard<std::recursive_mutex> lock(_mutex);
  std::uint64_t k = key(_preamble, text);
  auto found = _images.find(k);
  if (found != _images.end()) return &found->second;
  String path = cachePath(k);
  std::string svg;
  LatexImage image;
  if (path.isNull() || !readFile(path, svg) || !parseSvg(svg, image)) return nullptr;
  return &(_images[k] = image);
  }

  const LatexImage * LatexRenderer::image(const String & text)
  {
  if (kind(text) != kLatex || !enabled()) return nullptr;
  const LatexImage * image = cached(text);
  if (image) return image;
  typeset({text});
  return cached(text);
  }

  void LatexRenderer::prepare(const std::vector<String> & texts)
  {
  if (!enabled()) return;
  std::vector<String> missing;
  for (const auto & text : texts)
    {
    if (kind(text) != kLatex || cached(text)) continue;
    bool seen = false;
    for (const auto & m : missing) if (m.equalTo(text)) { seen = true; break; }
    if (!seen) missing.push_back(text);
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
    if (std::chrono::steady_clock::now() > deadline)
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
        run({_dvisvgm, "--no-fonts=1", "--exact-bbox", "--page=1-", "-o", FileSystem::join(dir, "p%p.svg").str(), dvi.str()});
        while (FileSystem::isFile(FileSystem::join(dir, String("p" + std::to_string(nPages+1) + ".svg")))) nPages++;
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
        else if (!readFile(FileSystem::join(dir, String("p" + std::to_string(j+1) + ".svg")), svg) || !parseSvg(svg, image))
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
  for (std::size_t k=0; k<texts.size(); k++) _images[key(preamble, texts[k])] = images[k];
  }

  } // namespace GRAPHICS
} // namespace CAP

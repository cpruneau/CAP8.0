/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <fstream>
#include <map>
#include "PptxPainter.hpp"
#include "Canvas.hpp"
#include "LatexRenderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  const double kPi = 3.14159265358979323846;
  //! English Metric Units of a length in points (12700 per point).
  long long emu(double points) { return (long long)std::llround(points*12700.0); }

  const char * kNsA = "http://schemas.openxmlformats.org/drawingml/2006/main";
  const char * kNsR = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
  const char * kNsP = "http://schemas.openxmlformats.org/presentationml/2006/main";
  const char * kRelBase = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

  std::string namespaces()
  {
  return std::string(" xmlns:a=\"") + kNsA + "\" xmlns:r=\"" + kNsR + "\" xmlns:p=\"" + kNsP + "\"";
  }

  //! Group shape properties of an empty tree (slide master, layout).
  const char * kEmptyTree =
    "<p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
    "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr></p:spTree>";

  std::string theme()
  {
  std::string fill3 =
    "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
    "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
    "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>";
  return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")
    + "<a:theme xmlns:a=\"" + kNsA + "\" name=\"CAP\"><a:themeElements>"
    "<a:clrScheme name=\"CAP\">"
    "<a:dk1><a:srgbClr val=\"000000\"/></a:dk1><a:lt1><a:srgbClr val=\"FFFFFF\"/></a:lt1>"
    "<a:dk2><a:srgbClr val=\"1F2937\"/></a:dk2><a:lt2><a:srgbClr val=\"F3F4F6\"/></a:lt2>"
    "<a:accent1><a:srgbClr val=\"1F77B4\"/></a:accent1><a:accent2><a:srgbClr val=\"D62728\"/></a:accent2>"
    "<a:accent3><a:srgbClr val=\"2CA02C\"/></a:accent3><a:accent4><a:srgbClr val=\"9467BD\"/></a:accent4>"
    "<a:accent5><a:srgbClr val=\"FF7F0E\"/></a:accent5><a:accent6><a:srgbClr val=\"8C564B\"/></a:accent6>"
    "<a:hlink><a:srgbClr val=\"0563C1\"/></a:hlink><a:folHlink><a:srgbClr val=\"954F72\"/></a:folHlink>"
    "</a:clrScheme>"
    "<a:fontScheme name=\"CAP\">"
    "<a:majorFont><a:latin typeface=\"Arial\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:majorFont>"
    "<a:minorFont><a:latin typeface=\"Arial\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:minorFont>"
    "</a:fontScheme>"
    "<a:fmtScheme name=\"CAP\">"
    "<a:fillStyleLst>" + fill3 + "</a:fillStyleLst>"
    "<a:lnStyleLst>"
    "<a:ln w=\"6350\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
    "<a:ln w=\"12700\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
    "<a:ln w=\"19050\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
    "</a:lnStyleLst>"
    "<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle>"
    "<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>"
    "<a:bgFillStyleLst>" + fill3 + "</a:bgFillStyleLst>"
    "</a:fmtScheme></a:themeElements><a:objectDefaults/><a:extraClrSchemeLst/></a:theme>";
  }

  std::string slideMaster()
  {
  return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")
    + "<p:sldMaster" + namespaces() + "><p:cSld><p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>"
    + kEmptyTree + "</p:cSld>"
    "<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" "
    "accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
    "<p:sldLayoutIdLst><p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/></p:sldLayoutIdLst>"
    "<p:txStyles>"
    "<p:titleStyle><a:lvl1pPr><a:defRPr sz=\"3200\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
    "<a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"
    "<p:bodyStyle><a:lvl1pPr><a:defRPr sz=\"1800\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
    "<a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr></p:bodyStyle>"
    "<p:otherStyle><a:lvl1pPr><a:defRPr sz=\"1800\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
    "<a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr></p:otherStyle>"
    "</p:txStyles></p:sldMaster>";
  }

  std::string slideLayout()
  {
  return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")
    + "<p:sldLayout" + namespaces() + " type=\"blank\" preserve=\"1\"><p:cSld name=\"Blank\">" + kEmptyTree
    + "</p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>";
  }

  std::string relationships(const std::vector<std::pair<std::string,std::string>> & targets)
  {
  std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
  for (std::size_t k=0; k<targets.size(); k++)
    xml += "<Relationship Id=\"rId" + std::to_string(k + 1) + "\" Type=\"" + targets[k].first + "\" Target=\"" + targets[k].second + "\"/>";
  return xml + "</Relationships>";
  }

  //! CRC-32 of bytes (ZIP).
  std::uint32_t crc32(const std::string & data)
  {
  static std::uint32_t table[256];
  static bool ready = false;
  if (!ready)
    {
    for (std::uint32_t n=0; n<256; n++)
      {
      std::uint32_t c = n;
      for (int k=0; k<8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      table[n] = c;
      }
    ready = true;
    }
  std::uint32_t c = 0xFFFFFFFFu;
  for (unsigned char b : data) c = table[(c ^ b) & 0xFF] ^ (c >> 8);
  return c ^ 0xFFFFFFFFu;
  }

  //! Unicode of the LaTeX commands of letters and symbols.
  const std::map<std::string,std::string> & symbols()
  {
  static const std::map<std::string,std::string> table = {
    {"alpha","α"},{"beta","β"},{"gamma","γ"},{"delta","δ"},{"epsilon","ϵ"},{"varepsilon","ε"},{"zeta","ζ"},{"eta","η"},
    {"theta","θ"},{"vartheta","ϑ"},{"iota","ι"},{"kappa","κ"},{"lambda","λ"},{"mu","μ"},{"nu","ν"},{"xi","ξ"},{"pi","π"},
    {"varpi","ϖ"},{"rho","ρ"},{"varrho","ϱ"},{"sigma","σ"},{"varsigma","ς"},{"tau","τ"},{"upsilon","υ"},{"phi","ϕ"},
    {"varphi","φ"},{"chi","χ"},{"psi","ψ"},{"omega","ω"},{"Gamma","Γ"},{"Delta","Δ"},{"Theta","Θ"},{"Lambda","Λ"},
    {"Xi","Ξ"},{"Pi","Π"},{"Sigma","Σ"},{"Upsilon","Υ"},{"Phi","Φ"},{"Psi","Ψ"},{"Omega","Ω"},
    {"pm","±"},{"mp","∓"},{"times","×"},{"cdot","·"},{"div","÷"},{"to","→"},{"rightarrow","→"},{"leftarrow","←"},
    {"leftrightarrow","↔"},{"Rightarrow","⇒"},{"Leftarrow","⇐"},{"uparrow","↑"},{"downarrow","↓"},{"leq","≤"},{"le","≤"},
    {"geq","≥"},{"ge","≥"},{"neq","≠"},{"ne","≠"},{"approx","≈"},{"sim","∼"},{"simeq","≃"},{"propto","∝"},{"equiv","≡"},
    {"ll","≪"},{"gg","≫"},{"infty","∞"},{"partial","∂"},{"nabla","∇"},{"sum","∑"},{"prod","∏"},{"int","∫"},{"oint","∮"},
    {"langle","⟨"},{"rangle","⟩"},{"ell","ℓ"},{"hbar","ℏ"},{"circ","°"},{"degree","°"},{"prime","′"},{"perp","⊥"},
    {"parallel","∥"},{"in","∈"},{"notin","∉"},{"subset","⊂"},{"cup","∪"},{"cap","∩"},{"forall","∀"},{"exists","∃"},
    {"ldots","…"},{"cdots","⋯"},{"dots","…"},{"star","⋆"},{"ast","∗"},{"bullet","•"},{"odot","⊙"},{"oplus","⊕"},
    {"otimes","⊗"},{"wedge","∧"},{"vee","∨"},{"neg","¬"},{"Re","ℜ"},{"Im","ℑ"},{"aleph","ℵ"},{"AA","Å"},
    {"lbrace","{"},{"rbrace","}"},{"vert","|"},{"Vert","‖"},{"mid","|"},{"backslash","\\"},{"textbackslash","\\"},
    {"quad","  "},{"qquad","    "},{"colon",":"},{"%","%"},{"&","&"},{"#","#"},{"_","_"},{"{","{"},{"}","}"},{"$","$"}};
  return table;
  }

  //! Combining characters of the LaTeX accents.
  const std::map<std::string,std::string> & accents()
  {
  static const std::map<std::string,std::string> table = {
    {"bar","\xCC\x85"},{"overline","\xCC\x85"},{"hat","\xCC\x82"},{"widehat","\xCC\x82"},{"tilde","\xCC\x83"},
    {"widetilde","\xCC\x83"},{"vec","\xE2\x83\x97"},{"dot","\xCC\x87"},{"ddot","\xCC\x88"},{"check","\xCC\x8C"},
    {"acute","\xCC\x81"},{"grave","\xCC\x80"},{"breve","\xCC\x86"}};
  return table;
  }

  //! Converts LaTeX to runs (see PptxPainter::textRuns()).
  class RunWriter
  {
  public:
  explicit RunWriter(const std::string & text) : _s(text), _i(0) { }
  std::vector<PptxPainter::Run> runs()
  {
  sequence(0, false);
  std::vector<PptxPainter::Run> merged;
  for (const auto & r : _runs)
    {
    if (r.text.empty()) continue;
    if (!merged.empty() && merged.back().script == r.script) merged.back().text += r.text;
    else merged.push_back(r);
    }
  return merged;
  }
  private:
  std::string _s;
  std::size_t _i;
  std::vector<PptxPainter::Run> _runs;
  void put(const std::string & text, int script)
  {
  if (_runs.empty() || _runs.back().script != script) _runs.push_back(PptxPainter::Run{text, script});
  else _runs.back().text += text;
  }
  //! The text up to the end of the group (at the closing brace if inGroup), at the given script position.
  void sequence(int script, bool inGroup)
  {
  while (_i < _s.size())
    {
    char c = _s[_i];
    if (c == '}') { _i++; if (inGroup) return; continue; }
    if (c == '{') { _i++; sequence(script, true); continue; }
    if (c == '$') { _i++; continue; }
    if (c == '~') { _i++; put(" ", script); continue; }
    if (c == '^' || c == '_')
      {
      _i++;
      int inner = c == '^' ? 1 : -1;
      if (script != 0) inner = script;          // no nested scripts: stay where we are
      argument(inner);
      continue;
      }
    if (c == '\\') { command(script); continue; }
    put(std::string(1, c), script);
    _i++;
    }
  }
  //! One argument: a group, a command or a character.
  void argument(int script)
  {
  while (_i < _s.size() && _s[_i] == ' ') _i++;
  if (_i >= _s.size()) return;
  if (_s[_i] == '{') { _i++; sequence(script, true); return; }
  if (_s[_i] == '\\') { command(script); return; }
  // one character (a UTF-8 sequence whole)
  std::size_t n = 1;
  unsigned char b = (unsigned char)_s[_i];
  if (b >= 0xF0) n = 4; else if (b >= 0xE0) n = 3; else if (b >= 0xC0) n = 2;
  put(_s.substr(_i, n), script);
  _i += n;
  }
  //! An argument read as text, without drawing it.
  std::string argumentText(int script)
  {
  std::vector<PptxPainter::Run> saved;
  saved.swap(_runs);
  argument(script);
  std::string text;
  for (const auto & r : _runs) text += r.text;
  _runs.swap(saved);
  return text;
  }
  void command(int script)
  {
  _i++;   // the backslash
  if (_i >= _s.size()) return;
  std::string name;
  if (std::isalpha((unsigned char)_s[_i])) { while (_i < _s.size() && std::isalpha((unsigned char)_s[_i])) name += _s[_i++]; }
  else name = std::string(1, _s[_i++]);
  if (name == "," || name == ";" || name == ":" || name == " ") { put(" ", script); return; }
  if (name == "!" || name == "left" || name == "right" || name == "big" || name == "Big" || name == "bigg" || name == "displaystyle"
      || name == "limits" || name == "nolimits") return;
  if (name == "\\") { put(" ", script); return; }
  if (name == "frac")
    {
    std::string a = argumentText(script), b = argumentText(script);
    put(a + "/" + b, script);
    return;
    }
  if (name == "sqrt")
    {
    if (_i < _s.size() && _s[_i] == '[') { while (_i < _s.size() && _s[_i] != ']') _i++; _i++; }
    std::string a = argumentText(script);
    put("√" + (a.size() > 1 ? "(" + a + ")" : a), script);
    return;
    }
  if (name == "splitline") { std::string a = argumentText(script), b = argumentText(script); put(a + " " + b, script); return; }
  auto accent = accents().find(name);
  if (accent != accents().end()) { put(argumentText(script) + accent->second, script); return; }
  auto symbol = symbols().find(name);
  if (symbol != symbols().end()) { put(symbol->second, script); return; }
  // font and text commands: their argument as text (\text, \mathrm, \it, \bf, \textbf, \mbox, ...)
  while (_i < _s.size() && _s[_i] == ' ') _i++;
  if (_i < _s.size() && _s[_i] == '{') argument(script);
  }
  };

  void put16(std::string & out, unsigned v) { out.push_back(char(v & 0xFF)); out.push_back(char((v >> 8) & 0xFF)); }
  void put32(std::string & out, std::uint32_t v) { for (int k=0; k<4; k++) out.push_back(char((v >> (8*k)) & 0xFF)); }
  } // namespace

  // ------------------------------------------------------------------------------------------------ ZIP

  std::string PptxPainter::zip(const std::vector<std::pair<std::string,std::string>> & files)
  {
  std::string out, central;
  const unsigned dosTime = 0, dosDate = (1 << 5) | 1;   // 1980-01-01
  for (const auto & file : files)
    {
    std::uint32_t offset = std::uint32_t(out.size());
    std::uint32_t crc = crc32(file.second), size = std::uint32_t(file.second.size());
    put32(out, 0x04034b50); put16(out, 20); put16(out, 0); put16(out, 0); put16(out, dosTime); put16(out, dosDate);
    put32(out, crc); put32(out, size); put32(out, size); put16(out, unsigned(file.first.size())); put16(out, 0);
    out += file.first;
    out += file.second;
    put32(central, 0x02014b50); put16(central, 20); put16(central, 20); put16(central, 0); put16(central, 0);
    put16(central, dosTime); put16(central, dosDate); put32(central, crc); put32(central, size); put32(central, size);
    put16(central, unsigned(file.first.size())); put16(central, 0); put16(central, 0); put16(central, 0); put16(central, 0);
    put32(central, 0); put32(central, offset);
    central += file.first;
    }
  std::uint32_t centralOffset = std::uint32_t(out.size());
  out += central;
  put32(out, 0x06054b50); put16(out, 0); put16(out, 0); put16(out, unsigned(files.size())); put16(out, unsigned(files.size()));
  put32(out, std::uint32_t(central.size())); put32(out, centralOffset); put16(out, 0);
  return out;
  }

  std::vector<PptxPainter::Run> PptxPainter::textRuns(const String & text)
  {
  std::string s;
  switch (LatexRenderer::kind(text))
    {
    case LatexRenderer::kLatex: s = text.str(); break;
    case LatexRenderer::kRoot:  s = LatexRenderer::fromRoot(text).str(); break;
    default: return { Run{text.str(), 0} };
    }
  return RunWriter(s).runs();
  }

  // ------------------------------------------------------------------------------------------------ painter

  PptxPainter::PptxPainter(float slideWidth, float slideHeight)
  :
  _slideWidth(slideWidth > 0 ? slideWidth : 960.0f),
  _slideHeight(slideHeight > 0 ? slideHeight : 540.0f),
  _regionX(0), _regionY(0), _regionWidth(_slideWidth), _regionHeight(_slideHeight),
  _open(false),
  _nextId(2),
  _shapes(),
  _slides()
  { }

  void PptxPainter::newSlide(float width, float height)
  {
  closeSlide();
  if (width <= 0 || height <= 0) { width = _slideWidth; height = _slideHeight; }
  float scale = std::min(_slideWidth/width, _slideHeight/height);
  _regionWidth  = width*scale;
  _regionHeight = height*scale;
  _regionX = 0.5f*(_slideWidth - _regionWidth);
  _regionY = 0.5f*(_slideHeight - _regionHeight);
  _shapes.str("");
  _shapes.clear();
  _nextId = 3;   // 1: the tree, 2: the group of the drawing
  _open = true;
  }

  void PptxPainter::addCanvas(const Canvas & canvas)
  {
  newSlide(float(canvas.width()), float(canvas.height()));
  canvas.draw(*this);
  closeSlide();
  }

  void PptxPainter::closeSlide()
  {
  if (!_open) return;
  _slides.push_back(slideXml(_shapes.str()));
  _open = false;
  }

  std::string PptxPainter::escape(const std::string & text)
  {
  std::string out;
  for (char c : text)
    {
    switch (c)
      {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default:  if ((unsigned char)c >= 0x20 || c == '\t') out += c; break;
      }
    }
  return out;
  }

  std::string PptxPainter::solidFill(float r, float g, float b, float alpha)
  {
  char hex[8];
  std::snprintf(hex, sizeof(hex), "%02X%02X%02X", int(std::lround(std::clamp(r, 0.0f, 1.0f)*255)),
                int(std::lround(std::clamp(g, 0.0f, 1.0f)*255)), int(std::lround(std::clamp(b, 0.0f, 1.0f)*255)));
  std::string xml = std::string("<a:solidFill><a:srgbClr val=\"") + hex + "\"";
  if (alpha < 0.999f) xml += "><a:alpha val=\"" + std::to_string(int(std::lround(std::clamp(alpha, 0.0f, 1.0f)*100000))) + "\"/></a:srgbClr>";
  else xml += "/>";
  return xml + "</a:solidFill>";
  }

  std::string PptxPainter::lineXml(int color, float alpha, int style, float width)
  {
  if (style == 0 || width <= 0) return "<a:ln><a:noFill/></a:ln>";
  float r, g, b;
  rgb(color, r, g, b);
  std::string xml = "<a:ln w=\"" + std::to_string(emu(width)) + "\" cap=\"flat\">" + solidFill(r, g, b, alpha);
  const char * dash = style == 2 ? "dash" : (style == 3 ? "sysDot" : (style == 4 ? "dashDot" : nullptr));
  if (dash) xml += std::string("<a:prstDash val=\"") + dash + "\"/>";
  xml += "<a:round/></a:ln>";
  return xml;
  }

  void PptxPainter::freeform(const std::vector<float> & xs, const std::vector<float> & ys, bool close,
                             const std::string & fill, const std::string & line, const char * name)
  {
  if (xs.size() < 2 || !_open) return;
  float x0 = *std::min_element(xs.begin(), xs.end()), x1 = *std::max_element(xs.begin(), xs.end());
  float y0 = *std::min_element(ys.begin(), ys.end()), y1 = *std::max_element(ys.begin(), ys.end());
  long long ox = emu(x0), oy = emu(y0);
  long long w = std::max<long long>(1, emu(x1) - ox), h = std::max<long long>(1, emu(y1) - oy);
  int id = _nextId++;
  _shapes << "<p:sp><p:nvSpPr><p:cNvPr id=\"" << id << "\" name=\"" << name << " " << id << "\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>"
          << "<p:spPr><a:xfrm><a:off x=\"" << ox << "\" y=\"" << oy << "\"/><a:ext cx=\"" << w << "\" cy=\"" << h << "\"/></a:xfrm>"
          << "<a:custGeom><a:avLst/><a:gdLst/><a:ahLst/><a:cxnLst/><a:rect l=\"0\" t=\"0\" r=\"r\" b=\"b\"/><a:pathLst>"
          << "<a:path w=\"" << w << "\" h=\"" << h << "\"" << (fill.empty() ? " fill=\"none\"" : "") << ">";
  for (std::size_t k=0; k<xs.size(); k++)
    {
    long long x = std::clamp<long long>(emu(xs[k]) - ox, 0, w), y = std::clamp<long long>(emu(ys[k]) - oy, 0, h);
    _shapes << (k == 0 ? "<a:moveTo>" : "<a:lnTo>") << "<a:pt x=\"" << x << "\" y=\"" << y << "\"/>" << (k == 0 ? "</a:moveTo>" : "</a:lnTo>");
    }
  if (close) _shapes << "<a:close/>";
  _shapes << "</a:path></a:pathLst></a:custGeom>" << (fill.empty() ? std::string("<a:noFill/>") : fill) << line << "</p:spPr></p:sp>";
  }

  void PptxPainter::preset(const char * prst, float x, float y, float w, float h, const std::string & fill,
                           const std::string & line, bool flipV)
  {
  if (!_open) return;
  int id = _nextId++;
  _shapes << "<p:sp><p:nvSpPr><p:cNvPr id=\"" << id << "\" name=\"Marker " << id << "\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>"
          << "<p:spPr><a:xfrm" << (flipV ? " flipV=\"1\"" : "") << "><a:off x=\"" << emu(x) << "\" y=\"" << emu(y)
          << "\"/><a:ext cx=\"" << std::max<long long>(1, emu(w)) << "\" cy=\"" << std::max<long long>(1, emu(h)) << "\"/></a:xfrm>"
          << "<a:prstGeom prst=\"" << prst << "\"><a:avLst/></a:prstGeom>"
          << (fill.empty() ? std::string("<a:noFill/>") : fill) << line << "</p:spPr></p:sp>";
  }

  void PptxPainter::drawLine(const std::vector<Point2D> & points, const LineAttributes & line)
  {
  if (points.size() < 2 || line.style() == 0 || line.width() <= 0) return;
  std::vector<float> xs, ys;
  for (const auto & p : points) { xs.push_back(px(p)); ys.push_back(py(p)); }
  freeform(xs, ys, false, "", lineXml(line.color(), line.alpha(), line.style(), line.width()), "Line");
  }

  void PptxPainter::drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  if (points.size() < 3) return;
  std::vector<float> xs, ys;
  for (const auto & p : points) { xs.push_back(px(p)); ys.push_back(py(p)); }
  std::string fillXml;
  if (fill.style() != 0)
    {
    float r, g, b;
    rgb(fill.color(), r, g, b);
    fillXml = solidFill(r, g, b, fill.alpha());
    }
  std::string lineX = lineXml(line.color(), line.alpha(), line.style(), line.width());
  if (fillXml.empty() && lineX.find("noFill") != std::string::npos) return;
  freeform(xs, ys, true, fillXml, lineX, "Polygon");
  }

  void PptxPainter::fillPolygon(const std::vector<Point2D> & points, const Color & color)
  {
  if (points.size() < 3) return;
  std::vector<float> xs, ys;
  for (const auto & p : points) { xs.push_back(px(p)); ys.push_back(py(p)); }
  freeform(xs, ys, true, solidFill(color.red, color.green, color.blue, color.alpha), "<a:ln><a:noFill/></a:ln>", "Cell");
  }

  void PptxPainter::fillRectangle(float x0, float y0, float x1, float y1, const Color & color)
  {
  float left = px(Point2D(std::min(x0, x1), 0)), right = px(Point2D(std::max(x0, x1), 0));
  float top = py(Point2D(0, std::max(y0, y1))), bottom = py(Point2D(0, std::min(y0, y1)));
  const float overlap = 0.3f;   // adjacent cells overlap slightly: no seams between them in the viewers
  preset("rect", left - overlap, top - overlap, right - left + 2*overlap, bottom - top + 2*overlap, solidFill(color.red, color.green, color.blue, color.alpha),
         "<a:ln><a:noFill/></a:ln>");
  }

  void PptxPainter::drawMarker(const Point2D & position, const MarkerAttributes & marker)
  {
  float s = 4.0f*marker.size();   // half size: size 1 is 8 points across
  if (s <= 0) return;
  float x = px(position), y = py(position);
  int style = marker.style();
  bool open = (style == 4 || style == 24 || style == 25 || style == 26 || style == 27 || style == 30 || style == 32);
  float r, g, b;
  rgb(marker.color(), r, g, b);
  std::string fill = open ? std::string() : solidFill(r, g, b, marker.alpha());
  std::string line = open ? lineXml(marker.color(), marker.alpha(), 1, 1.0f) : std::string("<a:ln><a:noFill/></a:ln>");
  switch (style)
    {
    case 21: case 25: preset("rect", x - s, y - s, 2*s, 2*s, fill, line); break;
    case 22: case 26: preset("triangle", x - s, y - s, 2*s, 2*s, fill, line); break;
    case 23: case 32: preset("triangle", x - s, y - s, 2*s, 2*s, fill, line, true); break;
    case 33: case 27: preset("diamond", x - s, y - s, 2*s, 2*s, fill, line); break;
    case 29: case 30: preset("star5", x - s, y - s, 2*s, 2*s, fill, line); break;
    case 2: case 3: case 5:
      {
      std::string cross = lineXml(marker.color(), marker.alpha(), 1, 1.0f);
      if (style != 5) { freeform({x - s, x + s}, {y, y}, false, "", cross, "Marker"); freeform({x, x}, {y - s, y + s}, false, "", cross, "Marker"); }
      if (style != 2)
        {
        float d = (style == 3) ? 0.7f*s : s;
        freeform({x - d, x + d}, {y - d, y + d}, false, "", cross, "Marker");
        freeform({x - d, x + d}, {y + d, y - d}, false, "", cross, "Marker");
        }
      break;
      }
    case 1:  preset("ellipse", x - 0.75f, y - 0.75f, 1.5f, 1.5f, fill, line); break;
    case 6:  preset("ellipse", x - 1.0f, y - 1.0f, 2.0f, 2.0f, fill, line); break;
    case 7:  preset("ellipse", x - 1.5f, y - 1.5f, 3.0f, 3.0f, fill, line); break;
    default: preset("ellipse", x - s, y - s, 2*s, 2*s, fill, line); break;
    }
  }

  void PptxPainter::drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  if (text.isNull() || !_open) return;
  float size = attributes.size()*_regionHeight;
  if (size <= 0) return;
  std::vector<Run> runs = textRuns(text);
  std::string plain;
  for (const Run & run : runs) plain += run.text;
  if (plain.empty()) return;
  // the box of the text: its width estimated (PowerPoint measures it again), its height that of a line
  float w = 1.15f*plainTextWidth(String(plain), attributes) + 0.6f*size, h = 1.4f*size;
  float x = px(position), y = py(position);
  TextAttributes::Justification j = attributes.justification();
  float dx = j == TextAttributes::kLeft ? 0.5f*w : (j == TextAttributes::kRight ? -0.5f*w : 0.0f);
  if (j == TextAttributes::kLeft) dx -= 0.3f*size; else if (j == TextAttributes::kRight) dx += 0.3f*size;
  // the box rotates about its centre: place the centre where the anchor puts it (counter-clockwise angle, y down)
  double a = angle*kPi/180.0;
  float cx = x + float(dx*std::cos(a)), cy = y - float(dx*std::sin(a));
  float r, g, b;
  rgb(attributes.color(), r, g, b);
  const int font = attributes.font();
  const char * family = TextAttributes::fontFamily(font) == TextAttributes::kSerif ? "Times New Roman"
                      : TextAttributes::fontFamily(font) == TextAttributes::kMono  ? "Courier New"
                      : TextAttributes::fontFamily(font) == TextAttributes::kSymbol ? "Symbol" : "Arial";
  const char * align = j == TextAttributes::kLeft ? "l" : (j == TextAttributes::kRight ? "r" : "ctr");
  int id = _nextId++;
  long long rot = std::llround(-angle*60000.0);
  while (rot < 0) rot += 21600000;
  rot %= 21600000;
  _shapes << "<p:sp><p:nvSpPr><p:cNvPr id=\"" << id << "\" name=\"Text " << id << "\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr>"
          << "<p:spPr><a:xfrm" << (rot ? " rot=\"" + std::to_string(rot) + "\"" : std::string()) << "><a:off x=\"" << emu(cx - 0.5f*w)
          << "\" y=\"" << emu(cy - 0.5f*h) << "\"/><a:ext cx=\"" << emu(w) << "\" cy=\"" << emu(h) << "\"/></a:xfrm>"
          << "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>"
          << "<p:txBody><a:bodyPr wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"ctr\" rtlCol=\"0\"><a:noAutofit/></a:bodyPr>"
          << "<a:lstStyle/><a:p><a:pPr algn=\"" << align << "\"/>";
  for (const Run & run : runs)
    {
    _shapes << "<a:r><a:rPr lang=\"en-US\" sz=\"" << std::max(100L, std::lround(size*100.0f)) << "\""
            << (TextAttributes::isBold(font) ? " b=\"1\"" : "") << (TextAttributes::isItalic(font) ? " i=\"1\"" : "")
            << (run.script > 0 ? " baseline=\"30000\"" : (run.script < 0 ? " baseline=\"-25000\"" : "")) << " dirty=\"0\">"
            << solidFill(r, g, b, attributes.alpha()) << "<a:latin typeface=\"" << family << "\"/><a:cs typeface=\"" << family << "\"/></a:rPr>"
            << "<a:t>" << escape(run.text) << "</a:t></a:r>";
    }
  _shapes << "</a:p></p:txBody></p:sp>";
  }

  std::string PptxPainter::slideXml(const std::string & shapes) const
  {
  long long gx = emu(_regionX), gy = emu(_regionY), gw = emu(_regionWidth), gh = emu(_regionHeight);
  std::ostringstream xml;
  xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
      << "<p:sld" << namespaces() << "><p:cSld><p:spTree>"
      << "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
      << "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>";
  if (!shapes.empty())
    xml << "<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"2\" name=\"Canvas\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
        << "<p:grpSpPr><a:xfrm><a:off x=\"" << gx << "\" y=\"" << gy << "\"/><a:ext cx=\"" << gw << "\" cy=\"" << gh << "\"/>"
        << "<a:chOff x=\"" << gx << "\" y=\"" << gy << "\"/><a:chExt cx=\"" << gw << "\" cy=\"" << gh << "\"/></a:xfrm></p:grpSpPr>"
        << shapes << "</p:grpSp>";
  xml << "</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>";
  return xml.str();
  }

  std::string PptxPainter::package()
  {
  closeSlide();
  const int n = int(_slides.size());
  const std::string header = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
  std::vector<std::pair<std::string,std::string>> files;

  // [Content_Types].xml
  std::string types = header + "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
    "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
    "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
    "<Override PartName=\"/ppt/presentation.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml\"/>"
    "<Override PartName=\"/ppt/slideMasters/slideMaster1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml\"/>"
    "<Override PartName=\"/ppt/slideLayouts/slideLayout1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>"
    "<Override PartName=\"/ppt/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>"
    "<Override PartName=\"/ppt/presProps.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presProps+xml\"/>"
    "<Override PartName=\"/ppt/viewProps.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.viewProps+xml\"/>"
    "<Override PartName=\"/ppt/tableStyles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml\"/>"
    "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
    "<Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>";
  for (int k=1; k<=n; k++)
    types += "<Override PartName=\"/ppt/slides/slide" + std::to_string(k) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>";
  types += "</Types>";
  files.emplace_back("[Content_Types].xml", types);

  files.emplace_back("_rels/.rels", relationships({
    {std::string(kRelBase) + "officeDocument", "ppt/presentation.xml"},
    {"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties", "docProps/core.xml"},
    {std::string(kRelBase) + "extended-properties", "docProps/app.xml"}}));
  files.emplace_back("docProps/core.xml", header +
    "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" "
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><dc:title>CAP plots</dc:title><dc:creator>CAP</dc:creator>"
    "</cp:coreProperties>");
  files.emplace_back("docProps/app.xml", header +
    "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\">"
    "<Application>CAP</Application><Slides>" + std::to_string(n) + "</Slides></Properties>");

  // presentation
  std::string presentation = header + "<p:presentation" + namespaces() + " saveSubsetFonts=\"1\">"
    "<p:sldMasterIdLst><p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/></p:sldMasterIdLst><p:sldIdLst>";
  for (int k=1; k<=n; k++) presentation += "<p:sldId id=\"" + std::to_string(255 + k) + "\" r:id=\"rId" + std::to_string(k + 5) + "\"/>";
  presentation += "</p:sldIdLst><p:sldSz cx=\"" + std::to_string(emu(_slideWidth)) + "\" cy=\"" + std::to_string(emu(_slideHeight)) + "\"/>"
    "<p:notesSz cx=\"6858000\" cy=\"9144000\"/></p:presentation>";
  files.emplace_back("ppt/presentation.xml", presentation);
  std::vector<std::pair<std::string,std::string>> presentationRels = {
    {std::string(kRelBase) + "slideMaster", "slideMasters/slideMaster1.xml"},
    {std::string(kRelBase) + "theme", "theme/theme1.xml"},
    {std::string(kRelBase) + "presProps", "presProps.xml"},
    {std::string(kRelBase) + "viewProps", "viewProps.xml"},
    {std::string(kRelBase) + "tableStyles", "tableStyles.xml"}};
  for (int k=1; k<=n; k++) presentationRels.emplace_back(std::string(kRelBase) + "slide", "slides/slide" + std::to_string(k) + ".xml");
  files.emplace_back("ppt/_rels/presentation.xml.rels", relationships(presentationRels));
  files.emplace_back("ppt/presProps.xml", header + "<p:presentationPr" + namespaces() + "/>");
  files.emplace_back("ppt/viewProps.xml", header + "<p:viewPr" + namespaces() + "><p:gridSpacing cx=\"76200\" cy=\"76200\"/></p:viewPr>");
  files.emplace_back("ppt/tableStyles.xml", header + "<a:tblStyleLst xmlns:a=\"" + kNsA + "\" def=\"{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}\"/>");
  files.emplace_back("ppt/theme/theme1.xml", theme());
  files.emplace_back("ppt/slideMasters/slideMaster1.xml", slideMaster());
  files.emplace_back("ppt/slideMasters/_rels/slideMaster1.xml.rels", relationships({
    {std::string(kRelBase) + "slideLayout", "../slideLayouts/slideLayout1.xml"},
    {std::string(kRelBase) + "theme", "../theme/theme1.xml"}}));
  files.emplace_back("ppt/slideLayouts/slideLayout1.xml", slideLayout());
  files.emplace_back("ppt/slideLayouts/_rels/slideLayout1.xml.rels", relationships({
    {std::string(kRelBase) + "slideMaster", "../slideMasters/slideMaster1.xml"}}));
  for (int k=1; k<=n; k++)
    {
    files.emplace_back("ppt/slides/slide" + std::to_string(k) + ".xml", _slides[std::size_t(k - 1)]);
    files.emplace_back("ppt/slides/_rels/slide" + std::to_string(k) + ".xml.rels", relationships({
      {std::string(kRelBase) + "slideLayout", "../slideLayouts/slideLayout1.xml"}}));
    }
  return zip(files);
  }

  bool PptxPainter::write(const String & fileName)
  {
  std::string bytes = package();
  std::ofstream file(fileName.data(), std::ios::binary);
  if (!file) return false;
  file.write(bytes.data(), std::streamsize(bytes.size()));
  return bool(file);
  }

  } // namespace GRAPHICS
} // namespace CAP

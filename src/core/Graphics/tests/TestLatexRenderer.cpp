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
//! @file TestLatexRenderer.cpp
//! @brief Test program of LatexRenderer and of the LaTeX texts of SvgPainter (run by ctest).
//
// TestLatexRenderer -- tests of CAP::GRAPHICS::LatexRenderer.
//
// The pieces that do not need LaTeX (kinds of texts, checks, documents, keys, reading SVG) are always tested; the
// typesetting, the caches and SvgPainter only if latex and dvisvgm are installed (otherwise these checks are skipped,
// and said so). The disk cache is a temporary directory, removed at the end. Prints each failed check and exits with
// the number of failures (0: all passed).
//
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "FileSystem.hpp"
#include "LatexRenderer.hpp"
#include "SvgPainter.hpp"

using namespace CAP;
using namespace CAP::GRAPHICS;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

bool contains(const String & text, const char * part) { return text.contains(part); }

int count(const std::string & text, const std::string & part)
{
  int n = 0;
  for (std::size_t at = text.find(part); at != std::string::npos; at = text.find(part, at+1)) n++;
  return n;
}

// SVG document of one text drawn at the center of a 400 x 300 page
std::string drawn(const String & text)
{
  SvgPainter svg(400, 300);
  TextAttributes attributes;
  attributes.setSize(0.05f);
  svg.drawText(Point2D(0.5f, 0.5f), text, attributes);
  return svg.document().str();
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- kinds of texts
  CHECK(LatexRenderer::kind("pT (GeV/c)") == LatexRenderer::kPlain);
  CHECK(LatexRenderer::kind("file_name <y>") == LatexRenderer::kPlain);
  CHECK(LatexRenderer::kind("p_{T} (GeV/c)") == LatexRenderer::kRoot);
  CHECK(LatexRenderer::kind("M (GeV/c^{2})") == LatexRenderer::kRoot);
  CHECK(LatexRenderer::kind("$p_{T}$ (GeV/$c$)") == LatexRenderer::kLatex);
  CHECK(LatexRenderer::kind("\\LaTeX") == LatexRenderer::kLatex);
  CHECK(LatexRenderer::kind("#Delta p_{T}") == LatexRenderer::kRoot);
  CHECK(LatexRenderer::plainText("$p_{T}$ (GeV/$c$)").equalTo("p_{T} (GeV/c)"));
  CHECK(LatexRenderer::plainText("x").equalTo("x"));

  // ---- translation of the ROOT syntax
  {
    struct { const char * root; const char * latex; } table[] = {
      {"p_{T}",                    "p$_{\\text{T}}$"},
      {"p_{T,1}",                  "p$_{\\text{T,1}}$"},
      {"#Delta#varphi",            "$\\Delta\\varphi$"},
      {"#Delta y",                 "$\\Delta$ y"},
      {"#varphi_{1}",              "$\\varphi_{\\text{1}}$"},
      {"x^2",                      "x$^{\\text{2}}$"},
      {"x_{i}^{2}",                "x$_{\\text{i}}^{\\text{2}}$"},
      {"M (GeV/c^{2})",            "M (GeV/c$^{\\text{2}}$)"},
      {"1/(2#pi p_{T}) dN/dp_{T}", "1/(2$\\pi$ p$_{\\text{T}}$) dN/dp$_{\\text{T}}$"},
      {"#rho_{#pi^{+}}",           "$\\rho_{\\text{$\\pi^{\\text{+}}$}}$"},
      {"#frac{1}{N}",              "$\\frac{\\text{1}}{\\text{N}}$"},
      {"#sqrt{s_{NN}}",            "$\\sqrt{\\text{s$_{\\text{NN}}$}}$"},
      {"#sqrt[3]{x}",              "$\\sqrt[\\text{3}]{\\text{x}}$"},
      {"#bar{p}",                  "$\\bar{\\text{p}}$"},
      {"#it{p}_{T}",               "\\textit{p}$_{\\text{T}}$"},
      {"#bf{x}",                   "\\textbf{x}"},
      {"#splitline{a}{b}",         "\\shortstack[l]{a\\\\b}"},
      {"#color[2]{red}",           "{red}"},
      {"#kern[0.3]{x}",            "{x}"},
      {"#left(x#right)",           "(x)"},
      {"#{a#}",                    "\\{a\\}"},
      {"a < b | c",                "a $<$ b $|$ c"},
      {"100% & more",              "100\\% \\& more"},
      {"#Alpha#Rho",               "$\\mathrm{A}\\mathrm{P}$"},
      {"#pm#3dots",                "$\\pm\\ldots$"},
      {"#nosuch x",                "\\#nosuch x"},
      {"a}",                       "a\\}"},
    };
    for (const auto & t : table)
      {
      String latex = LatexRenderer::fromRoot(t.root);
      check(latex.equalTo(t.latex), std::string("fromRoot(\"") + t.root + "\"): got \"" + latex.data() + "\", expected \"" + t.latex + "\"", __LINE__);
      }
    LatexRenderer & r = LatexRenderer::instance();
    CHECK(r.source("plain").isNull() && r.source("$x$").equalTo("$x$") && r.source("p_{T}").equalTo("p$_{\\text{T}}$"));
    r.setRootSyntax(false);
    CHECK(r.source("p_{T}").isNull());
    r.setRootSyntax(true);
  }

  // ---- checks made before running LaTeX
  CHECK(LatexRenderer::check("$\\frac{a}{b}$ \\{ \\$").isNull());
  CHECK(LatexRenderer::check("100\\% {x}").isNull());
  CHECK(contains(LatexRenderer::check("$x^{2$"), "'{' without '}'"));
  CHECK(contains(LatexRenderer::check("x}"), "'}' without '{'"));
  CHECK(contains(LatexRenderer::check("$x"), "'$'"));

  // ---- documents and keys
  {
    String preamble("\\usepackage{amsmath}\n\\usepackage{amssymb}");
    std::string d = LatexRenderer::document(preamble, {"a", "two\nlines", "c"});
    std::vector<std::string> lines;
    std::istringstream in(d);
    for (std::string line; std::getline(in, line); ) lines.push_back(line);
    int first = LatexRenderer::firstLine(preamble);
    CHECK(int(lines.size()) > first + 6);
    CHECK(lines[first-1] == "a" && lines[first+2] == "two lines" && lines[first+5] == "c");
    CHECK(lines[first-2] == "\\begin{preview}" && lines[first] == "\\end{preview}");
    CHECK(count(d, "\\begin{preview}") == 3);
    CHECK(LatexRenderer::key("p", "$x$") == LatexRenderer::key("p", "$x$"));
    CHECK(LatexRenderer::key("p", "$x$") != LatexRenderer::key("p", "$y$"));
    CHECK(LatexRenderer::key("p", "$x$") != LatexRenderer::key("q", "$x$"));
    CHECK(LatexRenderer::key("ab", "c") != LatexRenderer::key("a", "bc"));
  }

  // ---- reading the SVG of dvisvgm
  {
    LatexImage image;
    std::string svg = "<?xml version='1.0'?>\n<svg version='1.1' xmlns='http://www.w3.org/2000/svg' width='10pt' height='8pt' "
                      "viewBox='-0.5 -6 10 8'>\n<g id='page1'>\n<path d='M0 0L1 1'/>\n</g>\n</svg>";
    CHECK(LatexRenderer::parseSvg(svg, image));
    CHECK(image.ok && image.x == -0.5f && image.width == 10.0f && image.height == 6.0f && image.depth == 2.0f);
    CHECK(image.svg.find("<path d='M0 0L1 1'/>") != std::string::npos && image.svg.find("id=") == std::string::npos);
    LatexImage bad;
    CHECK(!LatexRenderer::parseSvg("<svg width='1pt'></svg>", bad) && !bad.ok);
  }

  LatexRenderer & latex = LatexRenderer::instance();
  if (!latex.available())
    {
    std::printf("TestLatexRenderer: latex or dvisvgm not found, typesetting not tested\n");
    std::string plain = drawn("$x$");
    CHECK(plain.find("<text") != std::string::npos && plain.find(">x</text>") != std::string::npos);
    std::printf("TestLatexRenderer: %d checks, %d failed\n", nChecks, nFailures);
    return nFailures;
    }

  char buffer[] = "/tmp/CapTestLatex.XXXXXX";
  String cache(mkdtemp(buffer));
  latex.setCacheDirectory(cache);
  latex.clearMemoryCache();

  // ---- typesetting one text
  int runs = latex.nRuns();
  const LatexImage * x = latex.image("$x$");
  CHECK(x && x->ok);
  CHECK(latex.nRuns() == runs + 1);
  if (x && x->ok)
    {
    // the x of Computer Modern at 10 pt: about 5.7 pt wide, 4.3 pt high (x-height), on the baseline
    CHECK(x->width > 4.0f && x->width < 7.0f);
    CHECK(std::fabs(x->height - 4.3f) < 0.6f);
    CHECK(std::fabs(x->depth) < 0.3f);
    CHECK(count(x->svg, "<path") >= 1);
    }
  CHECK(latex.image("$x$") == x && latex.nRuns() == runs + 1);              // from the memory cache
  const LatexImage * pt = latex.image("$p_{\\mathrm{T}}$");
  CHECK(pt && pt->ok && pt->depth > 1.0f);                                  // the p descends below the baseline
  CHECK(latex.image("p_T") == nullptr);                                     // plain text: not typeset
  const LatexImage * delta = latex.image("#Delta");                         // ROOT syntax: translated
  CHECK(delta && delta->ok && delta == latex.image("$\\Delta$"));           // the same image as its translation

  // ---- errors
  runs = latex.nRuns();
  const LatexImage * unbalanced = latex.image("$x^{2$");
  CHECK(unbalanced && !unbalanced->ok && contains(unbalanced->error, "Unbalanced"));
  CHECK(latex.nRuns() == runs);                                             // found without running LaTeX
  const LatexImage * undefined = latex.image("$\\nosuchmacro x$");
  CHECK(undefined && !undefined->ok && contains(undefined->error, "Undefined control sequence"));

  // ---- one run for many texts, with a bad one among them
  {
    std::vector<String> texts = {"$\\alpha$", "$\\alpha\\beta$", "$\\nosuchmacro$", "$\\alpha\\beta\\gamma$", "$\\sqrt{s_{NN}}$"};
    runs = latex.nRuns();
    latex.prepare(texts);
    CHECK(latex.nRuns() == runs + 1);
    const LatexImage * a = latex.cached(texts[0]);
    const LatexImage * ab = latex.cached(texts[1]);
    const LatexImage * bad = latex.cached(texts[2]);
    const LatexImage * abc = latex.cached(texts[3]);
    const LatexImage * root = latex.cached(texts[4]);
    CHECK(a && a->ok && ab && ab->ok && abc && abc->ok && root && root->ok);
    CHECK(bad && !bad->ok && contains(bad->error, "Undefined control sequence"));
    if (a && ab && abc && a->ok && ab->ok && abc->ok) CHECK(a->width < ab->width && ab->width < abc->width);   // pages in order
    latex.prepare(texts);
    CHECK(latex.nRuns() == runs + 1);                                       // all cached (the bad one in memory)
  }

  // ---- disk cache
  {
    CHECK(FileSystem::isFile(FileSystem::join(cache, String([&]{ char n[32]; std::snprintf(n, 32, "%016llx.svg",
          (unsigned long long)LatexRenderer::key(latex.preamble(), "$x$")); return std::string(n); }()))));
    LatexRenderer other;
    other.setCacheDirectory(cache);
    const LatexImage * again = other.image("$x$");
    CHECK(again && again->ok && other.nRuns() == 0);                        // read from the disk
    if (again && x && again->ok && x->ok) CHECK(again->width == x->width && again->svg == x->svg);
    other.setCacheDirectory("");
    other.setPreamble("\\usepackage{amsmath}");
    CHECK(other.image("$x$") && other.nRuns() == 1);                        // another preamble: typeset again
    other.setPreamble("\\usepackage{nosuchpackagecap}");
    const LatexImage * broken = other.image("$y$");
    CHECK(broken && !broken->ok && contains(broken->error, "preamble"));
  }

  // ---- labels of CAP in ROOT syntax: all typeset, in one run
  {
    std::vector<String> labels = {"p_{T}", "#Delta#varphi", "#varphi_{1}", "#Delta#eta", "#Delta y", "#alpha", "#eta_{2}",
      "p_{T,2}", "#Delta#phi", "p_{T} (GeV/c)", "Q_{inv}", "#theta_{1}", "M (GeV/c^{2})", "#rho_{1}(pT)", "p_{T}(GeV)",
      "1/(2#pi p_{T}) dN/dp_{T}", "j_{T,2}", "N_{part}", "#Delta #rho_{2}(Q_{inv})", "#Delta q_{1}#Delta q_{1}",
      "#bar{p}", "#sqrt{s_{NN}}", "#int f(x) dx", "#sum_{i} x_{i}^{2}", "#pm 1#sigma", "#varepsilon", "#Lambda",
      "#frac{dN}{d#eta}", "#it{p}_{T} (GeV/#it{c})", "#splitline{line 1}{line 2}", "#color[2]{a} #kern[0.5]{b}",
      "x < y | z_{1}", "50% & #infty", "#left(#frac{a}{b}#right)", "#AA #rightarrow #nu#bar{#nu}"};
    runs = latex.nRuns();
    latex.prepare(labels);
    CHECK(latex.nRuns() == runs + 1);
    for (const auto & label : labels)
      {
      const LatexImage * image = latex.cached(label);
      check(image && image->ok, std::string("ROOT label \"") + label.data() + "\": " + (image ? image->error.data() : "no image"), __LINE__);
      }
    const LatexImage * pt = latex.cached("p_{T}");
    const LatexImage * ptTwo = latex.cached("p_{T,2}");
    if (pt && ptTwo && pt->ok && ptTwo->ok) CHECK(pt->width < ptTwo->width && pt->depth > 1.0f);
  }

  // ---- typesetting in the background
  {
    std::vector<String> texts = {"$\\omega_{1}$", "$\\omega_{2}$", "$\\omega_{3}$"};
    std::atomic<int> calls(0);
    latex.setReadyCallback([&calls]() { calls++; });
    runs = latex.nRuns();
    for (const auto & text : texts) CHECK(latex.imageLater(text) == nullptr);   // queued
    CHECK(latex.imageLater(texts[0]) == nullptr);                             // queued once
    CHECK(latex.imageLater("plain") == nullptr && latex.imageLater("$x$") == x);   // plain; already cached
    for (int k=0; k<2000 && latex.busy(); k++) usleep(5000);
    CHECK(!latex.busy() && calls == 1);
    CHECK(latex.nRuns() == runs + 1);                                         // one run for the three texts
    for (const auto & text : texts) { const LatexImage * image = latex.imageLater(text); CHECK(image && image->ok); }
    const LatexImage * bad = nullptr;
    latex.imageLater("$\\nosuchmacro y$");
    for (int k=0; k<2000 && latex.busy(); k++) usleep(5000);
    bad = latex.imageLater("$\\nosuchmacro y$");
    CHECK(bad && !bad->ok && latex.nRuns() == runs + 2);                    // failed, and not queued again
    latex.setReadyCallback(nullptr);
  }

  // ---- SvgPainter
  {
    std::string svg = drawn("$x^{2}$");
    CHECK(svg.find("<path") != std::string::npos && svg.find("<text") == std::string::npos);
    CHECK(svg.find("scale(") != std::string::npos);
    std::string plain = drawn("plain");
    CHECK(plain.find(">plain</text>") != std::string::npos);
    std::string failed = drawn("$\\nosuchmacro x$");                        // drawn as plain text, without the $
    CHECK(failed.find("\\nosuchmacro x</text>") != std::string::npos);
    latex.setEnabled(false);
    CHECK(latex.image("$x$") == nullptr);
    CHECK(drawn("$x$").find(">x</text>") != std::string::npos);
    latex.setEnabled(true);
  }

  FileSystem::removeTree(cache);
  std::printf("TestLatexRenderer: %d checks, %d failed (%s)\n", nChecks, nFailures, latex.binDirectory().data());
  return nFailures;
}

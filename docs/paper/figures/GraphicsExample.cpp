// Source of figures/GraphicsExample.pdf (the figure of the Graphics section of the paper), drawn by CAP itself.
// Build it against the Graphics, Math and Base libraries of a CAP build directory, and run it in docs/paper/figures.
#include <cmath>
#include "Canvas.hpp"
#include "Graph.hpp"
#include "PdfPainter.hpp"
#include "Histogram.hpp"
#include "Fitter.hpp"
#include "FitViews.hpp"
#include "Normal.hpp"
#include "Linear.hpp"
#include "Normal2D.hpp"
#include "ParameterizedComposite.hpp"
#include "Region.hpp"
#include "Palette.hpp"
using namespace CAP;
using namespace CAP::GRAPHICS;

static unsigned long long seed = 12345;
static double uniform() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; }
static double gauss() { return std::sqrt(-2*std::log(uniform()))*std::cos(2*M_PI*uniform()); }

int main()
{
  // (a) a peak over a linear background, fitted by a composite with a region left out
  MATH::Histogram h("h", "Peak over a background", 60, 0.0, 6.0);
  h.setDimensionTitle(0, "$m$ (GeV/$c^2$)");
  for (int k=0; k<20000; k++) { double x = 6.0*std::sqrt(uniform()); h.fill(6.0 - x, 1.0); }
  for (int k=0; k<6000; k++) h.fill(3.0 + 0.3*gauss(), 1.0);
  for (int k=0; k<1500; k++) h.fill(4.6 + 0.08*gauss(), 1.0);   // a narrow structure excluded from the fit
  MATH::ParameterizedComposite model;
  model.add("signal", MATH::Normal(800.0, 3.0, 0.3));
  model.add("background", MATH::Linear(650.0, -100.0));
  MATH::Fitter::FitOptions o;
  o.statistic = MATH::Fitter::kPoisson;
  o.label = "signal + background";
  o.excluded.push_back(MATH::Region::interval(0, 4.2, 5.0));
  MATH::Fitter::fitAndAttach(model, h, o);

  // (b-d) a correlated peak of two variables, fitted by a Normal2D
  MATH::Histogram h2("h2", "Fit of a 2D Gaussian", 30, -3.0, 3.0, 30, -3.0, 3.0);
  h2.setDimensionTitle(0, "$\\Delta\\eta$");
  h2.setDimensionTitle(1, "$\\Delta\\varphi$");
  for (int k=0; k<40000; k++)
    {
    double z1 = gauss(), z2 = gauss();
    h2.fill(0.8*z1, 0.6*(0.5*z1 + std::sqrt(0.75)*z2), 1.0);
    }
  MATH::Normal2D peak;
  MATH::Fitter::estimate(peak, h2, std::vector<double>{ -3.0, -3.0 }, std::vector<double>{ 3.0, 3.0 });
  MATH::Fitter::FitOptions o2;
  o2.statistic = MATH::Fitter::kPoisson;
  o2.label = "Normal2D";
  MATH::Fitter::fitAndAttach(peak, h2, o2);
  MATH::Histogram pulls = MATH::residualMap(h2, h2.fit(0));
  pulls.setTitle("Pulls of the fit");

  Canvas canvas("c", "graphics", 1000, 800);
  canvas.divide(2, 2);
  Graph & a = canvas.graph(0, 0);
  a.show(GraphItem(&h, "data"));
  a.setShowResiduals(true);
  a.setResidualMode(Graph::kPull);
  a.setYRange(0.0f, 1600.0f);
  a.setLegendPosition(0.03f, 0.97f);
  Graph & b = canvas.graph(1, 0);
  b.show(GraphItem(&h2, "data"));
  b.items()[0].setShowContours(true);
  b.items()[0].setContourIntervals(5);
  b.setYRange(-3.0f, 4.5f);
  Graph & c = canvas.graph(0, 1);
  c.show(GraphItem(&h2, "data"));
  c.setTitle("3D view");
  c.setDimension(Graph::k3D);
  c.items()[0].setStyle3D(GraphItem::kSurface);
  Graph & d = canvas.graph(1, 1);
  d.show(GraphItem(&pulls, "pulls"));
  d.setResidualMapStyle();
  d.setYRange(-3.0f, 4.5f);
  PdfPainter pdf(1000, 800);
  canvas.draw(pdf);
  return pdf.write("GraphicsExample.pdf") ? 0 : 1;
}

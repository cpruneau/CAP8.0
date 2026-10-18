#include <cmath>
#include <cstdio>
#include <vector>
#include "AxisRenderer.hpp"
#include "Axis.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::AxisRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  AxisRenderer::AxisRenderer()
  :
  Renderer()
  { }

  AxisRenderer::AxisRenderer(const AxisRenderer & src)
  :
  Renderer(src)
  { }

  AxisRenderer  & AxisRenderer::operator=(const AxisRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const AxisRenderer * AxisRenderer::defaultRenderer()
  {
  static const AxisRenderer renderer;
  return &renderer;
  }

  namespace
  {
  struct Tick { float value; bool major; };

  //! Tick values within the axis' current range.
  std::vector<Tick> ticks(const Axis & axis)
  {
  std::vector<Tick> result;
  float min = axis.currentRangeMinimum();
  float max = axis.currentRangeMaximum();
  if (axis.usesLogScale())
    {
    // majors at each decade, minors at 2..9 x decade
    int kLow  = int(std::floor(std::log10(min)));
    int kHigh = int(std::ceil(std::log10(max)));
    for (int k=kLow; k<=kHigh; k++)
      {
      float decade = std::pow(10.0f, float(k));
      if (decade >= min*(1-1E-5) && decade <= max*(1+1E-5)) result.push_back({decade, true});
      for (int m=2; m<=9; m++)
        {
        float v = m*decade;
        if (v > min && v < max) result.push_back({v, false});
        }
      }
    return result;
    }
  // linear: majors at the multiples of a 1, 2, 5 x 10^k step giving about nMajorTicks() intervals (for a range
  // already rounded to that step, as the automatic ranges of Graph are, the ticks start and end at the edges)
  int nMajor = axis.nMajorTicks() < 1 ? 1 : axis.nMajorTicks();
  double span = double(max) - double(min);
  if (!(span > 0)) return result;
  double raw = span/nMajor;
  double decade = std::pow(10.0, std::floor(std::log10(raw)));
  double mantissa = raw/decade;
  double step = decade*(mantissa < 1.5 ? 1.0 : (mantissa < 3.5 ? 2.0 : (mantissa < 7.5 ? 5.0 : 10.0)));
  int nMinor = axis.nMinorTicks() < 1 ? 1 : axis.nMinorTicks();
  if (nMinor == 5 && std::fabs(step/decade - 2.0) < 1E-6) nMinor = 4;   // 0.5 steps between majors 2 apart
  double minorStep = step/nMinor;
  double tolerance = 1E-6*span;
  long first = long(std::ceil((min - tolerance)/minorStep));
  long last  = long(std::floor((max + tolerance)/minorStep));
  for (long k=first; k<=last; k++)
    {
    double v = k*minorStep;
    bool major = (k % nMinor) == 0;
    if (major && std::fabs(v) < 1E-9*step) v = 0.0;   // avoid labels like -1.4e-08
    result.push_back({float(v), major});
    }
  return result;
  }
  } // anonymous namespace

  void AxisRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  const Axis * axis = dynamic_cast<const Axis*>(&drawable);
  if (!axis || axis->points().size() < 2) return;
  const Point2D a = axis->points()[0];
  const Point2D b = axis->points()[1];

  // work in device units so ticks stay perpendicular on a non-square canvas
  float W = painter.width();
  float H = painter.height();
  if (W <= 0 || H <= 0) return;
  float dx = (b.x()-a.x())*W;
  float dy = (b.y()-a.y())*H;
  float length = std::sqrt(dx*dx + dy*dy);
  if (length <= 0) return;
  float ux = dx/length;
  float uy = dy/length;
  // unit normal on the tick side (device units), and its conversion to canvas units per unit of H
  float nx = -uy*axis->tickSide();
  float ny =  ux*axis->tickSide();
  auto offset = [&](const Point2D & p, float sizeFractionOfH)
    {
    float d = sizeFractionOfH*H;
    return Point2D(p.x() + nx*d/W, p.y() + ny*d/H);
    };

  // axis line
  std::vector<Point2D> segment{a, b};
  painter.drawLine(segment, axis->lineAttributes());
  Rect2D body(a.x(), a.y(), b.x(), b.y());

  // ticks and labels
  TextAttributes labelAttributes = axis->textAttributes();
  const bool sideLabels = std::fabs(nx) > std::fabs(ny);   // labels to the side (vertical-ish axis)
  if (sideLabels)
    labelAttributes.setJustification(nx > 0 ? TextAttributes::kRight : TextAttributes::kLeft);
  else
    labelAttributes.setJustification(TextAttributes::kCenter);

  float labelExtent = 0.0; // largest label size along the label direction, device units
  float textHeight  = axis->textAttributes().size()*H;
  for (const auto & tick : ticks(*axis))
    {
    if (tick.major ? !axis->showMajorTicks() : !axis->showMinorTicks()) continue;
    Point2D base = axis->toCanvas(tick.value);
    float size   = tick.major ? axis->majorTickSize() : axis->minorTickSize();
    Point2D tip  = offset(base, size);
    std::vector<Point2D> tickLine{base, tip};
    painter.drawLine(tickLine, axis->lineAttributes());
    body.include(tip);
    // a label at an end of the axis may be left out (where it would meet the label of another axis)
    bool atStart = std::fabs((base.x()-a.x())*W) + std::fabs((base.y()-a.y())*H) < 0.5f;
    bool atEnd   = std::fabs((base.x()-b.x())*W) + std::fabs((base.y()-b.y())*H) < 0.5f;
    bool hidden  = (atStart && axis->hidesStartLabel()) || (atEnd && axis->hidesEndLabel());
    if (tick.major && axis->showLabels() && !hidden)
      {
      char text[32];
      std::snprintf(text, sizeof(text), "%.6g", tick.value);
      String label(text);
      Point2D anchor = offset(base, -axis->labelOffset());
      painter.drawText(anchor, label, labelAttributes, axis->labelAngle());
      float w = painter.textWidth(label, labelAttributes);
      // labels centered on the anchor extend by half their size along the normal; side-justified ones by their full width
      float extent = sideLabels ? w : 0.5*textHeight;
      if (extent > labelExtent) labelExtent = extent;
      float xLeft = anchor.x()*W;
      if      (labelAttributes.justification() == TextAttributes::kCenter) xLeft -= 0.5f*w;
      else if (labelAttributes.justification() == TextAttributes::kRight)  xLeft -= w;
      body.include(Rect2D(xLeft/W, anchor.y() - 0.5f*textHeight/H, (xLeft + w)/W, anchor.y() + 0.5f*textHeight/H));
      }
    }

  // title, centered beyond the labels and parallel to the axis, then moved by the title offset
  Rect2D titleRect;
  if (!axis->title().isNull())
    {
    Point2D middle((a.x()+b.x())/2, (a.y()+b.y())/2);
    // beyond the labels, plus a gap and half the title height (in fractions of H)
    float labelSpan   = axis->showLabels() ? labelExtent : 0.0;
    float titleHeight = axis->titleAttributes().size()*H;
    float distance    = axis->labelOffset() + (labelSpan + 0.3*textHeight + 0.5*titleHeight)/H;
    TextAttributes titleAttributes = axis->titleAttributes();
    titleAttributes.setJustification(TextAttributes::kCenter);
    float angle = std::atan2(dy, dx)*180.0/3.14159265358979323846;
    Point2D position = offset(middle, -distance);
    position = Point2D(position.x() + axis->titleOffsetX(), position.y() + axis->titleOffsetY());
    painter.drawText(position, axis->title(), titleAttributes, angle);
    // box of the rotated text (device units), then in canvas units
    float w = painter.textWidth(axis->title(), titleAttributes);
    float c = std::fabs(ux), s = std::fabs(uy);
    float boxW = w*c + titleHeight*s, boxH = w*s + titleHeight*c;
    titleRect = Rect2D::centered(position, boxW/W, boxH/H);
    }
  axis->setDrawnRects(titleRect, body);
  }

  } // namespace GRAPHICS
} // namespace CAP

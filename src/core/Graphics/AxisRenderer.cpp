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
  int nMajor = axis.nMajorTicks() < 1 ? 1 : axis.nMajorTicks();
  int nMinor = axis.nMinorTicks() < 1 ? 1 : axis.nMinorTicks();
  float w = (max - min)/nMajor;
  for (int iMajor=0; iMajor<=nMajor; iMajor++)
    {
    float v = min + iMajor*w;
    if (std::fabs(v) < 1E-6*std::fabs(w)) v = 0.0; // avoid labels like -1.4e-08
    result.push_back({v, true});
    if (iMajor == nMajor) break;
    for (int iMinor=1; iMinor<nMinor; iMinor++)
      result.push_back({min + (iMajor + float(iMinor)/nMinor)*w, false});
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

  // ticks and labels
  TextAttributes labelAttributes = axis->textAttributes();
  if (std::fabs(nx) > std::fabs(ny))   // labels to the side (vertical-ish axis)
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
    std::vector<Point2D> tickLine{base, offset(base, size)};
    painter.drawLine(tickLine, axis->lineAttributes());
    if (tick.major && axis->showLabels())
      {
      char text[32];
      std::snprintf(text, sizeof(text), "%.6g", tick.value);
      String label(text);
      painter.drawText(offset(base, -axis->labelOffset()), label, labelAttributes, axis->labelAngle());
      float w = painter.textWidth(label, labelAttributes);
      // labels centered on the anchor extend by half their size along the normal; side-justified ones by their full width
      float extent = (std::fabs(nx) > std::fabs(ny)) ? w : 0.5*textHeight;
      if (extent > labelExtent) labelExtent = extent;
      }
    }

  // title, centered beyond the labels and parallel to the axis
  if (!Painter::isEmpty(axis->title()))
    {
    Point2D middle((a.x()+b.x())/2, (a.y()+b.y())/2);
    // beyond the labels, plus a gap and half the title height (in fractions of H)
    float labelSpan = axis->showLabels() ? labelExtent : 0.0;
    float distance  = axis->labelOffset() + (labelSpan + 0.8*textHeight)/H;
    TextAttributes titleAttributes = axis->textAttributes();
    titleAttributes.setJustification(TextAttributes::kCenter);
    float angle = std::atan2(dy, dx)*180.0/3.14159265358979323846;
    painter.drawText(offset(middle, -distance), axis->title(), titleAttributes, angle);
    }
  }

  } // namespace GRAPHICS
} // namespace CAP

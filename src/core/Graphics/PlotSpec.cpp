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
#include <cctype>
#include <cmath>
#include <map>
#include <cstdio>
#include <sstream>
#include <string>
#include "PlotSpec.hpp"
#include "CanvasRegistry.hpp"
#include "CapFile.hpp"
#include "ColorTable.hpp"
#include "DataPoints.hpp"
#include "FileSystem.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "ParameterizedComposite.hpp"
#include "ParameterizedFunction.hpp"
#include "PdfPainter.hpp"
#include "SvgPainter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  //! Numbers of a list separated by commas, semicolons or spaces (entries that are not numbers are skipped).
  std::vector<double> numbers(const String & text)
  {
  std::vector<double> values;
  std::string list(text.str());
  for (char & c : list) if (c == ',' || c == ';' || c == '\t' || c == '\n') c = ' ';
  std::istringstream in(list);
  std::string entry;
  while (in >> entry)
    {
    try { std::size_t used = 0; double v = std::stod(entry, &used); if (used == entry.size()) values.push_back(v); }
    catch (...) { }
    }
  return values;
  }

  //! A list of numbers as text, with full precision.
  String numberList(const std::vector<double> & values)
  {
  std::ostringstream out;
  out.precision(10);
  for (std::size_t k=0; k<values.size(); k++) { if (k) out << ", "; out << values[k]; }
  return String(out.str());
  }

  //! A number as written in the files: 6 significant digits (the settings are floats).
  String num(double value)
  {
  char text[32];
  std::snprintf(text, sizeof(text), "%.6g", value);
  return String(text);
  }

  //! Names of the axes in the Axis elements.
  const char * axisName(int k) { return k == 0 ? "x" : (k == 1 ? "y" : (k == 2 ? "z" : "residual")); }

  //! Index of an axis named in an Axis element (-1 if unknown).
  int axisIndex(const String & name)
  {
  if (name == "x" || name == "X") return 0;
  if (name == "y" || name == "Y") return 1;
  if (name == "z" || name == "Z") return 2;
  if (name == "residual" || name == "residuals") return 3;
  return -1;
  }

  const char * sysStyleName(GraphItem::SysStyle s) { return s == GraphItem::kSysBoxes ? "boxes" : (s == GraphItem::kSysBrackets ? "brackets" : "band"); }
  const char * style3DName(GraphItem::Style3D s) { return s == GraphItem::kSurface ? "surface" : (s == GraphItem::kMesh ? "mesh" : "lego"); }
  const char * justificationName(TextAttributes::Justification j)
  { return j == TextAttributes::kLeft ? "left" : (j == TextAttributes::kCenter ? "center" : "right"); }


  //! Whether a name is that of an item element.
  bool isDataElement(const XmlElement & e) { return e.isNamed("Histogram") || e.isNamed("Data") || e.isNamed("Profile"); }
  } // namespace

  // ------------------------------------------------------------------------------------------------ GraphXml

  namespace GraphXml
  {
  int parseColor(const String & text, int fallback)
  {
  std::string s = text.str();
  while (!s.empty() && s.front() == ' ') s.erase(s.begin());
  while (!s.empty() && s.back() == ' ') s.pop_back();
  if (s.empty()) return fallback;
  ColorTable & table = ColorTable::instance();
  if (s[0] == '#' && (s.size() == 7 || s.size() == 9))
    {
    unsigned int v = 0;
    if (std::sscanf(s.c_str() + 1, "%6x", &v) != 1) return fallback;
    return table.index(Color::fromBytes(int((v >> 16) & 0xff), int((v >> 8) & 0xff), int(v & 0xff)));
    }
  bool numeric = true;
  for (char c : s) if (!(std::isdigit((unsigned char)c) || c == '-' || c == '+')) { numeric = false; break; }
  if (numeric) { try { return std::stoi(s); } catch (...) { return fallback; } }
  // a name of the table ("black", "kRed+2", ...)
  static std::map<std::string,int> byName;
  if (byName.empty())
    for (int k=0; k<2000; k++) if (table.has(k)) byName.emplace(table.name(k).str(), k);
  auto found = byName.find(s);
  if (found != byName.end()) return found->second;
  for (int k=0; k<5000; k++) if (table.has(k) && table.name(k).str() == s) return k;   // a color added since
  return fallback;
  }

  String colorName(int color)
  {
  ColorTable & table = ColorTable::instance();
  return table.has(color) ? table.name(color) : String(std::to_string(color));
  }

  namespace
  {
  //! Color and alpha of an element.
  void readColor(const XmlElement & e, Attributes & a)
  {
  int color = a.color();
  float alpha = a.alpha();
  if (e.hasAttribute("color")) color = parseColor(e.attribute("color"), color);
  if (e.hasAttribute("alpha")) alpha = float(e.attributeDouble("alpha", alpha));
  a.setColor(color, alpha);
  if (e.hasAttribute("style")) a.setStyle(int(e.attributeInt("style", a.style())));
  }
  void writeColor(XmlElement & e, const Attributes & a)
  {
  e.setAttribute("color", colorName(a.color()));
  e.setAttribute("alpha", num(a.alpha()));
  e.setAttribute("style", a.style());
  }
  } // namespace

  void read(const XmlElement * e, LineAttributes & line)
  {
  if (!e) return;
  readColor(*e, line);
  if (e->hasAttribute("width")) line.setWidth(float(e->attributeDouble("width", line.width())));
  }

  void read(const XmlElement * e, FillAttributes & fill)
  {
  if (!e) return;
  readColor(*e, fill);
  }

  void read(const XmlElement * e, MarkerAttributes & marker)
  {
  if (!e) return;
  readColor(*e, marker);
  if (e->hasAttribute("size")) marker.setSize(float(e->attributeDouble("size", marker.size())));
  }

  void read(const XmlElement * e, TextAttributes & text)
  {
  if (!e) return;
  readColor(*e, text);
  if (e->hasAttribute("font")) text.setFont(int(e->attributeInt("font", text.font())));
  if (e->hasAttribute("size")) text.setSize(float(e->attributeDouble("size", text.size())));
  if (e->hasAttribute("justification"))
    {
    String j = e->attribute("justification");
    if (j == "left") text.setJustification(TextAttributes::kLeft);
    else if (j == "center" || j == "centre") text.setJustification(TextAttributes::kCenter);
    else if (j == "right") text.setJustification(TextAttributes::kRight);
    }
  }

  void read(const XmlElement * e, Margins & m)
  {
  if (!e) return;
  m.set(float(e->attributeDouble("left", m.left())), float(e->attributeDouble("bottom", m.bottom())),
        float(e->attributeDouble("right", m.right())), float(e->attributeDouble("top", m.top())));
  }

  void write(XmlElement & e, const LineAttributes & line) { writeColor(e, line); e.setAttribute("width", num(line.width())); }
  void write(XmlElement & e, const FillAttributes & fill) { writeColor(e, fill); }
  void write(XmlElement & e, const MarkerAttributes & marker) { writeColor(e, marker); e.setAttribute("size", num(marker.size())); }
  void write(XmlElement & e, const TextAttributes & text)
  {
  writeColor(e, text);
  e.setAttribute("font", text.font());
  e.setAttribute("size", num(text.size()));
  e.setAttribute("justification", justificationName(text.justification()));
  }
  void write(XmlElement & e, const Margins & m)
  {
  e.setAttribute("left", num(m.left()));
  e.setAttribute("bottom", num(m.bottom()));
  e.setAttribute("right", num(m.right()));
  e.setAttribute("top", num(m.top()));
  }

  void readAxis(const XmlElement & e, Graph & graph, int k)
  {
  Axis & axis = (k == 3) ? graph.residualAxis() : graph.axis(k);
  if (e.hasAttribute("title")) axis.setTitle(e.attribute("title"));
  if (k < 3)
    {
    if (e.hasAttribute("log")) graph.setLog(k, e.attributeBool("log", graph.isLog(k)));
    bool hasMin = e.hasAttribute("min"), hasMax = e.hasAttribute("max");
    if (hasMin && hasMax) graph.setRange(k, float(e.attributeDouble("min")), float(e.attributeDouble("max")));
    else if (e.attribute("range", "") == "auto") graph.setAuto(k);
    }
  else
    {
    if (e.hasAttribute("min") && e.hasAttribute("max"))
      graph.setResidualRange(float(e.attributeDouble("min")), float(e.attributeDouble("max")));
    else if (e.attribute("range", "") == "auto") graph.setAutoResidualRange();
    }
  if (e.hasAttribute("reversed"))       axis.setReversed(e.attributeBool("reversed", axis.isReversed()));
  if (e.hasAttribute("showMajorTicks")) axis.setShowMajorTicks(e.attributeBool("showMajorTicks", axis.showMajorTicks()));
  if (e.hasAttribute("showMinorTicks")) axis.setShowMinorTicks(e.attributeBool("showMinorTicks", axis.showMinorTicks()));
  if (e.hasAttribute("showLabels"))     axis.setShowLabels(e.attributeBool("showLabels", axis.showLabels()));
  if (e.hasAttribute("nMajorTicks"))    axis.setNMajorTicks(int(e.attributeInt("nMajorTicks", axis.nMajorTicks())));
  if (e.hasAttribute("nMinorTicks"))    axis.setNMinorTicks(int(e.attributeInt("nMinorTicks", axis.nMinorTicks())));
  if (e.hasAttribute("majorTickSize"))  axis.setMajorTickSize(float(e.attributeDouble("majorTickSize", axis.majorTickSize())));
  if (e.hasAttribute("minorTickSize"))  axis.setMinorTickSize(float(e.attributeDouble("minorTickSize", axis.minorTickSize())));
  if (e.hasAttribute("labelOffset"))    axis.setLabelOffset(float(e.attributeDouble("labelOffset", axis.labelOffset())));
  if (e.hasAttribute("labelAngle"))     axis.setLabelAngle(float(e.attributeDouble("labelAngle", axis.labelAngle())));
  if (e.hasAttribute("labelPointSize")) axis.setLabelPointSize(float(e.attributeDouble("labelPointSize", axis.labelPointSize())));
  if (e.hasAttribute("titlePointSize")) axis.setTitlePointSize(float(e.attributeDouble("titlePointSize", axis.titlePointSize())));
  if (e.hasAttribute("labelGap"))       axis.setLabelGap(float(e.attributeDouble("labelGap", axis.labelGap())));
  if (e.hasAttribute("titleOffsetX") || e.hasAttribute("titleOffsetY"))
    axis.setTitleOffset(float(e.attributeDouble("titleOffsetX", axis.titleOffsetX())),
                        float(e.attributeDouble("titleOffsetY", axis.titleOffsetY())));
  if (e.hasAttribute("hideStartLabel") || e.hasAttribute("hideEndLabel"))
    axis.hideEndLabels(e.attributeBool("hideStartLabel", axis.hidesStartLabel()), e.attributeBool("hideEndLabel", axis.hidesEndLabel()));
  read(e.child("Line"), axis.lineAttributes());
  read(e.child("Labels"), axis.textAttributes());
  read(e.child("TitleText"), axis.titleAttributes());
  }

  void writeAxis(XmlElement & parent, const Graph & graph, int k)
  {
  const Axis & axis = (k == 3) ? graph.residualAxis() : graph.axis(k);
  XmlElement & e = parent.addChild("Axis");
  e.setAttribute("which", axisName(k));
  e.setAttribute("title", axis.title());
  if (k < 3)
    {
    e.setAttribute("log", graph.isLog(k));
    if (graph.isAuto(k)) e.setAttribute("range", "auto");
    else
      {
      Graph::View v = graph.view();
      e.setAttribute("min", num(v.minimum(k)));
      e.setAttribute("max", num(v.maximum(k)));
      }
    }
  else if (graph.isAutoResidualRange()) e.setAttribute("range", "auto");
  else
    {
    double low = 0, high = 0;
    graph.residualRange(graph.view(), low, high);
    e.setAttribute("min", num(low));
    e.setAttribute("max", num(high));
    }
  e.setAttribute("reversed", axis.isReversed());
  e.setAttribute("showMajorTicks", axis.showMajorTicks());
  e.setAttribute("showMinorTicks", axis.showMinorTicks());
  e.setAttribute("showLabels", axis.showLabels());
  e.setAttribute("nMajorTicks", axis.nMajorTicks());
  e.setAttribute("nMinorTicks", axis.nMinorTicks());
  e.setAttribute("majorTickSize", num(axis.majorTickSize()));
  e.setAttribute("minorTickSize", num(axis.minorTickSize()));
  e.setAttribute("labelOffset", num(axis.labelOffset()));
  e.setAttribute("labelAngle", num(axis.labelAngle()));
  e.setAttribute("labelPointSize", num(axis.labelPointSize()));
  e.setAttribute("titlePointSize", num(axis.titlePointSize()));
  e.setAttribute("labelGap", num(axis.labelGap()));
  e.setAttribute("titleOffsetX", num(axis.titleOffsetX()));
  e.setAttribute("titleOffsetY", num(axis.titleOffsetY()));
  e.setAttribute("hideStartLabel", axis.hidesStartLabel());
  e.setAttribute("hideEndLabel", axis.hidesEndLabel());
  write(e.addChild("Line"), axis.lineAttributes());
  write(e.addChild("Labels"), axis.textAttributes());
  write(e.addChild("TitleText"), axis.titleAttributes());
  }

  void readGraph(const XmlElement & e, Graph & graph)
  {
  if (e.hasAttribute("legend"))            graph.setShowLegend(e.attributeBool("legend", graph.showLegend()));
  if (e.hasAttribute("logX"))              graph.setLogX(e.attributeBool("logX", graph.isLogX()));
  if (e.hasAttribute("logY"))              graph.setLogY(e.attributeBool("logY", graph.isLogY()));
  if (e.hasAttribute("logZ"))              graph.setLogZ(e.attributeBool("logZ", graph.isLogZ()));
  if (e.hasAttribute("titlePointSize"))    graph.setTitlePointSize(float(e.attributeDouble("titlePointSize", graph.titlePointSize())));
  if (e.hasAttribute("titleOffsetX") || e.hasAttribute("titleOffsetY"))
    graph.setTitleOffset(float(e.attributeDouble("titleOffsetX", graph.titleOffsetX())), float(e.attributeDouble("titleOffsetY", graph.titleOffsetY())));
  if (e.hasAttribute("showFits"))          graph.setShowFits(e.attributeBool("showFits", graph.showFits()));
  if (e.hasAttribute("showFitStatistics")) graph.setShowFitStatistics(e.attributeBool("showFitStatistics", graph.showFitStatistics()));
  if (e.hasAttribute("showResiduals"))     graph.setShowResiduals(e.attributeBool("showResiduals", graph.showResiduals()));
  if (e.hasAttribute("residualMode"))      graph.setResidualMode(e.attribute("residualMode") == "pull" ? Graph::kPull : Graph::kDifference);
  if (e.hasAttribute("residualFraction"))  graph.setResidualFraction(float(e.attributeDouble("residualFraction", graph.residualFraction())));
  if (e.hasAttribute("dimension"))         graph.setDimension(e.attribute("dimension") == "3D" ? Graph::k3D : Graph::k2D);
  if (e.hasAttribute("azimuth") || e.hasAttribute("elevation"))
    graph.setViewAngles(float(e.attributeDouble("azimuth", graph.azimuth())), float(e.attributeDouble("elevation", graph.elevation())));
  if (e.hasAttribute("palette") || e.hasAttribute("paletteSteps"))
    {
    Palette p = graph.palette();
    Palette::Type type = e.hasAttribute("palette") ? Palette::type(e.attribute("palette")) : p.type();
    graph.setPalette(Palette(type, int(e.attributeInt("paletteSteps", p.nSteps()))));
    }
  if (e.hasAttribute("showColorScale"))    graph.setShowColorScale(e.attributeBool("showColorScale", graph.showColorScale()));
  if (e.hasAttribute("symmetricZ"))        graph.setSymmetricZ(e.attributeBool("symmetricZ", graph.isSymmetricZ()));
  if (e.hasAttribute("colorPalette"))      graph.setColorPaletteName(e.attribute("colorPalette"));
  if (e.hasAttribute("markerPalette"))     graph.setMarkerPaletteName(e.attribute("markerPalette"));
  read(e.child("Margins"), graph.margins());
  read(e.child("TitleText"), graph.titleAttributes());
  for (const XmlElement * a : e.children("Axis"))
    {
    int k = axisIndex(a->attribute("which", ""));
    if (k >= 0) readAxis(*a, graph, k);
    }
  if (const XmlElement * frame = e.child("Frame"))
    {
    read(frame->child("Fill"), graph.frameFill());
    read(frame->child("Line"), graph.frameLine());
    }
  if (const XmlElement * legend = e.child("Legend"))
    {
    if (legend->hasAttribute("show")) graph.setShowLegend(legend->attributeBool("show", graph.showLegend()));
    if (legend->hasAttribute("x") && legend->hasAttribute("y"))
      graph.setLegendPosition(float(legend->attributeDouble("x")), float(legend->attributeDouble("y")));
    else if (legend->attribute("position", "") == "auto") graph.setAutoLegendPosition();
    if (legend->hasAttribute("pointSize")) graph.setLegendPointSize(float(legend->attributeDouble("pointSize", graph.legendPointSize())));
    read(legend->child("Fill"), graph.legendFill());
    read(legend->child("Line"), graph.legendLine());
    }
  // the title last: adding the items gives the graph the title of the first one
  if (e.hasAttribute("title")) graph.setTitle(e.attribute("title"));
  }

  void writeGraph(XmlElement & e, const Graph & graph)
  {
  e.setAttribute("title", graph.title());
  e.setAttribute("legend", graph.showLegend());
  e.setAttribute("titlePointSize", num(graph.titlePointSize()));
  e.setAttribute("titleOffsetX", num(graph.titleOffsetX()));
  e.setAttribute("titleOffsetY", num(graph.titleOffsetY()));
  e.setAttribute("showFits", graph.showFits());
  e.setAttribute("showFitStatistics", graph.showFitStatistics());
  e.setAttribute("showResiduals", graph.showResiduals());
  e.setAttribute("residualMode", graph.residualMode() == Graph::kPull ? "pull" : "difference");
  e.setAttribute("residualFraction", num(graph.residualFraction()));
  e.setAttribute("dimension", graph.dimension() == Graph::k3D ? "3D" : "2D");
  e.setAttribute("azimuth", num(graph.azimuth()));
  e.setAttribute("elevation", num(graph.elevation()));
  e.setAttribute("palette", Palette::name(graph.palette().type()));
  e.setAttribute("paletteSteps", graph.palette().nSteps());
  e.setAttribute("showColorScale", graph.showColorScale());
  e.setAttribute("symmetricZ", graph.isSymmetricZ());
  e.setAttribute("colorPalette", graph.colorPaletteName());
  e.setAttribute("markerPalette", graph.markerPaletteName());
  write(e.addChild("Margins"), graph.margins());
  write(e.addChild("TitleText"), graph.titleAttributes());
  for (int k=0; k<4; k++) writeAxis(e, graph, k);
  XmlElement & frame = e.addChild("Frame");
  write(frame.addChild("Fill"), graph.frameFill());
  write(frame.addChild("Line"), graph.frameLine());
  XmlElement & legend = e.addChild("Legend");
  legend.setAttribute("show", graph.showLegend());
  if (graph.isAutoLegendPosition()) legend.setAttribute("position", "auto");
  else { legend.setAttribute("x", num(graph.legendX())); legend.setAttribute("y", num(graph.legendY())); }
  legend.setAttribute("pointSize", num(graph.legendPointSize()));
  write(legend.addChild("Fill"), graph.legendFill());
  write(legend.addChild("Line"), graph.legendLine());
  }

  void readItem(const XmlElement & e, GraphItem & item)
  {
  if (e.hasAttribute("label"))        item.setLabel(e.attribute("label"));
  if (e.hasAttribute("showLine"))     item.setShowLine(e.attributeBool("showLine", item.showLine()));
  if (e.hasAttribute("showMarkers"))  item.setShowMarkers(e.attributeBool("showMarkers", item.showMarkers()));
  if (e.hasAttribute("showErrors"))   item.setShowErrors(e.attributeBool("showErrors", item.showErrors()));
  if (e.hasAttribute("showSysErrors"))item.setShowSysErrors(e.attributeBool("showSysErrors", item.showSysErrors()));
  if (e.hasAttribute("sysStyle"))
    {
    String s = e.attribute("sysStyle");
    item.setSysStyle(s == "brackets" ? GraphItem::kSysBrackets : (s == "band" ? GraphItem::kSysBand : GraphItem::kSysBoxes));
    }
  if (e.hasAttribute("sysWidth"))     item.setSysWidth(float(e.attributeDouble("sysWidth", item.sysWidth())));
  if (e.hasAttribute("sysUsesMarkerColor")) item.setSysUsesMarkerColor(e.attributeBool("sysUsesMarkerColor", item.sysUsesMarkerColor()));
  if (e.hasAttribute("nSamples"))     item.setNSamples(int(e.attributeInt("nSamples", item.nSamples())));
  if (e.hasAttribute("showFits"))     item.setShowFits(e.attributeBool("showFits", item.showFits()));
  if (e.hasAttribute("extendFits"))   item.setExtendFits(e.attributeBool("extendFits", item.extendFits()));
  if (e.hasAttribute("showColorMap")) item.setShowColorMap(e.attributeBool("showColorMap", item.showColorMap()));
  if (e.hasAttribute("mapSamples"))   item.setMapSamples(int(e.attributeInt("mapSamples", item.mapSamples())));
  if (e.hasAttribute("showContours")) item.setShowContours(e.attributeBool("showContours", item.showContours()));
  if (e.hasAttribute("contourIntervals")) item.setContourIntervals(int(e.attributeInt("contourIntervals", item.contourIntervals())));
  if (e.hasAttribute("contourLevels"))    item.setContourLevels(numbers(e.attribute("contourLevels")));
  if (e.hasAttribute("contourUsesPalette")) item.setContourUsesPalette(e.attributeBool("contourUsesPalette", item.contourUsesPalette()));
  if (e.hasAttribute("showValues"))   item.setShowValues(e.attributeBool("showValues", item.showValues()));
  if (e.hasAttribute("valueFormat"))  item.setValueFormat(e.attribute("valueFormat"));
  if (e.hasAttribute("valuePointSize")) item.setValuePointSize(float(e.attributeDouble("valuePointSize", item.valuePointSize())));
  if (e.hasAttribute("valueColor"))   item.setValueColor(e.attribute("valueColor") == "auto" ? -1 : parseColor(e.attribute("valueColor"), item.valueColor()));
  if (e.hasAttribute("showMapPoints"))    item.setShowMapPoints(e.attributeBool("showMapPoints", item.showMapPoints()));
  if (e.hasAttribute("pointSizeByValue")) item.setPointSizeByValue(e.attributeBool("pointSizeByValue", item.pointSizeByValue()));
  if (e.hasAttribute("pointOutline"))     item.setPointOutline(e.attributeBool("pointOutline", item.pointOutline()));
  if (e.hasAttribute("style3D"))
    {
    String s = e.attribute("style3D");
    item.setStyle3D(s == "mesh" ? GraphItem::kMesh : (s == "lego" ? GraphItem::kLego : GraphItem::kSurface));
    }
  if (e.hasAttribute("showSurfaceLines")) item.setShowSurfaceLines(e.attributeBool("showSurfaceLines", item.showSurfaceLines()));
  if (e.hasAttribute("surfaceSamples"))   item.setSurfaceSamples(int(e.attributeInt("surfaceSamples", item.surfaceSamples())));
  read(e.child("Line"), item.lineAttributes());
  read(e.child("Marker"), item.markerAttributes());
  read(e.child("SysFill"), item.sysFillAttributes());
  read(e.child("SysLine"), item.sysLineAttributes());
  read(e.child("ContourLine"), item.contourLineAttributes());
  }

  void writeItem(XmlElement & e, const GraphItem & item)
  {
  e.setAttribute("label", item.label());
  e.setAttribute("showLine", item.showLine());
  e.setAttribute("showMarkers", item.showMarkers());
  e.setAttribute("showErrors", item.showErrors());
  e.setAttribute("showSysErrors", item.showSysErrors());
  e.setAttribute("sysStyle", sysStyleName(item.sysStyle()));
  e.setAttribute("sysWidth", num(item.sysWidth()));
  e.setAttribute("sysUsesMarkerColor", item.sysUsesMarkerColor());
  e.setAttribute("showFits", item.showFits());
  e.setAttribute("extendFits", item.extendFits());
  if (item.kind() == GraphItem::kFunction) e.setAttribute("nSamples", item.nSamples());
  if (item.isMap())
    {
    e.setAttribute("showColorMap", item.showColorMap());
    e.setAttribute("mapSamples", item.mapSamples());
    e.setAttribute("showContours", item.showContours());
    e.setAttribute("contourIntervals", item.contourIntervals());
    if (!item.contourLevels().empty()) e.setAttribute("contourLevels", numberList(item.contourLevels()));
    e.setAttribute("contourUsesPalette", item.contourUsesPalette());
    e.setAttribute("showValues", item.showValues());
    e.setAttribute("valueFormat", item.valueFormat());
    e.setAttribute("valuePointSize", num(item.valuePointSize()));
    e.setAttribute("valueColor", item.valueColor() < 0 ? String("auto") : colorName(item.valueColor()));
    e.setAttribute("showMapPoints", item.showMapPoints());
    e.setAttribute("pointSizeByValue", item.pointSizeByValue());
    e.setAttribute("pointOutline", item.pointOutline());
    e.setAttribute("style3D", style3DName(item.style3D()));
    e.setAttribute("showSurfaceLines", item.showSurfaceLines());
    e.setAttribute("surfaceSamples", item.surfaceSamples());
    write(e.addChild("ContourLine"), item.contourLineAttributes());
    }
  write(e.addChild("Line"), item.lineAttributes());
  write(e.addChild("Marker"), item.markerAttributes());
  write(e.addChild("SysFill"), item.sysFillAttributes());
  write(e.addChild("SysLine"), item.sysLineAttributes());
  }

  void readCanvas(const XmlElement & e, Canvas & canvas)
  {
  canvas.setSize(int(e.attributeInt("width", canvas.width())), int(e.attributeInt("height", canvas.height())));
  read(e.child("Background"), canvas.background());
  }

  void writeCanvas(XmlElement & e, const Canvas & canvas)
  {
  e.setAttribute("width", canvas.width());
  e.setAttribute("height", canvas.height());
  e.setAttribute("columns", canvas.nColumns());
  e.setAttribute("rows", canvas.nRows());
  write(e.addChild("Background"), canvas.background());
  }
  } // namespace GraphXml

  // ------------------------------------------------------------------------------------------------ PlotSpec

  PlotSpec::PlotSpec() { }

  PlotSpec::~PlotSpec() { }

  bool PlotSpec::read(const String & fileName, String & error)
  {
  try
    {
    XmlDocument document = XmlDocument::read(fileName);
    _fileName = fileName;
    return read(document, FileSystem::parent(fileName), error);
    }
  catch (const std::exception & e) { error = String(e.what()); }
  catch (...) { error = "cannot read the file"; }
  return false;
  }

  bool PlotSpec::read(const XmlDocument & document, const String & directory, String & error)
  {
  const XmlElement & root = document.root();
  if (!root.isNamed("PlotHistos")) { error = "the root element is not PlotHistos"; return false; }
  _document = document;
  _directory = directory;
  _inputs.clear();
  _windows.clear();
  _output = OutputSpec();
  for (const XmlElement * e : root.children("Input"))
    {
    InputSpec input;
    input.path = e->attribute("file", "");
    input.id   = e->attribute("id", String("input") + String(std::to_string(_inputs.size() + 1)));
    _inputs.push_back(input);
    }
  if (const XmlElement * e = root.child("Output"))
    {
    _output.directory    = e->attribute("directory", _output.directory);
    _output.pdf          = e->attribute("pdf", _output.pdf);
    _output.pptx         = e->attribute("pptx", _output.pptx);
    _output.latex        = e->attribute("latex", _output.latex);
    _output.latexStyle   = e->attribute("latexStyle", _output.latexStyle);
    _output.latexCompile = e->attributeBool("latexCompile", _output.latexCompile);
    _output.title        = e->attribute("title", _output.title);
    _output.author       = e->attribute("author", _output.author);
    _output.affiliation  = e->attribute("affiliation", _output.affiliation);
    }
  int nCanvases = 0;
  std::vector<String> names;
  for (const XmlElement * w : root.children("Window"))
    {
    WindowSpec window;
    window.title   = w->attribute("title", String("Window ") + String(std::to_string(_windows.size() + 1)));
    window.width   = int(w->attributeInt("width", window.width));
    window.height  = int(w->attributeInt("height", window.height));
    std::vector<const XmlElement *> canvases = w->children("Canvas");
    int n = std::max(1, int(canvases.size()));
    window.columns = int(w->attributeInt("columns", 0));
    window.rows    = int(w->attributeInt("rows", 0));
    if (window.columns < 1 && window.rows < 1) { window.columns = int(std::ceil(std::sqrt(double(n)))); }
    if (window.columns < 1) window.columns = (n + window.rows - 1)/window.rows;
    if (window.rows < 1)    window.rows = (n + window.columns - 1)/window.columns;
    for (const XmlElement * c : canvases)
      {
      CanvasSpec canvas;
      canvas.element = c;
      canvas.name  = c->attribute("name", String("canvas") + String(std::to_string(++nCanvases)));
      // names unique among the canvases
      String base = canvas.name;
      for (int k=2; std::find(names.begin(), names.end(), canvas.name) != names.end(); k++)
        canvas.name = base + String("_") + String(std::to_string(k));
      names.push_back(canvas.name);
      canvas.title = c->attribute("title", canvas.name);
      window.canvases.push_back(canvas);
      }
    _windows.push_back(window);
    }
  // the elements point into _document: the copy, not the argument
  int iw = 0;
  for (const XmlElement * w : _document.root().children("Window"))
    {
    int ic = 0;
    for (const XmlElement * c : w->children("Canvas")) _windows[iw].canvases[ic++].element = c;
    iw++;
    }
  if (_windows.empty()) { error = "no Window element"; return false; }
  return true;
  }

  String PlotSpec::resolve(const String & path) const
  {
  String p = FileSystem::expand(path);
  if (p.isNull() || p.str()[0] == '/' || _directory.isNull()) return p;
  return FileSystem::join(_directory, p);
  }

  String PlotSpec::outputPath(const String & name) const
  {
  String directory = resolve(_output.directory.isNull() ? String(".") : _output.directory);
  if (name.isNull()) return directory;
  if (name.str()[0] == '/') return name;
  return FileSystem::join(directory, name);
  }

  void PlotSpec::setInputPath(int k, const String & path)
  {
  if (k < 0) return;
  if (k >= int(_inputs.size()))
    {
    InputSpec input;
    input.id = String("input") + String(std::to_string(_inputs.size() + 1));
    _inputs.push_back(input);
    k = int(_inputs.size()) - 1;
    }
  _inputs[k].path = path;
  _inputs[k].file.reset();
  }

  void PlotSpec::setInputFile(int k, const std::shared_ptr<CapFile> & file)
  {
  if (k >= 0 && k < int(_inputs.size())) _inputs[k].file = file;
  }

  bool PlotSpec::openInputs(String & error)
  {
  for (auto & input : _inputs)
    {
    if (input.file) continue;
    String path = resolve(input.path);
    try
      {
      auto file = std::make_shared<CapFile>(path, File::kReadOnly);
      file->load(File::Loading::kLazy);
      input.file = file;
      }
    catch (const std::exception & e) { error = path + String(": ") + String(e.what()); return false; }
    catch (...) { error = path + String(": cannot be read"); return false; }
    }
  return true;
  }

  int PlotSpec::inputIndex(const String & id) const
  {
  if (_inputs.empty()) return -1;
  if (id.isNull()) return 0;
  for (std::size_t k=0; k<_inputs.size(); k++) if (_inputs[k].id == id) return int(k);
  return -1;
  }

  void PlotSpec::addItems(const XmlElement & element, Graph & graph, String & problems)
  {
  for (const XmlElement * e : element.children())
    {
    GraphItem * added = nullptr;
    if (isDataElement(*e))
      {
      String id = e->attribute("input", ""), path = e->attribute("path", "");
      int k = inputIndex(id);
      if (k < 0) { problems += String("no input ") + id + String(" for ") + path + String("\n"); continue; }
      const InputSpec & input = _inputs[k];
      if (!input.file) { problems += String("input ") + input.id + String(" not open: ") + path + String("\n"); continue; }
      const MATH::DataPoints * data = nullptr;
      try { data = input.file->get<MATH::DataPoints>(path); }
      catch (const std::exception & ex) { problems += path + String(": ") + String(ex.what()) + String("\n"); continue; }
      if (!data) { problems += path + String(": not found in ") + input.path + String("\n"); continue; }
      const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram *>(data);
      unsigned int rank = histogram ? histogram->rank() : data->rank();
      if (rank > 2) { problems += path + String(": rank 3 and more cannot be drawn\n"); continue; }
      _dataNames[data] = std::make_pair(input.id, path);
      String label = data->name().isNull() ? path : data->name();
      added = &graph.show(GraphItem(data, label), false);
      }
    else if (e->isNamed("Function"))
      {
      String className = e->attribute("class", "");
      std::vector<double> values = numbers(e->attribute("parameters", ""));
      if (!MATH::FunctionRegistry::isRegistered(className) && !MATH::FunctionRegistry::isComposite(className))
        {
        String composites = MATH::FunctionRegistry::defaultCompositesFile();
        if (FileSystem::isFile(composites)) MATH::FunctionRegistry::readComposites(composites);
        }
      MATH::ParameterizedFunction * f = nullptr;
      if (MATH::FunctionRegistry::isComposite(className)) f = MATH::FunctionRegistry::composite(className)->clone();
      else if (!values.empty()) f = MATH::FunctionRegistry::create(className, (unsigned int)values.size());
      else f = MATH::FunctionRegistry::create(className);
      if (!f) { problems += String("no function ") + className + String(" with ") + String(std::to_string(values.size())) + String(" parameters\n"); continue; }
      if (!values.empty())
        {
        if (values.size() != f->nParameters()) { problems += className + String(": wrong number of parameters\n"); delete f; continue; }
        try { f->setParameterValues(values); }
        catch (const std::exception & ex) { problems += className + String(": ") + String(ex.what()) + String("\n"); delete f; continue; }
        }
      if (e->hasAttribute("name")) f->setName(e->attribute("name"));
      _functions.push_back(FunctionRecord{std::unique_ptr<MATH::ParameterizedFunction>(f), className});
      float xMin = float(e->attributeDouble("xMin", 0.0)), xMax = float(e->attributeDouble("xMax", 1.0));
      String label = e->attribute("label", className);
      if (f->rank() == 2)
        added = &graph.show(GraphItem(f, xMin, xMax, float(e->attributeDouble("yMin", 0.0)), float(e->attributeDouble("yMax", 1.0)), label), false);
      else if (f->rank() == 1)
        added = &graph.show(GraphItem(f, xMin, xMax, label), false);
      else { problems += className + String(": functions of rank 3 and more cannot be drawn\n"); continue; }
      }
    if (added) GraphXml::readItem(*e, *added);
    }
  }

  void PlotSpec::build(const CanvasSpec & spec, Canvas & canvas, String & problems)
  {
  const XmlElement & e = *spec.element;
  canvas.setName(spec.name);
  canvas.setTitle(spec.title);
  GraphXml::readCanvas(e, canvas);
  std::vector<const XmlElement *> graphs = e.children("Graph");
  int columns = int(e.attributeInt("columns", 0)), rows = int(e.attributeInt("rows", 0));
  int n = std::max(1, int(graphs.size()));
  if (columns < 1 && rows < 1) canvas.divideFor(n);
  else
    {
    if (columns < 1) columns = (n + rows - 1)/rows;
    if (rows < 1)    rows = (n + columns - 1)/columns;
    canvas.divide(columns, rows);
    }
  for (int k=0; k<canvas.nGraphs(); k++) { canvas.graph(k).clearItems(); canvas.graph(k).clearShapes(); }
  CanvasInfo info;
  info.name = spec.name;
  info.file = e.attribute("file", "");
  info.graphFiles.assign(std::size_t(canvas.nGraphs()), String(""));
  info.width = canvas.width();
  info.height = canvas.height();
  int next = 0;
  for (const XmlElement * g : graphs)
    {
    int k = g->hasAttribute("cell") ? int(g->attributeInt("cell")) : next;
    next = k + 1;
    if (k < 0 || k >= canvas.nGraphs()) { problems += spec.name + String(": no cell ") + String(std::to_string(k)) + String("\n"); continue; }
    Graph & graph = canvas.graph(k);
    addItems(*g, graph, problems);
    GraphXml::readGraph(*g, graph);
    info.graphFiles[std::size_t(k)] = g->attribute("file", "");
    }
  _canvasInfo[&canvas] = info;
  }

  std::vector<PlotSpec::WindowState> PlotSpec::buildAll(String & problems)
  {
  std::vector<WindowState> states;
  for (const auto & window : _windows)
    {
    WindowState state;
    state.title = window.title; state.width = window.width; state.height = window.height;
    state.columns = window.columns; state.rows = window.rows;
    for (const auto & spec : window.canvases)
      {
      _ownedCanvases.emplace_back(new Canvas(spec.name, spec.title));
      build(spec, *_ownedCanvases.back(), problems);
      state.canvases.push_back(_ownedCanvases.back().get());
      }
    states.push_back(state);
    }
  return states;
  }

  const PlotSpec::CanvasInfo * PlotSpec::canvasInfo(const Canvas & canvas) const
  {
  auto found = _canvasInfo.find(&canvas);
  return found == _canvasInfo.end() ? nullptr : &found->second;
  }

  void PlotSpec::writeItems(XmlElement & element, const Graph & graph) const
  {
  for (const auto & item : graph.items())
    {
    if (item.kind() == GraphItem::kData)
      {
      String input, path;
      auto found = _dataNames.find(item.data());
      if (found != _dataNames.end()) { input = found->second.first; path = found->second.second; }
      else if (!locateData || !locateData(item.data(), input, path)) continue;
      XmlElement & e = element.addChild("Histogram");
      e.setAttribute("input", input);
      e.setAttribute("path", path);
      GraphXml::writeItem(e, item);
      }
    else if (item.function())
      {
      const MATH::ParameterizedFunction * f = dynamic_cast<const MATH::ParameterizedFunction *>(item.function());
      if (!f) continue;
      String className;
      for (const auto & record : _functions) if (record.function.get() == f) className = record.className;
      if (className.isNull()) className = MATH::FunctionRegistry::typeName(*f);
      XmlElement & e = element.addChild("Function");
      e.setAttribute("class", className);
      e.setAttribute("parameters", numberList(f->parameterValues()));
      e.setAttribute("xMin", num(item.xMinimum()));
      e.setAttribute("xMax", num(item.xMaximum()));
      if (item.isMap()) { e.setAttribute("yMin", num(item.yMinimum())); e.setAttribute("yMax", num(item.yMaximum())); }
      GraphXml::writeItem(e, item);
      }
    }
  }

  XmlDocument PlotSpec::toXml(const std::vector<WindowState> & windows, const String & directory) const
  {
  bool relocated = !directory.isNull() && FileSystem::normalize(directory) != FileSystem::normalize(_directory.isNull() ? String(".") : _directory);
  auto path = [&](const String & p)
    {
    if (!relocated || p.isNull()) return p;
    String r = resolve(p);
    if (r.str()[0] != '/') r = FileSystem::join(FileSystem::current(), r);
    return FileSystem::normalize(r);
    };
  XmlDocument document("PlotHistos");
  XmlElement & root = document.root();
  root.setAttribute("version", 1);
  for (const auto & input : _inputs)
    {
    XmlElement & e = root.addChild("Input");
    e.setAttribute("id", input.id);
    e.setAttribute("file", path(input.path));
    }
  XmlElement & output = root.addChild("Output");
  output.setAttribute("directory", path(_output.directory.isNull() ? String(".") : _output.directory));
  output.setAttribute("pdf", _output.pdf);
  output.setAttribute("pptx", _output.pptx);
  output.setAttribute("latex", _output.latex);
  output.setAttribute("latexStyle", _output.latexStyle);
  output.setAttribute("latexCompile", _output.latexCompile);
  output.setAttribute("title", _output.title);
  output.setAttribute("author", _output.author);
  output.setAttribute("affiliation", _output.affiliation);
  for (const auto & window : windows)
    {
    XmlElement & w = root.addChild("Window");
    w.setAttribute("title", window.title);
    w.setAttribute("width", window.width);
    w.setAttribute("height", window.height);
    w.setAttribute("columns", window.columns);
    w.setAttribute("rows", window.rows);
    for (const Canvas * canvas : window.canvases)
      {
      const CanvasInfo * info = canvasInfo(*canvas);
      XmlElement & c = w.addChild("Canvas");
      String name = info ? info->name : (canvas->registry() ? canvas->registry()->nameOf(*canvas) : canvas->name());
      c.setAttribute("name", name);
      c.setAttribute("title", canvas->title().isNull() ? name : canvas->title());
      if (info && !info->file.isNull()) c.setAttribute("file", info->file);
      GraphXml::writeCanvas(c, *canvas);
      if (info && info->width > 0 && info->height > 0) { c.setAttribute("width", info->width); c.setAttribute("height", info->height); }
      for (int k=0; k<canvas->nGraphs(); k++)
        {
        const Graph & graph = canvas->graph(k);
        XmlElement & g = c.addChild("Graph");
        g.setAttribute("cell", k);
        if (info && k < int(info->graphFiles.size()) && !info->graphFiles[std::size_t(k)].isNull())
          g.setAttribute("file", info->graphFiles[std::size_t(k)]);
        GraphXml::writeGraph(g, graph);
        writeItems(g, graph);
        }
      }
    }
  return document;
  }

  XmlElement PlotSpec::canvasXml(const Canvas & canvas) const
  {
  WindowState window;
  window.canvases.push_back(&canvas);
  XmlDocument document = toXml({ window });
  const XmlElement * w = document.root().child("Window");
  const XmlElement * c = w ? w->child("Canvas") : nullptr;
  return c ? *c : XmlElement("Canvas");
  }

  bool PlotSpec::write(const String & fileName, const std::vector<WindowState> & windows, String & error) const
  {
  try { toXml(windows, FileSystem::parent(fileName)).write(fileName); return true; }
  catch (const std::exception & e) { error = String(e.what()); }
  catch (...) { error = "cannot write the file"; }
  return false;
  }

  std::vector<String> PlotSpec::print(const std::vector<const Canvas *> & canvases) const
  {
  std::vector<String> written;
  String directory = outputPath("");
  if (!directory.isNull() && !FileSystem::isDirectory(directory)) FileSystem::createDirectory(directory);
  auto writeOne = [&](const String & name, const Canvas & canvas, int graph)
    {
    String path = outputPath(name);
    String parent = FileSystem::parent(path);
    if (!parent.isNull() && !FileSystem::isDirectory(parent)) FileSystem::createDirectory(parent);
    bool pdf = FileSystem::hasExtension(path, ".pdf");
    float w = float(canvas.width()), h = float(canvas.height());
    bool ok;
    if (graph < 0)
      {
      if (pdf) { PdfPainter p(w, h); canvas.draw(p); ok = p.write(path); }
      else     { SvgPainter p(w, h); canvas.draw(p); ok = p.write(path); }
      }
    else
      {
      const Graph & g = canvas.graph(graph);
      float gw = (g.cellXHigh() - g.cellXLow())*w, gh = (g.cellYHigh() - g.cellYLow())*h;
      if (pdf) { PdfPainter p(gw, gh); CanvasRegistry::drawGraph(p, canvas, graph, w, h); ok = p.write(path); }
      else     { SvgPainter p(gw, gh); CanvasRegistry::drawGraph(p, canvas, graph, w, h); ok = p.write(path); }
      }
    if (ok) written.push_back(path);
    };
  std::vector<std::unique_ptr<Canvas>> copies;
  std::vector<const Canvas *> sized = atOutputSize(canvases, copies);
  for (std::size_t n=0; n<canvases.size(); n++)
    {
    const CanvasInfo * info = canvasInfo(*canvases[n]);
    if (!info) continue;
    const Canvas & canvas = *sized[n];
    if (!info->file.isNull()) writeOne(info->file, canvas, -1);
    for (int k=0; k<canvas.nGraphs() && k<int(info->graphFiles.size()); k++)
      if (!info->graphFiles[std::size_t(k)].isNull()) writeOne(info->graphFiles[std::size_t(k)], canvas, k);
    }
  return written;
  }

  std::vector<const Canvas *> PlotSpec::atOutputSize(const std::vector<const Canvas *> & canvases,
                                                     std::vector<std::unique_ptr<Canvas>> & copies) const
  {
  std::vector<const Canvas *> sized;
  for (const Canvas * canvas : canvases)
    {
    const CanvasInfo * info = canvasInfo(*canvas);
    if (info && info->width > 0 && info->height > 0 && (info->width != canvas->width() || info->height != canvas->height()))
      {
      copies.emplace_back(new Canvas(*canvas));
      copies.back()->setSize(info->width, info->height);
      sized.push_back(copies.back().get());
      }
    else sized.push_back(canvas);
    }
  return sized;
  }

  bool PlotSpec::writePdf(const String & fileName, const std::vector<const Canvas *> & canvases)
  {
  std::unique_ptr<PdfPainter> painter;
  for (const Canvas * canvas : canvases)
    {
    float w = float(canvas->width()), h = float(canvas->height());
    if (!painter) painter.reset(new PdfPainter(w, h));
    else painter->newPage(w, h);
    canvas->draw(*painter);
    }
  return painter && painter->write(fileName);
  }

  } // namespace GRAPHICS
} // namespace CAP

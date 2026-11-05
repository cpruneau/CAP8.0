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
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include "CapGuiApp.hpp"
#include "PlotOutputs.hpp"
#include "CapFile.hpp"
#include "ColorTable.hpp"
#include "FileSystem.hpp"
#include "FunctionRegistry.hpp"
#include "GraphEditor.hpp"
#include "ImGuiPainter.hpp"
#include "Palettes.hpp"
#include "XmlDocument.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

namespace CAP
{
  namespace GUI
  {
  using GRAPHICS::PlotDocument;
  using GRAPHICS::PlotSpec;
  using Path = PlotDocument::Path;
  namespace GraphXml = GRAPHICS::GraphXml;

  namespace
  {
  const char * kEditorWindow = "Plot Tree";
  const char * kNodePayload = "CAP_PLOT_NODE";

  std::string pathKey(int tab, const Path & path)
  {
  std::string s = std::to_string(tab) + ":";
  for (std::size_t k=0; k<path.size(); k++) s += (k ? "." : "") + std::to_string(path[k]);
  return s;
  }

  bool parsePathKey(const std::string & key, int & tab, Path & path)
  {
  std::size_t colon = key.find(':');
  if (colon == std::string::npos) return false;
  try
    {
    tab = std::stoi(key.substr(0, colon));
    path.clear();
    std::string rest = key.substr(colon + 1);
    std::size_t start = 0;
    while (start < rest.size())
      {
      std::size_t dot = rest.find('.', start);
      if (dot == std::string::npos) dot = rest.size();
      path.push_back(std::stoi(rest.substr(start, dot - start)));
      start = dot + 1;
      }
    }
  catch (...) { return false; }
  return true;
  }

  ImVec4 kindColor(PlotDocument::Kind kind)
  {
  bool dark = ImGui::GetStyle().Colors[ImGuiCol_WindowBg].x < 0.5f;
  switch (kind)
    {
    case PlotDocument::kWindow:    return dark ? ImVec4(0.45f, 0.70f, 1.00f, 1) : ImVec4(0.10f, 0.35f, 0.75f, 1);
    case PlotDocument::kCanvas:    return dark ? ImVec4(0.45f, 0.85f, 0.50f, 1) : ImVec4(0.10f, 0.50f, 0.15f, 1);
    case PlotDocument::kGraph:     return dark ? ImVec4(1.00f, 0.70f, 0.35f, 1) : ImVec4(0.75f, 0.40f, 0.00f, 1);
    case PlotDocument::kHistogram: return dark ? ImVec4(0.90f, 0.90f, 0.90f, 1) : ImVec4(0.10f, 0.10f, 0.10f, 1);
    case PlotDocument::kFunction:  return dark ? ImVec4(0.85f, 0.60f, 1.00f, 1) : ImVec4(0.50f, 0.15f, 0.65f, 1);
    default:                       return ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
    }
  }

  const char * kindTag(PlotDocument::Kind kind)
  {
  switch (kind)
    {
    case PlotDocument::kRoot:      return "Plots";
    case PlotDocument::kInput:     return "Input";
    case PlotDocument::kOutput:    return "Output";
    case PlotDocument::kWindow:    return "Window";
    case PlotDocument::kCanvas:    return "Canvas";
    case PlotDocument::kGraph:     return "Graph";
    case PlotDocument::kHistogram: return "Hist";
    case PlotDocument::kFunction:  return "Func";
    case PlotDocument::kStyle:     return "Style";
    default:                       return "";
    }
  }

  std::string number(double v)
  {
  return XmlElement::formatNumber(v).str();
  }

  //! A text attribute of an element
  bool textField(PlotDocument & d, const Path & p, const char * attribute, const char * label, float width=-1, const char * hint=nullptr)
  {
  const XmlElement * e = d.element(p);
  if (!e) return false;
  std::string value = e->attribute(attribute, "").str();
  ImGui::SetNextItemWidth(width > 0 ? width : ImGui::GetContentRegionAvail().x*0.6f);
  bool edited = hint ? ImGui::InputTextWithHint(label, hint, &value) : ImGui::InputText(label, &value);
  return edited && d.setAttribute(p, attribute, String(value));
  }

  //! A number attribute (an empty field: the default)
  bool numberField(PlotDocument & d, const Path & p, const char * attribute, const char * label, double fallback,
                   double step=0.0, const char * format="%.6g", float width=130)
  {
  const XmlElement * e = d.element(p);
  if (!e) return false;
  double value = e->attributeDouble(attribute, fallback);
  ImGui::SetNextItemWidth(width);
  if (!ImGui::InputDouble(label, &value, step, step*10, format)) return false;
  return d.setAttribute(p, attribute, String(number(value)));
  }

  bool intField(PlotDocument & d, const Path & p, const char * attribute, const char * label, int fallback, int low, int high,
                float width=110)
  {
  const XmlElement * e = d.element(p);
  if (!e) return false;
  int value = int(e->attributeInt(attribute, fallback));
  ImGui::SetNextItemWidth(width);
  if (!ImGui::InputInt(label, &value)) return false;
  value = std::max(low, std::min(high, value));
  return d.setAttribute(p, attribute, String(std::to_string(value)));
  }

  bool checkField(PlotDocument & d, const Path & p, const char * attribute, const char * label, bool fallback)
  {
  const XmlElement * e = d.element(p);
  if (!e) return false;
  bool value = e->attributeBool(attribute, fallback);
  if (!ImGui::Checkbox(label, &value)) return false;
  return d.setAttribute(p, attribute, String(value ? "true" : "false"));
  }

  //! A grid of columns x rows chosen by the mouse (in a popup). @return true if chosen
  bool gridPicker(int & columns, int & rows, int maxColumns=6, int maxRows=6)
  {
  static int hoverC = 0, hoverR = 0;
  const float s = ImGui::GetFrameHeight();
  ImVec2 origin = ImGui::GetCursorScreenPos();
  ImDrawList * draw = ImGui::GetWindowDrawList();
  ImGui::InvisibleButton("##grid", ImVec2(maxColumns*(s + 3), maxRows*(s + 3)));
  bool chosen = false;
  if (ImGui::IsItemHovered())
    {
    ImVec2 m = ImGui::GetMousePos();
    hoverC = std::max(1, std::min(maxColumns, int((m.x - origin.x)/(s + 3)) + 1));
    hoverR = std::max(1, std::min(maxRows,    int((m.y - origin.y)/(s + 3)) + 1));
    if (ImGui::IsItemClicked()) { columns = hoverC; rows = hoverR; chosen = true; }
    }
  else { hoverC = columns; hoverR = rows; }
  for (int r=0; r<maxRows; r++)
    for (int c=0; c<maxColumns; c++)
      {
      ImVec2 a(origin.x + c*(s + 3), origin.y + r*(s + 3)), b(a.x + s, a.y + s);
      bool on = c < hoverC && r < hoverR;
      draw->AddRectFilled(a, b, ImGui::GetColorU32(on ? ImGuiCol_ButtonActive : ImGuiCol_FrameBg), 2.0f);
      draw->AddRect(a, b, ImGui::GetColorU32(ImGuiCol_Border), 2.0f);
      }
  ImGui::Text("%d x %d", hoverC, hoverR);
  return chosen;
  }

  //! A grid pull-down: "c x r" button opening the grid picker; writes columns and rows
  bool gridControl(PlotDocument & d, const Path & p, const char * id, const char * tooltip)
  {
  const XmlElement * e = d.element(p);
  if (!e) return false;
  int columns = int(e->attributeInt("columns", 0)), rows = int(e->attributeInt("rows", 0));
  char text[48];
  if (columns > 0 || rows > 0) std::snprintf(text, sizeof(text), "%d x %d##%s", std::max(columns, 1), std::max(rows, 1), id);
  else std::snprintf(text, sizeof(text), "auto##%s", id);
  if (ImGui::SmallButton(text)) ImGui::OpenPopup(id);
  ImGui::SetItemTooltip("%s", tooltip);
  bool changed = false;
  if (ImGui::BeginPopup(id))
    {
    ImGui::TextDisabled("%s", tooltip);
    int c = std::max(columns, 1), r = std::max(rows, 1);
    if (gridPicker(c, r))
      {
      d.beginEdit();
      changed |= d.setAttribute(p, "columns", String(std::to_string(c)));
      changed |= d.setAttribute(p, "rows", String(std::to_string(r)));
      d.endEdit();
      ImGui::CloseCurrentPopup();
      }
    if (ImGui::Button("Automatic"))
      {
      d.beginEdit();
      changed |= d.setAttribute(p, "columns", "");
      changed |= d.setAttribute(p, "rows", "");
      d.endEdit();
      ImGui::CloseCurrentPopup();
      }
    ImGui::SetItemTooltip("As many cells as needed");
    ImGui::EndPopup();
    }
  return changed;
  }

  //! Index of an item among the items of its graph, and the graph's element
  int itemIndex(const PlotDocument & d, const Path & item, const XmlElement *& graph)
  {
  Path g = PlotDocument::parent(item);
  graph = d.element(g);
  int k = 0;
  if (!graph || item.empty()) return 0;
  for (int j=0; j<item.back() && j<int(graph->nChildren()); j++)
    {
    PlotDocument::Kind kind = PlotDocument::kind(graph->child(unsigned(j)));
    if (kind == PlotDocument::kHistogram || kind == PlotDocument::kFunction) k++;
    }
  return k;
  }

  //! Color of an item as drawn: its own, or that of the palette of its graph
  int itemColor(const PlotDocument & d, const Path & item, const char * child)
  {
  String own = d.style(item, child, "color");
  if (!own.isNull()) return GraphXml::parseColor(own, 1);
  const XmlElement * graph = nullptr;
  int k = itemIndex(d, item, graph);
  String palette = graph ? graph->attribute("colorPalette", "") : String("");
  return GRAPHICS::Palettes::session().colorPaletteOr(palette).color(k);
  }

  int itemMarker(const PlotDocument & d, const Path & item)
  {
  String own = d.style(item, "Marker", "style");
  if (!own.isNull()) return std::atoi(own.data());
  const XmlElement * graph = nullptr;
  int k = itemIndex(d, item, graph);
  String palette = graph ? graph->attribute("markerPalette", "") : String("");
  return GRAPHICS::Palettes::session().markerPaletteOr(palette).marker(k);
  }

  //! A marker as a small button showing it, opening the list of the markers
  bool markerButton(const char * id, int & style, int color)
  {
  float h = ImGui::GetFrameHeight();
  ImVec2 p = ImGui::GetCursorScreenPos();
  ImGui::PushID(id);
  bool clicked = ImGui::Button("##marker", ImVec2(h, h));
  ImGui::SetItemTooltip("marker: click to choose");
  ImGuiPainter painter(ImGui::GetWindowDrawList(), nullptr, p.x, p.y, h, h);
  GRAPHICS::MarkerAttributes marker;
  marker.set(color, 1.0, style, 1.2);
  painter.drawMarker(GRAPHICS::Point2D(0.5, 0.5), marker);
  if (clicked) ImGui::OpenPopup("markers");
  bool changed = false;
  if (ImGui::BeginPopup("markers"))
    {
    changed = GraphEditor::markerEdit("marker", style, color);
    if (changed) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    }
  ImGui::PopID();
  return changed;
  }

  //! A line style as a small button, opening the list of the styles
  bool lineButton(const char * id, int & style)
  {
  static const char * shown[] = { "none", "____", "- - -", ".....", "-.-.-" };
  ImGui::PushID(id);
  bool clicked = ImGui::SmallButton(shown[(style >= 0 && style <= 4) ? style : 1]);
  ImGui::SetItemTooltip("line style: click to choose");
  if (clicked) ImGui::OpenPopup("lines");
  bool changed = false;
  if (ImGui::BeginPopup("lines"))
    {
    changed = GraphEditor::lineStyleEdit("line style", style, false);
    if (changed) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    }
  ImGui::PopID();
  return changed;
  }

  //! A color of a child of style, as a swatch with the ROOT colors
  bool colorStyle(PlotDocument & d, const Path & p, const char * child, const char * label, int shown,
                  const char * which="", const char * also=nullptr)
  {
  int color = shown;
  if (!GraphEditor::colorEdit(label, color)) return false;
  d.beginEdit();
  bool changed = d.setStyle(p, child, "color", GraphXml::colorName(color), which);
  if (also) changed |= d.setStyle(p, also, "color", GraphXml::colorName(color), which);
  d.endEdit();
  return changed;
  }

  void helpText()
  {
  ImGui::TextWrapped("Every CapGui session is a plot file (the XML files of PlotHistos): windows of canvases, canvases of "
                     "graphs, graphs of histograms and functions. The canvases on screen are the windows of the plot file "
                     "shown in the Plot Tree; what is changed on them is written to the file, and what is changed in the "
                     "tree draws them again.");
  ImGui::Spacing();
  ImGui::BulletText("File > New, Open, Close, Duplicate, Save act on plot files (a tab each in the Plot Tree);\n"
                    "Open Data and Open Data Folder show CAP files in the browser; File > Export writes figures,\n"
                    "the PDF book, the PowerPoint presentation and the LaTeX document.");
  ImGui::BulletText("Drag a histogram from the browser onto a graph of a canvas or onto the tree: on a graph it is\n"
                    "overlaid; on a histogram it goes after it; on a canvas it gets a new graph; on a window a new\n"
                    "canvas. A folder adds all its histograms. Clicking a histogram in the browser adds it to the\n"
                    "selected graph.");
  ImGui::BulletText("The selection is shared: click a graph on a canvas, its row is selected in the tree, and the other\n"
                    "way around. Edit > Copy, Cut, Paste, Duplicate, Delete act on it; Copy puts its XML on the clipboard.");
  ImGui::BulletText("Window > Canvases and Graphs in Canvas set the grids of the window and of the canvas in the file.");
  ImGui::BulletText("View > All Windows shows every window of the plot file; otherwise the window of the selection.");
  ImGui::BulletText("Shapes drawn on the canvases and fits are shown, not written to the plot file.");
  ImGui::BulletText("PlotHistos plots.xml opens CapGui on a plot file; PlotHistos --all plots.xml writes its outputs.");
  }
  } // namespace

  // ------------------------------------------------------------------------------------------------ application

  CapGuiApp::CapGuiApp(Mode mode, const std::vector<String> & plotFiles, int argc, char ** argv)
  :
  CapApp(argc, argv),
  _mode(mode),
  _plotFiles(plotFiles)
  { }

  CapGuiApp::~CapGuiApp()
  {
  // the canvases refer to the functions of the specification: emptied first
  for (int k=0; k<_gui.nPanes(); k++) _gui.pane(k).canvas() = GRAPHICS::Canvas();
  }

  void CapGuiApp::buildMenus()
  {
  MenuBar & bar = _gui.menuBar();
  Menu & file = bar.addMenu("File");
  file.addItem("New Plot File",     "plot.new",       "Ctrl+N",       "A new, untitled plot file (a new tab of the Plot Tree)");
  file.addItem("Open Plot File...", "plot.open",      "Ctrl+O",       "Open a plot file (PlotHistos XML) in a new tab");
  file.addItem("New from Data...",  "plot.fromData",  "",             "A new plot file listing the histograms of a CAP file or of a RunCAP workflow: choose them and their layout");
  file.addItem("Close Plot File",   "plot.close",     "Ctrl+W",       "Close the plot file shown (asks to save its changes)");
  file.addItem("Duplicate Plot File", "plot.duplicate", "",           "A copy of the plot file shown, in a new tab (not saved yet)");
  file.addSeparator();
  file.addItem("Save",              "plot.save",      "Ctrl+S",       "Save the plot file shown");
  file.addItem("Save As...",        "plot.saveAs",    "Ctrl+Shift+S", "Save the plot file shown under a new name");
  file.addItem("Save All",          "plot.saveAll",   "",             "Save all the plot files changed");
  file.addSeparator();
  file.addItem("Open Data...",        "file.open",       "Ctrl+Shift+O", "Open a CAP file (or the histograms of a ROOT file) in the browser");
  file.addItem("Open Data Folder...", "file.openFolder", "",             "Show a folder of CAP files in the browser");
  file.addItem("Close All Data",      "file.closeAll",   "",             "Close the data files open in the browsers");
  file.addItem("New Browser",         "file.newBrowser", "",             "Open another file browser, with files of its own");
  file.addItem("New Tree",            "file.newTree",    "",             "Make a new, empty tree of histograms in the browser: drag objects onto it, save it as a CAP file");
  file.addSeparator();
  Menu & exporting = file.addMenu("Export", "Figures and documents made from the canvases");
  exporting.addItem("Window As...",  "file.saveAs",     "", "The canvases of the main window, laid out as on screen, to an SVG or PDF file");
  exporting.addItem("Canvas As...",  "file.saveCanvas", "", "The current canvas to an SVG or PDF file");
  exporting.addItem("Graph As...",   "file.saveGraph",  "", "The selected graph of the current canvas to an SVG or PDF file");
  exporting.addSeparator();
  exporting.addItem("Named Plots",   "plot.print",  "", "Each canvas and graph of the plot file that has a file name, to that file in the output folder");
  exporting.addItem("PDF Book",      "plot.pdf",    "", "All the canvases of the plot file, one per page, to the PDF book of the output folder");
  exporting.addItem("PowerPoint",    "plot.pptx",   "", "All the canvases of the plot file, one per slide (native shapes)");
  exporting.addItem("LaTeX",         "plot.latex",  "", "A Beamer presentation or an article of all the canvases of the plot file");
  exporting.addItem("All Outputs",   "plot.all",    "", "Named plots, PDF book, PowerPoint and LaTeX");
  exporting.addSeparator();
  exporting.addItem("Output Folder...", "plot.folder", "", "Choose the folder where the outputs of the plot file go");
  file.addSeparator();
  file.addItem("Exit",              "file.exit",      "Ctrl+Q",       "Quit (asks to save the changes of the plot files)");

  Menu & edit = bar.addMenu("Edit");
  edit.addItem("Undo",   "plot.undo",  "Ctrl+Z",       "Undo the last change of the plot file (made on the canvases or in the tree)");
  edit.addItem("Redo",   "plot.redo",  "Ctrl+Shift+Z", "Redo the change undone last");
  edit.addSeparator();
  edit.addItem("Cut",    "plot.cut",   "Ctrl+X", "Copy the selection (graph, canvas, histogram, ...), then remove it");
  edit.addItem("Copy",   "plot.copy",  "Ctrl+C", "Copy the selection: its XML also goes to the clipboard");
  edit.addItem("Paste",  "plot.paste", "Ctrl+V", "Paste what was copied where it belongs, at the selection");
  edit.addItem("Duplicate", "plot.duplicateElement", "Ctrl+D", "A copy of the selection, after it");
  edit.addItem("Delete", "plot.delete", "Delete", "Remove the shape selected on a canvas, otherwise the selection");
  edit.addSeparator();
  edit.addItem("Clear Canvas",       "edit.clearCanvas", "", "Remove the histograms and functions of all the graphs of the current canvas");
  edit.addItem("Clear All Canvases", "edit.clearAll",    "", "Remove the histograms and functions of all the canvases shown");
  edit.addSeparator();
  edit.addItem("Add Window",   "plot.addWindow",   "", "A new window of canvases");
  edit.addItem("Add Canvas",   "plot.addCanvas",   "", "A new canvas in the window of the selection");
  edit.addItem("Add Graph",    "plot.addGraph",    "", "A new graph in the canvas of the selection");
  edit.addItem("Add Function", "plot.addFunction", "", "A function in the graph of the selection");
  edit.addItem("Move Up",      "plot.up",          "", "Move the selection before the previous element");
  edit.addItem("Move Down",    "plot.down",        "", "Move the selection after the next element");
  edit.addSeparator();
  edit.addItem("Fit...", "edit.fit", "", "Fit a function to data points, a histogram or a function of the selected graph (shown on screen only)");
  edit.addItem("Preferences...", "edit.preferences", "", "Settings of CapGui: window, fonts, graphs, LaTeX");

  Menu & view = bar.addMenu("View");
  view.addItem("File Browser",   "view.browser",   "Ctrl+B", "Show or hide the file browser").setCheckable(true);
  view.addItem("Browser in Separate Window", "view.browserWindow", "", "The file browser in a window of its own").setCheckable(true);
  view.addItem("Plot Tree",      "plot.tree",      "Ctrl+T", "Show or hide the tree of the plot files").setCheckable(true);
  view.addItem("Inspector",      "plot.inspector", "", "All the settings of the selection, below the tree").setCheckable(true);
  view.addItem("Style Elements", "plot.showStyle", "", "Show the elements of style (Margins, Axis, Marker, ...) in the tree").setCheckable(true);
  view.addItem("All Windows",    "plot.allWindows", "", "Every window of the plot file: the first one here, the others in windows of their own "
                                                        "(otherwise the window of the selection)").setCheckable(true);
  view.addSeparator();
  view.addItem("Canvases",       "view.canvases",  "", "The canvases of the session by window, with their graphs").setCheckable(true);
  view.addItem("Fit Results",    "view.fits",      "", "The results of the fits of the selected graph, below the canvases").setCheckable(true);
  view.addItem("Canvas Toolbars", "view.toolbars", "", "Show the buttons above each canvas").setCheckable(true);
  view.addItem("Legends",        "view.legends",   "", "Show the legend of every graph").setCheckable(true);
  Menu & theme = view.addMenu("Theme");
  for (const char * t : { "light", "dark", "classic" })
    theme.addItem(t, String("view.theme.") + t, "", String("Use the ") + t + String(" theme")).setCheckable(true);

  Menu & window = bar.addMenu("Window");
  Menu & layout = window.addMenu("Canvases", "Canvases of the window shown (columns x rows), in the plot file");
  for (auto cr : std::vector<std::pair<int,int>>{{1,1},{2,1},{1,2},{2,2},{3,2},{3,3}})
    {
    char name[16], command[40];
    std::snprintf(name, sizeof(name), "%d x %d", cr.first, cr.second);
    std::snprintf(command, sizeof(command), "window.layout.%dx%d", cr.first, cr.second);
    layout.addItem(name, command).setCheckable(true);
    }
  layout.addItem("Custom...", "window.layout.custom", "", "Any grid of canvases");
  Menu & graphs = window.addMenu("Graphs in Canvas", "Graphs of the current canvas (columns x rows), in the plot file");
  for (auto cr : std::vector<std::pair<int,int>>{{1,1},{2,1},{1,2},{2,2},{3,2},{3,3}})
    {
    char name[16], command[40];
    std::snprintf(name, sizeof(name), "%d x %d", cr.first, cr.second);
    std::snprintf(command, sizeof(command), "window.graphs.%dx%d", cr.first, cr.second);
    graphs.addItem(name, command).setCheckable(true);
    }
  graphs.addItem("Custom...", "window.graphs.custom", "", "Any grid of graphs");

  Menu & help = bar.addMenu("Help");
  help.addItem("CapGui Help",        "plot.help",      "F1", "How the canvases, the plot files and the browser work together");
  help.addItem("Keyboard Shortcuts", "help.shortcuts", "",   "List of the keyboard shortcuts");
  help.addItem("About CAP GUI",      "help.about",     "",   "Version and components");
  help.addSeparator();
  help.addItem("ImGui Demo (developers)", "help.imguiDemo", "", "Dear ImGui's demonstration window").setCheckable(true);
  }

  void CapGuiApp::updateMenuStates()
  {
  CapApp::updateMenuStates();
  MenuBar & bar = _gui.menuBar();
  Tab * t = tab();
  bool any = t != nullptr, selected = t && t->hasSelection && !t->selected.empty();
  auto enable = [&](const char * c, bool v) { if (MenuItem * item = bar.findItem(c)) item->setEnabled(v); };
  enable("plot.close", any);
  enable("plot.duplicate", any);
  enable("plot.save", any);
  enable("plot.saveAs", any);
  enable("plot.undo", any && t->document->canUndo());
  enable("plot.redo", any && t->document->canRedo());
  enable("plot.cut", selected);
  enable("plot.copy", selected);
  enable("plot.paste", any && !_clipboard.empty());
  enable("plot.duplicateElement", selected);
  enable("plot.up", selected);
  enable("plot.down", selected);
  enable("plot.fromData", bool(_catalog));
  if (MenuItem * item = bar.findItem("plot.tree"))       item->setChecked(_showTree);
  if (MenuItem * item = bar.findItem("plot.inspector"))  item->setChecked(_showInspector);
  if (MenuItem * item = bar.findItem("plot.showStyle"))  item->setChecked(_showStyle);
  if (MenuItem * item = bar.findItem("plot.allWindows")) item->setChecked(_allWindows);
  }

  void CapGuiApp::connectToPlot(FileBrowser & browser)
  {
  browser.onShowObject = [this](const ObjectReference & object) { drop(dropTarget(), object, false); };
  browser.onShowFolder = [this](int fileId, const String & folderPath)
    {
    // all the objects of a folder: a graph each, in the canvas of the selection (or a new canvas)
    Tab * t = tab();
    if (!t) return;
    bool ok = false;
    GRAPHICS::PlotDocument::Path canvas = t->hasSelection ? t->document->enclosing(t->selected, PlotDocument::kCanvas, ok) : Path();
    drop(ok ? canvas : Path(), ObjectReference{fileId, folderPath}, true);
    };
  }

  void CapGuiApp::onStartup()
  {
  _gui.setSideWindow(kEditorWindow, 0.40f);
  std::function<void()> previous = _gui.onFrame;
  _gui.onFrame = [this, previous]() { if (previous) previous(); renderFrame(); };
  for (const auto & b : _gui.browsers()) connectToPlot(*b);
  std::function<void(FileBrowser &)> previousBrowser = _gui.onNewBrowser;
  _gui.onNewBrowser = [this, previousBrowser](FileBrowser & b) { if (previousBrowser) previousBrowser(b); connectToPlot(b); };
  _gui.onClosePanes = [this](const std::vector<int> & panes) { return closeCanvases(panes); };
  _gui.onCloseRequest = [this]() -> bool
    {
    _closing.clear();
    for (const auto & t : _documents) if (t.document->isDirty()) _closing.push_back(t.id);
    if (_closing.empty()) return true;
    _exitAfterClosing = true;
    _closeDialog = true;
    return false;
    };
  for (const String & f : _plotFiles) openDocument(f);
  if (!_inputFiles.empty() && !_documents.empty())
    {
    // files replacing the inputs of the first plot file (as shown, not a change of the file)
    XmlElement & root = _documents.front().document->document().root();
    std::size_t k = 0;
    for (XmlElement * e : root.children())
      if (e->isNamed("Input") && k < _inputFiles.size()) e->setAttribute("file", FileSystem::normalize(_inputFiles[k++]));
    for (; k < _inputFiles.size(); k++)
      {
      XmlElement in("Input");
      in.setAttribute("id", String("input") + String(std::to_string(k)));
      in.setAttribute("file", FileSystem::normalize(_inputFiles[k]));
      root.insertChild(unsigned(k), in);
      }
    }
  if (_documents.empty())
    {
    newDocument();
    if (_mode == kEdit && !_templateOpen) actionPerformed(ActionEvent{"plot.open", nullptr});
    else if (_mode == kBasic)
      {
      bool folders = false;
      for (const auto & b : _gui.browsers()) if (!b->folders().empty()) folders = true;
      if (!folders) openFolder(FileSystem::current());
      }
    }
  _gui.setStatus(_mode == kBasic ? "Drag histograms or folders from the browser onto the canvas or the Plot Tree"
                                 : "The canvases are the plot file: change them, or the tree, and save the file");
  }

  int CapGuiApp::newDocument()
  {
  Tab t;
  t.document.reset(new PlotDocument());
  t.document->setUntitledName(String("Untitled ") + String(std::to_string(++_untitled)));
  t.id = _nextId++;
  _documents.push_back(std::move(t));
  _current = int(_documents.size()) - 1;
  _selectTab = _current;
  return _current;
  }

  bool CapGuiApp::openDocument(const String & fileName)
  {
  String path = FileSystem::normalize(fileName);
  for (std::size_t k=0; k<_documents.size(); k++)
    if (_documents[k].document->fileName() == path) { _current = int(k); _selectTab = _current; return true; }
  std::unique_ptr<PlotDocument> d(new PlotDocument());
  String error;
  if (!d->read(path, error)) { _gui.showMessage("Open", path + String(":\n") + error); return false; }
  // an untitled, unchanged, empty tab is replaced
  if (Tab * t = tab())
    if (t->document->fileName().isNull() && !t->document->isDirty() && !t->document->canUndo())
      {
      t->document = std::move(d);
      t->selected.clear();
      t->hasSelection = false;
      _selectTab = _current;
      _gui.setStatus(String("Opened ") + path);
      return true;
      }
  Tab t;
  t.document = std::move(d);
  t.id = _nextId++;
  _documents.push_back(std::move(t));
  _current = int(_documents.size()) - 1;
  _selectTab = _current;
  _gui.setStatus(String("Opened ") + path);
  return true;
  }

  void CapGuiApp::select(const Path & path)
  {
  if (Tab * t = tab()) { t->selected = path; t->hasSelection = true; }
  }

  void CapGuiApp::requestClose(int k)
  {
  if (k < 0 || k >= int(_documents.size())) return;
  if (!_documents[std::size_t(k)].document->isDirty()) { closeTab(k); return; }
  _closing = { _documents[std::size_t(k)].id };
  _exitAfterClosing = false;
  _closeDialog = true;
  }

  void CapGuiApp::closeTab(int k)
  {
  if (k < 0 || k >= int(_documents.size())) return;
  _documents.erase(_documents.begin() + k);
  if (_current >= int(_documents.size())) _current = int(_documents.size()) - 1;
  _selectTab = _current;
  _drawnTab = -1;
  if (_documents.empty() && !_exitAfterClosing) newDocument();   // every session is a plot file
  }

  void CapGuiApp::save(int k, bool ask, const std::function<void()> & after)
  {
  if (k < 0 || k >= int(_documents.size())) return;
  int id = _documents[std::size_t(k)].id;
  auto write = [this, id, after](const String & fileName)
    {
    for (auto & t : _documents)
      {
      if (t.id != id) continue;
      String name = FileSystem::hasExtension(fileName, ".xml") ? fileName : fileName + String(".xml");
      String error;
      if (!t.document->write(name, error)) { _gui.showMessage("Save", name + String(":\n") + error); return; }
      _gui.setStatus(String("Saved ") + name);
      if (after) after();
      return;
      }
    };
  PlotDocument & d = *_documents[std::size_t(k)].document;
  if (!ask && !d.fileName().isNull()) { write(d.fileName()); return; }
  String start = d.fileName().isNull() ? _configuration.directory : d.directory();
  String name = d.fileName().isNull() ? String("plots.xml") : d.displayName();
  _gui.fileDialog().open(FileDialog::kSaveFile, "Save the plot file", start, {".xml"}, name, write);
  }

  std::vector<String> CapGuiApp::objectsOf(const ObjectReference & object, bool folder, String & fileName)
  {
  std::vector<String> objects;
  BrowserFile * f = file(object.file);
  if (!f || !f->file) return objects;
  fileName = f->path;
  if (!folder) { objects.push_back(object.path); return objects; }
  Folder * dir = object.path.isNull() ? &f->file->folder() : f->file->folder().folder(object.path);
  if (!dir) return objects;
  std::function<void(Folder &, const String &, int)> collect = [&](Folder & d, const String & prefix, int depth)
    {
    for (const String & name : d.names())
      {
      String path = prefix.isNull() ? name : prefix + String("/") + name;
      if (Folder * sub = d.folder(name)) { if (depth > 0) collect(*sub, path, depth - 1); continue; }
      String kind, details;
      if (FileBrowser::describe(d, name, kind, details)) objects.push_back(path);
      }
    };
  collect(*dir, object.path, 0);
  if (objects.empty()) collect(*dir, object.path, 3);   // no histogram here: those of the sub-folders
  return objects;
  }

  bool CapGuiApp::drop(const Path & target, const ObjectReference & object, bool folder)
  {
  Tab * t = tab();
  if (!t) return false;
  String fileName;
  std::vector<String> objects = objectsOf(object, folder, fileName);
  if (objects.empty() || fileName.isNull()) { _gui.setStatus("Nothing to plot there"); return false; }
  if (objects.size() > 48) objects.resize(48);
  Path added = t->document->addData(target, fileName, objects);
  if (added.empty()) return false;
  t->selected = added;
  t->hasSelection = true;
  _gui.setStatus(String(std::to_string(objects.size())) + String(objects.size() > 1 ? " histograms added" : " histogram added"));
  return true;
  }

  // ------------------------------------------------------------------------------------------------ commands

  void CapGuiApp::actionPerformed(const ActionEvent & event)
  {
  const String & c = event.command;
  Tab * t = tab();
  PlotDocument * d = t ? t->document.get() : nullptr;
  bool selected = t && t->hasSelection && !t->selected.empty() && d->element(t->selected);
  int columns = 0, rows = 0;
  if (c == "plot.new") newDocument();
  else if (c == "plot.open")
    _gui.fileDialog().open(FileDialog::kOpenFile, "Open a plot file", _configuration.directory, {".xml"}, "",
                           [this](const String & path) { openDocument(path); });
  else if (c == "plot.fromData")
    _gui.fileDialog().open(FileDialog::kOpenFile, "CAP file or RunCAP workflow (.xml) whose histograms to plot",
                           _configuration.directory, {".cap", ".xml"}, "",
      [this](const String & path)
      {
      GRAPHICS::PlotTemplate catalog;
      String problems;
      if (!_catalog || !_catalog({ path }, String(""), catalog, problems))
        { _gui.showMessage("New from Data", String("No histogram found in ") + path + String("\n") + problems); return; }
      if (!problems.isNull()) _gui.setStatus(String("New from Data: ") + problems);
      startTemplate(catalog, GRAPHICS::PlotTemplate::Options(), String(""));
      });
  else if (c == "plot.close") requestClose(_current);
  else if (c == "plot.duplicate" && d)
    {
    int from = _current;
    int k = newDocument();
    _documents[std::size_t(k)].document->document() = _documents[std::size_t(from)].document->document();
    _documents[std::size_t(k)].document->setUntitledName(String("Copy of ") + _documents[std::size_t(from)].document->displayName());
    _documents[std::size_t(k)].document->setDirty(true);
    }
  else if (c == "plot.save" && d) save(_current, false);
  else if (c == "plot.saveAs" && d) save(_current, true);
  else if (c == "plot.saveAll")
    {
    for (std::size_t k=0; k<_documents.size(); k++)
      if (_documents[k].document->isDirty()) save(int(k), false);
    }
  else if (c == "plot.undo" && d) d->undo();
  else if (c == "plot.redo" && d) d->redo();
  else if ((c == "plot.copy" || c == "plot.cut") && selected)
    {
    _clipboard = d->copy(t->selected).str();
    ImGui::SetClipboardText(_clipboard.c_str());
    if (c == "plot.cut") { d->remove(t->selected); t->selected = PlotDocument::parent(t->selected); }
    _gui.setStatus(c == "plot.cut" ? "Cut" : "Copied (its XML is on the clipboard)");
    }
  else if (c == "plot.paste" && d)
    {
    // the clipboard of the system, if it holds an element (copied from a text editor), otherwise the last copy
    const char * system = ImGui::GetClipboardText();
    std::string text = system && std::strchr(system, '<') ? std::string(system) : _clipboard;
    bool ok = false;
    Path p = d->paste(String(text), t->hasSelection ? t->selected : Path(), ok);
    if (!ok && text != _clipboard && !_clipboard.empty()) p = d->paste(String(_clipboard), t->hasSelection ? t->selected : Path(), ok);
    if (ok) { t->selected = p; t->hasSelection = true; }
    else _gui.setStatus("Nothing to paste here");
    }
  else if (c == "plot.duplicateElement" && selected) t->selected = d->duplicate(t->selected);
  else if (c == "plot.delete")
    {
    // a shape selected on the current canvas (the tree not in use) goes first
    if (!_treeFocused && _gui.nPanes() > 0 && _gui.currentPane().hasSelectedShape()) CapApp::actionPerformed(ActionEvent{"edit.delete", nullptr});
    else if (selected)
      {
      Path p = t->selected;
      if (d->remove(p)) t->selected = PlotDocument::parent(p);
      }
    }
  else if (c == "plot.up" && selected)   t->selected = d->shift(t->selected, -1);
  else if (c == "plot.down" && selected) t->selected = d->shift(t->selected, +1);
  else if (c == "plot.addWindow" && d) { t->selected = d->addWindow(); t->hasSelection = true; }
  else if (c == "plot.addCanvas" && d)
    {
    bool ok = false;
    Path w = d->enclosing(t->selected, PlotDocument::kWindow, ok);
    if (!ok) { std::vector<Path> ws = d->children(Path(), PlotDocument::kWindow); w = ws.empty() ? d->addWindow() : ws.back(); }
    t->selected = d->addCanvas(w);
    t->hasSelection = true;
    }
  else if (c == "plot.addGraph" && d)
    {
    bool ok = false;
    Path cv = d->enclosing(t->selected, PlotDocument::kCanvas, ok);
    if (!ok) { _gui.setStatus("Select a canvas first"); return; }
    t->selected = d->addGraph(cv);
    }
  else if (c == "plot.addFunction" && d)
    {
    bool ok = false;
    Path g = d->enclosing(t->selected, PlotDocument::kGraph, ok);
    if (!ok) { _gui.setStatus("Select a graph first"); return; }
    t->selected = d->addFunction(g);
    }
  else if (c == "plot.print" || c == "plot.pdf" || c == "plot.pptx" || c == "plot.latex" || c == "plot.all")
    {
    GRAPHICS::PlotSpec spec;
    std::vector<GRAPHICS::PlotSpec::WindowState> windows;
    if (!makeOutputs(spec, windows)) return;
    String done, error;
    bool all = c == "plot.all";
    if (all || c == "plot.print")
      {
      std::vector<String> files = PlotOutputs::print(spec, windows);
      done += String(std::to_string(files.size())) + String(" plots printed in ") + spec.outputPath("") + String("\n");
      }
    if (all || c == "plot.pdf")
      {
      String path = PlotOutputs::pdf(spec, windows);
      done += path.isNull() ? String("cannot write the PDF book\n") : String("PDF book: ") + path + String("\n");
      }
    if (all || c == "plot.pptx")
      {
      String path = PlotOutputs::pptx(spec, windows);
      done += path.isNull() ? String("cannot write the PowerPoint file\n") : String("PowerPoint: ") + path + String("\n");
      }
    if (all || c == "plot.latex")
      {
      String path = PlotOutputs::latex(spec, windows, error);
      if (!error.isNull()) done += error + String("\n");
      if (!path.isNull()) done += String("LaTeX: ") + path + String("\n");
      }
    _gui.showMessage("Export", done);
    }
  else if (c == "plot.folder" && d)
    {
    int id = t->id;
    _gui.fileDialog().open(FileDialog::kOpenFolder, "Folder of the outputs", d->directory(), {}, "",
      [this, id](const String & folder)
      {
      for (auto & tt : _documents)
        if (tt.id == id)
          for (const Path & p : tt.document->children(Path(), PlotDocument::kOutput))
            tt.document->setAttribute(p, "directory", PlotDocument::relativePath(folder, tt.document->directory()));
      });
    }
  else if (c == "plot.tree") _showTree = !_showTree;
  else if (c == "plot.inspector") _showInspector = !_showInspector;
  else if (c == "plot.showStyle") _showStyle = !_showStyle;
  else if (c == "plot.allWindows") { _allWindows = !_allWindows; _drawnTab = -1; }
  else if (c == "plot.help") _showHelp = true;
  else if (std::sscanf(c.data(), "window.layout.%dx%d", &columns, &rows) == 2) setGrid(false, columns, rows);
  else if (std::sscanf(c.data(), "window.graphs.%dx%d", &columns, &rows) == 2) setGrid(true, columns, rows);
  else if (c == "window.layout.custom" || c == "window.graphs.custom") { _gridOfGraphs = c == "window.graphs.custom"; _gridDialog = true; }
  else if (c.str().compare(0, 5, "plot.") == 0) { }
  else CapApp::actionPerformed(event);
  }

  void CapGuiApp::setGrid(bool graphs, int columns, int rows)
  {
  Tab * t = tab();
  if (!t) return;
  PlotDocument & d = *t->document;
  Path target;
  if (graphs)
    {
    int k = _gui.currentPaneIndex();
    for (const auto & link : _links) if (paneIndexOf(link.pane) == k) target = link.canvas;
    if (target.empty()) { _gui.setStatus("The current canvas is not one of the plot file"); return; }
    }
  else
    {
    std::vector<Path> windows = d.children(Path(), PlotDocument::kWindow);
    if (_drawnWindow < 0 || _drawnWindow >= int(windows.size())) { _gui.setStatus("No window in the plot file"); return; }
    target = windows[std::size_t(_drawnWindow)];
    }
  d.beginEdit();
  d.setAttribute(target, "columns", String(std::to_string(columns)));
  d.setAttribute(target, "rows", String(std::to_string(rows)));
  d.endEdit();
  }

  bool CapGuiApp::makeOutputs(GRAPHICS::PlotSpec & spec, std::vector<GRAPHICS::PlotSpec::WindowState> & windows)
  {
  Tab * t = tab();
  if (!t) return false;
  String error, problems;
  if (!spec.read(t->document->document(), t->document->directory(), error)) { _gui.showMessage("Export", error); return false; }
  for (std::size_t k=0; k<spec.inputs().size(); k++)
    {
    String path = spec.resolve(spec.inputs()[k].path);
    BrowserFile * f = fileOfPath(path);
    if (f) spec.setInputFile(int(k), f->file);
    }
  if (!spec.openInputs(error)) { _gui.showMessage("Export", error); return false; }
  windows = spec.buildAll(problems);
  if (!problems.isNull()) _gui.setStatus(String("Export: not everything could be drawn: ") + problems);
  return true;
  }

  // ------------------------------------------------------------------------------------------------ editor

  void CapGuiApp::renderFrame()
  {
  // changes made while a widget is active (typing, dragging) are one step of undo
  bool active = ImGui::IsAnyItemActive();
  if (active && !_grouping && tab()) { tab()->document->beginEdit(); _grouping = true; _groupTab = tab()->id; }
  if (!active && _grouping)
    {
    for (auto & t : _documents) if (t.id == _groupTab) t.document->endEdit();
    _grouping = false;
    }

  ImGui::SetNextWindowSize(ImVec2(620, 700), ImGuiCond_FirstUseEver);
  _treeFocused = false;
  // Begin() and End() are paired on the visibility at the start of the frame: closing the window (its close button,
  // which Begin() reports by clearing _showTree) must not skip End(), which ImGui reports as missing EndChild/End.
  const bool treeShown = _showTree;
  if (treeShown && ImGui::Begin(kEditorWindow, &_showTree, ImGuiWindowFlags_NoCollapse))
    {
    _treeFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (ImGui::BeginTabBar("##documents", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs
                                          | ImGuiTabBarFlags_FittingPolicyScroll))
      {
      if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) newDocument();
      ImGui::SetItemTooltip("New plot file");
      int closeRequest = -1;
      for (std::size_t k=0; k<_documents.size(); k++)
        {
        Tab & t = _documents[k];
        std::string label = t.document->displayName().str() + "###tab" + std::to_string(t.id);
        ImGuiTabItemFlags flags = t.document->isDirty() ? ImGuiTabItemFlags_UnsavedDocument : 0;
        if (_selectTab == int(k)) flags |= ImGuiTabItemFlags_SetSelected;
        bool open = true;
        if (ImGui::BeginTabItem(label.c_str(), &open, flags))
          {
          if (_selectTab < 0 || _selectTab == int(k)) _current = int(k);
          if (!t.document->fileName().isNull()) ImGui::SetItemTooltip("%s", t.document->fileName().data());
          renderTree(t);
          ImGui::EndTabItem();
          }
        if (!open) closeRequest = int(k);
        }
      _selectTab = -1;
      ImGui::EndTabBar();
      if (closeRequest >= 0) requestClose(closeRequest);
      }
    if (_documents.empty())
      {
      ImGui::TextDisabled("No plot file open.");
      if (ImGui::Button("New plot file")) newDocument();
      ImGui::SameLine();
      if (ImGui::Button("Open...")) actionPerformed(ActionEvent{"plot.open", nullptr});
      }
    }
  if (treeShown) ImGui::End();
  renderDialogs();
  renderTemplate();
  layoutCanvases();
  recordCanvases();
  syncSelection();
  }

  void CapGuiApp::renderTree(Tab & t)
  {
  PlotDocument & d = *t.document;
  // toolbar
  if (ImGui::SmallButton("+ Window")) actionPerformed(ActionEvent{"plot.addWindow", nullptr});
  ImGui::SameLine();
  if (ImGui::SmallButton("+ Canvas")) actionPerformed(ActionEvent{"plot.addCanvas", nullptr});
  ImGui::SameLine();
  if (ImGui::SmallButton("+ Graph")) actionPerformed(ActionEvent{"plot.addGraph", nullptr});
  ImGui::SameLine();
  if (ImGui::SmallButton("+ Function")) actionPerformed(ActionEvent{"plot.addFunction", nullptr});
  ImGui::SameLine(0, 18);
  ImGui::BeginDisabled(!d.canUndo());
  if (ImGui::SmallButton("Undo")) d.undo();
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::BeginDisabled(!d.canRedo());
  if (ImGui::SmallButton("Redo")) d.redo();
  ImGui::EndDisabled();
  ImGui::SameLine(0, 18);
  ImGui::TextDisabled("%s", _mode == kBasic ? "BASIC" : "EDIT");
  ImGui::SetItemTooltip("BASIC: started from a new file; EDIT: from an existing one. The tools are the same.");

  float inspectorHeight = _showInspector ? std::max(220.0f, ImGui::GetContentRegionAvail().y*0.42f) : 0.0f;
  if (ImGui::BeginChild("##tree", ImVec2(0, -inspectorHeight), ImGuiChildFlags_Borders))
    {
    renderNode(t, Path());
    // below the rows: dropping there adds to the top of the document
    ImGui::InvisibleButton("##below", ImVec2(std::max(1.0f, ImGui::GetContentRegionAvail().x), std::max(40.0f, ImGui::GetContentRegionAvail().y)));
    rowDragAndDrop(t, Path());
    if (ImGui::IsItemClicked()) { t.selected.clear(); t.hasSelection = false; }
    if (d.children(Path(), PlotDocument::kWindow).empty())
      {
      ImVec2 p = ImGui::GetItemRectMin();
      ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + 10, p.y + 10), ImGui::GetColorU32(ImGuiCol_TextDisabled),
                                          "Drag histograms or folders from the browser here");
      }
    }
  ImGui::EndChild();
  if (_showInspector)
    {
    if (ImGui::BeginChild("##inspector", ImVec2(0, 0), ImGuiChildFlags_Borders)) renderInspector(t);
    ImGui::EndChild();
    }
  }

  void CapGuiApp::renderNode(Tab & t, const Path & path)
  {
  PlotDocument & d = *t.document;
  const XmlElement * e = d.element(path);
  if (!e) return;
  PlotDocument::Kind kind = PlotDocument::kind(*e);
  if (kind == PlotDocument::kStyle && !_showStyle) return;
  bool hasChildren = false;
  for (unsigned k=0; k<e->nChildren() && !hasChildren; k++)
    if (_showStyle || PlotDocument::kind(e->child(k)) != PlotDocument::kStyle) hasChildren = true;
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                             | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
  if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
  if (t.hasSelection && t.selected == path) flags |= ImGuiTreeNodeFlags_Selected;
  if (kind == PlotDocument::kRoot || kind == PlotDocument::kWindow || kind == PlotDocument::kCanvas
      || kind == PlotDocument::kGraph)
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  // the selection is made visible
  if (t.hasSelection && t.selected.size() > path.size() && std::equal(path.begin(), path.end(), t.selected.begin()))
    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
  ImGui::PushID(path.empty() ? -1 : path.back());
  bool open = ImGui::TreeNodeEx("##node", flags);
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) { t.selected = path; t.hasSelection = true; }
  rowDragAndDrop(t, path);
  rowMenu(t, path);
  ImGui::SameLine();
  ImGui::TextColored(kindColor(kind), "%s", kindTag(kind));
  ImGui::SameLine();
  String text = PlotDocument::label(*e);
  if (kind == PlotDocument::kRoot && !d.fileName().isNull()) text = d.fileName();
  ImGui::TextUnformatted(text.data());
  rowControls(t, path);
  if (open)
    {
    for (unsigned k=0; k<e->nChildren(); k++)
      {
      Path child = path;
      child.push_back(int(k));
      renderNode(t, child);
      e = d.element(path);   // an edit of a row may have changed the tree
      if (!e) break;
      }
    ImGui::TreePop();
    }
  ImGui::PopID();
  }

  void CapGuiApp::rowDragAndDrop(Tab & t, const Path & path)
  {
  PlotDocument & d = *t.document;
  if (!path.empty() && ImGui::BeginDragDropSource())
    {
    std::string key = pathKey(t.id, path);
    ImGui::SetDragDropPayload(kNodePayload, key.c_str(), key.size() + 1);
    const XmlElement * e = d.element(path);
    ImGui::Text("%s %s", kindTag(e ? PlotDocument::kind(*e) : PlotDocument::kOther), e ? PlotDocument::label(*e).data() : "");
    ImGui::TextDisabled("drop it on the element that is to hold it");
    ImGui::EndDragDropSource();
    }
  if (!ImGui::BeginDragDropTarget()) return;
  if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(kNodePayload))
    {
    int from = -1;
    Path source;
    if (parsePathKey(static_cast<const char *>(payload->Data), from, source))
      {
      if (from == t.id)
        {
        bool ok = false;
        Path moved = d.move(source, path, ok);
        if (ok) { t.selected = moved; t.hasSelection = true; }
        else _gui.setStatus("It cannot go there");
        }
      else
        {
        // from another tab: copied
        for (auto & other : _documents)
          if (other.id == from)
            {
            bool ok = false;
            Path p = d.paste(other.document->copy(source), path, ok);
            if (ok) { t.selected = p; t.hasSelection = true; }
            }
        }
      }
    }
  for (const char * type : { FileBrowser::payloadType(), FileBrowser::folderPayloadType() })
    if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(type))
      {
      ObjectReference object;
      if (FileBrowser::decodePayload(payload->Data, payload->DataSize, object))
        drop(path, object, std::strcmp(type, FileBrowser::folderPayloadType()) == 0);
      }
  ImGui::EndDragDropTarget();
  }

  void CapGuiApp::rowMenu(Tab & t, const Path & path)
  {
  PlotDocument & d = *t.document;
  if (!ImGui::BeginPopupContextItem("##rowmenu")) return;
  t.selected = path;
  t.hasSelection = true;
  PlotDocument::Kind kind = d.kind(path);
  auto item = [&](const char * label, const char * command, bool enabled=true)
    {
    if (ImGui::MenuItem(label, nullptr, false, enabled)) actionPerformed(ActionEvent{command, nullptr});
    };
  if (kind == PlotDocument::kRoot || kind == PlotDocument::kWindow) item("Add Canvas", "plot.addCanvas");
  if (kind == PlotDocument::kCanvas || kind == PlotDocument::kGraph) item("Add Graph", "plot.addGraph");
  if (kind == PlotDocument::kGraph || kind == PlotDocument::kHistogram || kind == PlotDocument::kFunction)
    item("Add Function", "plot.addFunction");
  if (kind == PlotDocument::kRoot) item("Add Window", "plot.addWindow");
  ImGui::Separator();
  bool element = !path.empty();
  item("Cut", "plot.cut", element);
  item("Copy", "plot.copy", element);
  item("Paste", "plot.paste", !_clipboard.empty());
  item("Duplicate", "plot.duplicateElement", element);
  item("Delete", "plot.delete", element);
  ImGui::Separator();
  item("Move Up", "plot.up", element && path.back() > 0);
  item("Move Down", "plot.down", element);
  ImGui::EndPopup();
  }

  void CapGuiApp::rowControls(Tab & t, const Path & path)
  {
  PlotDocument & d = *t.document;
  const XmlElement * e = d.element(path);
  if (!e) return;
  PlotDocument::Kind kind = PlotDocument::kind(*e);
  // the controls in a column on the right, or after the label if it is long
  float x = std::max(ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x + ImGui::GetScrollX() + 10.0f,
                     ImGui::GetWindowWidth() - 250.0f);
  auto next = [&](float spacing=4.0f) { ImGui::SameLine(0, spacing); };
  ImGui::SameLine(x);
  switch (kind)
    {
    case PlotDocument::kWindow:
      gridControl(d, path, "windowgrid", "canvases of the window: columns x rows");
      break;
    case PlotDocument::kCanvas:
      {
      gridControl(d, path, "canvasgrid", "graphs of the canvas: columns x rows");
      next();
      String bg = d.style(path, "Background", "color");
      int color = bg.isNull() ? 0 : GraphXml::parseColor(bg, 0);
      colorStyle(d, path, "Background", "##bg", color);
      ImGui::SetItemTooltip("background of the canvas");
      break;
      }
    case PlotDocument::kGraph:
      {
      bool logX = d.style(path, "Axis", "log", "x") == "true", logY = d.style(path, "Axis", "log", "y") == "true";
      if (ImGui::Checkbox("log x", &logX)) d.setStyle(path, "Axis", "log", logX ? "true" : "", "x");
      next();
      if (ImGui::Checkbox("log y", &logY)) d.setStyle(path, "Axis", "log", logY ? "true" : "", "y");
      next();
      if (ImGui::SmallButton("Palettes")) ImGui::OpenPopup("palettes");
      ImGui::SetItemTooltip("color and marker palettes of the items of the graph");
      if (ImGui::BeginPopup("palettes"))
        {
        GRAPHICS::Palettes & palettes = GRAPHICS::Palettes::session();
        String cp = e->attribute("colorPalette", ""), mp = e->attribute("markerPalette", "");
        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo("colors", cp.isNull() ? "default" : cp.data()))
          {
          for (const String & name : palettes.colorPaletteNames())
            {
            const GRAPHICS::ColorPalette * palette = palettes.colorPalette(name);
            if (ImGui::Selectable(name.data(), name == cp)) d.setAttribute(path, "colorPalette", name);
            if (palette)
              for (int k=0; k<std::min(8, palette->size()); k++)
                {
                ImGui::SameLine(0, k ? 1 : 8);
                ImGui::PushID(k);
                int c = palette->color(k);
                GRAPHICS::Color rgb = GRAPHICS::ColorTable::instance().color(c);
                ImGui::ColorButton("##p", ImVec4(rgb.red, rgb.green, rgb.blue, 1), ImGuiColorEditFlags_NoTooltip,
                                   ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
                ImGui::PopID();
                }
            }
          ImGui::EndCombo();
          }
        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo("markers", mp.isNull() ? "default" : mp.data()))
          {
          for (const String & name : palettes.markerPaletteNames())
            if (ImGui::Selectable(name.data(), name == mp)) d.setAttribute(path, "markerPalette", name);
          ImGui::EndCombo();
          }
        if (ImGui::Button("Default palettes"))
          {
          d.beginEdit();
          d.setAttribute(path, "colorPalette", "");
          d.setAttribute(path, "markerPalette", "");
          d.endEdit();
          }
        ImGui::EndPopup();
        }
      break;
      }
    case PlotDocument::kHistogram:
      {
      int color = itemColor(d, path, "Marker");
      if (colorStyle(d, path, "Marker", "##color", color, "", "Line")) { }
      ImGui::SetItemTooltip("color of the markers and line");
      next();
      int marker = itemMarker(d, path);
      if (markerButton("marker", marker, color)) d.setStyle(path, "Marker", "style", String(std::to_string(marker)));
      break;
      }
    case PlotDocument::kFunction:
      {
      int color = itemColor(d, path, "Line");
      colorStyle(d, path, "Line", "##color", color);
      ImGui::SetItemTooltip("color of the line");
      next();
      String s = d.style(path, "Line", "style");
      int style = s.isNull() ? 1 : std::atoi(s.data());
      if (lineButton("line", style)) d.setStyle(path, "Line", "style", String(std::to_string(style)));
      break;
      }
    default:
      break;
    }
  if (kind != PlotDocument::kStyle && kind != PlotDocument::kOther)
    {
    next(6);
    if (ImGui::SmallButton("...")) ImGui::OpenPopup("attributes");
    ImGui::SetItemTooltip("all the settings");
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 0), ImVec2(700, 640));
    if (ImGui::BeginPopup("attributes"))
      {
      editElement(t, path);
      ImGui::EndPopup();
      }
    }
  }

  void CapGuiApp::renderInspector(Tab & t)
  {
  PlotDocument & d = *t.document;
  const XmlElement * e = t.hasSelection ? d.element(t.selected) : nullptr;
  if (!e)
    {
    ImGui::TextDisabled("Select an element of the tree to see its settings.");
    if (!t.hasSelection) e = nullptr;
    return;
    }
  PlotDocument::Kind kind = PlotDocument::kind(*e);
  ImGui::TextColored(kindColor(kind), "%s", PlotDocument::kindName(kind));
  ImGui::SameLine();
  ImGui::TextUnformatted(PlotDocument::label(*e).data());
  ImGui::Separator();
  editElement(t, t.selected);
  e = d.element(t.selected);
  if (e && ImGui::CollapsingHeader("XML"))
    {
    // shown as plain text in a child of its own, with a Copy button (not a read-only text field, whose state ImGui keeps
    // from frame to frame while the element shown changes)
    std::string text = e->toString().str();
    if (ImGui::SmallButton("Copy")) ImGui::SetClipboardText(text.c_str());
    ImGui::SetItemTooltip("Copy the XML of the element to the clipboard");
    float lines = float(std::count(text.begin(), text.end(), '\n') + 1);
    float height = std::min(300.0f, ImGui::GetStyle().WindowPadding.y*2.0f + lines*ImGui::GetTextLineHeightWithSpacing());
    if (ImGui::BeginChild("##xml", ImVec2(-1, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
      ImGui::TextUnformatted(text.c_str(), text.c_str() + text.size());
    ImGui::EndChild();
    }
  }

  void CapGuiApp::editElement(Tab & t, const Path & p)
  {
  PlotDocument & d = *t.document;
  const XmlElement * e = d.element(p);
  if (!e) return;
  ImGui::PushID(pathKey(t.id, p).c_str());
  switch (PlotDocument::kind(*e))
    {
    case PlotDocument::kRoot:
      ImGui::TextWrapped("The plot file: inputs (CAP files), outputs, and windows of canvases. Drop histograms here to "
                         "add a canvas to the last window.");
      break;
    case PlotDocument::kInput:
      {
      textField(d, p, "id", "identifier", 200);
      textField(d, p, "file", "file", -1, "path of the CAP file (relative to the plot file)");
      ImGui::SameLine();
      if (ImGui::SmallButton("Browse..."))
        {
        int id = t.id;
        Path path = p;
        _gui.fileDialog().open(FileDialog::kOpenFile, "CAP file of the input", d.directory(), {".cap"}, "",
          [this, id, path](const String & f)
          {
          for (auto & tt : _documents)
            if (tt.id == id) tt.document->setAttribute(path, "file", PlotDocument::relativePath(f, tt.document->directory()));
          });
        }
      String f = e->attribute("file", "");
      String resolved = f.isNull() ? f : (f.str()[0] == '/' ? f : FileSystem::join(d.directory(), f));
      if (!f.isNull() && !FileSystem::isFile(resolved)) ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.1f, 1), "file not found: %s", resolved.data());
      break;
      }
    case PlotDocument::kOutput:
      {
      textField(d, p, "directory", "directory", -1, "folder of the outputs (relative to the plot file)");
      textField(d, p, "pdf", "PDF book", 260);
      textField(d, p, "pptx", "PowerPoint", 260);
      textField(d, p, "latex", "LaTeX", 260);
      int style = e->attribute("latexStyle", "beamer") == "article" ? 1 : 0;
      const char * styles[] = { "beamer (one frame per canvas)", "article (one figure per canvas)" };
      ImGui::SetNextItemWidth(260);
      if (ImGui::Combo("LaTeX style", &style, styles, 2)) d.setAttribute(p, "latexStyle", style ? "article" : "beamer");
      checkField(d, p, "latexCompile", "compile with pdflatex", false);
      textField(d, p, "title", "title");
      textField(d, p, "author", "author");
      textField(d, p, "affiliation", "affiliation");
      break;
      }
    case PlotDocument::kWindow:
      {
      textField(d, p, "title", "title");
      intField(d, p, "width", "width", 1200, 200, 6000);
      ImGui::SameLine();
      intField(d, p, "height", "height", 800, 200, 6000);
      ImGui::TextUnformatted("canvases:");
      ImGui::SameLine();
      gridControl(d, p, "inspgrid", "canvases of the window: columns x rows");
      break;
      }
    case PlotDocument::kCanvas:
      {
      textField(d, p, "name", "name", 220);
      textField(d, p, "title", "title");
      textField(d, p, "file", "printed to", 260, "e.g. spectra.pdf (or .svg); empty: not printed");
      intField(d, p, "width", "width", 800, 100, 6000);
      ImGui::SameLine();
      intField(d, p, "height", "height", 600, 100, 6000);
      ImGui::TextUnformatted("graphs:");
      ImGui::SameLine();
      gridControl(d, p, "inspcanvasgrid", "graphs of the canvas: columns x rows");
      String bg = d.style(p, "Background", "color");
      int color = bg.isNull() ? 0 : GraphXml::parseColor(bg, 0);
      colorStyle(d, p, "Background", "background", color);
      break;
      }
    case PlotDocument::kGraph:
      {
      textField(d, p, "title", "title");
      textField(d, p, "file", "printed to", 260, "e.g. graph.pdf; empty: not printed");
      intField(d, p, "cell", "cell", -1, -1, 99);
      ImGui::SetItemTooltip("cell of the canvas (0, 1, ... row by row); -1: the next one");
      checkField(d, p, "legend", "legend", true);
      ImGui::SameLine();
      for (const char * axis : { "x", "y", "z" })
        {
        bool log = d.style(p, "Axis", "log", axis) == "true";
        std::string label = std::string("log ") + axis;
        if (ImGui::Checkbox(label.c_str(), &log)) d.setStyle(p, "Axis", "log", log ? "true" : "", axis);
        ImGui::SameLine();
        }
      ImGui::NewLine();
      if (ImGui::TreeNode("Axes"))
        {
        for (const char * axis : { "x", "y", "z" })
          {
          ImGui::PushID(axis);
          std::string title = d.style(p, "Axis", "title", axis).str();
          ImGui::SetNextItemWidth(220);
          if (ImGui::InputText((std::string(axis) + " title").c_str(), &title)) d.setStyle(p, "Axis", "title", String(title), axis);
          ImGui::SameLine();
          std::string mn = d.style(p, "Axis", "min", axis).str(), mx = d.style(p, "Axis", "max", axis).str();
          ImGui::SetNextItemWidth(70);
          if (ImGui::InputTextWithHint("##min", "min", &mn)) d.setStyle(p, "Axis", "min", String(mn), axis);
          ImGui::SameLine();
          ImGui::SetNextItemWidth(70);
          if (ImGui::InputTextWithHint("##max", "max", &mx)) d.setStyle(p, "Axis", "max", String(mx), axis);
          ImGui::PopID();
          }
        ImGui::TreePop();
        }
      if (ImGui::TreeNode("Margins"))
        {
        const char * sides[] = { "left", "bottom", "right", "top" };
        const float defaults[] = { 0.15f, 0.15f, 0.05f, 0.10f };
        for (int k=0; k<4; k++)
          {
          String v = d.style(p, "Margins", sides[k]);
          float value = v.isNull() ? defaults[k] : float(std::atof(v.data()));
          ImGui::SetNextItemWidth(160);
          if (ImGui::SliderFloat(sides[k], &value, 0.0f, 0.45f, "%.3f")) d.setStyle(p, "Margins", sides[k], String(number(value)));
          if (k % 2 == 0) ImGui::SameLine();
          }
        if (ImGui::SmallButton("Default margins")) { d.beginEdit(); for (const char * s : sides) d.setStyle(p, "Margins", s, ""); d.endEdit(); }
        ImGui::TreePop();
        }
      // everything else, with the editors of CapGui: written back as the differences to the defaults
      GRAPHICS::Graph graph;
      GraphXml::readGraph(*e, graph);
      bool changed = false;
      if (ImGui::TreeNode("Title: color, font, size"))   { changed |= GraphEditor::editTitle(graph); ImGui::TreePop(); }
      for (int k=0; k<3; k++)
        {
        const char * names[] = { "x axis: ticks, labels, title style", "y axis: ticks, labels, title style", "z axis: ticks, labels, title style" };
        if (ImGui::TreeNode(names[k])) { changed |= GraphEditor::editAxis(graph, k); changed |= GraphEditor::editAxisTitle(graph, k); ImGui::TreePop(); }
        }
      if (ImGui::TreeNode("Legend: position, text, box")) { changed |= GraphEditor::editLegend(graph); ImGui::TreePop(); }
      if (changed)
        {
        XmlElement full("Graph"), defaults("Graph");
        GraphXml::writeGraph(full, graph);
        GraphXml::writeGraph(defaults, GRAPHICS::Graph());
        XmlElement merged = *d.element(p);
        PlotDocument::mergeDifferences(merged, full, defaults);
        d.replace(p, merged);
        }
      break;
      }
    case PlotDocument::kHistogram:
      {
      std::vector<String> ids;
      for (const XmlElement * in : d.root().children("Input")) ids.push_back(in->attribute("id", ""));
      String input = e->attribute("input", "");
      ImGui::SetNextItemWidth(200);
      if (ImGui::BeginCombo("input", input.isNull() ? "(first)" : input.data()))
        {
        for (const String & id : ids) if (ImGui::Selectable(id.data(), id == input)) d.setAttribute(p, "input", id);
        ImGui::EndCombo();
        }
      textField(d, p, "path", "path", -1, "path of the histogram in the file");
      textField(d, p, "label", "label", -1, "text of the legend (ROOT or LaTeX syntax)");
      checkField(d, p, "showMarkers", "markers", true);
      ImGui::SameLine();
      checkField(d, p, "showLine", "line", false);
      ImGui::SameLine();
      checkField(d, p, "showErrors", "error bars", true);
      int color = itemColor(d, p, "Marker");
      colorStyle(d, p, "Marker", "marker color", color);
      ImGui::SameLine(0, 20);
      int marker = itemMarker(d, p);
      if (GraphEditor::markerEdit("marker", marker, color)) d.setStyle(p, "Marker", "style", String(std::to_string(marker)));
      String size = d.style(p, "Marker", "size");
      float s = size.isNull() ? 1.0f : float(std::atof(size.data()));
      ImGui::SetNextItemWidth(160);
      if (ImGui::SliderFloat("marker size", &s, 0.2f, 4.0f, "%.2f")) d.setStyle(p, "Marker", "size", String(number(s)));
      int lineColor = itemColor(d, p, "Line");
      colorStyle(d, p, "Line", "line color", lineColor);
      ImGui::SameLine(0, 20);
      String ls = d.style(p, "Line", "style");
      int lineStyle = ls.isNull() ? 1 : std::atoi(ls.data());
      if (GraphEditor::lineStyleEdit("line style", lineStyle)) d.setStyle(p, "Line", "style", String(std::to_string(lineStyle)));
      String lw = d.style(p, "Line", "width");
      float w = lw.isNull() ? 1.0f : float(std::atof(lw.data()));
      ImGui::SetNextItemWidth(160);
      if (ImGui::SliderFloat("line width", &w, 0.5f, 6.0f, "%.1f")) d.setStyle(p, "Line", "width", String(number(w)));
      if (ImGui::SmallButton("Default style"))
        {
        d.beginEdit();
        for (const char * child : { "Marker", "Line" })
          for (const char * a : { "color", "style", "size", "width", "alpha" }) d.setStyle(p, child, a, "");
        d.endEdit();
        }
      break;
      }
    case PlotDocument::kFunction:
      {
      std::vector<String> names = MATH::FunctionRegistry::names();
      for (const String & n : MATH::FunctionRegistry::compositeNames()) names.push_back(n);
      String cls = e->attribute("class", "");
      ImGui::SetNextItemWidth(220);
      if (ImGui::BeginCombo("class", cls.data()))
        {
        for (const String & n : names) if (ImGui::Selectable(n.data(), n == cls)) d.setAttribute(p, "class", n);
        ImGui::EndCombo();
        }
      textField(d, p, "parameters", "parameters", -1, "values separated by commas");
      textField(d, p, "label", "label");
      numberField(d, p, "xMin", "x min", -1.0);
      ImGui::SameLine();
      numberField(d, p, "xMax", "x max", 1.0);
      numberField(d, p, "yMin", "y min (rank 2)", 0.0);
      ImGui::SameLine();
      numberField(d, p, "yMax", "y max (rank 2)", 1.0);
      intField(d, p, "nSamples", "points drawn", 200, 2, 100000);
      int color = itemColor(d, p, "Line");
      colorStyle(d, p, "Line", "line color", color);
      ImGui::SameLine(0, 20);
      String ls = d.style(p, "Line", "style");
      int lineStyle = ls.isNull() ? 1 : std::atoi(ls.data());
      if (GraphEditor::lineStyleEdit("line style", lineStyle, false)) d.setStyle(p, "Line", "style", String(std::to_string(lineStyle)));
      String lw = d.style(p, "Line", "width");
      float w = lw.isNull() ? 2.0f : float(std::atof(lw.data()));
      ImGui::SetNextItemWidth(160);
      if (ImGui::SliderFloat("line width", &w, 0.5f, 6.0f, "%.1f")) d.setStyle(p, "Line", "width", String(number(w)));
      break;
      }
    default:
      {
      // any other element: its attributes as text
      std::vector<std::pair<String,String>> attributes = e->attributes();
      for (const auto & a : attributes)
        {
        std::string value = a.second.str();
        ImGui::SetNextItemWidth(260);
        if (ImGui::InputText(a.first.data(), &value)) d.setAttribute(p, a.first, String(value));
        ImGui::SameLine();
        ImGui::PushID(a.first.data());
        if (ImGui::SmallButton("x")) d.setAttribute(p, a.first, "");
        ImGui::PopID();
        }
      static std::string newName, newValue;
      ImGui::SetNextItemWidth(120);
      ImGui::InputTextWithHint("##newname", "attribute", &newName);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(140);
      ImGui::InputTextWithHint("##newvalue", "value", &newValue);
      ImGui::SameLine();
      if (ImGui::SmallButton("Add") && XmlElement::isValidName(String(newName)) && !newValue.empty())
        {
        d.setAttribute(p, String(newName), String(newValue));
        newName.clear();
        newValue.clear();
        }
      break;
      }
    }
  ImGui::PopID();
  }

  void CapGuiApp::renderDialogs()
  {
  if (_closeDialog) { ImGui::OpenPopup("Save changes?"); _closeDialog = false; }
  if (ImGui::BeginPopupModal("Save changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
    int k = -1;
    for (std::size_t j=0; j<_documents.size() && !_closing.empty(); j++) if (_documents[j].id == _closing.front()) k = int(j);
    if (k < 0) { _closing.clear(); ImGui::CloseCurrentPopup(); }
    else
      {
      ImGui::Text("Save the changes made to %s?", _documents[std::size_t(k)].document->displayName().data());
      auto nextOne = [this]()
        {
        _closing.erase(_closing.begin());
        if (_closing.empty() && _exitAfterClosing) _gui.requestExit();
        else if (!_closing.empty()) _closeDialog = true;
        };
      if (ImGui::Button("Save", ImVec2(110, 0)))
        {
        int id = _closing.front();
        ImGui::CloseCurrentPopup();
        save(k, false, [this, id, nextOne]()
          {
          for (std::size_t j=0; j<_documents.size(); j++) if (_documents[j].id == id) { closeTab(int(j)); break; }
          nextOne();
          });
        }
      ImGui::SameLine();
      if (ImGui::Button("Don't Save", ImVec2(110, 0)))
        {
        closeTab(k);
        ImGui::CloseCurrentPopup();
        nextOne();
        }
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(110, 0)))
        {
        _closing.clear();
        _exitAfterClosing = false;
        ImGui::CloseCurrentPopup();
        }
      }
    ImGui::EndPopup();
    }
  if (_showHelp)
    {
    ImGui::SetNextWindowSize(ImVec2(760, 360), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("CapGui Help", &_showHelp)) helpText();
    ImGui::End();
    }
  if (_gridDialog) { ImGui::OpenPopup("Grid"); _gridDialog = false; }
  if (ImGui::BeginPopup("Grid"))
    {
    ImGui::TextDisabled("%s", _gridOfGraphs ? "graphs of the current canvas" : "canvases of the window");
    if (gridPicker(_gridColumns, _gridRows)) { setGrid(_gridOfGraphs, _gridColumns, _gridRows); ImGui::CloseCurrentPopup(); }
    ImGui::EndPopup();
    }
  }

  // ------------------------------------------------------------------------------------------------ canvases

  int CapGuiApp::paneIndexOf(const CanvasPane * pane)
  {
  for (int k=0; k<_gui.nPanes(); k++) if (&_gui.pane(k) == pane) return k;
  return -1;
  }

  std::vector<int> CapGuiApp::closeCanvases(const std::vector<int> & panes)
  {
  std::vector<int> rest;
  std::vector<Path> canvases;
  Tab * t = tab();
  for (int k : panes)
    {
    const PaneLink * found = nullptr;
    for (const auto & link : _links)
      if (paneIndexOf(link.pane) == k && !link.canvas.empty()) { found = &link; break; }
    if (t && found && t->document->element(found->canvas)) canvases.push_back(found->canvas);
    else rest.push_back(k);
    }
  if (canvases.empty()) return rest;
  // removed last first, so that the paths of the others stay valid
  std::sort(canvases.begin(), canvases.end());
  canvases.erase(std::unique(canvases.begin(), canvases.end()), canvases.end());
  PlotDocument & d = *t->document;
  d.beginEdit();
  for (auto p = canvases.rbegin(); p != canvases.rend(); ++p)
    {
    if (t->hasSelection && t->selected.size() >= p->size() && std::equal(p->begin(), p->end(), t->selected.begin()))
      t->selected = PlotDocument::parent(*p);   // the selection was in the canvas: its window
    d.remove(*p);
    }
  d.endEdit();
  _gui.setStatus(canvases.size() == 1 ? String("Canvas removed from the plot file (Undo to restore it)")
                                      : String("Canvases removed from the plot file (Undo to restore them)"));
  return rest;
  }

  PlotDocument::Path CapGuiApp::dropTarget()
  {
  Tab * t = tab();
  if (!t || !t->hasSelection) return Path();
  bool ok = false;
  Path g = t->document->enclosing(t->selected, PlotDocument::kGraph, ok);
  if (ok) return g;
  Path c = t->document->enclosing(t->selected, PlotDocument::kCanvas, ok);
  if (ok) return c;
  return t->selected;
  }

  void CapGuiApp::layoutCanvases()
  {
  Tab * t = tab();
  if (!t)
    {
    _links.clear();
    return;
    }
  PlotDocument & d = *t->document;
  std::vector<Path> windows = d.children(Path(), PlotDocument::kWindow);
  int window = _drawnTab == t->id ? _drawnWindow : 0, canvas = -1;
  if (t->hasSelection) d.locate(t->selected, window, canvas);
  if (window < 0 || window >= int(windows.size())) window = windows.empty() ? -1 : 0;
  if (_allWindows) window = windows.empty() ? -1 : 0;
  bool linksValid = true;
  for (const auto & link : _links) if (paneIndexOf(link.pane) < 0) linksValid = false;   // a pane closed by the user
  if (linksValid && _drawnTab == t->id && _drawnVersion == d.version() && _drawnWindow == window && _drawnAll == _allWindows) return;
  // shapes drawn on the canvases are kept when the canvases are drawn again
  std::map<std::string, std::vector<GRAPHICS::Shape>> shapes;
  for (const auto & link : _links)
    {
    if (paneIndexOf(link.pane) < 0) continue;
    const GRAPHICS::Canvas & c = link.pane->canvas();
    for (int k=0; k<c.nGraphs(); k++)
      if (!c.graph(k).shapes().empty())
        {
        std::string key;
        for (int i : link.canvas) key += std::to_string(i) + ".";
        shapes[key + "#" + std::to_string(k)] = c.graph(k).shapes();
        }
    }
  _links.clear();
  _drawnTab = t->id;
  _drawnVersion = d.version();
  _drawnWindow = window;
  _drawnAll = _allWindows;
  std::unique_ptr<GRAPHICS::PlotSpec> spec(new GRAPHICS::PlotSpec());
  String error, problems;
  String title = String("CapGui: ") + d.displayName();
  if (window < 0 || !spec->read(d.document(), d.directory(), error))
    {
    _gui.closePaneWindowsFrom(0);
    _gui.setPaneCount(1);
    CanvasPane & pane = _gui.pane(0);
    pane.canvas() = GRAPHICS::Canvas();
    pane.divide(1, 1);
    pane.setSelectedGraph(0);
    pane.setTitle(window < 0 ? "Drop histograms here" : "Cannot draw the plot file");
    pane.onDrop = [this](int, const ObjectReference & object) { drop(Path(), object, false); };
    if (window >= 0) _gui.setStatus(String("Cannot draw the plot file: ") + error);
    _gui.setWindowTitle(title);
    _spec = std::move(spec);
    return;
    }
  // the inputs, opened by the application (shown in the browser, shared with it)
  for (std::size_t k=0; k<spec->inputs().size(); k++)
    {
    String path = spec->resolve(spec->inputs()[k].path);
    if (!FileSystem::isFile(path))
      {
      if (_missing.insert(path.str()).second) _gui.setStatus(String("No input file ") + path);
      continue;
      }
    BrowserFile * f = fileOfPath(path);
    int id = f ? f->id : openFile(path);
    f = id >= 0 ? file(id) : nullptr;
    if (f) spec->setInputFile(int(k), f->file);
    }
  // the windows drawn: one (the main window), or all (the first in the main window)
  std::vector<int> shown;
  if (_allWindows) for (int w=0; w<int(windows.size()); w++) shown.push_back(w);
  else shown.push_back(window);
  _gui.closePaneWindowsFrom(int(shown.size()) - 1);
  for (std::size_t s=0; s<shown.size(); s++)
    {
    int w = shown[s], host = int(s) - 1;
    const GRAPHICS::PlotSpec::WindowSpec & ws = spec->windows()[std::size_t(w)];
    int n = int(ws.canvases.size());
    int columns = std::max(1, std::min(6, ws.columns));
    int rows = std::max(1, std::min(6, std::max(ws.rows, (std::max(n, 1) + columns - 1)/columns)));
    if (host < 0)
      {
      if (_gui.nColumns() != columns || _gui.nRows() != rows || int(_gui.panesOf(-1).size()) < columns*rows) _gui.setLayout(columns, rows);
      title += String(": ") + ws.title;
      }
    else
      {
      _gui.setPaneWindow(host, ws.title, columns, rows, ws.width, ws.height);
      _gui.setPaneWindowPaneCount(host, std::max(1, n));
      }
    std::vector<Path> canvasPaths = d.children(windows[std::size_t(w)], PlotDocument::kCanvas);
    std::vector<CanvasPane *> panes = _gui.panesOf(host);
    for (int k=0; k<int(panes.size()); k++)
      {
      CanvasPane & pane = *panes[std::size_t(k)];
      if (k >= n)
        {
        pane.canvas() = GRAPHICS::Canvas();
        pane.divide(1, 1);
        pane.setSelectedGraph(0);
        pane.setTitle("(empty cell)");
        Path windowPath = windows[std::size_t(w)];
        pane.onDrop = [this, windowPath](int, const ObjectReference & object) { drop(windowPath, object, false); };
        continue;
        }
      pane.canvas() = GRAPHICS::Canvas();   // nothing of the canvas drawn before stays (logarithmic axes, ranges, ...)
      spec->build(ws.canvases[std::size_t(k)], pane.canvas(), problems);
      pane.setTitle(ws.canvases[std::size_t(k)].title);
      if (pane.selectedGraph() >= pane.canvas().nGraphs()) pane.setSelectedGraph(0);
      PaneLink link;
      link.pane = &pane;
      link.canvas = k < int(canvasPaths.size()) ? canvasPaths[std::size_t(k)] : Path();
      // the Graph element of each cell, as PlotSpec::build() places them
      int next = 0;
      for (const Path & g : d.children(link.canvas, PlotDocument::kGraph))
        {
        const XmlElement * e = d.element(g);
        int cell = e->hasAttribute("cell") ? int(e->attributeInt("cell")) : next;
        next = cell + 1;
        link.graphs[cell] = g;
        }
      std::string key;
      for (int i : link.canvas) key += std::to_string(i) + ".";
      for (int g=0; g<pane.canvas().nGraphs(); g++)
        {
        auto found = shapes.find(key + "#" + std::to_string(g));
        if (found != shapes.end()) for (const auto & shape : found->second) pane.canvas().graph(g).addShape(shape);
        }
      link.drawn = spec->canvasXml(pane.canvas());
      link.drawnText = link.drawn.toString().str();
      Path canvasPath = link.canvas;
      int id = t->id;
      pane.onDrop = [this, id, canvasPath](int graph, const ObjectReference & object)
        {
        if (!tab() || tab()->id != id) return;
        Path target = canvasPath;
        for (const auto & l : _links)
          if (l.canvas == canvasPath) { auto f = l.graphs.find(graph); if (f != l.graphs.end()) target = f->second; }
        drop(target, object, false);
        };
      _links.push_back(link);
      }
    }
  _gui.setWindowTitle(title);
  _spec = std::move(spec);   // the old specification goes after the canvases stopped using its functions
  if (!problems.isNull())
    {
    std::string first = problems.str().substr(0, problems.str().find('\n'));
    _gui.setStatus(String("Not drawn: ") + String(first));
    }
  // the selection follows on the canvases
  _seenPane = nullptr;
  _seenHasSelection = !t->hasSelection;
  }

  void CapGuiApp::recordCanvases()
  {
  Tab * t = tab();
  if (!t || !_spec || _links.empty() || _drawnTab != t->id || _drawnVersion != t->document->version()) return;
  // the canvases as drawn, once drawn (the first frames complete their state)
  for (auto & link : _links)
    if (link.settle > 0 && --link.settle == 0 && paneIndexOf(link.pane) >= 0)
      {
      link.drawn = _spec->canvasXml(link.pane->canvas());
      link.drawnText = link.drawn.toString().str();
      }
  // not while the mouse is pressed or a widget is in use: once an edit on a canvas is done
  if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsAnyItemActive()) return;
  if (++_frame % 6 != 0) return;
  PlotDocument & d = *t->document;
  for (auto & link : _links)
    {
    if (link.settle > 0 || paneIndexOf(link.pane) < 0 || !d.element(link.canvas)) continue;
    XmlElement now = _spec->canvasXml(link.pane->canvas());
    std::string text = now.toString().str();
    if (text == link.drawnText) continue;
    XmlElement element = *d.element(link.canvas);
    if (PlotDocument::syncCanvas(element, link.drawn, now)) d.replace(link.canvas, element);
    link.drawn = now;
    link.drawnText = text;
    // the canvases already show it: not drawn again; the cells of the graphs may have new elements
    int next = 0;
    link.graphs.clear();
    for (const Path & g : d.children(link.canvas, PlotDocument::kGraph))
      {
      const XmlElement * e = d.element(g);
      int cell = e->hasAttribute("cell") ? int(e->attributeInt("cell")) : next;
      next = cell + 1;
      link.graphs[cell] = g;
      }
    _drawnVersion = d.version();
    }
  }

  void CapGuiApp::syncSelection()
  {
  Tab * t = tab();
  if (!t || _gui.nPanes() == 0) return;
  CanvasPane & pane = _gui.currentPane();
  int graph = pane.selectedGraph();
  bool treeChanged = t->hasSelection != _seenHasSelection || (t->hasSelection && !(t->selected == _seenSelection));
  bool canvasChanged = &pane != _seenPane || graph != _seenGraph;
  if (treeChanged && t->hasSelection)
    {
    // the canvas of the selection made current, its graph selected
    for (const auto & link : _links)
      {
      if (t->selected.size() < link.canvas.size() || !std::equal(link.canvas.begin(), link.canvas.end(), t->selected.begin())) continue;
      int k = paneIndexOf(link.pane);
      if (k < 0) break;
      _gui.setCurrentPane(k, false);
      for (const auto & g : link.graphs)
        if (t->selected.size() >= g.second.size() && std::equal(g.second.begin(), g.second.end(), t->selected.begin()))
          link.pane->setSelectedGraph(g.first);
      break;
      }
    _seenPane = &_gui.currentPane();
    _seenGraph = _gui.currentPane().selectedGraph();
    }
  else if (canvasChanged && _seenPane)
    {
    // a graph clicked on a canvas: its Graph selected in the tree
    for (const auto & link : _links)
      if (link.pane == &pane)
        {
        auto found = link.graphs.find(graph);
        t->selected = found != link.graphs.end() ? found->second : link.canvas;
        t->hasSelection = true;
        }
    _seenPane = &pane;
    _seenGraph = graph;
    }
  else if (!_seenPane) { _seenPane = &pane; _seenGraph = graph; }
  _seenSelection = t->selected;
  _seenHasSelection = t->hasSelection;
  }

  // ------------------------------------------------------------------------------------------------ new from data

  void CapGuiApp::startTemplate(const GRAPHICS::PlotTemplate & catalog, const GRAPHICS::PlotTemplate::Options & options,
                                const String & fileName)
  {
  _template = catalog;
  _templateOptions = options;
  _templateFile = fileName.str();
  _templateFilter.clear();
  _templateOpen = true;
  }

  bool CapGuiApp::createFromTemplate()
  {
  if (_template.nSelected() == 0) { _gui.showMessage("Plot Template", "No histogram selected"); return false; }
  XmlDocument document = _template.makeSpec(_templateOptions);
  if (!_templateFile.empty())
    {
    String path(_templateFile.c_str());
    if (!FileSystem::hasExtension(path, ".xml")) path += String(".xml");
    try { document.write(path); }
    catch (const std::exception & e) { _gui.showMessage("Plot Template", path + String(": ") + String(e.what())); return false; }
    return openDocument(path);
    }
  int k = newDocument();
  PlotDocument & d = *_documents[std::size_t(k)].document;
  d.document() = document;
  // the inputs of the catalog are absolute or relative to the current folder: written absolute until saved
  for (XmlElement * e : d.document().root().children())
    if (e->isNamed("Input")) e->setAttribute("file", FileSystem::normalize(e->attribute("file", "")));
  d.setDirty(true);
  return true;
  }

  void CapGuiApp::renderTemplate()
  {
  if (!_templateOpen) return;
  ImGui::SetNextWindowSize(ImVec2(820, 640), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Plot Template", &_templateOpen)) { ImGui::End(); return; }
  using Options = GRAPHICS::PlotTemplate::Options;
  ImGui::Text("Histograms: %d, tasks: %d, selected: %d", int(_template.entries().size()), int(_template.tasks().size()), _template.nSelected());
  ImGui::SetNextItemWidth(260);
  ImGui::InputTextWithHint("##filter", "filter the names (e.g. *R2*)", &_templateFilter);
  ImGui::SameLine();
  auto shown = [this](const String & name) { return _templateFilter.empty() || GRAPHICS::PlotTemplate::matches(name, String(_templateFilter.c_str())); };
  if (ImGui::Button("Select shown"))
    for (const String & t : _template.tasks()) for (const String & n : _template.names(t)) if (shown(n)) _template.setNameSelected(t, n, true);
  ImGui::SameLine();
  if (ImGui::Button("Unselect shown"))
    for (const String & t : _template.tasks()) for (const String & n : _template.names(t)) if (shown(n)) _template.setNameSelected(t, n, false);
  ImGui::Separator();
  float optionsHeight = 7.0f*ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("##catalog", ImVec2(0, -optionsHeight), ImGuiChildFlags_Borders);
  for (const String & task : _template.tasks())
    {
    String label = task.isNull() ? String("(top folder)") : task;
    ImGui::PushID(label.data());
    if (ImGui::TreeNodeEx(label.data(), ImGuiTreeNodeFlags_DefaultOpen))
      {
      if (ImGui::SmallButton("All")) _template.selectAll(task, true);
      ImGui::SameLine();
      if (ImGui::SmallButton("None")) _template.selectAll(task, false);
      if (ImGui::BeginTable("##lists", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
        {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("Histograms");
        for (const String & name : _template.names(task))
          {
          if (!shown(name)) continue;
          bool v = _template.isNameSelected(task, name);
          String text = name + (_template.rankOf(task, name) == 2 ? String("  (2D)") : String(""));
          if (ImGui::Checkbox((text + String("##n")).data(), &v)) _template.setNameSelected(task, name, v);
          }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextDisabled("Combinations of filters");
        for (const String & cb : _template.combinations(task))
          {
          bool v = _template.isCombinationSelected(task, cb);
          if (ImGui::Checkbox(((cb.isNull() ? String("(none)") : cb) + String("##c")).data(), &v)) _template.setCombinationSelected(task, cb, v);
          }
        ImGui::EndTable();
        }
      ImGui::TreePop();
      }
    ImGui::PopID();
    }
  ImGui::EndChild();
  int grouping = int(_templateOptions.grouping);
  ImGui::TextUnformatted("Canvases:");
  ImGui::SameLine();
  if (ImGui::RadioButton("one per histogram, a graph per combination", grouping == 0)) _templateOptions.grouping = Options::kByHistogram;
  ImGui::SameLine();
  if (ImGui::RadioButton("one per combination, a graph per histogram", grouping == 1)) _templateOptions.grouping = Options::kByCombination;
  ImGui::Checkbox("Overlay the combinations of 1D histograms in one graph", &_templateOptions.overlay);
  ImGui::SetNextItemWidth(120);
  ImGui::InputInt("graphs per canvas at most", &_templateOptions.maxGraphs);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::InputInt("canvases per window at most", &_templateOptions.maxCanvases);
  _templateOptions.maxGraphs = std::max(1, std::min(36, _templateOptions.maxGraphs));
  _templateOptions.maxCanvases = std::max(1, std::min(36, _templateOptions.maxCanvases));
  ImGui::SetNextItemWidth(420);
  ImGui::InputTextWithHint("plot file", "empty: a new, untitled plot file", &_templateFile);
  if (ImGui::Button(_templateFile.empty() ? "Create" : "Write and Open")) { if (createFromTemplate()) _templateOpen = false; }
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) _templateOpen = false;
  ImGui::End();
  }

  } // namespace GUI
} // namespace CAP

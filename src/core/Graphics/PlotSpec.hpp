/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotSpec.hpp
//! @brief PlotSpec: windows, canvases and graphs of histograms and functions described in an XML file (PlotHistos);
//! GraphXml: the style of graphs, items, axes and attributes read from and written to XML elements.
#ifndef CAP__PlotSpec
#define CAP__PlotSpec
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "String.hpp"
#include "XmlDocument.hpp"
#include "XmlElement.hpp"
#include "Canvas.hpp"
#include "Graph.hpp"

namespace CAP
{
  class CapFile;
  namespace MATH
  {
  class DataPoints;
  class Function;
  class ParameterizedFunction;
  }
  namespace GRAPHICS
  {

  //!
  //! @brief The style of the graphics objects as XML: attributes, margins, axes, graphs and the items of graphs.
  //!
  //! Each read function changes only what the element gives (attributes or children that are present); each write
  //! function writes every setting, so that what is written reads back into the same state.
  //!
  //! Colors are written as their names in the color table ("black", "kRed+2", "#1f77b4"); they are read as names, as
  //! "#rrggbb", or as ROOT color numbers ("2", "634").
  //!
  namespace GraphXml
  {
  //! @brief Color number of a text: a ROOT number, a name of the color table, or "#rrggbb". @param text text
  //! @param fallback color returned if the text is not a color @return color number
  int  parseColor(const String & text, int fallback);
  //! @brief Name of a color number (see ColorTable::name()). @param color number @return name
  String colorName(int color);

  //! @brief Read a line (color, alpha, style, width). @param element element (null: nothing) @param line line
  void read(const XmlElement * element, LineAttributes & line);
  //! @brief Read a fill (color, alpha, style). @param element element (null: nothing) @param fill fill
  void read(const XmlElement * element, FillAttributes & fill);
  //! @brief Read a marker (color, alpha, style, size). @param element element (null: nothing) @param marker marker
  void read(const XmlElement * element, MarkerAttributes & marker);
  //! @brief Read a text style (color, alpha, font, size). @param element element (null: nothing) @param text text
  void read(const XmlElement * element, TextAttributes & text);
  //! @brief Read margins (left, bottom, right, top: fractions of the cell). @param element element @param margins margins
  void read(const XmlElement * element, Margins & margins);

  //! @brief Write a line as attributes of an element. @param element element @param line line
  void write(XmlElement & element, const LineAttributes & line);
  //! @brief Write a fill. @param element element @param fill fill
  void write(XmlElement & element, const FillAttributes & fill);
  //! @brief Write a marker. @param element element @param marker marker
  void write(XmlElement & element, const MarkerAttributes & marker);
  //! @brief Write a text style. @param element element @param text text
  void write(XmlElement & element, const TextAttributes & text);
  //! @brief Write margins. @param element element @param margins margins
  void write(XmlElement & element, const Margins & margins);

  //! @brief Read the style of axis k of a graph (0 x, 1 y, 2 z, 3 the axis of the residual panel): title, range,
  //! scale, ticks, labels, sizes and positions. @param element Axis element @param graph graph @param k axis
  void readAxis(const XmlElement & element, Graph & graph, int k);
  //! @brief Write axis k of a graph as an Axis element. @param parent parent element @param graph graph @param k axis
  void writeAxis(XmlElement & parent, const Graph & graph, int k);
  //! @brief Read the style of a graph (not its items): title, margins, axes, frame, legend, maps, fits, residuals.
  //! @param element Graph element @param graph graph
  void readGraph(const XmlElement & element, Graph & graph);
  //! @brief Write the style of a graph (not its items) as attributes and children of a Graph element.
  //! @param element Graph element @param graph graph
  void writeGraph(XmlElement & element, const Graph & graph);
  //! @brief Read the style of an item: what is drawn, line, marker, uncertainties, fits, maps.
  //! @param element item element @param item item
  void readItem(const XmlElement & element, GraphItem & item);
  //! @brief Write the style of an item. @param element item element @param item item
  void writeItem(XmlElement & element, const GraphItem & item);
  //! @brief Read the size, background and grid of a canvas (not its graphs). @param element Canvas element
  //! @param canvas canvas
  void readCanvas(const XmlElement & element, Canvas & canvas);
  //! @brief Write the size, background and grid of a canvas. @param element Canvas element @param canvas canvas
  void writeCanvas(XmlElement & element, const Canvas & canvas);
  } // namespace GraphXml

  //!
  //! @brief Windows of canvases of histograms and functions, described in an XML file: what PlotHistos shows, prints
  //! and exports.
  //!
  //! @code
  //!   <PlotHistos version="1">
  //!     <Input id="data" file="results.cap"/>                   <!-- paths relative to the XML file -->
  //!     <Output directory="figures" pdf="plots.pdf" pptx="plots.pptx" latex="presentation.tex" latexStyle="beamer"
  //!             title="Spectra" author="C. Pruneau" affiliation="Wayne State University"/>
  //!     <Window title="Spectra" width="1400" height="900" columns="2" rows="1">       <!-- n x m canvases -->
  //!       <Canvas name="pt" title="pT spectra" width="800" height="600" columns="2" rows="1" file="pt.pdf">
  //!         <Background color="white"/>
  //!         <Graph title="#pi^{+}" file="pt_pip.pdf" legend="true">                 <!-- graphs row by row -->
  //!           <Margins left="0.15" bottom="0.15" right="0.05" top="0.1"/>
  //!           <Axis which="x" title="p_{T} (GeV/c)" min="0" max="3"/>
  //!           <Axis which="y" title="dN/dp_{T}" log="true"/>
  //!           <Histogram input="data" path="Ana/MB_PiP/n1_pt" label="#pi^{+}">
  //!             <Marker color="kRed+1" style="20" size="1"/>
  //!           </Histogram>
  //!           <Function class="Exponential" parameters="100, -2" xMin="0" xMax="3" label="fit">
  //!             <Line color="blue" style="2" width="2"/>
  //!           </Function>
  //!         </Graph>
  //!       </Canvas>
  //!     </Window>
  //!   </PlotHistos>
  //! @endcode
  //!
  //! Every element and attribute is optional except the paths of the histograms. A canvas without columns and rows is
  //! divided for the number of its Graph elements; a graph takes the next cell, or the one given by cell="k". An item
  //! takes the default style of its place in the graph, changed by its attributes and children. Histogram (or Data)
  //! items are data points of the inputs (histograms, profiles, points), found by their paths; Function items are
  //! functions of the registry of the Math library (a class, e.g. Normal, Exponential, Polynomial, or a registered
  //! composite) with the given parameters, drawn over [xMin, xMax] (and [yMin, yMax] for a function of rank 2).
  //!
  //! The settings of the canvases, changed on screen or not, are written back with write(); print() writes each
  //! canvas and graph that has a file name to that file (PDF or SVG by its extension).
  //!
  class PlotSpec
  {
  public:
  //! An input file
  struct InputSpec
  {
  String id;     //!< identifier used by the items (input="...")
  String path;   //!< path of the CAP file
  std::shared_ptr<CapFile> file;   //!< contents (null until opened)
  };
  //! A canvas of a window
  struct CanvasSpec
  {
  const XmlElement * element = nullptr;   //!< its element in the document
  String name;    //!< name (unique among the canvases)
  String title;   //!< title (the name if not given)
  };
  //! A window of canvases
  struct WindowSpec
  {
  String title;        //!< title of the window
  int    width = 1200;   //!< size (points)
  int    height = 800;   //!< size (points)
  int    columns = 1;   //!< grid of canvases
  int    rows = 1;      //!< grid of canvases
  std::vector<CanvasSpec> canvases;   //!< canvases, row by row
  };
  //! Where the outputs go
  struct OutputSpec
  {
  String directory = ".";      //!< directory of the outputs (relative to the XML file)
  String pdf = "plots.pdf";     //!< PDF book: one canvas per page
  String pptx = "plots.pptx";   //!< PowerPoint: one canvas per slide
  String latex = "plots_latex.tex";   //!< LaTeX document (not named as the PDF book: pdflatex would replace it)
  String latexStyle = "beamer"; //!< "beamer" (a presentation, one frame per canvas) or "article" (one figure per canvas)
  bool   latexCompile = false;  //!< compile the LaTeX document with pdflatex
  String title = "Plots";       //!< title of the presentation or document
  String author;                //!< author
  String affiliation;           //!< affiliation of the author
  };
  //! A window as shown: what write() records
  struct WindowState
  {
  String title;        //!< title
  int    width = 1200;   //!< size (points)
  int    height = 800;   //!< size (points)
  int    columns = 1;   //!< grid of canvases
  int    rows = 1;      //!< grid of canvases
  std::vector<const Canvas *> canvases;   //!< canvases, row by row
  };
  //! Names given to a canvas built from the specification, for printing and writing
  struct CanvasInfo
  {
  String name;    //!< name
  String file;    //!< file the canvas is printed to ("": none)
  int width = 0;  //!< size of the canvas in the outputs and in the file written (0: its size on screen)
  int height = 0; //!< size of the canvas in the outputs and in the file written
  std::vector<String> graphFiles;   //!< file each graph is printed to ("": none)
  };

  //! @brief Empty specification.
  PlotSpec();
  //! @brief Destructor.
  ~PlotSpec();
  PlotSpec(const PlotSpec &) = delete;
  PlotSpec & operator=(const PlotSpec &) = delete;

  //! @brief Read a specification file. @param fileName XML file @param error set to the reason of a failure
  //! @return false on failure
  bool read(const String & fileName, String & error);
  //! @brief Read a specification from a document. @param document document @param directory directory of the
  //! relative paths @param error set to the reason of a failure @return false on failure
  bool read(const XmlDocument & document, const String & directory, String & error);
  //! @brief File read last ("" if none). @return path
  const String & fileName() const { return _fileName; }
  //! @brief Directory of the relative paths (that of the file read). @return directory
  const String & directory() const { return _directory; }

  const std::vector<InputSpec> & inputs() const { return _inputs; }   //!< Input files
  const std::vector<WindowSpec> & windows() const { return _windows; }   //!< Windows
  const OutputSpec & output() const { return _output; }   //!< Outputs
  OutputSpec & output() { return _output; }   //!< Outputs

  //! @brief Replace the path of input k (e.g. a file given on the command line); an input is added if k is the
  //! number of inputs. @param k index @param path path
  void setInputPath(int k, const String & path);
  //! @brief Give the contents of input k, opened elsewhere (e.g. by CapGui, which shows it in its browser).
  //! @param k index @param file contents
  void setInputFile(int k, const std::shared_ptr<CapFile> & file);
  //! @brief Open the inputs not opened yet (objects read when first used). @param error set to the reason of a
  //! failure @return false if one cannot be opened
  bool openInputs(String & error);
  //! @brief A path of the specification (inputs, outputs) resolved: relative paths are relative to the directory of
  //! the XML file. @param path path @return path
  String resolve(const String & path) const;
  //! @brief Path of an output file (relative to the output directory, itself relative to the XML file).
  //! @param name file name ("" for the directory) @return path
  String outputPath(const String & name) const;

  //!
  //! @brief Make a canvas as described: size, background, grid of graphs, graphs with their items and styles.
  //! @param spec canvas @param canvas canvas (its graphs are replaced) @param problems set to what could not be shown
  //! (objects not found...), one per line
  //!
  void build(const CanvasSpec & spec, Canvas & canvas, String & problems);
  //! @brief Make all the canvases of all the windows (programs without a window: batch mode).
  //! @param problems set to what could not be shown @return windows, with their canvases (owned by the
  //! specification)
  std::vector<WindowState> buildAll(String & problems);

  //! @brief Names of a canvas made by build() (or set with setCanvasInfo()). @param canvas canvas @return names, or
  //! null for a canvas not made from the specification
  const CanvasInfo * canvasInfo(const Canvas & canvas) const;
  //! @brief Set the names of a canvas. @param canvas canvas @param info names
  void setCanvasInfo(const Canvas & canvas, const CanvasInfo & info) { _canvasInfo[&canvas] = info; }

  //! Finds the input and the path of data points shown but not made by build() (e.g. added on screen); returns
  //! false if they are not found.
  std::function<bool(const MATH::DataPoints * data, String & input, String & path)> locateData;

  //!
  //! @brief The specification of windows as shown: the outputs, the inputs, and the windows with their canvases,
  //! graphs and items, every setting written. Items whose objects cannot be named (see locateData) are left out.
  //! @param windows windows @param directory directory of the file the document is for: if it is not that of the
  //! specification, the relative paths (inputs, output directory) are written absolute, so that they still point to
  //! the same files @return document
  //!
  XmlDocument toXml(const std::vector<WindowState> & windows, const String & directory="") const;
  //! @brief The element a canvas is written as by toXml() (every setting; for PlotDocument::syncCanvas()).
  //! @param canvas canvas @return Canvas element
  XmlElement canvasXml(const Canvas & canvas) const;
  //! @brief Write toXml() to a file. @param fileName file @param windows windows @param error set to the reason of
  //! a failure @return false on failure
  bool write(const String & fileName, const std::vector<WindowState> & windows, String & error) const;

  //!
  //! @brief Print the canvases and graphs that have a file name (CanvasInfo) to their files, in the output
  //! directory: PDF or SVG by the extension. @param canvases canvases @return paths of the files written
  //!
  std::vector<String> print(const std::vector<const Canvas *> & canvases) const;
  //!
  //! @brief The canvases at the size the specification gives them (CanvasInfo), for the outputs: on screen, a canvas
  //! takes the size of its pane. @param canvases canvases @param copies copies made of the canvases resized (owned
  //! by the caller) @return canvases to draw, in order
  //!
  std::vector<const Canvas *> atOutputSize(const std::vector<const Canvas *> & canvases,
                                           std::vector<std::unique_ptr<Canvas>> & copies) const;
  //! @brief Write canvases as the pages of one PDF file, each page the size of its canvas. @param fileName file
  //! @param canvases canvases @return false if the file cannot be written
  static bool writePdf(const String & fileName, const std::vector<const Canvas *> & canvases);

  private:
  String _fileName;    //!< file read
  String _directory;   //!< directory of the relative paths
  XmlDocument _document;   //!< document read
  std::vector<InputSpec>  _inputs;   //!< inputs
  std::vector<WindowSpec> _windows;  //!< windows
  OutputSpec _output;   //!< outputs
  std::map<const Canvas *, CanvasInfo> _canvasInfo;   //!< names of the canvases made
  //! Objects shown, by pointer: their input and path
  std::map<const MATH::DataPoints *, std::pair<String,String>> _dataNames;
  //! Function made from the specification
  struct FunctionRecord { std::unique_ptr<MATH::ParameterizedFunction> function; String className; };
  std::vector<FunctionRecord> _functions;   //!< functions made (owned)
  std::vector<std::unique_ptr<Canvas>> _ownedCanvases;   //!< canvases of buildAll()

  //! @brief Index of an input of an id ("" for the first input). @param id id @return index, or -1
  int inputIndex(const String & id) const;
  //! @brief Add the items of a graph element to a graph. @param element Graph element @param graph graph
  //! @param problems problems (appended)
  void addItems(const XmlElement & element, Graph & graph, String & problems);
  //! @brief Write the items of a graph. @param element Graph element @param graph graph
  void writeItems(XmlElement & element, const Graph & graph) const;
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PlotSpec

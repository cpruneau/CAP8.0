/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitDialog.hpp
//! @brief FitDialog: the Fit window of CapGui, fitting a function to an object of the selected graph.
#ifndef CAP__GUI__DetectorMapsDialog
#define CAP__GUI__DetectorMapsDialog
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "String.hpp"
#include "FileBrowser.hpp"
#include "KinematicMapFolder.hpp"
#include "BezierCurve.hpp"

namespace CAP
{
  namespace MATH
  {
  class DataPoints;
  class Function;
  class Histogram;
  class ParameterizedFunction;
  }

  namespace GUI
  {
  class FileDialog;

  //!
  //! @brief The Detector Maps window of CapGui (Edit > Detector Maps...): views and edits the particle efficiencies
  //! (ParticleEfficiency) and smearers (ParticleSmear) of a maps file, the CAP file that the jobs import (Import of the
  //! efficiencies and smearers in the Filters pane of RunCAP).
  //!
  //! A maps file holds any number of efficiencies and smearers, each in a folder named after it (see
  //! KinematicMapFolder). The window lists them (add, rename, remove) and, for the selected one, each of its maps (the
  //! efficiency; the resolutions of pT, eta, phi, their deviate distributions and their biases), with its source:
  //!  - None (smearers) or Constant: its value;
  //!  - Function: a function of MATH::FunctionRegistry, its number of parameters, the parameters by name and its
  //!    variables in order; "To histogram" samples it at the bin centers of the binning of the object;
  //!  - Histogram: a histogram dropped from the file browser (e.g. an efficiency or resolution of the output of
  //!    PerformanceDerivedCalculator or CalibrationProducer), made empty (filled with a value) with the binning of the
  //!    object, its axes assigned to variables, its bins edited in a table.
  //! "Show" puts a copy of a map in the current graph (a function of rank 1 or 2, a histogram); after it is fitted
  //! there (Edit > Fit...), "Adopt fit" makes the fitted function the source of the map. Values outside the allowed
  //! range of a map (efficiencies in [0, 1], resolutions >= 0) are refused, with the reason shown.
  //!
  class DetectorMapsDialog
  {
  public:

  //! @brief Constructor: a closed window, with an empty list.
  DetectorMapsDialog();
  //! @brief Destructor.
  ~DetectorMapsDialog();
  //! @brief Title of the window (with its ImGui id). @return title
  static const char * windowName() { return "Detector Maps###DetectorMaps"; }
  //! @brief Open the window, and a maps file if one is given. @param fileName file ("": keep the current content)
  void open(const String & fileName = "");
  //! @brief Whether the window is open. @return true if it is
  bool isOpen() const { return _open; }
  //! @brief Draw the window if open; call once per frame.
  void render();

  //! @brief Read a maps file (replacing the list). @param fileName file @param error set to the reason of a failure
  //! @return false if it cannot be read
  bool load(const String & fileName, String & error);
  //! @brief Write the list to a maps file. @param fileName file (".cap" added if missing) @param error set to the reason
  //! of a failure @return false if it cannot be written
  bool save(const String & fileName, String & error);
  //! @brief Objects of the list. @return objects
  const std::vector<std::unique_ptr<KinematicMapFolder>> & objects() const { return _objects; }
  //! @brief Add a new object. @param kind PARTICLE_EFFICIENCY or PARTICLE_SMEARER @param name name (made unique)
  //! @return the object
  KinematicMapFolder & add(const String & kind, const String & name);

  //! @name Connections with the application
  //! @{
  //! The file dialog of the application (Open, Save As).
  FileDialog * fileDialog = nullptr;
  //! The histogram of an object of an open file (null if it is not one).
  std::function<const MATH::Histogram * (const ObjectReference &)> resolveHistogram;
  //! Show data points in the current graph (taking ownership).
  std::function<void(MATH::DataPoints *)> showData;
  //! Show a function of rank 1 in the current graph, over a range (taking ownership).
  std::function<void(MATH::Function *, double, double)> showFunction;
  //! Show a function of rank 2 in the current graph, over a box (taking ownership).
  std::function<void(MATH::Function *, double, double, double, double)> showFunction2D;
  //! Messages for the status bar.
  std::function<void(const String &)> setStatus;
  //! Edit a curve on the current canvas (shown over [x0, x1], the y range fixed to [y0, y1] if y0 < y1): the knots
  //! are dragged with the mouse, changed() is called after each change; the curve stays alive while it is edited.
  std::function<void(MATH::BezierCurve *, const String & title, double x0, double x1, double y0, double y1,
                     const std::function<void()> & changed)> editCurve;
  //! Stop editing a curve on the canvases.
  std::function<void(MATH::BezierCurve *)> stopEditingCurve;
  //! @}

  protected:

  //! @brief Draw the list of the objects (left).
  void renderList();
  //! @brief Draw the selected object (right).
  void renderObject(KinematicMapFolder & object);
  //! @brief Draw map k of an object. @param object object @param k index of the map
  void renderMap(KinematicMapFolder & object, unsigned int k);
  //! @brief Draw the fields of a Function map. @param object object @param k index
  void renderFunction(KinematicMapFolder & object, unsigned int k);
  //! @brief Draw the fields of a Histogram map. @param object object @param k index
  void renderHistogram(KinematicMapFolder & object, unsigned int k);
  //! @brief Draw the table of the bins of a histogram map. @param object object @param k index
  void renderBins(KinematicMapFolder & object, unsigned int k);
  //! @brief Choice of the variables of a map, as many as rank (returns true when they change).
  //! @param id ImGui id @param variables variables (changed) @param rank number
  bool renderVariables(const char * id, std::vector<KinematicMap::Variable> & variables, unsigned int rank);
  //! @brief Show map k in the current graph. @param object object @param k index
  void show(KinematicMapFolder & object, unsigned int k);
  //! @name Curves
  //! A map made of curves (MATH::BezierCurve), one per variable used (pT, eta, phi), multiplied: f(pT) g(eta) h(phi).
  //! @{
  //! A curve of a map and its variable.
  struct Curve
  {
  KinematicMap::Variable variable;   //!< variable
  MATH::BezierCurve curve;           //!< curve
  };
  //! @brief Whether a map is made of curves. @param map map @return true if so
  static bool isCurves(const KinematicMap & map);
  //! @brief Curves of a map (copies; empty if it is not made of curves). @param map map @return curves
  static std::vector<Curve> curvesOf(const KinematicMap & map);
  //! @brief Bounds of the values of map k (efficiency [0, 1], resolution >= 0, bias free). @param object object
  //! @param k index @param lower set to the lower bound @param upper set to the upper bound
  static void boundsOf(const KinematicMapFolder & object, unsigned int k, double & lower, double & upper);
  //! @brief Make map k the product of curves (a curve alone for one variable); the curve edited on a canvas follows.
  //! @param object object @param k index @param curves curves, at least one, each variable once
  void setCurves(KinematicMapFolder & object, unsigned int k, std::vector<Curve> curves);
  //! @brief A flat curve over the binning of a variable. @param object object @param k index @param v variable
  //! @param value value @return curve
  static MATH::BezierCurve flatCurve(const KinematicMapFolder & object, unsigned int k, KinematicMap::Variable v, double value);
  //! @brief Draw the fields of a map made of curves. @param object object @param k index
  void renderCurves(KinematicMapFolder & object, unsigned int k);
  //! @brief Edit a curve of map k on the current canvas. @param object object @param k index @param v variable
  void editOnCanvas(KinematicMapFolder & object, unsigned int k, KinematicMap::Variable v);
  //! @brief Stop editing on the canvas.
  void stopEditing();
  //! The curve edited on a canvas: of which map and variable (the curve is kept alive in _editedCurves)
  struct Editing
  {
  const KinematicMapFolder * object = nullptr;   //!< object (checked against the list before use)
  unsigned int k = 0;                            //!< map
  KinematicMap::Variable variable = KinematicMap::kPt;   //!< variable
  MATH::BezierCurve * curve = nullptr;           //!< curve shown and edited (null: none)
  };
  //! @}
  //! @brief Sample the function of map k at the bin centers of the binning of the object: the map becomes a histogram.
  //! @param object object @param k index
  void toHistogram(KinematicMapFolder & object, unsigned int k);
  //! @brief Numbers of parameters a registered function can be made with (cached). @param name name @return numbers
  const std::vector<unsigned int> & variants(const String & name);
  //! @brief Run a change, showing the reason in the window if it is refused. @param change change @return success
  bool attempt(const std::function<void()> & change);
  //! @brief Label of a map (e.g. "Resolution of pT"). @param key key @return label
  static const char * mapLabel(const String & key);
  //! @brief Summary of a map (e.g. "Function Polynomial of pt"). @param map map @return text
  static std::string summary(const KinematicMap & map);

  bool _open;                                                   //!< the window is open
  String _fileName;                                             //!< file read or written last
  bool _modified;                                               //!< changed since read or written
  std::vector<std::unique_ptr<KinematicMapFolder>> _objects;    //!< efficiencies and smearers
  int _selected;                                                //!< selected object (-1: none)
  std::string _rename;                                          //!< name being typed for the selected object
  std::string _error;                                           //!< last change refused, and why
  std::map<std::string,std::vector<unsigned int>> _variants;    //!< numbers of parameters, by function name
  std::map<std::string,const MATH::DataPoints *> _shown;        //!< data shown, by "<object>/<map>" (for Adopt fit)
  std::map<std::string,bool> _editBins;                         //!< bin tables open, by "<object>/<map>"
  Editing _editing;                                             //!< curve edited on a canvas
  std::vector<std::unique_ptr<MATH::BezierCurve>> _editedCurves; //!< curves given to the canvases (kept alive)
  std::map<std::string,bool> _knotTables;                       //!< knot tables open, by "<object>/<map>/<variable>"
  };

  } // namespace GUI
} // namespace CAP

#endif /* CAP__GUI__DetectorMapsDialog */

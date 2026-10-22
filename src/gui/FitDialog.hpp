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
//! @file FitDialog.hpp
//! @brief FitDialog: the Fit window of CapGui, fitting a function to an object of the selected graph.
#ifndef CAP__GUI__FitDialog
#define CAP__GUI__FitDialog
#include <memory>
#include <vector>
#include "String.hpp"

namespace CAP
{
  namespace MATH
  {
  class ParameterizedFunction;
  }

  namespace GRAPHICS
  {
  class Graph;
  }

  namespace GUI
  {

  //!
  //! @brief The Fit window of CapGui: fits a function to an object of one or two variables of the selected graph, and attaches the
  //! fit to it (see MATH::Fitter, MATH::FitRecord).
  //!
  //! The window chooses:
  //!  - the object fitted, of one or two variables: data points, a histogram (a map, for two), or a function (fitted
  //!    through samples of it over the range, on a grid for two variables);
  //!  - the function, among those of MATH::FunctionRegistry: the functions of CAP (polynomials, Normal, LogNormal,
  //!    Exponential, series of Legendre, Chebyshev, Hermite or Laguerre polynomials, Bessel functions J, Y, I, K),
  //!    classes registered by programs, and composites (sums of registered functions, MATH::ParameterizedComposite),
  //!    with the degree of a Polynomial, a Polynomial2D or a series; only the functions of the rank of the object are
  //!    offered; a series or a Polynomial2D is mapped onto the range of the fit;
  //!  - new composites, sums or products f(x) g(y), made in the window ("Composite..."): they are registered and written to
  //!    MATH::FunctionRegistry::defaultCompositesFile(), read again when CapGui starts (and where programs can put
  //!    theirs); a composite can be removed, or its current parameters kept as its starting values;
  //!  - the starting value of each parameter, whether it is fixed, and its bounds; Estimate sets the starting values
  //!    from the data (MATH::Fitter::estimate());
  //!  - the statistic: chi square, or the Poisson likelihood for counts (histograms);
  //!  - the range in x (and y), by default that of the object ("View" takes the range shown), and for a function the
  //!    number of samples along each axis; for a histogram, whether the function is averaged over each bin;
  //!  - the method: Levenberg-Marquardt from the starting values, or a global search (differential evolution, needs
  //!    bounds on the free parameters) refined by Levenberg-Marquardt;
  //!  - the label of the fit.
  //! Fit makes the fit and, if it succeeds, attaches it to the object: the graphs showing the object draw it, and
  //! the fit pane shows its results. The fitted values become the starting values of the next fit.
  //!
  //! The objects shown are not owned by the graphs (they belong to the application or to the files it opened): the
  //! fit is attached to them in memory; saving them (e.g. in a CAP file) saves their fits.
  //!
  class FitDialog
  {
  public:

  //! @brief Constructor (closed).
  FitDialog();
  //! @brief Destructor.
  ~FitDialog();

  //! @brief Name of the window. @return name
  static const char * windowName() { return "Fit###FitDialog"; }
  //! @brief Open the window, to fit an item of the selected graph. @param item index of the item (-1: the first
  //! item of rank 1)
  void open(int item=-1);
  //! @brief Whether the window is open. @return true if so
  bool isOpen() const { return _open; }
  //!
  //! @brief Draw the window, if open, for the selected graph.
  //! @param graph selected graph (may be null) @return true if a fit was attached during this frame
  //!
  bool render(GRAPHICS::Graph * graph);

  protected:

  //! @brief Choose the item fitted: its range, and the statistic it allows. @param graph graph @param item index
  void selectItem(GRAPHICS::Graph & graph, int item);
  //! @brief Make a new function of the chosen class (the degree for a Polynomial).
  void makeFunction();
  //! @brief Set the starting values from the data of the item. @param graph graph
  void estimate(GRAPHICS::Graph & graph);
  //! @brief Fit, and attach the fit to the object. @param graph graph @return true if the fit was attached
  bool fit(GRAPHICS::Graph & graph);
  //! @brief Draw the table of the parameters (value, fixed, bounds).
  void renderParameters();
  //! @brief Draw the window that makes a composite, if open.
  void renderCompositeBuilder();
  //! @brief Write the composites to the file shared with programs; report a failure.
  void saveComposites();

  //! A component of the composite being made.
  struct ComponentDraft
  {
    String type = "Normal";   //!< registered name
    int degree = 2;   //!< degree of a Polynomial, a Polynomial2D or a series
    String label;   //!< label
  };

  bool _open;   //!< the window is open
  bool _openRequested;   //!< open() was called: select the item at the next frame
  int  _requestedItem;   //!< item asked by open()
  int  _item;   //!< index of the item fitted in the graph
  const void * _object;   //!< object of the item fitted (the selection is reset if it changes)
  String _functionName;   //!< registered name of the function
  int  _degree;   //!< degree of a Polynomial
  std::unique_ptr<MATH::ParameterizedFunction> _function;   //!< function fitted, with the starting values
  int  _statistic;   //!< 0 chi square, 1 Poisson likelihood
  bool _search;   //!< global search before Levenberg-Marquardt
  unsigned int _rank = 1;   //!< rank of the object fitted (1 or 2)
  std::vector<double> _low;   //!< range of the fit, one lower end per variable
  std::vector<double> _high;   //!< range of the fit, one upper end per variable
  bool _binAveraging = false;   //!< average the function over the bins of a histogram
  int  _samples;   //!< samples of a function fitted
  String _label;   //!< label of the fit
  String _message;   //!< outcome of the last action
  bool _error;   //!< the message is an error
  bool _builderRequested = false;   //!< open the composite builder at the next frame
  String _compositeName;   //!< name of the composite being made
  std::vector<ComponentDraft> _drafts;   //!< components of the composite being made
  int _operation = 0;   //!< the composite being made is a sum (0) or a product (1)
  String _builderMessage;   //!< problem of the composite being made
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FitDialog

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__KinematicMap
#define CAP__KinematicMap
#include <memory>
#include <vector>
#include "Function.hpp"
#include "Histogram.hpp"
#include "String.hpp"

namespace CAP
{

//!
//! @brief A quantity that depends on the kinematics of a particle (its pT, pseudorapidity eta and azimuth phi), given by
//! one source: none, a constant, a function (MATH::Function) or a histogram (MATH::Histogram) of rank 1 to 3. Used for
//! the efficiencies (ParticleEfficiency) and for the resolutions and biases of the smearing (ParticleSmear).
//!
//! The variables of a function or histogram are chosen among pT, eta and phi, in a given order; by default pT, then
//! (pT, eta), then (pT, eta, phi) according to the rank. Phi is taken in [phiMinimum(), phiMinimum() + 2pi): from 0 by
//! default, from the lower edge of the phi axis for a histogram. Outside the range of a histogram the value is
//! outsideValue() (0 by default).
//!
//! The function is shared (by the copies, read only). The histogram is not owned: it belongs to the object holding
//! the map (a HistogramFolder, which saves it and repoints the map after a copy).
//!
class KinematicMap
{
public:

  //! Variables.
  enum Variable { kPt=0, kEta, kPhi };
  //! Source of the values.
  enum Model { kNone=0, kConstant, kFunction, kHistogram };

  //! @brief No source (model kNone): value() is 0.
  KinematicMap();
  //! @brief Constant. @param value value
  explicit KinematicMap(double value);

  //! @brief No source: value() is 0.
  void setNone();
  //! @brief Constant value. @param value value
  void setConstant(double value);
  //! @brief Function of rank 1 to 3. @param function function (shared) @param variables its variables, in order (empty:
  //! the default of its rank) @throws Exception if the function is null, or its rank or the variables do not match
  void setFunction(std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables = {});
  //! @brief Histogram of rank 1 to 3 (not owned; phiMinimum() set from its phi axis, if it has one).
  //! @param histogram histogram @param variables its variables, in order (empty: the default of its rank)
  //! @throws Exception if the histogram is null, or its rank or the variables do not match
  void setHistogram(const MATH::Histogram * histogram, const std::vector<Variable> & variables = {});
  //! @brief Histogram model whose histogram is not there yet (it is created or imported later, see setHistogram()):
  //! value() throws until then. @param variables variables of the histogram to come (1 to 3, not repeated)
  //! @throws Exception if the variables are not 1 to 3 different ones
  void expectHistogram(const std::vector<Variable> & variables);
  //! @brief Point to another histogram with the same binning and variables (after a copy of the owner).
  //! @param histogram histogram
  void repointHistogram(const MATH::Histogram * histogram) { _histogram = histogram; }

  //! @brief Source of the values. @return model
  Model model() const { return _model; }
  //! @brief Whether there is no source. @return true if there is none
  bool isNone() const { return _model==kNone; }
  //! @brief The constant (model kConstant). @return value
  double constant() const { return _constant; }
  //! @brief The function (null if the model is not kFunction). @return function
  const MATH::Function * function() const { return _model==kFunction ? _function.get() : nullptr; }
  //! @brief The function, shared (null if the model is not kFunction). @return function
  std::shared_ptr<const MATH::Function> sharedFunction() const { return _model==kFunction ? _function : nullptr; }
  //! @brief The histogram (null if the model is not kHistogram). @return histogram
  const MATH::Histogram * histogram() const { return _model==kHistogram ? _histogram : nullptr; }
  //! @brief Variables of the function or histogram, in order (empty otherwise). @return variables
  const std::vector<Variable> & variables() const { return _variables; }
  //! @brief Lower edge of the phi range. @return edge
  double phiMinimum() const { return _phiMinimum; }
  //! @brief Set the lower edge of the phi range (phi taken in [phiMinimum, phiMinimum + 2pi)). @param phiMinimum edge
  void setPhiMinimum(double phiMinimum) { _phiMinimum = phiMinimum; }
  //! @brief Value outside the range of a histogram. @return value
  double outsideValue() const { return _outsideValue; }
  //! @brief Set the value outside the range of a histogram. @param value value
  void setOutsideValue(double value) { _outsideValue = value; }

  //! @brief Value at a point: 0 without source, the constant, the function or the bin of the histogram (outsideValue()
  //! outside its range). @param pt pT @param eta pseudorapidity @param phi azimuth @return value
  //! @throws Exception if the model is kHistogram and there is no histogram (neither created nor imported)
  double value(double pt, double eta, double phi) const;

  //! @name Helpers
  //! @{
  //! @brief Variables of a rank: the given ones (checked), or the default. @param rank rank @param variables variables
  //! @return variables @throws Exception if they do not match the rank or repeat
  static std::vector<Variable> checkedVariables(unsigned int rank, const std::vector<Variable> & variables);
  //! @brief Variables from a text: names pt, eta, phi separated by commas or spaces (e.g. "pt,eta"); empty text: none.
  //! @param text text @return variables @throws Exception for an unknown name
  static std::vector<Variable> parseVariables(const String & text);
  //! @brief Text of variables (e.g. "pt,eta"). @param variables variables @return text
  static String variablesText(const std::vector<Variable> & variables);
  //! @brief Name of a variable (pt, eta, phi). @param v variable @return name
  static const char * variableName(Variable v);
  //! @brief Model from a text (NONE, CONSTANT, FUNCTION, HISTOGRAM; case ignored). @param text text @return model
  //! @throws Exception for an unknown model
  static Model parseModel(const String & text);
  //! @brief Text of a model (NONE, CONSTANT, FUNCTION, HISTOGRAM). @param model model @return text
  static const char * modelName(Model model);
  //! @brief Numbers from a text (separated by commas or spaces). @param text text @return numbers
  //! @throws Exception if an item is not a number
  static std::vector<double> parseNumbers(const String & text);
  //! @}

protected:

  Model _model;                                       //!< source
  double _constant;                                   //!< value of model kConstant
  std::shared_ptr<const MATH::Function> _function;    //!< function of model kFunction (shared)
  const MATH::Histogram * _histogram;                 //!< histogram of model kHistogram (not owned)
  std::vector<Variable> _variables;                   //!< variables of the function or histogram, in order
  double _phiMinimum;                                 //!< phi taken in [_phiMinimum, _phiMinimum + 2pi)
  double _outsideValue;                               //!< value outside the range of the histogram
};

} // namespace CAP

#endif /* CAP__KinematicMap */

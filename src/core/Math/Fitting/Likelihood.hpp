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
//! @file Likelihood.hpp
//! @brief Likelihood and its implementations: chi square, binned Poisson and unbinned likelihoods of a fit.
#ifndef CAP__MATH__Likelihood
#define CAP__MATH__Likelihood
#include <vector>
#include "Aliases.hpp"
#include "ChiSquare.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;
  class DataPoints;

  //!
  //! @brief What a fit minimizes: -2 ln L of the data given the parameters of a ParameterizedFunction (up to a
  //! constant), at the current values of the parameters.
  //!
  //! Implementations:
  //!  - ChiSquareLikelihood: the chi square (Gaussian uncertainties), whose residuals are the pulls;
  //!  - PoissonLikelihood: counts (e.g. a histogram), Poisson distributed: the likelihood ratio of Baker and Cousins,
  //!    whose residuals are the deviance residuals;
  //!  - UnbinnedLikelihood: individual events, the function being their (unnormalized) density.
  //!
  //! A statistic that is a sum of squared residuals (hasResiduals()) can be minimized by Levenberg-Marquardt (see
  //! Fitter); one that is asymptotically chi square distributed (goodnessOfFit()) gives a p-value.
  //!
  class Likelihood
  {
  public:

  //! @brief Likelihood of the parameters of a function. @param function function (not owned)
  explicit Likelihood(ParameterizedFunction & function) : _function(function) { }
  //! @brief Destructor.
  virtual ~Likelihood() { }
  //! @brief The function whose parameters are fitted. @return function
  ParameterizedFunction & function() { return _function; }
  //! @brief Name of the statistic (e.g. "chi square"). @return name
  virtual String name() const = 0;
  //! @brief Choose the points (bins, events) used, at the current parameters (called by the fitters before they
  //! start). @throws MathException if none can be used
  virtual void prepare() = 0;
  //! @brief Number of points (bins, events) used. @return number
  virtual unsigned int nPoints() const = 0;
  //! @brief -2 ln L at the current parameters (up to a constant). @return statistic, +infinity if it cannot be
  //! computed
  virtual double statistic() const = 0;
  //! @brief Whether statistic() is the sum of the squares of residuals(). @return true if so
  virtual bool hasResiduals() const { return false; }
  //! @brief Residuals whose squares add up to statistic() (if hasResiduals()). @param r residuals
  //! @return false if they cannot be computed
  virtual bool residuals(std::vector<double> & r) const { r.clear(); return false; }
  //! @brief Whether statistic() is asymptotically chi square distributed (it then gives a p-value). @return true if so
  virtual bool goodnessOfFit() const { return false; }
  //! @brief Range covered by the points used (after prepare()). @param low lower ends @param high upper ends
  //! @return false if there is none
  virtual bool range(std::vector<double> & low, std::vector<double> & high) const = 0;
  //!
  //! @brief Use only the points (bins, events) inside a box: low[k] <= x[k] <= high[k] for each variable k (empty
  //! vectors: all the points). The range of an unbinned likelihood is replaced.
  //! @param low lower ends @param high upper ends
  //! @throws MathException if the sizes differ from each other or from the rank, or a lower end is above an upper end
  //!
  virtual void setRange(const std::vector<double> & low, const std::vector<double> & high);
  //!
  //! @brief Compare the data of a histogram to the average of the function over each bin rather than to its value at
  //! the center of the bin (see binAverage()); no effect on other data. @param v average
  //!
  virtual void setBinAveraging(bool v) { _binAveraging = v; }
  //! @brief Whether the function is averaged over the bins of a histogram. @return true if so
  bool binAveraging() const { return _binAveraging; }
  //! @brief Lower ends of the range set by setRange() (empty: none). @return lower ends
  const std::vector<double> & rangeLow() const { return _rangeLow; }
  //! @brief Upper ends of the range set by setRange() (empty: none). @return upper ends
  const std::vector<double> & rangeHigh() const { return _rangeHigh; }

  protected:

  //! @brief Whether point i of the data is inside the range set by setRange() (always, if none). @param data data
  //! @param i index @return true if so
  bool inRange(const DataPoints & data, unsigned int i) const;

  ParameterizedFunction & _function;   //!< function fitted
  std::vector<double> _rangeLow;   //!< lower ends of the points used (empty: no limit)
  std::vector<double> _rangeHigh;   //!< upper ends of the points used (empty: no limit)
  bool _binAveraging = false;   //!< the function is averaged over the bins of a histogram
  };

  //!
  //! @brief The chi square of DataPoints with Gaussian uncertainties (see ChiSquare): -2 ln L up to a constant. The
  //! residuals are the pulls (y - f)/sigma; points with a zero uncertainty are left out.
  //!
  class ChiSquareLikelihood : public Likelihood
  {
  public:

  //! @brief Chi square. @param function function (not owned) @param data data (not owned)
  //! @throws MathException if the ranks differ
  ChiSquareLikelihood(ParameterizedFunction & function, const DataPoints & data);
  //! @brief Name. @return "chi square"
  String name() const override { return "chi square"; }
  //! @brief The points with a nonzero uncertainty. @throws MathException if there is none
  void prepare() override;
  //! @brief Points used. @return number
  unsigned int nPoints() const override { return (unsigned int)(_used.size()); }
  //! @brief Chi square of the points used. @return chi square
  double statistic() const override;
  //! @brief Yes: the chi square is a sum of squared pulls. @return true
  bool hasResiduals() const override { return true; }
  //! @brief Pulls of the points used. @param r pulls @return false if one cannot be computed
  bool residuals(std::vector<double> & r) const override;
  //! @brief Yes. @return true
  bool goodnessOfFit() const override { return true; }
  //! @brief The chi square (error mode, coordinate uncertainties). @return chi square
  ChiSquare & chiSquare() { return _chiSquare; }
  //! @brief Average the function over the bins of a histogram (see Likelihood::setBinAveraging()). @param v average
  void setBinAveraging(bool v) override { Likelihood::setBinAveraging(v); _chiSquare.setBinAveraging(v); }
  //! @brief Extent of the coordinates of the points used. @param low lower ends @param high upper ends
  //! @return false if no point is used
  bool range(std::vector<double> & low, std::vector<double> & high) const override;

  protected:

  ChiSquare _chiSquare;   //!< chi square of the data and the function
  const DataPoints & _data;   //!< data
  std::vector<unsigned int> _used;   //!< points used
  };

  //!
  //! @brief Binned Poisson likelihood of counts (e.g. the bins of a histogram): the value of each point is a number of
  //! entries n, Poisson distributed about the function f at its coordinates.
  //!
  //! statistic() is the likelihood ratio of Baker and Cousins, 2 sum [f - n + n ln(n/f)] (2 f for an empty bin): -2 ln L
  //! up to a constant, asymptotically chi square distributed, and unbiased where counts are small, unlike the chi
  //! square. Its residuals are the deviance residuals sign(n - f) sqrt(2 [f - n + n ln(n/f)]). All the points are used,
  //! empty bins included (their content is known: zero); points with a negative or non-finite value are left out.
  //! With a free amplitude, the fitted function sums to the number of entries.
  //!
  class PoissonLikelihood : public Likelihood
  {
  public:

  //! @brief Poisson likelihood. @param function function (not owned) @param data counts (not owned)
  //! @throws MathException if the ranks differ
  PoissonLikelihood(ParameterizedFunction & function, const DataPoints & data);
  //! @brief Name. @return "Poisson likelihood"
  String name() const override { return "Poisson likelihood"; }
  //! @brief The points with a finite, non-negative value. @throws MathException if there is none
  void prepare() override;
  //! @brief Points used. @return number
  unsigned int nPoints() const override { return (unsigned int)(_used.size()); }
  //! @brief Likelihood ratio of Baker and Cousins. @return statistic (+infinity where the function is not positive
  //! at a filled bin)
  double statistic() const override;
  //! @brief Yes: deviance residuals. @return true
  bool hasResiduals() const override { return true; }
  //! @brief Deviance residuals. @param r residuals @return false if one cannot be computed
  bool residuals(std::vector<double> & r) const override;
  //! @brief Yes. @return true
  bool goodnessOfFit() const override { return true; }
  //! @brief Extent of the coordinates of the points used. @param low lower ends @param high upper ends
  //! @return false if no point is used
  bool range(std::vector<double> & low, std::vector<double> & high) const override;

  protected:

  const DataPoints & _data;   //!< counts
  std::vector<unsigned int> _used;   //!< points used
  };

  //!
  //! @brief Unbinned likelihood of individual events: the function is their density, up to its normalization, over a
  //! range (a box in any number of dimensions).
  //!
  //! Without extension, statistic() = -2 sum ln[f(x_i)/N], N being the integral of f over the range: the amplitude of
  //! f does not matter (fix it, or the fit has no uncertainty for it). Extended (setExtended(true)), the integral N is
  //! the expected number of events: statistic() = 2 N - 2 sum ln f(x_i), and the amplitude is fitted too. Events
  //! outside the range are left out. The integral is ParameterizedFunction::integralInRange() (analytic for some
  //! functions, e.g. Normal, Exponential; numerical otherwise).
  //!
  class UnbinnedLikelihood : public Likelihood
  {
  public:

  //! @brief Events of rank 1. @param function density (not owned) @param events events @param low lower end of the
  //! range @param high upper end of the range @param extended extended likelihood
  UnbinnedLikelihood(ParameterizedFunction & function, const std::vector<double> & events, double low, double high, bool extended=false);
  //! @brief Events of any rank. @param function density (not owned) @param events events (each of the rank of the
  //! function) @param low lower ends of the range @param high upper ends of the range @param extended extended
  //! likelihood @throws MathException if the sizes differ from the rank
  UnbinnedLikelihood(ParameterizedFunction & function, const std::vector<std::vector<double>> & events,
                     const std::vector<double> & low, const std::vector<double> & high, bool extended=false);
  //! @brief Name. @return "unbinned likelihood" or "extended unbinned likelihood"
  String name() const override { return _extended ? "extended unbinned likelihood" : "unbinned likelihood"; }
  //! @brief The events inside the range. @throws MathException if there is none
  void prepare() override;
  //! @brief Events used. @return number
  unsigned int nPoints() const override { return (unsigned int)(_used.size()); }
  //! @brief -2 ln L. @return statistic (+infinity if the density is not positive at an event, or its integral is not)
  double statistic() const override;
  //! @brief Whether the likelihood is extended. @return true if so
  bool extended() const { return _extended; }
  //! @brief Make the likelihood extended (the integral of the function is the expected number of events), or not.
  //! @param v extended
  void setExtended(bool v) { _extended = v; }
  //! @brief The range of the events. @param low lower ends @param high upper ends @return true
  bool range(std::vector<double> & low, std::vector<double> & high) const override { low = _low; high = _high; return true; }
  //! @brief Replace the range of the events (empty vectors: unchanged). @param low lower ends @param high upper ends
  //! @throws MathException as Likelihood::setRange()
  void setRange(const std::vector<double> & low, const std::vector<double> & high) override;

  protected:

  std::vector<std::vector<double>> _events;   //!< events
  std::vector<double> _low;   //!< lower ends of the range
  std::vector<double> _high;   //!< upper ends of the range
  bool _extended;   //!< extended likelihood
  std::vector<unsigned int> _used;   //!< events inside the range
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Likelihood

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
//! @file SearchProblem.hpp
//! @brief SearchProblem, FunctionProblem, DataProblem: what an extremum search looks for.
#ifndef CAP__MATH__SearchProblem
#define CAP__MATH__SearchProblem
#include <vector>
#include "SearchDomain.hpp"

namespace CAP
{
  namespace MATH
  {

  class Function;
  class DataPoints;

  //!
  //! @brief What an extremum search looks for: the minimum or the maximum of a value over a domain.
  //!
  //! Engines (see SearchEngine) always minimize objective(): the value itself for a minimum, its opposite for a
  //! maximum, so that they need not know the goal. Each evaluation is counted (nEvaluations()).
  //!
  //! A problem is continuous (FunctionProblem: any point of the domain can be evaluated) or discrete (DataProblem:
  //! only its candidates, e.g. the bins of a histogram, which have neighbors). Engines handle both kinds.
  //!
  class SearchProblem
  {
  public:

  //! @brief What is searched.
  enum Goal { kMinimum, kMaximum };

  //! @brief Problem. @param dimension number of variables @param goal minimum or maximum
  SearchProblem(unsigned int dimension, Goal goal);
  //! @brief Destructor.
  virtual ~SearchProblem() { }

  //! @brief Number of variables. @return dimension
  unsigned int dimension() const { return _dimension; }
  //! @brief What is searched. @return goal
  Goal goal() const { return _goal; }
  //! @brief Set what is searched. @param goal minimum or maximum
  void setGoal(Goal goal) { _goal = goal; }

  //! @brief The value at a point (the function, or the value of the nearest data point). @param x point
  //! @return value (NaN where it cannot be computed)
  virtual double value(const std::vector<double> & x) const = 0;
  //! @brief The value to minimize: value() for a minimum, -value() for a maximum; counted. @param x point
  //! @return objective (+infinity where the value is NaN, so that such points are never chosen)
  double objective(const std::vector<double> & x) const;
  //! @brief The value corresponding to an objective. @param objective objective @return value
  double valueOf(double objective) const { return _goal == kMaximum ? -objective : objective; }
  //! @brief Whether gradient() is available (analytic, or computed by the function itself). @return true if so
  virtual bool hasGradient() const { return false; }
  //! @brief Gradient of value() at a point (only if hasGradient()). @param x point @return gradient
  virtual std::vector<double> gradient(const std::vector<double> & x) const;
  //!
  //! @brief Gradient of objective() at a point: gradient() if available (its sign follows the goal), otherwise
  //! central differences with the given steps (2 evaluations per variable, counted).
  //! @param x point @param steps steps of the differences, one per variable @return gradient
  //!
  std::vector<double> objectiveGradient(const std::vector<double> & x, const std::vector<double> & steps) const;
  //! @brief Number of evaluations of objective() (and of gradient()) so far. @return number
  long nEvaluations() const { return _nEvaluations; }
  //! @brief Number of evaluations of gradient() so far. @return number
  long nGradients() const { return _nGradients; }
  //! @brief Set the counts of evaluations back to zero.
  void resetCounts() const { _nEvaluations = 0; _nGradients = 0; }

  //! @name Discrete problems
  //! @{

  //! @brief Whether only the candidates can be evaluated. @return true for a discrete problem
  virtual bool isDiscrete() const { return false; }
  //! @brief Number of candidates. @return number (0 for a continuous problem)
  virtual unsigned int nCandidates() const { return 0; }
  //! @brief Position of a candidate. @param index candidate @return point
  virtual std::vector<double> candidate(unsigned int index) const;
  //! @brief Objective of a candidate (counted). @param index candidate @return objective
  double candidateObjective(unsigned int index) const;
  //! @brief Value of a candidate (not counted). @param index candidate @return value
  virtual double candidateValue(unsigned int index) const;
  //! @brief Neighbors of a candidate. @param index candidate @return indices of its neighbors
  virtual std::vector<unsigned int> neighbors(unsigned int index) const;
  //! @brief Candidate nearest to a point (distance scaled by the extent of the candidates). @param x point
  //! @return index, or -1 if there is no candidate
  virtual int nearestCandidate(const std::vector<double> & x) const;
  //!
  //! @brief Position and value refined around a candidate, between the candidates (e.g. a parabola through a bin
  //! and its neighbors on a grid). @param index candidate @param point refined point @param value refined value
  //! @return false if no refinement is possible (the candidate itself is the answer)
  //!
  virtual bool refine(unsigned int index, std::vector<double> & point, double & value) const;
  //! @}

  //! @brief The natural domain of the problem: the extent of the data for a discrete problem, unbounded otherwise.
  //! @return domain
  virtual SearchDomain naturalDomain() const { return SearchDomain(_dimension); }

  protected:

  unsigned int _dimension;   //!< number of variables
  Goal _goal;   //!< minimum or maximum
  mutable long _nEvaluations;   //!< evaluations of the objective so far
  mutable long _nGradients;   //!< evaluations of the gradient so far
  };

  //!
  //! @brief The extremum of a Function of any rank, over a continuous domain.
  //!
  //! For a ParameterizedFunction the gradient is that of ParameterizedFunction::gradientAt() (analytic for the
  //! functions that define it); for other functions engines use central differences.
  //!
  class FunctionProblem : public SearchProblem
  {
  public:

  //! @brief Problem. @param function function (not owned; it must outlive the problem) @param goal minimum or maximum
  FunctionProblem(const Function & function, Goal goal=kMinimum);
  //! @brief The function. @return function
  const Function & function() const { return _function; }
  //! @brief Value of the function. @param x point @return value, NaN if it cannot be evaluated
  double value(const std::vector<double> & x) const override;
  //! @brief Whether the function is a ParameterizedFunction (it computes its gradient). @return true if so
  bool hasGradient() const override;
  //! @brief Gradient of the function. @param x point @return gradient
  std::vector<double> gradient(const std::vector<double> & x) const override;

  protected:

  const Function & _function;   //!< function searched
  };

  //!
  //! @brief The extremum of a set of data points (DataPoints of any rank, e.g. a histogram): searched over the
  //! points themselves, the candidates.
  //!
  //! The candidates are the points with a finite value, except the empty bins of a histogram (no content and no
  //! uncertainty) unless setUseEmptyBins(true). Neighbors are the adjacent bins (along the axes and the diagonals)
  //! for a histogram or points on a regular grid, and the nearest points otherwise (3^n - 1 of them, as on a grid;
  //! see setNNeighbors()). A local search among scattered points stops where no neighbor is better: a local extremum
  //! of the graph of neighbors. On a grid, refine() places the
  //! extremum between bins: along each axis, the vertex of the parabola through a bin and its two neighbors.
  //!
  class DataProblem : public SearchProblem
  {
  public:

  //! @brief Problem. @param data data points (not owned; they must outlive the problem) @param goal minimum or
  //! maximum @param useEmptyBins also search the empty bins of a histogram
  DataProblem(const DataPoints & data, Goal goal=kMinimum, bool useEmptyBins=false);
  //! @brief The data. @return data
  const DataPoints & data() const { return _data; }
  //! @brief Whether the candidates are on a regular grid. @return true for a histogram or points on a grid
  bool isGrid() const { return !_gridSize.empty(); }
  //! @brief Number of neighbors of scattered points. @param n number (at least 1)
  void setNNeighbors(unsigned int n) { _nNeighbors = n < 1 ? 1 : n; }

  //! @brief Value of the nearest candidate (piecewise constant). @param x point @return value
  double value(const std::vector<double> & x) const override;
  //! @brief Always: only the candidates are evaluated. @return true
  bool isDiscrete() const override { return true; }
  //! @brief Number of candidates. @return number
  unsigned int nCandidates() const override { return (unsigned int)(_points.size()); }
  //! @brief Position of a candidate. @param index candidate @return point
  std::vector<double> candidate(unsigned int index) const override { return _points[index]; }
  //! @brief Value of a candidate. @param index candidate @return value
  double candidateValue(unsigned int index) const override { return _values[index]; }
  //! @brief Neighbors of a candidate: adjacent bins on a grid, nearest points otherwise. @param index candidate
  //! @return neighbors
  std::vector<unsigned int> neighbors(unsigned int index) const override;
  //! @brief Candidate nearest to a point. @param x point @return index, or -1
  int nearestCandidate(const std::vector<double> & x) const override;
  //! @brief Parabolic refinement on a grid. @param index candidate @param point refined point @param value refined
  //! value @return false if the candidates are not on a grid
  bool refine(unsigned int index, std::vector<double> & point, double & value) const override;
  //! @brief The extent of the candidates. @return domain
  SearchDomain naturalDomain() const override;

  protected:

  const DataPoints & _data;   //!< data searched
  std::vector<std::vector<double>> _points;   //!< positions of the candidates
  std::vector<double> _values;   //!< values of the candidates
  std::vector<double> _scale;   //!< extent of the candidates in each dimension (for distances)
  std::vector<unsigned int> _gridSize;   //!< nodes along each axis of a grid (empty: scattered points)
  std::vector<std::vector<double>> _gridNodes;   //!< coordinates of the nodes along each axis of a grid
  std::vector<int> _gridCandidate;   //!< candidate of each grid node (-1: none), axis 0 fastest
  std::vector<std::vector<unsigned int>> _gridIndex;   //!< node indices of each candidate on the grid
  unsigned int _nNeighbors;   //!< neighbors of scattered points
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__SearchProblem

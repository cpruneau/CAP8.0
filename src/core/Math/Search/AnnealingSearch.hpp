/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file AnnealingSearch.hpp
//! @brief AnnealingSearch: extremum search by simulated annealing.
#ifndef CAP__MATH__AnnealingSearch
#define CAP__MATH__AnnealingSearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by simulated annealing: random moves around the current point, a better point is always
  //! taken and a worse one with the probability exp(-increase/T) (Metropolis), while the temperature T decreases
  //! (exponential cooling). At a high temperature the search wanders over the domain and crosses the barriers between
  //! local extrema; as it cools it settles in the deepest region it found.
  //!
  //! The first temperature is, by default, the spread of the objective over random points of the domain; the search
  //! ends when it has cooled by finalRatio(). After each stage of movesPerStage() moves, the walk returns to the best
  //! point visited if it has strayed above it by more than the temperature (once below a tenth of the first
  //! temperature: hotter, it crosses valleys freely), and the step is enlarged or
  //! reduced to keep the fraction of accepted moves near targetAcceptance() (between 1E-4 and 0.5 of the widths); a
  //! fifth of the moves keep the first step, so that jumps between basins remain possible as the search cools.
  //! With the defaults (cooling 0.95 per stage of 30 moves per dimension), the schedule takes about 270 stages. The
  //! best point visited is the result; a
  //! local engine refines it best (see ExtremumSearch::setPolish()).
  //!
  //! Simulated annealing works well in a few dimensions; on functions with very many local extrema in more dimensions
  //! (e.g. Rastrigin in 4), differential evolution or the genetic algorithm are much more reliable.
  //!
  //! For a discrete problem, a move goes to a random neighbor.
  //!
  class AnnealingSearch : public SearchEngine
  {
  public:

  //! @brief Engine with the default settings.
  AnnealingSearch();
  //! @brief Set the first temperature. @param temperature temperature (0: the spread of the objective)
  void setInitialTemperature(double temperature) { _initialTemperature = temperature > 0 ? temperature : 0; }
  //! @brief First temperature. @return temperature (0: automatic)
  double initialTemperature() const { return _initialTemperature; }
  //! @brief Set the cooling factor applied after each stage. @param factor factor in (0,1)
  void setCooling(double factor) { _cooling = (factor > 0 && factor < 1) ? factor : 0.9; }
  //! @brief Cooling factor applied after each stage. @return factor
  double cooling() const { return _cooling; }
  //! @brief Set the ratio of the last temperature to the first. @param ratio ratio in (0,1)
  void setFinalRatio(double ratio) { _finalRatio = (ratio > 0 && ratio < 1) ? ratio : 1E-6; }
  //! @brief Ratio of the last temperature to the first. @return ratio
  double finalRatio() const { return _finalRatio; }
  //! @brief Set the moves at each temperature. @param n number (0: 30 per dimension)
  void setMovesPerStage(int n) { _movesPerStage = n < 0 ? 0 : n; }
  //! @brief Set the first step. @param step fraction of the widths of the domain
  void setInitialStep(double step) { _initialStep = step > 0 ? step : 0.2; }
  //! @brief Set the fraction of accepted moves the step adapts to. @param fraction fraction in (0,1)
  void setTargetAcceptance(double fraction) { _targetAcceptance = (fraction > 0 && fraction < 1) ? fraction : 0.4; }
  //! @brief Fraction of accepted moves the step adapts to. @return fraction
  double targetAcceptance() const { return _targetAcceptance; }
  //! @brief Anneal from the start point (a random point of a bounded domain if none is set).
  //! @param problem problem @param domain domain @param options options @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  double _initialTemperature;   //!< first temperature (0: automatic)
  double _cooling;   //!< cooling factor per stage
  double _finalRatio;   //!< last temperature / first
  int    _movesPerStage;   //!< moves per temperature (0: automatic)
  double _initialStep;   //!< first step (fraction of the widths)
  double _targetAcceptance;   //!< fraction of accepted moves aimed at
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__AnnealingSearch

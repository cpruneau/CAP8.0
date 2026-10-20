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
//! @file EvolutionarySearch.hpp
//! @brief EvolutionarySearch: extremum search by differential evolution or a genetic algorithm.
#ifndef CAP__MATH__EvolutionarySearch
#define CAP__MATH__EvolutionarySearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by evolution of a population of points over a bounded domain.
  //!
  //! Two strategies:
  //!  - kDifferentialEvolution (the default, DE/rand/1/bin): each point is challenged by a trial point made of the
  //!    point and, for a fraction crossover() of its coordinates, of a + F (b - c), a, b and c being three other
  //!    points (F: differentialWeight()); the better one stays. Robust on continuous problems, with few settings.
  //!    The default crossover (0.9) suits functions whose variables interact; for separable ones (a sum of terms in
  //!    one variable each), a low crossover (0.1 to 0.3) is much more efficient.
  //!  - kGenetic: parents are chosen by tournaments, their children made by blend crossover (BLX-alpha: each
  //!    coordinate taken in the interval spanned by the parents, enlarged by alpha) and mutated (gaussian moves of
  //!    a fraction mutationWidth() of the widths, with the probability mutationRate() per coordinate); the best
  //!    points (elites()) pass unchanged to the next generation.
  //!
  //! The search stops when the population has gathered within the tolerance in every dimension, when the best point
  //! has not improved for stallGenerations() generations, or when the budget is spent. A local engine refines the
  //! result best (see ExtremumSearch::setPolish()).
  //!
  //! For a discrete problem, points move in the space of the coordinates and each is evaluated at the allowed
  //! candidate nearest to it.
  //!
  class EvolutionarySearch : public SearchEngine
  {
  public:

  //! @brief Strategies of evolution.
  enum Strategy { kDifferentialEvolution, kGenetic };

  //! @brief Engine. @param strategy strategy
  explicit EvolutionarySearch(Strategy strategy=kDifferentialEvolution);
  //! @brief Strategy. @return strategy
  Strategy strategy() const { return _strategy; }
  //! @brief Set the strategy (and the name of the engine). @param strategy strategy
  void setStrategy(Strategy strategy);
  //! @brief Set the number of points of the population. @param n number (0: 10 per dimension, at least 40)
  void setPopulation(int n) { _population = n < 0 ? 0 : (n > 0 && n < 4 ? 4 : n); }
  //! @brief Number of points of the population. @return number (0: automatic)
  int  population() const { return _population; }
  //! @brief Set the probability of taking a coordinate from the mutant (DE) or of crossing two parents (GA).
  //! @param rate probability in [0,1]
  void setCrossover(double rate) { _crossover = rate < 0 ? 0 : (rate > 1 ? 1 : rate); }
  //! @brief Probability of crossover. @return probability
  double crossover() const { return _crossover; }
  //! @brief Set the differential weight F of differential evolution. @param weight weight in (0,2]
  void setDifferentialWeight(double weight) { _weight = (weight > 0 && weight <= 2) ? weight : 0.7; }
  //! @brief Differential weight F. @return weight
  double differentialWeight() const { return _weight; }
  //! @brief Set the probability of mutation of each coordinate (GA). @param rate probability (0: 1/dimension)
  void setMutationRate(double rate) { _mutationRate = rate < 0 ? 0 : (rate > 1 ? 1 : rate); }
  //! @brief Set the width of the mutations (GA), as a fraction of the widths of the domain. @param width fraction
  void setMutationWidth(double width) { _mutationWidth = width > 0 ? width : 0.1; }
  //! @brief Set the number of points that pass unchanged to the next generation (GA). @param n number
  void setElites(int n) { _elites = n < 0 ? 0 : n; }
  //! @brief Number of elites (GA). @return number
  int  elites() const { return _elites; }
  //! @brief Set the size of the tournaments that choose the parents (GA). @param n number (at least 2)
  void setTournament(int n) { _tournament = n < 2 ? 2 : n; }
  //! @brief Set the generations without improvement after which the search stops (default: 150 for differential
  //! evolution, 60 for the genetic algorithm). @param n number
  void setStallGenerations(int n) { _stall = n < 1 ? 1 : n; }
  //! @brief Generations without improvement after which the search stops. @return number
  int  stallGenerations() const { return _stall; }
  //! @brief Evolve a population over the domain (bounded; for a discrete problem, the extent of the candidates
  //! bounds the unbounded dimensions). @param problem problem @param domain domain @param options options
  //! @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  Strategy _strategy;   //!< strategy
  int    _population;   //!< points of the population (0: automatic)
  double _crossover;   //!< probability of crossover
  double _weight;   //!< differential weight F
  double _mutationRate;   //!< probability of mutation per coordinate (0: 1/dimension)
  double _mutationWidth;   //!< width of the mutations (fraction of the widths)
  int    _elites;   //!< points kept unchanged (GA)
  int    _tournament;   //!< size of the tournaments (GA)
  int    _stall;   //!< generations without improvement before stopping
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__EvolutionarySearch

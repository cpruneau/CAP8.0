/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MersenneTwister.hpp
//! @brief MersenneTwister: the MT19937 generator of uniform random numbers.
#ifndef CAP__MersenneTwister
#define CAP__MersenneTwister
#include <array>
#include <cstdint>
#include <vector>
#include "RandomGenerator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief The Mersenne Twister MT19937 (M. Matsumoto and T. Nishimura, ACM Transactions on Modeling and Computer
  //! Simulation 8 (1998) 3): period 2^19937 - 1, equidistributed in 623 dimensions.
  //!
  //! The 32-bit numbers are those of the reference implementation (mt19937ar.c) and of std::mt19937 for the same
  //! seed; random() combines two of them into a double with 53 random bits in [0,1) (genrand_res53). All the
  //! distributions of RandomGenerator are available.
  //!
  //! @code
  //!   CAP::MATH::MersenneTwister mt(4357);
  //!   double u = mt.random();
  //!   std::vector<std::uint32_t> saved = mt.state();   // to continue the same sequence later: mt.setState(saved)
  //! @endcode
  //!
  class MersenneTwister : public RandomGenerator
  {
  public:

  //! @brief Generator seeded with seed (0: a seed of its own, different for each generator). @param seed seed
  explicit MersenneTwister(std::uint64_t seed=5489);
  //! @brief Destructor.
  virtual ~MersenneTwister() { }

  //! @brief Random number uniformly distributed in [0,1), with 53 random bits. @return number
  double random() override;
  //! @brief Next 32-bit number of the sequence. @return number
  std::uint32_t next32();
  //! @brief Start again from a seed: init_genrand() of the reference implementation, with the low 32 bits of seed;
  //! 0 takes a seed from std::random_device and the clock. @param seed seed
  void setSeed(std::uint64_t seed) override;
  //! @brief Start again from several 32-bit seeds (init_by_array() of the reference implementation). @param keys seeds
  void setSeeds(const std::vector<std::uint32_t> & keys);
  //! @brief Seed actually used by the last setSeed() (useful after setSeed(0)). @return seed, or 0 after setSeeds()
  std::uint32_t seed() const { return _seed; }

  //! @brief State of the generator: the 624 words, then the position in them. @return state
  std::vector<std::uint32_t> state() const;
  //! @brief Continue from a state given by state(). @param state state @throws MathException if it has the wrong size
  void setState(const std::vector<std::uint32_t> & state);

  protected:

  static const int kN = 624;   //!< number of words of the state
  static const int kM = 397;   //!< offset of the recurrence
  std::array<std::uint32_t, kN> _mt;   //!< state
  int _index;   //!< next word of the state used (kN: the state must be renewed)
  std::uint32_t _seed;   //!< seed of the last setSeed()

  //! @brief Renew the 624 words of the state.
  void twist();
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MersenneTwister

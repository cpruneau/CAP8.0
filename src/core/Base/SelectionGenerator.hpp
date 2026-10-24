/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__SelectionGenerator
#define CAP__SelectionGenerator
#include <vector>
#include "TRandom.h"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Random selection of an index according to given probabilities.
//!
//! The generator is initialized with n probabilities (or relative weights: they are normalized to a sum of 1).
//! generate() then returns an index k in [0, n-1] with probability p_k, using ROOT's gRandom.
//!
//! Example:
//! @code
//!   CAP::SelectionGenerator g({0.7, 0.2, 0.1});
//!   int k = g.generate();   // 0 with probability 0.7, ...
//! @endcode
//!
class SelectionGenerator
{
public:
  //! @brief Default constructor: not initialized (generate() throws until initializeWith() is called).
  SelectionGenerator();
  //! @brief Constructor. @param probabilities probabilities or relative weights (see initializeWith())
  SelectionGenerator(const std::vector<double> & probabilities);
  //! @brief Copy assignment. @param source generator to copy @return this generator
  SelectionGenerator & operator=(const SelectionGenerator & source);

  //! @brief Destructor.
  virtual ~SelectionGenerator(){}
  //!
  //! @brief Set the probabilities of the indices.
  //! @param probabilities probabilities or relative weights, normalized to a sum of 1
  //! @throws Exception if the vector is empty, a value is negative, or the values sum to 0
  //!
  virtual void initializeWith(const std::vector<double> & probabilities);
  //! @brief Random index. @return index in [0, nPartitions()-1] @throws Exception if the generator is not initialized
  virtual int generate();
  //! @brief Number of indices. @return number of probabilities given
  int nPartitions() const
  {
    return cumulativeProbability.size();
  }

protected:

  std::vector<double> cumulativeProbability; //!< normalized cumulative probabilities
  ClassDef(SelectionGenerator,0)
};

}

#endif /* SelectionGenerator_hpp */

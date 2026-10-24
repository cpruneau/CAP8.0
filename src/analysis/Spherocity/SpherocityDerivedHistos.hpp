/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__SpherocityDerivedHistos
#define CAP__SpherocityDerivedHistos
#include "SpherocityHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{
      
//!
//! @brief Quantities derived from the spherocity histograms (in development: calculateDerivedHistograms() does nothing).
//!
class SpherocityDerivedHistos: public SpherocityHistos
{
public:
  //! @brief Default constructor.
  SpherocityDerivedHistos();
  //! @brief Copy constructor. @param group group to copy
  SpherocityDerivedHistos(const SpherocityDerivedHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  SpherocityDerivedHistos & operator=(const SpherocityDerivedHistos & group);
  //! @brief Destructor.
  virtual ~SpherocityDerivedHistos() {}
  //! @brief Read the fill options and binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Load the derived histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Compute the derived histograms (nothing yet). @param baseHistos base histograms
  virtual void calculateDerivedHistograms(SpherocityHistos & baseHistos);

protected:

  //! @brief Copy the content of another group. @param group group to copy
  void cloneD(const SpherocityDerivedHistos & group);
   
  ClassDef(SpherocityDerivedHistos, 1);
};

} // namespace CAP


#endif

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
#ifndef CAP__SpherocityHistos
#define CAP__SpherocityHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"
#include "ClassDef.hpp"

namespace CAP
{
      
//!
//! @brief Distributions of the transverse spherocity S0, of the unweighted spherocity S1, and of S1 vs S0.
//!
class SpherocityHistos: public HistogramFolder
{
public:
  //! @brief Default constructor.
  SpherocityHistos();
  //! @brief Copy constructor. @param group group to copy
  SpherocityHistos(const SpherocityHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  SpherocityHistos & operator=(const SpherocityHistos & group);
  //! @brief Destructor.
  virtual ~SpherocityHistos() {}
  //! @brief Read the fill options and binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms (according to the fill options).
  virtual void create();
  //! @brief Load the histograms from a file. @param file file
  virtual void loadFrom(const Folder & file);
  //! @brief Fill the histograms. @param s0 spherocity S0 @param s1 spherocity S1 @param weight weight
  virtual void fill(double s0, double s1, double weight);

protected:

  //! @brief Copy the content of another group. @param group group to copy
  void cloneB(const SpherocityHistos & group);

  //! Whether S0 is filled.
  bool  fillS0;
  //! Whether S1 is filled.
  bool  fillS1;
  //! Whether S1 vs S0 is filled.
  bool  fillS1VsS0;
  int spherocity_nbins;  ///< number of bins of the spherocity axis
  double spherocity_min;  ///< lower edge of the spherocity axis
  double spherocity_max;  ///< upper edge of the spherocity axis

  //! Distribution of S0.
  Histogram * h_s0;
  //! Distribution of S1.
  Histogram * h_s1;
  //! S1 vs S0.
  Histogram * h_s1VsS0;

  ClassDef(SpherocityHistos, 1);
};

} // namespace CAP


#endif

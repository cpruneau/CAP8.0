/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__NuDynHistos
#define CAP__NuDynHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"
#include "ClassDef.hpp"

namespace CAP
{


//!
//! @brief Multiplicity moments of two particle selections (1 and 2) as a function of an event variable: profiles of
//! the first-order moments f1_1, f1_2 and of the second-order factorial moments f2_11 = n1(n1-1), f2_12 = n1 n2,
//! f2_22 = n2(n2-1), and the distribution of the event variable.
//!
//! Calculate lowest moments up to order 4
//! The calculation is carried out for N distinct types (species) of particles (N distinct filters)
//!
//! Let the species be: a, b, c, etc
//! First order moments:  f_1(a), f_1(b), f_1(c), etc
//! Second order moments: f_2(a,a), f_2(a,b), f_2(a,c), .., f_2(b,b), f_2(b,c), etc
//! Third order moments: f_3(a,a,a), f_3(a,a,b), f_3(a,a,c), f(a,a,d), f(a,b,b), etc
//! Fourth order moments: f_4(a,a,a,a), f_(a,a,a,b), f_4(a,a,b,b), etc
//!
//!// ================================================================================
// Naming convention
// ================================================================================
// fk_i     : factorial moments of order "k" for particle "i" (i.e., accepted by filter 1)
// Fk_i.j  : factorial cumulants of order "k" for particles "i, .., j" (i.e., accepted by filter i.j)
// rk_i     : ratio of factorial moments of order "k" for particle "i" (i.e., accepted by filter 1)
//            to product of f1_i
//



class NuDynDerivedHistos;

class NuDynHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  NuDynHistos();
  //! @brief Copy constructor. @param source group to copy
  NuDynHistos(const NuDynHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  NuDynHistos & operator=(const NuDynHistos & rhs);
  //! @brief Destructor.
  virtual ~NuDynHistos(){ }

  //! @brief Read the event variable name and binning (\<taskName\>:\<objectType\>:evtName, evt_nbins, evt_min, evt_max).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the moments of an event. @param mult event variable @param n1_1 multiplicity of selection 1
  //! @param n1_2 multiplicity of selection 2 @param n2_11 n1(n1-1) @param n2_12 n1 n2 @param n2_22 n2(n2-1)
  virtual void fill(double mult, double n1_1, double n1_2, double n2_11, double n2_12, double n2_22);
  //! @brief Scale the histograms. @param factor factor
  virtual void scaleObject(double factor);

  friend NuDynDerivedHistos;

protected:

  //! @brief Copy the content of another group. @param source group to copy
  void cloneB(const NuDynHistos & source);


  //! Distribution of the event variable.
  Histogram * h_evt;
  //! Average multiplicity of selection 1 vs event variable.
  Profile* h_f1_1_evt;
  //! Average multiplicity of selection 2 vs event variable.
  Profile* h_f1_2_evt;
  //! Average of n1(n1-1) vs event variable.
  Profile* h_f2_11_evt;
  //! Average of n1 n2 vs event variable.
  Profile* h_f2_12_evt;
  //! Average of n2(n2-1) vs event variable.
  Profile* h_f2_22_evt;

  //! Name of the event variable.
  String evtName;
  int evt_nbins;  ///< number of bins of the event variable axis
  double evt_min;  ///< lower edge of the event variable axis
  double evt_max;  ///< upper edge of the event variable axis


  ClassDef(NuDynHistos,0)
};

} // namespace CAP

#endif /* CAP__NuDynHistos  */




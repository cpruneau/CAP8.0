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
#ifndef CAP__PtPtHistos
#define CAP__PtPtHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"
#include "Configuration.hpp"
#include "ClassDef.hpp"

namespace CAP
{

class PtPtDerivedHistos;

//!
//! @brief Event-wise moments for transverse momentum correlations, as profiles vs an event variable: factorial moments
//! f1 to f4 of the multiplicity and products of the pT sums Q_n = sum of pT^n (Q1, Q1Q1, ..., Q2Q1, Q3Q1, Q4).
//! @warning fill() does nothing yet.
//!
class PtPtHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  PtPtHistos();
  //! @brief Copy constructor. @param source group to copy
  PtPtHistos(const PtPtHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  PtPtHistos & operator=(const PtPtHistos & rhs);
  //! @brief Destructor.
  virtual ~PtPtHistos(){ }
  //! @brief Read the event variable name and binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the moments of an event (does nothing yet). @param mult event variable @param moments pT moments
  virtual void fill(double mult,vector<double> & moments);
  friend PtPtDerivedHistos;

protected:
  //! @brief Copy the content of another group. @param group group to copy
  void cloneB(const PtPtHistos & group);

  //! Distribution of the event variable.
  Histogram * h_evt;
  //! Factorial moments f1 to f4 of the multiplicity vs event variable.
  Profile* h_f1_evt;
  Profile* h_f2_evt;
  Profile* h_f3_evt;
  Profile* h_f4_evt;
  //! Averages of products of pT sums Q_n = sum pT^n vs event variable (Q1, Q1Q1, Q1Q1Q1, ..., Q4).
  Profile* h_Q1_evt;
  Profile* h_Q1Q1_evt;
  Profile* h_Q1Q1Q1_evt;
  Profile* h_Q1Q1Q1Q1_evt;
  Profile* h_Q2_evt;
  Profile* h_Q2Q1_evt;
  Profile* h_Q2Q1Q1_evt;
  Profile* h_Q2Q2_evt;
  Profile* h_Q3_evt;
  Profile* h_Q3Q1_evt;
  Profile* h_Q4_evt;


  //! Name of the event variable.
  String evtName;
  int    evt_nbins;  ///< number of bins of the event variable axis
  double evt_min;  ///< lower edge of the event variable axis
  double evt_max;  ///< upper edge of the event variable axis


  ClassDef(PtPtHistos,0)
};

} // namespace CAP

#endif /* CAP__PtPtHistos  */




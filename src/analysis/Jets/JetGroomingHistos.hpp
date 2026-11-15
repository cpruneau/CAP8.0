/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetGroomingHistos.hpp
//! @brief JetGroomingHistos: the histograms of the groomed jets of one combination of an event filter, a particle
//! filter, jet settings, a jet filter and a jet groomer.
#ifndef CAP__JetGroomingHistos
#define CAP__JetGroomingHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "JetGroomer.hpp"

namespace CAP
{

//!
//! @brief Histograms of the jets accepted by a jet filter after grooming by one jet groomer (JetGroomingAnalyzer).
//!
//! Per jet groomed (every jet accepted by the jet filter):
//!  - status: 0 groomed (a splitting or subjets kept), 1 no splitting satisfying the condition (kept), 2 failed;
//!  - the selected splitting (Soft Drop and dynamical grooming, jets with a splitting): zg, rg (Delta R), ktg; nSD
//!    (Soft Drop);
//!  - the groomed jet (jets kept): pt, mass, mult (constituents); ptLoss, the fraction of the pT removed,
//!    1 - pT_groomed/pT; massVsMass, the groomed mass versus the mass before grooming.
//!
//! Normalization (normalize()): all the histograms are divided by the number of jets groomed in this folder (failed
//! ones included), so that, e.g., the integral of zg is the fraction of the jets with a splitting.
//!
//! Binning (keys \<task\>:HISTOGRAM:\<name\>): zg_nbins (zg in [0, 0.5]), rg_nbins/max, ktg_nbins/max, nSD_nbins/max,
//! pt_nbins/min/max, mass_nbins/max, mult_nbins/max, ptLoss_nbins (in [0, 1]).
//!
class JetGroomingHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  JetGroomingHistos();
  //! @brief Copy constructor. @param source folder
  JetGroomingHistos(const JetGroomingHistos & source);
  //! @brief Copy assignment. @param rhs folder @return this folder
  JetGroomingHistos & operator=(const JetGroomingHistos & rhs);
  //! @brief Destructor.
  virtual ~JetGroomingHistos() {}
  //! @brief Add the default binning. @param configuration configuration @param taskName task
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the binning. @param taskName task @param objectType HISTOGRAM @param configuration configuration
  //! @param index index (not used)
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a folder. @param inputFile folder
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with a groomed jet. @param groomed groomed jet
  void fill(const GroomedJet & groomed);
  //! @brief Normalize: every histogram per jet groomed. @param eventFactor factor (not used: there is no per-event
  //! histogram)
  void normalize(double eventFactor);
  //! @brief Nothing to scale per event. @param factor factor
  virtual void scaleObject(double factor);
  //! @brief Number of jets groomed. @return number
  double jetCount() const { return _jetCount; }

protected:

  //! @brief Set the pointers to the histograms of another folder. @param source folder
  void cloneB(const JetGroomingHistos & source);
  //! @brief All the histograms. @return histograms
  std::vector<Histogram*> allHistograms() const;

  int zg_nbins;                                   //!< zg in [0, 0.5]
  int rg_nbins;      double rg_max;               //!< rg
  int ktg_nbins;     double ktg_max;              //!< ktg
  int nSD_nbins;     double nSD_max;              //!< nSD
  int pt_nbins;      double pt_min, pt_max;       //!< groomed pT
  int mass_nbins;    double mass_max;             //!< groomed mass
  int mult_nbins;    double mult_max;             //!< groomed constituents
  int ptLoss_nbins;                               //!< 1 - pTg/pT in [0, 1]

  double _jetCount;   //!< jets groomed
  Histogram * h_status;
  Histogram * h_zg;  Histogram * h_rg;  Histogram * h_ktg;  Histogram * h_nSD;
  Histogram * h_pt;  Histogram * h_mass;  Histogram * h_mult;  Histogram * h_ptLoss;  Histogram * h_massVsMass;
};

} // namespace CAP

#endif /* CAP__JetGroomingHistos */

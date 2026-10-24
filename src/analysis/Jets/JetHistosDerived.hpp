/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetHistosDerived
#define CAP__JetHistosDerived
#include "Profile.hpp"
#include "JetHistos.hpp"
#include "fastjet/ClusterSequence.hh"
#include "MathConstants.hpp"
#include "ParticleTypeList.hpp"
#include "ClassDef.hpp"

using namespace fastjet;

namespace CAP
{

//!
//! @brief Quantities derived from the jet histograms vs jet pT: normalized multiplicity correlators R2 (00, ++, +-,
//! --), nu_dyn and rms net charge of the constituents.
//!
class JetHistosDerived : public JetHistos
{
public:
  //! @brief Default constructor.
  JetHistosDerived();
  //! @brief Copy constructor. @param source object to copy
  JetHistosDerived(const JetHistosDerived & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetHistosDerived & operator=(const JetHistosDerived & rhs);
  //! @brief Destructor.
  ~JetHistosDerived(){}

  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  void configure(const String & taskName,
                       const String & objectType,
                       const Configuration & configuration,
                       unsigned int index);
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void load(CapFile & inputFile);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Compute the derived histograms. @param baseHistos jet histograms
  virtual void calculateDerivedHistograms(const JetHistos & baseHistos);

protected:

  virtual void cloneD(onst JetHistosDerived & source);

  //! R2 of the neutral constituents vs jet pT.
  Histogram *h_jet_R200_pt;
  //! R2 of the (+,+) constituents vs jet pT.
  Histogram *h_jet_R2pp_pt;
  //! R2 of the (+,-) constituents vs jet pT.
  Histogram *h_jet_R2pm_pt;
  //! R2 of the (-,-) constituents vs jet pT.
  Histogram *h_jet_R2mm_pt;
  //! nu_dyn of the charged constituents vs jet pT.
  Histogram *h_jet_nudyn_pt;
  //! rms net charge of the constituents vs jet pT.
  Histogram *h_jet_netQrms_pt;

  ClassDef(JetHistosDerived,0)

};

} // namespace CAP
#endif // JetHistos_HPP

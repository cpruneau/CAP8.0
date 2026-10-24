/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetHistos
#define CAP__JetHistos
#include "HistogramFolder.hpp"
#include "fastjet/ClusterSequence.hh"
#include "MathConstants.hpp"
#include "ParticleTypeList.hpp"

using namespace fastjet;

namespace CAP
{

//class JetHistoDerived;

//!
//! @brief Histograms of jet global properties: number of jets, jet pT, azimuth, pseudorapidity, and the multiplicities
//! of all, positive, negative and neutral constituents and their net charge (distributions and averages vs jet p).
//!
class JetHistos : public HistogramFolder
{
public:
  friend class JetHistosDerived;

  //! @brief Default constructor.
  JetHistos();
  //! @brief Copy constructor. @param source object to copy
  JetHistos(const JetHistos & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetHistos & operator=(const JetHistos & rhs);
  //! @brief Destructor.
  ~JetHistos(){}

  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);

  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with a jet. @param J jet
  virtual void fill(PseudoJet& J);
  //! @brief Scale the histograms. @param value factor
  virtual void scaleHistograms(double value);
  //! @brief Number of jets filled. @return number
  virtual double acceptedJets() const;

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void   cloneB(const JetHistos & source);

  //! Particle types used to find the charges of the constituents (not owned).
  ParticleTypeList * particleTypeList;

  //
  // jet global properties
  //
  Histogram *h_jet_n1;  ///< number of jets of the selected type
  Histogram *h_jet_pt;  ///< pt
  Histogram *h_jet_phi;  ///< phi
  Histogram *h_jet_eta;  ///< rapidity or eta
  Histogram *h_jet_etaPt;  ///< rapidity vs pt
  Histogram *h_jet_n;  ///< multiplicity of constituent particles
  Histogram *h_jet_np;  ///< multiplicity of +ve constituent particles
  Histogram *h_jet_nm;  ///< multiplicity of -ve particles
  Histogram *h_jet_n0;  ///< multiplicity of neutral particles
  Histogram *h_jet_netQ;  ///< net charge of the constituents
  Profile *p_jet_n;  ///< multiplicity of constituent particles vs jet p
  Profile *p_jet_np;  ///< multiplicity of +ve constituent particles vs jet p
  Profile *p_jet_nm;  ///< multiplicity of -ve particles vs jet p
  Profile *p_jet_n0;  ///< multiplicity of neutral particles vs jet p
  Profile *p_jet_npSq;  ///< average squared number of positive constituents vs jet p
  Profile *p_jet_nmSq;  ///< average squared number of negative constituents vs jet p
  Profile *p_jet_n0Sq;  ///< average squared number of neutral constituents vs jet p
  Profile *p_jet_npnm;  ///< average number of (+,-) constituent pairs vs jet p
  Profile *p_jet_npn0;  ///< average number of (+, neutral) constituent pairs vs jet p
  Profile *p_jet_nmn0;  ///< average number of (-, neutral) constituent pairs vs jet p
  Profile *p_jet_netQ;  ///< average net charge of the constituents vs jet p
  Profile *p_jet_netQSq;  ///< average squared net charge of the constituents vs jet p

  int jet_n1_nbins;  ///< number of bins of the jet multiplicity axis
  int jet_p_nbins;  ///< number of bins of the jet momentum axis
  int jet_pt_nbins;  ///< number of bins of the jet pT axis
  int jet_phi_nbins;  ///< number of bins of the jet azimuth axis
  int jet_eta_nbins;  ///< number of bins of the jet pseudorapidity axis
  int jet_netQ_nbins;  ///< number of bins of the jet net charge axis
  double jet_n1_min;  ///< lower edge of the jet multiplicity axis
  double jet_p_min;  ///< lower edge of the jet momentum axis
  double jet_pt_min;  ///< lower edge of the jet pT axis
  double jet_phi_min;  ///< lower edge of the jet azimuth axis
  double jet_eta_min;  ///< lower edge of the jet pseudorapidity axis
  double jet_netQ_min;  ///< lower edge of the jet net charge axis
  double jet_n1_max;  ///< upper edge of the jet multiplicity axis
  double jet_p_max;  ///< upper edge of the jet momentum axis
  double jet_pt_max;  ///< upper edge of the jet pT axis
  double jet_phi_max;  ///< upper edge of the jet azimuth axis
  double jet_eta_max;  ///< upper edge of the jet pseudorapidity axis
  double jet_netQ_max;  ///< upper edge of the jet net charge axis


};

} // namespace CAP
#endif // JetHistos_HPP

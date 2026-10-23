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
#ifndef CAP__ParticlePairHistos
#define CAP__ParticlePairHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"


namespace CAP
{

//!
//! @brief Pair histograms of one (event filter, particle filter 1, particle filter 2) combination.
//!
//! Filled for each pair: n2 vs (pT1, pT2) and (phi1, phi2); if eta_fill, vs (eta1, eta2) and (Delta eta, Delta phi);
//! if rapidity_fill, vs (y1, y2) and (Delta y, Delta phi); if p2_fill, the same weighted by pT1 pT2 (DptDpt). The
//! number of pairs per event is filled in n2. Pairs with a particle outside the pT, phi, or both eta and y ranges are
//! skipped. Binning keys: \<taskName\>:HISTOGRAM:n2_nbins, n2_min, n2_max, pt_*, phi_*, eta_*, rapidity_*.
//!
class ParticlePairHistos : public HistogramFolder
{
public:

  friend class ParticlePairDerivedHistos;

  //! @brief Default constructor.
  ParticlePairHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticlePairHistos(const ParticlePairHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticlePairHistos & operator=(const ParticlePairHistos & group);
  //! @brief Destructor.
  virtual ~ParticlePairHistos() {}
  //! @brief Read the binning and fill options. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms (according to the fill options).
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with a pair. @param particle1 first particle @param particle2 second particle
  //! @param weight weight
  virtual void fill(Particle & particle1, Particle & particle2, double weight=1.0);
  //! @brief Fill the number of pairs of the event. @param nPairs number of pairs @param weigh weight
  virtual void fillMultiplicity(double nPairs, double weigh=1.0);
  //! @brief Scale the histograms. @param factor factor
  virtual void scaleObject(double factor);

  //! @brief pT bin of a value. @param v value @return bin (1 to pt_nbins), or 0 if out of range
  inline int ptBinFor(float v) const
  {
  int index = 0; // indicates a value out of bounds
  if (v<pt_min || v>=pt_max) return index;
  index = 1+int(pt_scale*(v-pt_min));
  if (index>pt_nbins) index = pt_nbins; // rounding at the upper edge
  return index;
  }

  //! @brief phi bin of a value. @param v value @return bin (1 to phi_nbins), or 0 if out of range
  inline int phiBinFor(float v) const
  {
  int index = 0; // indicates a value out of bounds
  if (v<phi_min || v>=phi_max) return index;
  index = 1+int(phi_scale*(v-phi_min));
  if (index>phi_nbins) index = phi_nbins; // rounding at the upper edge
  return index;
  }

  //! @brief eta bin of a value. @param v value @return bin (1 to eta_nbins), or 0 if out of range
  inline int getEtaBinFor(float v) const
  {
  int index = 0; // indicates a value out of bounds
  if (v<eta_min || v>=eta_max) return index;
  index = 1+int(eta_scale*(v-eta_min));
  if (index>eta_nbins) index = eta_nbins; // rounding at the upper edge
  return index;
  }

  //! @brief Rapidity bin of a value. @param v value @return bin (1 to rapidity_nbins), or 0 if out of range
  inline int getYBinFor(float v) const
  {
  int index = 0; // indicates a value out of bounds
  if (v<rapidity_minimum || v>=rapidity_maximum) return index;
  index = 1+int(rapidity_scale*(v-rapidity_minimum));
  if (index>rapidity_nbins) index = rapidity_nbins; // rounding at the upper edge
  return index;
  }

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const ParticlePairHistos & source);

  //! Whether the eta histograms are filled.
  bool eta_fill;
  //! Whether the rapidity histograms are filled.
  bool rapidity_fill;
  //! Whether the pT1 pT2 weighted histograms are filled.
  bool p2_fill;
  int n2_nbins;  ///< number of bins of the pair multiplicity axis
  int pt_nbins;  ///< number of bins of the pT axis
  int phi_nbins;  ///< number of bins of the azimuth axis
  int eta_nbins;  ///< number of bins of the pseudorapidity axis
  int rapidity_nbins;  ///< number of bins of the rapidity axis
  int Dphi_nbins;  ///< number of bins of the Delta phi axis
  int Deta_nbins;  ///< number of bins of the Delta eta axis
  int Drapidity_nbins;  ///< number of bins of the Delta y axis
  int Dphi_nbins_shft;

  double n2_min,  n2_max;
  double pt_min,  pt_max;
  double phi_min, phi_max;
  double eta_min, eta_max;
  double rapidity_minimum, rapidity_maximum;
  double phi_scale;  ///< inverse bin width of the azimuth axis
  double rapidity_scale;  ///< inverse bin width of the rapidity axis
  double eta_scale;  ///< inverse bin width of the pseudorapidity axis
  double pt_scale;  ///< inverse bin width of the pT axis
  double Dphi_width;  ///< bin width of the Delta phi axis
  double Dphi_min;  ///< lower edge of the Delta phi axis
  double Dphi_max;  ///< upper edge of the Delta phi axis
  double Deta_min;  ///< lower edge of the Delta eta axis
  double Deta_max;  ///< upper edge of the Delta eta axis
  double Drapidity_min;  ///< lower edge of the Delta y axis
  double Drapidity_max;  ///< upper edge of the Delta y axis
  double Dphi_min_shft;
  double Dphi_max_shft;
  double rapidity_maximumAcc;

  //! Number of pairs per event.
  Histogram * h_n2;
  Histogram * h_n2_ptpt;  ///< pair density vs (pT1, pT2)
  Histogram * h_n2_etaeta;  ///< pair density vs (eta1, eta2)
  Histogram * h_DptDpt_etaeta;  ///< pair density weighted by pT1 pT2 vs (eta1, eta2)
  Histogram * h_n2_phiphi;  ///< pair density vs (phi1, phi2)
  Histogram * h_DptDpt_phiphi;  ///< pair density weighted by pT1 pT2 vs (phi1, phi2)
  Histogram * h_n2_yY;  ///< pair density vs (y1, y2)
  Histogram * h_DptDpt_yY;  ///< pair density weighted by pT1 pT2 vs (y1, y2)
  Histogram * h_n2_DetaDphi;  ///< pair density vs (Delta eta, Delta phi)
  Histogram * h_DptDpt_DetaDphi;  ///< pair density weighted by pT1 pT2 vs (Delta eta, Delta phi)
  Histogram * h_n2_DyDphi;  ///< pair density vs (Delta y, Delta phi)
  Histogram * h_DptDpt_DyDphi;  ///< pair density weighted by pT1 pT2 vs (Delta y, Delta phi)

  ClassDef(ParticlePairHistos,0)
};

} // namespace CAP

#endif /* CAP__ParticlePairHistos  */

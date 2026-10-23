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
#ifndef CAP__GlobalHistos
#define CAP__GlobalHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"

namespace CAP
{
class GlobalDerivedHistos;
class GlobalCalculator;
//!
//! @brief Histograms of global event observables for one (event filter, particle filter) combination.
//!
//! HistogramFolder of "global" observables.
//!
//! Observables are computed in the acceptance defined by
//! particle filters for each of the selected event filters. The particle filters can be set to select narrow
//! or broad ranges of kinematic variables as well as species
//! of particles.
//!
//! n:  number of selected particles in an  event
//! e:  total energy of the selected particles in an event
//! q:  total (net) electric charge of the selected particles in an event
//! b:  total (net) baryon charge of the selected particles in an event
//!
//! The fill method of this class must be called at most once per event
//! otherwise weird multiple counting will happen..
//!
class GlobalHistos : public HistogramFolder
{
public:

  friend GlobalDerivedHistos;
  friend GlobalCalculator;

  //! @brief Default constructor.
  GlobalHistos();

  //! @brief Copy constructor. @param group group to copy
  GlobalHistos(const GlobalHistos & group);

  //! @brief Copy assignment. @param group group to copy @return this group
  GlobalHistos & operator=(const GlobalHistos & group);

  //! @brief Destructor.
  virtual ~GlobalHistos() {}

  //! @brief Read the binning of the observables (n, e, q, s, b, ptSum, ptAvg). @param taskName name of the task
  //! @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //!
  //!Creating histograms corresponding to the selected event and particle filters from the given file.
  //!
  virtual void create();

  //!
  //!Load histograms corresponding to the selected event and particle filters from the given file.
 virtual void loadFrom(const Folder & inputFile);

  //!
  //!Fill eventswise observables for the selected filters
  //!
  //! @param n  number of particles accepted
  //! @param pTsum total transverse momentum (pT) of  accepted particles
  //! @param e  total energyof  accepted particles
  //! @param q  net charge of  accepted particles
  //! @param s  net strangeness of  accepted particles
  //! @param b net baryon number of  accepted particles
  //!
  //! By default, only histograms of single particle quantities are filled. These include the number of particles (multiplicity), the pt sum,
  //! the total energy, the net charge, the net strangeness, and the net baryon number. Scatter plot histograms of observable correlations
  //! are also filled including for instance ptSum vs multiplicity, total E vs multiplicity, net charge vs multiplicity, and so on. An eventwise  pt average
  //! histogram is also filled (event by event), for all event and particle filters, when the selected mulitplicity in non zero.
  //!
  //! If the bool variable fillCorrelationHistos is set to true, cross-species histograms are also filled including for instance the multiplicity selected by
  //! particle filter k1 and the multiplicity selected by filter k2.
  //!
  void fill(double n,
            double ptSum,
            double e,
            double q,
            double s,
            double b,
            double weight);

protected:
  //! @brief Copy the content of another group. @param group group to copy
  void cloneB(const GlobalHistos & group);
  //! Distribution of the multiplicity.
  Histogram * h_n;
  //! Distribution of the total energy.
  Histogram * h_e;
  //! Distribution of the net charge.
  Histogram * h_q;
  //! Distribution of the net strangeness.
  Histogram * h_s;
  //! Distribution of the net baryon number.
  Histogram * h_b;
  //! Distribution of the pT sum.
  Histogram * h_ptSum;
  //! Distribution of the event-wise average pT.
  Histogram * h_ptAvg;

  //! Average total energy vs multiplicity.
  Profile * p_eVsN;
  //! Average net charge vs multiplicity.
  Profile * p_qVsN;
  //! Average net strangeness vs multiplicity.
  Profile * p_sVsN;
  //! Average net baryon number vs multiplicity.
  Profile * p_bVsN;
  //! Average pT sum vs multiplicity.
  Profile * p_ptSumVsN;
  //! Average event-wise average pT vs multiplicity.
  Profile * p_ptAvgVsN;

  int n_nbins;  ///< number of bins of the multiplicity axis
  int e_nbins;  ///< number of bins of the energy axis
  int q_nbins;  ///< number of bins of the net charge axis
  int b_nbins;  ///< number of bins of the net baryon number axis
  int ptSum_nbins;  ///< number of bins of the pT sum axis
  int s_nbins;  ///< number of bins of the net strangeness axis
  int ptAvg_nbins;  ///< number of bins of the average pT axis
  double n_min;  ///< lower edge of the multiplicity axis
  double n_max;  ///< upper edge of the multiplicity axis
  double e_min;  ///< lower edge of the energy axis
  double e_max;  ///< upper edge of the energy axis
  double q_min;  ///< lower edge of the net charge axis
  double q_max;  ///< upper edge of the net charge axis
  double b_min;  ///< lower edge of the net baryon number axis
  double b_max;  ///< upper edge of the net baryon number axis
  double s_min;  ///< lower edge of the net strangeness axis
  double s_max;  ///< upper edge of the net strangeness axis
  double ptSum_min;  ///< lower edge of the pT sum axis
  double ptSum_max;  ///< upper edge of the pT sum axis
  double ptAvg_min;  ///< lower edge of the average pT axis
  double ptAvg_max;  ///< upper edge of the average pT axis


  ClassDef(GlobalHistos,0)
};

} // namespace CAP

#endif /* CAP__GlobalHistos  */




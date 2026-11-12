/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FlowCumulants.hpp
//! @brief FlowCumulants: multi-particle cumulants c_n{2k} and differential cumulants d_n{2k} of any order from
//! event-averaged correlators, and the flow coefficients v_n{2k}, v'_n{2k}.
#ifndef CAP__FlowCumulants
#define CAP__FlowCumulants
#include <vector>

namespace CAP
{

  //!
  //! @brief Cumulants of azimuthal correlations of any order, and the flow coefficients they estimate.
  //!
  //! With m_k = <<2k>>_n the event-averaged 2k-particle correlators (k = 1..K), the cumulants c_k = c_n{2k} are the
  //! coefficients of the generating function (Borghini, Dinh, Ollitrault, PRC 64, 054901 (2001); Bilandzic et al.,
  //! PRC 89, 064904 (2014)):
  //!   sum_k c_k z^k/(k!)^2 = ln(1 + sum_k m_k z^k/(k!)^2),
  //! computed by the recursion of the logarithm of a series, b_k = a_k - (1/k) sum_{j<k} j b_j a_{k-j}
  //! (a_k = m_k/(k!)^2, c_k = (k!)^2 b_k):
  //!   c{2} = <2>, c{4} = <4> - 2<2>^2, c{6} = <6> - 9<4><2> + 12<2>^3,
  //!   c{8} = <8> - 16<6><2> - 18<4>^2 + 144<4><2>^2 - 144<2>^4, ...
  //! For a single flow magnitude v, m_k = v^{2k} and c_k = A_k v^{2k}, A_k = 1, -1, 4, -33, 456, -9460, ... (the
  //! cumulants of m_k = 1): v_n{2k} = (c_k/A_k)^{1/(2k)}, defined when c_k/A_k > 0.
  //!
  //! Differential cumulants: with m'_k = <<2k'>>_n, the correlators of a particle of interest with 2k-1 reference
  //! particles, d_k = d_n{2k} is the cumulant with the block of the particle of interest replaced, i.e. the
  //! derivative of c_k along m_j -> m_j + (j/k) m'_j:
  //!   d{2} = <2'>, d{4} = <4'> - 2<2'><2>, d{6} = <6'> - 6<4'><2> - 3<4><2'> + 12<2'><2>^2, ...
  //! and v'_n{2k} = d_k/(A_k v_n{2k}^{2k-1}).
  //!
  //! Uncertainties: propagated linearly from those of the correlators, their covariances neglected (an estimate; the
  //! sub-samples, see SubSample, give the full uncertainties).
  //!
  class FlowCumulants
  {
  public:

  //! @brief Cumulants c_1..c_K (c_n{2}, c_n{4}, ...). @param m correlators <2>, <4>, ... @return cumulants
  static std::vector<double> cumulants(const std::vector<double> & m);
  //! @brief Cumulant c_k (k = 1..m.size()). @param m correlators @param k order/2 @return cumulant
  static double cumulant(const std::vector<double> & m, unsigned int k);
  //! @brief Differential cumulant d_k. @param m correlators <2>, <4>, ... @param mp differential correlators <2'>,
  //! <4'>, ... @param k order/2 @return d_n{2k}
  static double differentialCumulant(const std::vector<double> & m, const std::vector<double> & mp, unsigned int k);
  //! @brief Coefficient A_k: c_k of a single flow magnitude v is A_k v^{2k}. @param k order/2 @return A_k
  static double coefficient(unsigned int k);
  //! @brief v_n{2k} from c_n{2k}. @param c cumulant @param k order/2 @param defined set false (and 0 returned) if
  //! c/A_k is not positive @return flow coefficient
  static double flow(double c, unsigned int k, bool & defined);
  //! @brief v'_n{2k} from d_n{2k} and v_n{2k}. @param d differential cumulant @param v v_n{2k} (non-zero)
  //! @param k order/2 @return differential flow coefficient
  static double differentialFlow(double d, double v, unsigned int k);

  //! @name With uncertainties (linear propagation, covariances neglected)
  //! @{
  //! @brief v_n{2k} and its uncertainty. @param m correlators @param em their uncertainties @param k order/2
  //! @param c cumulant set @param ec its uncertainty @param v flow set @param ev its uncertainty
  //! @return true if v_n{2k} is defined
  static bool flowWithError(const std::vector<double> & m, const std::vector<double> & em, unsigned int k,
                            double & c, double & ec, double & v, double & ev);
  //! @brief v'_n{2k} and its uncertainty. @param m correlators @param em uncertainties @param mp differential
  //! correlators @param emp uncertainties @param k order/2 @param d d_n{2k} set @param ed its uncertainty @param vp
  //! v'_n{2k} set @param evp its uncertainty @return true if v'_n{2k} is defined (v_n{2k} defined and non-zero)
  static bool differentialFlowWithError(const std::vector<double> & m, const std::vector<double> & em,
                                        const std::vector<double> & mp, const std::vector<double> & emp,
                                        unsigned int k, double & d, double & ed, double & vp, double & evp);
  //! @}
  };

} // namespace CAP

#endif /* CAP__FlowCumulants */

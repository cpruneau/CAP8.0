/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FlowCorrelator.hpp
//! @brief FlowVector (the weighted flow vectors Q_{h,p} of a set of particles) and FlowCorrelator (multi-particle
//! azimuthal correlators of any order and harmonics from flow vectors: the generic framework).
#ifndef CAP__FlowCorrelator
#define CAP__FlowCorrelator
#include <complex>
#include <functional>
#include <vector>

namespace CAP
{

  //!
  //! @brief Weighted flow vectors of a set of particles: Q_{h,p} = sum_i w_i^p exp(i h phi_i), for harmonics
  //! 0 <= h <= maxHarmonic() and powers 0 <= p <= maxPower(). Q_{-h,p} = conj(Q_{h,p}); Q_{0,0} is the number of
  //! particles and Q_{0,p} the sum of the weights to the power p.
  //!
  //! In the generic framework (Bilandzic et al., PRC 89, 064904 (2014)), Q vectors are those of the reference particles,
  //! p vectors (power 1) those of the particles of interest (POI), and q vectors those of the particles that are both.
  //!
  class FlowVector
  {
  public:

  typedef std::complex<double> Complex;

  //! @brief Empty flow vectors. @param maxHarmonic highest harmonic @param maxPower highest power of the weights
  FlowVector(unsigned int maxHarmonic=0, unsigned int maxPower=0);
  //! @brief Set the highest harmonic and power, and clear. @param maxHarmonic harmonic @param maxPower power
  void resize(unsigned int maxHarmonic, unsigned int maxPower);
  //! @brief Set all the vectors to zero.
  void clear();
  //! @brief Add a particle. @param phi azimuth @param weight weight
  void add(double phi, double weight=1.0);
  //! @brief Add the vectors of another set (same sizes). @param other vectors
  FlowVector & operator+=(const FlowVector & other);
  //! @brief Q_{h,p}. @param h harmonic (|h| <= maxHarmonic()) @param p power (<= maxPower()) @return vector
  //! @throws Exception if h or p is out of range
  Complex q(int h, unsigned int p=1) const;
  //! @brief Sum of the weights to the power p (Q_{0,p}). @param p power @return sum
  double weightSum(unsigned int p=1) const { return q(0,p).real(); }
  //! @brief Number of particles added. @return number
  unsigned int count() const { return _count; }
  //! @brief Highest harmonic. @return harmonic
  unsigned int maxHarmonic() const { return _maxHarmonic; }
  //! @brief Highest power of the weights. @return power
  unsigned int maxPower() const { return _maxPower; }

  protected:

  unsigned int _maxHarmonic;     //!< highest harmonic
  unsigned int _maxPower;        //!< highest power
  unsigned int _count;           //!< particles added
  std::vector<Complex> _q;       //!< Q_{h,p} at h*(maxPower+1)+p
  };

  //!
  //! @brief Multi-particle azimuthal correlators of any order and harmonics, computed from flow vectors in a single
  //! pass over the particles (generic framework, Bilandzic et al., PRC 89, 064904 (2014)).
  //!
  //! The numerator of the m-particle correlator of harmonics h_1..h_m is the sum over all m-tuples of distinct
  //! particles, N(h_1..h_m) = sum_{i1 != ... != im} w_i1...w_im exp(i(h_1 phi_i1 + ... + h_m phi_im)), and its
  //! denominator D the same sum with all harmonics zero: <m> = N/D. Summing over the partitions of the m slots into
  //! blocks B (the slots given to the same particle), with the Moebius coefficients (-1)^{|B|-1}(|B|-1)!,
  //!   N(S) = sum_{B containing the first slot of S} (-1)^{|B|-1} (|B|-1)! Q_{h_B,|B|} N(S \ B),  N(empty) = 1,
  //! where h_B is the sum of the harmonics of B. subsetSums() computes N(S) for all the 2^m subsets S of the slots at
  //! once (3^m operations), so that a single call gives the correlators of all orders 2, 4, ..., m: with the harmonics
  //! arranged as (+n, -n, +n, -n, ...), the slots {0..2k-1} give <2k>_n.
  //!
  //! Differential correlators (a particle of interest in slot 0, reference particles in the others, all distinct,
  //! Bilandzic et al. PRC 83, 044913 (2011)): the block of slot 0 is a POI alone (p vector) or a particle both POI and
  //! reference (q vector, power |B|): differential() sums over the blocks of slot 0 with the subset sums N of the
  //! other slots.
  //!
  //! @code
  //!   FlowVector Q(8, 4);  for (...) Q.add(phi, w);                    // reference particles
  //!   std::vector<int> h = FlowCorrelator::alternating(2, 4);           // +2 -2 +2 -2
  //!   std::vector<std::complex<double>> N, D;
  //!   FlowCorrelator::subsetSums(Q, h, N);  FlowCorrelator::subsetSums(Q, std::vector<int>(4,0), D);
  //!   double c4 = (N[15]/D[15]).real();                                 // <4>_2; N[3]/D[3]: <2>_2
  //! @endcode
  //!
  class FlowCorrelator
  {
  public:

  typedef std::complex<double> Complex;
  //! @brief Flow vector of a block: harmonic and power (number of slots) -> vector.
  typedef std::function<Complex(int,unsigned int)> BlockVector;

  //! @brief Subset sums N(S) of the slots of harmonics h, from the flow vectors Q. @param q flow vectors (harmonics up
  //! to the largest sum of harmonics of a subset, powers up to h.size()) @param h harmonics (at most 20 slots)
  //! @param n sums, indexed by the bit mask of the slots (n[0] = 1)
  static void subsetSums(const FlowVector & q, const std::vector<int> & h, std::vector<Complex> & n);
  //! @brief Subset sums N(S) with a block vector of one's own. @param block vector of a block @param h harmonics
  //! @param n sums
  static void subsetSums(const BlockVector & block, const std::vector<int> & h, std::vector<Complex> & n);
  //!
  //! @brief Differential numerator: slot 0 a particle of interest (harmonic h0), the slots of the mask others
  //! reference particles (harmonics h), all distinct: sum over the subsets T of others of
  //! (-1)^{|T|} |T|! X_{h0+h_T,|T|+1} N(others \ T), with X_{h,1} = p_{h} and X_{h,b} = q_{h,b} (b >= 2).
  //! @param p vectors of the particles of interest (power 1) @param q vectors of the particles both POI and reference
  //! @param h0 harmonic of slot 0 @param h harmonics of the slots (indices of the subset sums) @param others mask of
  //! the reference slots @param n subset sums of the reference particles for h @return numerator
  //!
  static Complex differential(const FlowVector & p, const FlowVector & q, int h0, const std::vector<int> & h,
                              unsigned int others, const std::vector<Complex> & n);
  //! @brief Harmonics (+n, -n, +n, -n, ...) of m slots. @param n harmonic @param m slots @return harmonics
  static std::vector<int> alternating(int n, unsigned int m);
  //! @brief Mask of the first m slots. @param m slots @return mask (2^m - 1)
  static unsigned int firstSlots(unsigned int m) { return (1u<<m)-1u; }
  //! @brief Moebius coefficient of a block of b slots: (-1)^{b-1} (b-1)!. @param b slots @return coefficient
  static double blockCoefficient(unsigned int b);
  };

} // namespace CAP

#endif /* CAP__FlowCorrelator */

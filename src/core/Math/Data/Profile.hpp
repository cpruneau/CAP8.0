/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__Profile
#define CAP__Profile
#include "Histogram.hpp"
#include "CapFile.hpp"

namespace CAP
{
namespace MATH
{

//!
//! @brief Profile histogram: weighted average of a value in each bin of a rank 1 or rank 2 binning.
//!
//! A Profile replaces ROOT's TProfile and TProfile2D. Each call fill(x, value, weight) adds an entry to the bin of x;
//! the bin holds the sums of the weights w, of the squared weights w^2, of w*value and of w*value^2, from which the
//! weighted mean of the value and its uncertainty (standard error of the mean, as ROOT's default error option) are
//! computed.
//!
//! <b>As a Histogram.</b> The bins of the underlying Histogram hold the profile: one entry per non-empty bin, with the
//! mean as bin value and its uncertainty as bin uncertainty. All Histogram getters (binValueAt(), binAvgValueAt(), ...)
//! thus return the mean, as TProfile::GetBinContent() does. The Histogram arithmetic and setters change these means only:
//! the accumulated sums are then out of date. The functions overridden by Profile (fill(), reset(), scale(), add(),
//! setAsSumOf(), operators + and -) keep the sums and the means consistent; add() and setAsSumOf() combine the sums, so
//! that merging profiles (e.g., sub-samples) gives the exact weighted mean.
//!
//! Means and their uncertainties are stored in double precision.
//!
//! <b>Binary files.</b> "CAPQ" records: the Histogram record followed by the sums. The class is registered with Folder.
//!
//! Example:
//! @code
//!   CAP::MATH::Profile p("nPartVsB","<N_{part}> vs b",20,0.0,20.0);
//!   p.fill(impact, nParticipants, eventWeight);
//!   double avg = p.binValueAt(3);
//! @endcode
//!
class Profile : public Histogram
{
public:

  //! @brief Default constructor: an empty profile (rank 0).
  Profile();
  //! @brief Profile with the binning of a partition (rank 1 or 2). @param partition binning
  //! @throws Exception if the rank is not 1 or 2
  Profile(const Partition & partition);
  //! @brief Rank 1 profile. @param name name @param title title @param n number of bins
  //! @param minimum lower edge @param maximum upper edge
  Profile(const String & name, const String & title, unsigned int n, double minimum, double maximum);
  //! @brief Rank 2 profile. @param name name @param title title @param n0 number of bins along axis 0
  //! @param minimum0 lower edge of axis 0 @param maximum0 upper edge of axis 0 @param n1 number of bins along axis 1
  //! @param minimum1 lower edge of axis 1 @param maximum1 upper edge of axis 1
  Profile(const String & name, const String & title,
          unsigned int n0, double minimum0, double maximum0,
          unsigned int n1, double minimum1, double maximum1);
  //! @brief Copy constructor. @param source profile to copy
  Profile(const Profile & source);
  //! @brief Copy assignment. @param rhs profile to copy @return this profile
  Profile & operator=(const Profile & rhs);
  //! @brief Destructor.
  virtual ~Profile() {}

  //! @brief Polymorphic copy. @return new copy, owned by the caller
  Profile * clone() const override { return new Profile(*this); }

  //! @name Filling
  //! @{
  //! @brief Add an entry to a rank 1 profile. @param x coordinate @param value averaged value @param weight weight
  void fill(double x, double value, double weight=1.0);
  //! @brief Add an entry to a rank 2 profile. @param x0 coordinate along axis 0 @param x1 coordinate along axis 1
  //! @param value averaged value @param weight weight
  void fill(double x0, double x1, double value, double weight);
  //! @}

  //! @name Sums
  //! @{
  //! @brief Sum of the weights of a bin. @param k global bin index @return sum of w
  double sumWeights(unsigned int k) const        { checkGlobal(k,__FUNCTION__); return _sumW[k]; }
  //! @brief Sum of the squared weights of a bin. @param k global bin index @return sum of w^2
  double sumWeights2(unsigned int k) const       { checkGlobal(k,__FUNCTION__); return _sumW2[k]; }
  //! @brief Sum of weight*value of a bin. @param k global bin index @return sum of w*value
  double sumWeightedValues(unsigned int k) const { checkGlobal(k,__FUNCTION__); return _sumWY[k]; }
  //! @brief Sum of weight*value^2 of a bin. @param k global bin index @return sum of w*value^2
  double sumWeightedValues2(unsigned int k) const{ checkGlobal(k,__FUNCTION__); return _sumWY2[k]; }
  //! @brief Number of entries of a bin. @param k global bin index @return number of fill() calls in the bin
  long   nEntries(unsigned int k) const          { checkGlobal(k,__FUNCTION__); return _counts[k]; }
  //! @}

  //! @name Operations keeping the sums
  //! @{
  //! @brief Empty all bins.
  void reset() override;
  //! @brief Remove the binning and the content.
  void clear() override;
  //! @brief Multiply the averaged values by a coefficient (as TProfile::Scale()). @param coefficient coefficient
  void scale(double coefficient) override;
  //!
  //! @brief Merge a profile: its sums, weighted by coefficient, are added to those of this profile (as TProfile::Add()).
  //! A negative coefficient removes the entries of h (the numbers of entries are subtracted).
  //! @param h profile with the same binning @param coefficient weight of h's entries
  //! @throws Exception if h is not a Profile or the binnings differ
  //!
  void add(const Histogram & h, double coefficient=1.0) override;
  //! @brief Set to the merge of two profiles (see add()). @param h1 first profile @param h2 second profile
  //! @param coefficient1 weight of h1 @param coefficient2 weight of h2
  void setAsSumOf(const Histogram & h1, const Histogram & h2,
                  double coefficient1=1.0, double coefficient2=1.0) override;
  //! @brief Set to the merge of profiles (see add()). @param histograms profiles @param coefficients weights
  void setAsSumOf(const std::vector<const Histogram *> & histograms,
                  const std::vector<double> & coefficients) override;
  //! @}

  //! @brief Print the profile.
  void print() const override;

  //! @name Binary files (independent of ROOT)
  //! The payload is that of Histogram (with the fits attached since format version 2), then the sums of the bins.
  //! @{
  //! @brief Tag of the records of this class. @return "CAPQ"
  static const char * binaryTag() { return "CAPQ"; }
  //! @brief Version of the record format. @return version
  static unsigned int binaryVersion() { return 2; }
  //! @brief Encode the profile. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Decode a profile. @param r reader @param name name of the record @param version version of the record
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @}

protected:

  //! @brief Size the sums to the number of bins (all zero).
  void resizeSums();
  //! @brief Recompute the mean and its uncertainty of bin k from the sums. @param k global bin index
  void updateBin(unsigned int k);
  //! @brief Check that h is a Profile with the same binning. @param h histogram @param source calling function
  //! @return h as a Profile
  const Profile & checkProfile(const Histogram & h, const char * source) const;

  std::vector<long>   _counts;  //!< number of entries of each bin
  std::vector<double> _sumW;    //!< sum of the weights of each bin
  std::vector<double> _sumW2;   //!< sum of the squared weights of each bin
  std::vector<double> _sumWY;   //!< sum of weight*value of each bin
  std::vector<double> _sumWY2;  //!< sum of weight*value^2 of each bin

  ClassDefOverride(Profile,1)
};

//! @brief Copy of a profile (nullptr if p is null). @param p profile @return copy owned by the caller, or nullptr
Profile * safeClone(const Profile * p);
//! @brief Copy of a profile stored in the current folder of a file. @param inputFile file @param name name of the profile
//! @return copy owned by the caller @throws FileException if the file has no profile with that name
Profile * loadProfile(const CapFile & inputFile, const String & name);

//! @brief Merge of two profiles (see Profile::add()). @param lhs first profile @param rhs second profile @return merge
Profile operator+(const Profile & lhs, const Profile & rhs);
//! @brief lhs with the entries of rhs removed (sums of rhs subtracted). @param lhs profile @param rhs profile @return result
Profile operator-(const Profile & lhs, const Profile & rhs);

} // namespace MATH

// Short names in namespace CAP for the analysis classes.
using MATH::Histogram;
using MATH::Profile;

} // namespace CAP

#endif

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__RootConversion
#define CAP__RootConversion
#include "TH1.h"
#include "Histogram.hpp"
#include "Profile.hpp"

namespace CAP
{

//!
//! @file RootConversion.hpp
//! @brief Conversions between CAP histograms (MATH::Histogram, MATH::Profile) and ROOT histograms.
//!
//! The analysis classes fill and save MATH::Histogram objects in CAP files. These conversions are for drawing with
//! ROOT (plotters) and for importing histograms from older ROOT files.
//!
//! Bin numbering: CAP bins are numbered from 0, ROOT bins from 1 (ROOT's under- and overflow bins are not converted).
//!
//! Example:
//! @code
//!   TH1 * h = CAP::toRootHistogram(*histogram);   // rank 1, 2 or 3; caller owns h
//!   h->Draw();
//! @endcode
//!

//!
//! @brief ROOT copy of a histogram: a TH1D, TH2D or TH3D for ranks 1, 2 and 3, with the same name, title, axis titles,
//! bin labels, bin values and uncertainties. A Profile becomes a TProfile or TProfile2D (see toRootProfile()).
//! @param histogram histogram to convert
//! @param name name of the ROOT histogram (the histogram's name if empty)
//! @return new ROOT histogram, owned by the caller (not attached to any ROOT directory)
//! @throws Exception if the rank is not 1, 2 or 3
//!
TH1 * toRootHistogram(const MATH::Histogram & histogram, const String & name="");

//!
//! @brief CAP copy of a ROOT histogram (TH1, TH2 or TH3, with fixed-width bins): bin contents become bin values and bin
//! errors their uncertainties. A bin of unweighted counts (error^2 = content, a whole number) gets content entries,
//! other non-empty bins one entry. Under- and overflow bins are dropped. A TProfile or TProfile2D becomes a Profile
//! (see fromRootProfile()).
//! @param histogram ROOT histogram
//! @return new histogram (double precision), owned by the caller
//! @throws Exception if the histogram has variable-width bins or a dimension above 3
//!
MATH::Histogram * fromRootHistogram(const TH1 & histogram);

//!
//! @brief ROOT copy of a profile of rank 1 or 2: a TProfile or TProfile2D with the same sums per bin (w, w^2, w*y,
//! w*y^2), hence the same means and uncertainties.
//! @param profile profile to convert @param name name of the ROOT profile (the profile's name if empty)
//! @return new ROOT profile, owned by the caller @throws Exception if the rank is not 1 or 2
//!
TH1 * toRootProfile(const MATH::Profile & profile, const String & name="");

//!
//! @brief CAP copy of a TProfile or TProfile2D (fixed-width bins), with its sums per bin. The number of entries of a bin
//! is its sum of weights for unweighted fills, its effective number of entries otherwise.
//! @param profile ROOT profile @return new profile, owned by the caller
//! @throws Exception if it is not a TProfile or TProfile2D, or has variable-width bins
//!
MATH::Profile * fromRootProfile(const TH1 & profile);

//!
//! @brief ROOT copy of a histogram for drawing, or nullptr if h is null (see toRootHistogram()).
//! @param h histogram (may be null) @return new ROOT histogram owned by the caller, or nullptr
//!
inline TH1 * rootCopy(const MATH::Histogram * h) { return h ? toRootHistogram(*h) : nullptr; }

} // namespace CAP

#endif

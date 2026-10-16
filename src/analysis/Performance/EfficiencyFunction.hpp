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
#ifndef CAP__EfficiencyFunction
#define CAP__EfficiencyFunction

namespace CAP
{

//!
//! @brief Parameterized detection efficiency: polynomial in pT above ptMin and in eta, with Gaussian fall-offs below
//! etaLowEdge and above etaHighEdge.
//! @note Not part of the build (not listed in Performance/CMakeLists.txt).
//!
class EfficiencyFunction
{
public:
  
  //! @brief Constructor. @param _eff0 constant term @param _ptMin pT threshold @param _eff1Pt pT coefficients (orders 1 to 4)
  //! @param _eff2Pt see _eff1Pt @param _eff3Pt see _eff1Pt @param _eff4Pt see _eff1Pt @param _etaMin lower eta limit
  //! @param _etaMax upper eta limit @param _eff1Eta eta coefficients (orders 1 to 4) @param _eff2Eta see _eff1Eta
  //! @param _eff3Eta see _eff1Eta @param _eff4Eta see _eff1Eta @param _etaLowEdge start of the low-eta fall-off
  //! @param _etaLowSigma2 its width squared @param _etaHighEdge start of the high-eta fall-off @param _etaHighSigma2 its width squared
  EfficiencyFunction(double _eff0,
                     double _ptMin,
                     double _eff1Pt,
                     double _eff2Pt,
                     double _eff3Pt,
                     double _eff4Pt,
                     double _etaMin,
                     double _etaMax,
                     double _eff1Eta,
                     double _eff2Eta,
                     double _eff3Eta,
                     double _eff4Eta,
                     double _etaLowEdge,
                     double _etaLowSigma2,
                     double _etaHighEdge,
                     double _etaHighSigma2);
  //! @brief Copy constructor. @param f object to copy
  //! @brief Copy constructor. @param f function to copy
  EfficiencyFunction(const EfficiencyFunction & f);
  //! @brief Copy assignment. @param f object to copy @return this object
  //! @brief Copy assignment. @param f function to copy @return copy
  EfficiencyFunction operator=(const EfficiencyFunction & f);
  //! @brief Destructor.
  //! @brief Destructor.
  ~EfficiencyFunction() {}
  //! @brief Efficiency at a momentum. @param pt pT @param eta pseudorapidity @param phi azimuth (unused) @return efficiency
  double efficiency(double pt, double eta, double phi);
  //! @brief Random acceptance according to the efficiency. @param pt pT @param eta pseudorapidity @param phi azimuth
  //! @return true if accepted
  bool accept(double pt, double eta, double phi);


protected:
  
  double eff0;
  double ptMin;
  double eff1Pt;
  double eff2Pt;
  double eff3Pt;
  double eff4Pt;
  double etaMin;
  double etaMax;
  double eff1Eta;
  double eff2Eta;
  double eff3Eta;
  double eff4Eta;
  double etaLowEdge;
  double etaLowSigma2;
  double etaHighEdge;
  double etaHighSigma2;
};


} // namespace CAP

#endif /* CAP__EfficiencyFunction */

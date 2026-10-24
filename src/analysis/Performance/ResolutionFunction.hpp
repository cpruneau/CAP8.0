/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ResolutionFunction
#define CAP__ResolutionFunction
#include "HistogramFolder.hpp"

namespace CAP
{

//!
//! @brief Parameterized detector resolution: bias and rms as polynomials in pT and eta.
//! @note Not part of the build (not listed in Performance/CMakeLists.txt).
//!
class ResolutionFunction
{
public:
   
  //! @brief Default constructor.
  //! @brief Default constructor.
  ResolutionFunction();
  //! @brief Copy constructor. @param f object to copy
  //! @brief Copy constructor. @param f function to copy
  ResolutionFunction(const ResolutionFunction & f);
  //! @brief Copy assignment. @param rf object to copy @return this object
  //! @brief Copy assignment. @param rf function to copy @return copy
  ResolutionFunction operator=(const ResolutionFunction & rf);
  //! @brief Destructor.
  //! @brief Destructor.
  ~ResolutionFunction() {}
  //! @brief Set the parameterization. @param _option form of the parameterization @param _biasVsPt bias coefficients vs pT
  //! @param _biasVsEta bias coefficients vs eta @param _rmsVsPt rms coefficients vs pT @param _rmsVsEta rms coefficients vs eta
  void setCoefficients(int _option,
                       const std::vector<double> & _biasVsPt,
                       const std::vector<double> & _biasVsEta,
                       const std::vector<double> & _rmsVsPt,
                       const std::vector<double> & _rmsVsEta);
  //! @brief Smear a value. @param pt pT @param eta pseudorapidity @param phi azimuth @param output smeared value (set) @return smeared value
  double smear(double pt, double eta, double phi, double & output);

protected:

  int option;
  std::vector<double> biasVsPt;
  std::vector<double> biasVsEta;
  std::vector<double> rmsVsPt;
  std::vector<double> rmsVsEta;
};

} // namespace CAP

#endif /* CAP__ResolutionFunction */

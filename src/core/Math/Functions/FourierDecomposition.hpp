/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierDecomposition.hpp
//! @brief FourierDecomposition: a Fourier series of order nMax, in one of two forms, a composite of its harmonics;
//! FourierHarmonic: one of its terms.
#ifndef CAP_MATH_FourierDecomposition
#define CAP_MATH_FourierDecomposition
#include "ParameterizedComposite.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief One harmonic n >= 1 (fixed, not a parameter) of a Fourier decomposition, in one of two forms:
  //!  - kCosineSine: h(x) = c cos(n x) + s sin(n x), parameters c, s;
  //!  - kFlow:       h(x) = v cos(n (x - psi)),      parameters v, psi.
  //! Derivative and integral analytical. Registered in FunctionRegistry as "FourierHarmonic" (its order and form are
  //! kept by the files with the parameters).
  //!
  class FourierHarmonic : public ParameterizedFunction
  {
  public:

  //! @brief Form of the harmonics (and of a FourierDecomposition).
  enum Form
  {
    kCosineSine = 1,   //!< c cos(n x) + s sin(n x)
    kFlow       = 2    //!< v cos(n (x - psi))
  };

  //! @brief Harmonic of order n. @param n order (1 or more) @param form form @param a c or v @param b s or psi
  //! @throws MathException if n is 0 or the form is unknown
  explicit FourierHarmonic(unsigned int n=1, Form form=kCosineSine, double a=0.0, double b=0.0);
  FourierHarmonic(const FourierHarmonic & source) = default;   //!< Copy
  FourierHarmonic & operator=(const FourierHarmonic & rhs) = default;   //!< Assignment
  virtual ~FourierHarmonic() { }
  //! @brief Copy. @return new harmonic (owned by the caller)
  FourierHarmonic * clone() const override { return new FourierHarmonic(*this); }

  //! @brief Order of the harmonic. @return n
  unsigned int order() const { return _n; }
  //! @brief Form of the harmonic. @return form
  Form form() const { return _form; }
  //! @brief Value at x. @param x x @return h(x)
  double evaluateAt(double x) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Derivative at x. @param x x @return h'(x)
  double derivativeAt(double x) const override;
  //! @brief The derivative is analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  //! @brief Integral from low to high. @param low lower end @param high upper end @return integral
  double integralInRange(double low, double high) const override;
  using ParameterizedFunction::integralInRange;
  //! @brief Write the order and the form. @param w writer
  void encodeState(BinaryWriter & w) const override;
  //! @brief Read the order and the form (the parameters renamed accordingly). @param r reader @param version format
  //! version @throws Exception if they are invalid
  void decodeState(BinaryReader & r, unsigned int version) override;

  protected:

  //! @brief Name the parameters after the form.
  void nameParameters();

  unsigned int _n;   //!< order
  Form _form;        //!< form
  };

  //!
  //! @brief Fourier decomposition of order nMax (chosen by the caller), in one of two forms:
  //!  - kCosineSine (form 1): f(x) = k + sum_{n=1}^{nMax} [c_n cos(n x) + s_n sin(n x)],
  //!    parameters k, c1, s1, c2, s2, ..., c_nMax, s_nMax;
  //!  - kFlow (form 2):       f(x) = k [1 + 2 sum_{n=1}^{nMax} v_n cos(n (x - psi_n))],
  //!    parameters k, v1, psi1, v2, psi2, ..., v_nMax, psi_nMax (e.g. flow coefficients and symmetry planes).
  //! In both, k is parameter 0 and the two parameters of harmonic n are 2n-1 and 2n (harmonicIndex()). x is an angle
  //! (radians); the period of f is 2 pi.
  //!
  //! A ParameterizedComposite of a Constant (label "k") and of the harmonics n = 1 to nMax (FourierHarmonic of the same
  //! form, labels "h1", "h2", ...), whose parameters are named as above rather than "label.name". In form 2 the
  //! constant multiplies the harmonics: f = k (1 + 2 sum h_n), not their sum. Derivative and integral analytical.
  //!
  //! The coefficients of either form can be read in both: cosine(n) and sine(n) are those of form 1, flow(n) and
  //! flowPhase(n) those of form 2 (c_n = 2 k v_n cos(n psi_n), s_n = 2 k v_n sin(n psi_n)); they are set in the form
  //! of the decomposition only.
  //!
  //! The class is registered in FunctionRegistry as "FourierDecomposition" (made with 2 nMax + 1 parameters, form 1;
  //! nMax = 4 when the registry makes it with no number of parameters), and form 2 as "FourierFlow" (FourierFlow), so
  //! that both can be fitted in the Fit window of CapGui (estimates: Fitter::estimate()) and saved with their fits in a
  //! CAP file; a decomposition is read back in its own form. CalculateFourierDecomposition computes the decomposition
  //! of a function or of data points, by averages or by a fit, with its chi square.
  //!
  //! @code
  //!   CAP::MATH::FourierDecomposition f(3);                                   // form 1: k, c1, s1, ..., s3
  //!   f.setConstant(1.0);
  //!   f.setCosine(2, 0.1);                                                    // c2
  //!   CAP::MATH::FourierDecomposition g(4, CAP::MATH::FourierDecomposition::kFlow);   // form 2: k, v1, psi1, ...
  //!   g.setConstant(100.0);
  //!   g.setFlow(2, 0.06);                                                     // v2
  //!   g.setFlowPhase(2, 0.3);                                                 // psi2
  //!   double y = g(0.5), c2 = g.cosine(2);
  //! @endcode
  //!
  class FourierDecomposition : public ParameterizedComposite
  {
  public:

  using Form = FourierHarmonic::Form;   //!< form of the decomposition
  static constexpr Form kCosineSine = FourierHarmonic::kCosineSine;   //!< form 1
  static constexpr Form kFlow       = FourierHarmonic::kFlow;         //!< form 2

  //! @brief Decomposition of order nMax, all coefficients 0. @param nMax highest harmonic (1 or more) @param form form
  //! @throws MathException if nMax is 0 or the form is unknown
  FourierDecomposition(unsigned int nMax, Form form=kCosineSine);
  //!
  //! @brief Decomposition with coefficients; its order is that of the longer list (missing coefficients are 0).
  //! @param form form @param k constant @param a c_1, c_2, ... (form 1) or v_1, v_2, ... (form 2)
  //! @param b s_1, s_2, ... (form 1) or psi_1, psi_2, ... (form 2) @throws MathException if both lists are empty
  //!
  FourierDecomposition(Form form, double k, const std::vector<double> & a, const std::vector<double> & b);
  FourierDecomposition(const FourierDecomposition & source) = default;   //!< Copy (the components are copied)
  FourierDecomposition & operator=(const FourierDecomposition & rhs) = default;   //!< Assignment
  virtual ~FourierDecomposition() { }
  //! @brief Copy. @return new decomposition (owned by the caller)
  FourierDecomposition * clone() const override { return new FourierDecomposition(*this); }

  //! @name Values
  //! @{
  //! @brief Value at x. @param x x @return f(x)
  double evaluateAt(double x) const override;
  //! @brief Value at a point (rank 1). @param x coordinates (one) @return f(x[0])
  double evaluateAt(const std::vector<double> & x) const override;
  using ParameterizedComposite::evaluateAt;
  //! @brief Derivative at x. @param x x @return f'(x)
  double derivativeAt(double x) const override;
  //! @brief The derivative is analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  //! @brief Derivative, a decomposition of form 1: c_n -> n s_n, s_n -> -n c_n, the constant 0. @param axis 0
  //! @return new FourierDecomposition (owned by the caller)
  Function * derivativeFunction(unsigned int axis=0) const override;
  //! @brief Integral from low to high. @param low lower end @param high upper end @return integral
  double integralInRange(double low, double high) const override;
  //! @brief Integral over a box (rank 1). @param low lower end (one) @param high upper end (one) @return integral
  double integralInRange(const std::vector<double> & low, const std::vector<double> & high) const override;
  using ParameterizedComposite::integralInRange;
  //! @}

  //! @name Coefficients
  //! @{
  //! @brief Form of the decomposition. @return form
  Form form() const { return _form; }
  //! @brief Highest harmonic. @return nMax
  unsigned int nMax() const { return (nParameters() - 1)/2; }
  //! @brief Index of the first parameter of harmonic n (c_n or v_n; the second, s_n or psi_n, follows).
  //! @param n harmonic (1 to nMax) @return 2n-1 @throws MathException if n is out of range
  unsigned int harmonicIndex(unsigned int n) const;
  //! @brief Constant. @return k
  double constant() const { return parameterValue(0); }
  //! @brief Set the constant. @param k value
  void setConstant(double k) { setParameterValue(0, k); }
  //! @brief Cosine coefficient c_n of form 1 (form 2: 2 k v_n cos(n psi_n)). @param n harmonic (1 to nMax) @return c_n
  double cosine(unsigned int n) const;
  //! @brief Sine coefficient s_n of form 1 (form 2: 2 k v_n sin(n psi_n)). @param n harmonic (1 to nMax) @return s_n
  double sine(unsigned int n) const;
  //! @brief Flow coefficient v_n of form 2 (form 1: sqrt(c_n^2 + s_n^2)/(2 k)). @param n harmonic (1 to nMax)
  //! @return v_n @throws MathException in form 1 if k is 0
  double flow(unsigned int n) const;
  //! @brief Phase psi_n of form 2 (form 1: atan2(s_n, c_n)/n, 0 if c_n = s_n = 0). @param n harmonic (1 to nMax)
  //! @return psi_n
  double flowPhase(unsigned int n) const;
  //! @brief Set c_n. @param n harmonic (1 to nMax) @param c value @throws MathException in form 2
  void setCosine(unsigned int n, double c);
  //! @brief Set s_n. @param n harmonic (1 to nMax) @param s value @throws MathException in form 2
  void setSine(unsigned int n, double s);
  //! @brief Set v_n. @param n harmonic (1 to nMax) @param v value @throws MathException in form 1
  void setFlow(unsigned int n, double v);
  //! @brief Set psi_n. @param n harmonic (1 to nMax) @param psi value @throws MathException in form 1
  void setFlowPhase(unsigned int n, double psi);
  //! @}

  //! @brief Write the form and the order (the components follow from them). @param w writer
  void encodeState(BinaryWriter & w) const override;
  //! @brief Read the form and the order, and rebuild the components in that form. @param r reader
  //! @param version format version @throws Exception if the order differs from the number of parameters
  void decodeState(BinaryReader & r, unsigned int version) override;

  protected:

  //! @brief Remove the components and build those of a form and an order (parameters named k, c1, s1, ... or k, v1,
  //! psi1, ...). @param nMax highest harmonic @param form form @throws MathException if nMax is 0 or the form unknown
  void build(unsigned int nMax, Form form);
  //! @brief Throw unless the decomposition has a form. @param wanted form @param what caller
  void checkForm(Form wanted, const char * what) const;

  Form _form;   //!< form
  };

  //!
  //! @brief A FourierDecomposition of form 2 (kFlow), k [1 + 2 sum v_n cos(n (x - psi_n))]: registered in
  //! FunctionRegistry as "FourierFlow" (made with 2 nMax + 1 parameters), so that both forms are offered by the Fit
  //! window of CapGui.
  //!
  class FourierFlow : public FourierDecomposition
  {
  public:

  //! @brief Decomposition of form 2 of order nMax. @param nMax highest harmonic (1 or more)
  //! @throws MathException if nMax is 0
  explicit FourierFlow(unsigned int nMax) : FourierDecomposition(nMax, kFlow) { }
  FourierFlow(const FourierFlow & source) = default;   //!< Copy
  FourierFlow & operator=(const FourierFlow & rhs) = default;   //!< Assignment
  virtual ~FourierFlow() { }
  //! @brief Copy. @return new decomposition (owned by the caller)
  FourierFlow * clone() const override { return new FourierFlow(*this); }
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_FourierDecomposition

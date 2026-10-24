#ifndef CAP_MATH_SphericalHarmonics
#define CAP_MATH_SphericalHarmonics
#include <vector>
#include <complex>
#include "String.hpp"
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Spherical harmonic Y_l^m(theta, phi) as a Function of rank 2, of the polar angle theta and azimuth phi (radians).
  //!
  //! Y_l^m(theta, phi) = N_lm P_l^m(cos theta) e^(i m phi), with N_lm = sqrt((2l+1)/(4 pi) (l-m)!/(l+m)!),
  //! l >= 0 and -l <= m <= l. Convention: orthonormal on the unit sphere, with the Condon-Shortley phase (-1)^m
  //! included in P_l^m, so that Y_l^(-m) = (-1)^m conj(Y_l^m) (as in Jackson, Arfken, and most physics texts).
  //!
  //! value() returns the complex Y_l^m. Since a Function is real-valued, evaluateAt() returns the part selected
  //! with setPart():
  //!  - RealPart      : Re Y_l^m (default)
  //!  - ImaginaryPart : Im Y_l^m
  //!  - RealHarmonic  : real spherical harmonic Y_lm, an orthonormal real basis:
  //!                    sqrt(2) (-1)^m Re Y_l^m for m > 0, Y_l^0 for m = 0, sqrt(2) (-1)^m Im Y_l^|m| for m < 0
  //!                    (i.e. sqrt(2) N_l|m| P_l^|m| cos(m phi) or sin(|m| phi), without the Condon-Shortley phase)
  //!
  //! The normalized functions N_lm P_l^m are computed by a stable recurrence that does not overflow for large l.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   SphericalHarmonics y21(2, 1);
  //!   std::complex<double> v = y21.value(0.3, 1.2);            // Y_2^1(0.3, 1.2)
  //!   double re = y21.evaluateAt(0.3, 1.2);                    // Re Y_2^1
  //!   SphericalHarmonics r(3, -2, SphericalHarmonics::RealHarmonic);
  //! @endcode
  //!
  class SphericalHarmonics : public Function
  {
  public:

  //! @brief Quantity returned by evaluateAt().
  enum Part
  {
    RealPart,       //!< Re Y_l^m
    ImaginaryPart,  //!< Im Y_l^m
    RealHarmonic    //!< real spherical harmonic Y_lm
  };

  //!
  //! @brief Constructor (also the default constructor: Y_0^0 = 1/sqrt(4 pi)).
  //! @param l degree (>= 0) @param m order (-l <= m <= l) @param part quantity returned by evaluateAt()
  //! @throws MathException if |m| > l
  //!
  SphericalHarmonics(unsigned int l=0, int m=0, Part part=RealPart);
  //! @brief Copy constructor. @param source function to copy
  SphericalHarmonics(const SphericalHarmonics & source);
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  SphericalHarmonics & operator=(const SphericalHarmonics & rhs);
  //! @brief Destructor.
  virtual ~SphericalHarmonics() {}

  //! @brief Degree l. @return l
  unsigned int degree() const { return _l; }
  //! @brief Order m. @return m
  int order() const { return _m; }
  //! @brief Quantity returned by evaluateAt(). @return part
  Part part() const { return _part; }
  //! @brief Set the degree and order. @param l degree @param m order @throws MathException if |m| > l (nothing is changed)
  void setDegreeAndOrder(unsigned int l, int m);
  //! @brief Set the quantity returned by evaluateAt(). @param part new part
  void setPart(Part part) { _part = part; }
  //! @brief The rank of a spherical harmonic is always 2. @param rank must be 2 @throws MathException otherwise
  void setRank(unsigned int rank) override;

  //! @brief Complex value Y_l^m(theta, phi). @param theta polar angle (radians) @param phi azimuth (radians) @return Y_l^m
  std::complex<double> value(double theta, double phi) const;
  //! @brief Selected part (see Part) at (theta, phi). @param theta polar angle (radians) @param phi azimuth (radians) @return value
  double evaluateAt(double theta, double phi) const override;
  //! @brief Selected part at (x[0], x[1]) = (theta, phi). @param x two values @return value @throws MathException if x.size() != 2
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;

  //!
  //! @brief Normalized associated Legendre function N_lm P_l^m(x), with the Condon-Shortley phase, for 0 <= m <= l.
  //!
  //! Equal to Y_l^m(theta, 0) with x = cos theta.
  //! @param l degree @param m order (0 <= m <= l) @param x argument, -1 <= x <= 1
  //! @return N_lm P_l^m(x)
  //! @throws MathException if m > l or |x| > 1
  //!
  static double normalizedLegendre(unsigned int l, unsigned int m, double x);

  protected:

  //!
  //! @brief Same as normalizedLegendre(l, m, x), with 1 - x^2 given separately: for x = cos theta, sin^2 theta computed
  //! from theta is accurate near the poles, where 1 - cos^2 theta loses most of its digits.
  //!
  static double normalizedLegendre(unsigned int l, unsigned int m, double x, double oneMinusX2);

  unsigned int _l;  //!< degree l
  int _m;           //!< order m
  Part _part;       //!< quantity returned by evaluateAt()

  };

  } // namespace MATH
} // namespace CAP

#endif

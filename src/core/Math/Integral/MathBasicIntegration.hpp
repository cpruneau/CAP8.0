#ifndef CAP_MathBasicIntegration
#define CAP_MathBasicIntegration
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <cmath>
#include "MathConstants.hpp"
#include "MathBasicFunctions.hpp"
#include "MathExceptions.hpp"

using namespace std;

//!
//! @file MathBasicIntegration.hpp
//! @brief Integration of sampled values: the trapezoidal rule, Simpson's rule and the cubic rule, in one dimension
//! (equally spaced or not), on regular grids of two or n dimensions, and cumulative integrals.
//!
//! The functions take values already computed (a table, a histogram, the nodes of a field). Functions are integrated
//! by the integrators (Integrator and its derived classes, Integration). By convention, all function names begin with a
//! lower case letter.
//!
//! Grids of n dimensions: the values are stored with axis 0 varying fastest (node (i0, i1, ...) at
//! i0 + n0 (i1 + n1 (...))), nPoints[k] nodes along axis k, steps[k] apart.
//!
//! @code
//!   std::vector<double> y = {0, 1, 4, 9, 16};                                   // x^2 at x = 0, 1, ..., 4
//!   double t = CAP::MATH::integrationTrapezoidRule(y, 1.0);                      // 22
//!   double s = CAP::MATH::integrationSimpsonRule(y, 1.0);                        // 64/3, exact
//!   std::vector<double> c = CAP::MATH::cumulativeTrapezoidRule(y, 1.0);          // 0, 0.5, 3, 9.5, 22
//! @endcode
//!
namespace CAP
{
namespace MATH
{

//! @brief Simpson's rule on equally spaced values (an odd number; with an even number the last interval is left out).
//! @param values values @param nValues number of values (3 or more) @param stepSize spacing @return integral
//! @throws MathException if values is null, nValues < 3 or stepSize <= 0
double integrationSimpsonRule(double* values, int nValues, double stepSize) ;
//! @brief Simpson's rule on equally spaced values (see the pointer version). @param values values
//! @param stepSize spacing @return integral @throws MathException if there are fewer than 3 values or stepSize <= 0
double integrationSimpsonRule(const vector<double> & values, double stepSize) ;

//! @brief Trapezoidal rule on equally spaced values. @param values values (2 or more) @param stepSize spacing
//! @return integral @throws MathException if there are fewer than 2 values
double integrationTrapezoidRule(const vector<double> & values, double stepSize);
//! @brief Trapezoidal rule on points at any spacing. @param x abscissas @param y values @return integral
//! @throws MathException if the sizes differ or there are fewer than 2 points
double integrationTrapezoidRule(const vector<double> & x, const vector<double> & y);
//! @brief Simpson's rule on points at any spacing (pairs of intervals; a last odd interval by a parabola through the
//! last three points). @param x increasing abscissas @param y values @return integral
//! @throws MathException if the sizes differ or there are fewer than 3 points
double integrationSimpsonRule(const vector<double> & x, const vector<double> & y);
//! @brief Cubic rule: the integral of the cubic spline (not-a-knot) through the points, at any spacing; exact for
//! cubic polynomials. @param x increasing abscissas @param y values @return integral
//! @throws MathException if the sizes differ or there are fewer than 2 points
double integrationCubicRule(const vector<double> & x, const vector<double> & y);
//! @brief Cubic rule on equally spaced values (see the other version). @param values values @param stepSize spacing
//! @return integral
double integrationCubicRule(const vector<double> & values, double stepSize);

//! @brief Cumulative trapezoidal rule: c[0] = 0, c[k] the integral from the first value to value k.
//! @param values equally spaced values @param stepSize spacing @return cumulative integrals (as many as the values)
vector<double> cumulativeTrapezoidRule(const vector<double> & values, double stepSize);
//! @brief Cumulative Simpson's rule on panels of two intervals: values 2k, 2k+1, 2k+2 make panel k.
//! @param values 2n+1 equally spaced values @param stepSize spacing of the values (half the width of a panel)
//! @return n+1 cumulative integrals, at the ends of the panels (c[0] = 0)
//! @throws MathException if the number of values is not odd or is less than 3
vector<double> cumulativeSimpsonRule(const vector<double> & values, double stepSize);

//! @brief Trapezoidal rule on a regular grid of two dimensions. @param values values (axis 0 fastest)
//! @param nX nodes along x @param nY nodes along y @param stepX spacing along x @param stepY spacing along y
//! @return integral
double integrationTrapezoidRule2D(const vector<double> & values, unsigned int nX, unsigned int nY, double stepX, double stepY);
//! @brief Simpson's rule on a regular grid of two dimensions (odd numbers of nodes). @param values values
//! @param nX nodes along x @param nY nodes along y @param stepX spacing along x @param stepY spacing along y @return integral
double integrationSimpsonRule2D(const vector<double> & values, unsigned int nX, unsigned int nY, double stepX, double stepY);
//! @brief Trapezoidal rule on a regular grid of n dimensions. @param values values (axis 0 fastest)
//! @param nPoints nodes along each axis (2 or more) @param steps spacing along each axis @return integral
//! @throws MathException if the sizes do not match
double integrationTrapezoidRuleND(const vector<double> & values, const vector<unsigned int> & nPoints, const vector<double> & steps);
//! @brief Simpson's rule on a regular grid of n dimensions. @param values values (axis 0 fastest)
//! @param nPoints nodes along each axis (odd, 3 or more) @param steps spacing along each axis @return integral
//! @throws MathException if the sizes do not match or a number of nodes is even or less than 3
double integrationSimpsonRuleND(const vector<double> & values, const vector<unsigned int> & nPoints, const vector<double> & steps);

} // namespace MATH

} // namespace CAP

#endif

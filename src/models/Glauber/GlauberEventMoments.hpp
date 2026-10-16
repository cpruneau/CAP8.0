#ifndef CAP__GlauberEventMoments
#define CAP__GlauberEventMoments
#include "PrintHelpers.hpp"

namespace CAP
{

//!
//! @brief Moments of a distribution of points in the transverse plane (e.g., participant positions): means,
//! variances, rms, eccentricity and participant-plane angle, and the eccentricities eps_n and angles psi_n of
//! orders 1 to nOrder. Filled in two passes (means, then moments about the center).
//!
class GlauberEventMoments
{
public:

  //! @brief Default constructor.
  GlauberEventMoments();
  //! @brief Copy constructor. @param analyzer object to copy
  GlauberEventMoments(const GlauberEventMoments & analyzer);
  //! @brief Copy assignment. @param analyzer object to copy @return this object
  GlauberEventMoments operator=(const GlauberEventMoments & analyzer);
  //! @brief Destructor.
  virtual ~GlauberEventMoments();
  //! @brief Size the arrays of the harmonics. @param nOrder highest order
  virtual void initialize(int nOrder=4);
  //! @brief Set all sums and moments to 0.
  virtual void reset();
  //! @brief Start again with the first pass.
  virtual void resetPass() { _pass = 0;}
  //! @brief Clear the moments.
  virtual void clear();
  //! @brief Add a point. @param x x position @param y y position @param weight weight
  virtual void fill(double x, double y, double weight=1.0);
  //! @brief Compute the means, variances, eccentricities and angles from the sums.
  virtual void calculateAverages();
  //! @brief Save the moments (not implemented).
  virtual void save();
  //! @brief Print the object.
  virtual void print() const;

  //! Options: number-weighted, density, area and length moments.
  bool _doN;
  bool _doDensity;
  bool _doArea;
  bool _dolength;

  //! Current pass (0: means, 1: moments about the center).
  int    _pass;
  //! Highest harmonic order.
  int    _nOrders;
  //! Sums of the weights and of x, y, x^2, y^2, xy.
  double _sum;
  double _xSum;
  double _ySum;
  double _x2Sum;
  double _y2Sum;
  double _xySum;
  //! Means of x, y, x^2, y^2, xy.
  double _meanX;
  double _meanY;
  double _meanX2;
  double _meanY2;
  double _meanXY;
  //! Variances and covariance.
  double _varX;
  double _varY;
  double _varXY;
  //! rms in x and y.
  double _rmsX;
  double _rmsY;
  //! Eccentricities and participant-plane angle.
  double _eccX;
  double _eccY;
  double _ecc;
  double _psi;

  //! Harmonic sums and eccentricities eps_n, angles psi_n of each order.
  std::vector<double> _cosPhiN;
  std::vector<double> _sinPhiN;
  std::vector<double> _rN;
  std::vector<double> _psiN;
  std::vector<double> _eccN;

  ClassDef(GlauberEventMoments,0)
};

} // namespace CAP

#endif


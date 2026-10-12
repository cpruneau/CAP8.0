#ifndef CAP__PTotProfile
#define CAP__PTotProfile
#include "MathConstants.hpp"
#include <vector>
using namespace std;
using CAP::MATH::pi;
using CAP::MATH::twoPi;

namespace CAP
{

  //!
  //! @brief Distribution of the nucleon-nucleon cross section (fluctuating nucleon sizes):
  //! f(x) = (u/(u+sigma)) exp(-(u/sigma-1)^2/omega^2)/lambda with u = x/lambda (Loizides et al.).
  //!
  class PTotProfile
  {
  public:

  //! @brief Default constructor.
  PTotProfile();
  //! @brief Copy constructor. @param src object to copy
  PTotProfile(const PTotProfile & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  PTotProfile & operator=(const PTotProfile & rhs);
  //! @brief Destructor.
  virtual ~PTotProfile(){}
  //! @brief Probability density at a cross section. @param x cross section @return density
  virtual double evaluate(double x);
  //! @brief Random cross section drawn from evaluate() (inverse transform of its cumulative distribution).
  //! @return cross section @throws Exception if the parameters are not set
  double random();
  //! @brief Set the parameters. @param nnXSect mean cross section @param nnXSectOmega relative width
  //! @param nnXSectLambda scale
  void setParameters(double nnXSect,
                     double nnXSectOmega,
                     double nnXSectLambda);
  //! @brief Mean cross section. @return cross section
  double nnXSection() { return _nnXSect; }
  //! @brief Relative width. @return omega
  double nnXSectionOmega() { return _nnXSectOmega; }
  //! @brief Scale. @return lambda
  double nnXSectionLambda() { return _nnXSectLambda; }
  //! @brief Reset the parameters.
  virtual void reset();

  protected:

  double _nnXSect;
  double _nnXSectOmega;
  double _nnXSectLambda;
  double _xMax;                 //!< upper edge of the tabulated range (cache, not saved)
  std::vector<double> _cdf;     //!< normalized cumulative distribution on [0,_xMax] (cache, not saved)

  //! @brief Tabulate the cumulative distribution used by random() (called by setParameters()).
  void buildTable();
  };

} // namespace CAP

#endif




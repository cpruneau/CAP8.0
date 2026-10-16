#include "GlauberExceptions.hpp"
#include "PTotProfile.hpp"
#include "TRandom.h"
#include <algorithm>
#include <cmath>

namespace CAP
{
  PTotProfile::PTotProfile()
  :
  _nnXSect(0),
  _nnXSectOmega(0),
  _nnXSectLambda(0),
  _xMax(0),
  _cdf()
  { }

  PTotProfile::PTotProfile(const PTotProfile & src)
  :
  _nnXSect(src._nnXSect),
  _nnXSectOmega(src._nnXSectOmega),
  _nnXSectLambda(src._nnXSectLambda),
  _xMax(src._xMax),
  _cdf(src._cdf)
  { }

  PTotProfile & PTotProfile::operator=(const PTotProfile & rhs)
  {
  if (this!=&rhs)
    {
    _nnXSect       = rhs._nnXSect;
    _nnXSectOmega  = rhs._nnXSectOmega;
    _nnXSectLambda = rhs._nnXSectLambda;
    _xMax          = rhs._xMax;
    _cdf           = rhs._cdf;
    }
  return *this;
  }

  void PTotProfile::setParameters(double nnXSect,
                                  double nnXSectOmega,
                                  double nnXSectLambda)
  {
  _nnXSect       = nnXSect;
  _nnXSectOmega  = nnXSectOmega;
  _nnXSectLambda = nnXSectLambda;
  buildTable();
  }

  void PTotProfile::buildTable()
  {
  // cumulative distribution of evaluate() on [0,_xMax], used by random() (inverse transform with linear interpolation)
  _cdf.clear();
  _xMax = 0.0;
  if (_nnXSect<=0.0 || _nnXSectLambda<=0.0 || _nnXSectOmega<=0.0) return;
  _xMax = _nnXSectLambda*_nnXSect*(1.0+8.0*_nnXSectOmega);   // the density falls as exp(-((x/lambda)/sigma-1)^2/omega^2)
  const int n = 4000;
  const double dx = _xMax/n;
  _cdf.assign(n+1,0.0);
  for (int k=1; k<=n; k++) _cdf[k] = _cdf[k-1] + std::max(evaluate((k-0.5)*dx),0.0)*dx;
  if (_cdf[n]<=0.0) { _cdf.clear(); _xMax = 0.0; return; }
  for (int k=1; k<=n; k++) _cdf[k] /= _cdf[n];
  }

  //! From Loizides
  //! f == ((x/[2])/(x/[2]+[0]))*exp(-(((x/[2])/[0]-1 )**2)/([1]*[1]))/[2]
  //!
  //!  ((x/sigma)/(x/sigma+[0]))*exp(-(((x/sigma)/[0]-1 )**2)/([1]*[1]))/[2]
  //!
  //!  let arg = x/sigma
  //!  (arg/(arg+[0]))*exp(-((arg/[0]-1)**2)/([1]*[1]))/[2]
  //! [0] _nnXSect,
  //! [1] _nnXSectOmega,
  //! [2] _nnXSectLambda
  //!
  double PTotProfile::evaluate(double x)
  {

  double xr = x/_nnXSectLambda;
  double ss = _nnXSectOmega*_nnXSectOmega;
  double aa = xr/_nnXSect-1.0;
  return (xr/(xr+_nnXSect))*std::exp(-aa*aa/ss)/_nnXSectLambda;
  }

  double PTotProfile::random()
  {
  // was: return 0; (not implemented)
  if (_cdf.empty()) throw Exception("Parameters not set (or invalid): call setParameters() first",__FUNCTION__);
  const unsigned int n = _cdf.size()-1;
  double r = gRandom->Rndm();
  unsigned int k = std::upper_bound(_cdf.begin(),_cdf.end(),r) - _cdf.begin();   // _cdf[k-1] <= r < _cdf[k]
  if (k<1) k = 1;
  if (k>n) k = n;
  double c0 = _cdf[k-1];
  double c1 = _cdf[k];
  double f  = (c1>c0) ? (r-c0)/(c1-c0) : 0.5;
  return (k-1+f)*_xMax/n;
  }

  void PTotProfile::reset()
  {
  _nnXSect = 0;
  _nnXSectOmega = 0;
  _nnXSectLambda = 0;
  _xMax = 0;
  _cdf.clear();
  }
  
}  // namespace CAP




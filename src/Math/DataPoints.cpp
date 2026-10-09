#include "DataPoints.hpp"
#include "MathExceptions.hpp"
#include <iostream>

ClassImp(CAP::MATH::DataPoints);

namespace CAP
{
  namespace MATH
  {

  void DataPoints::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  addPoint(DataPoint2D(x,xError,y,yStatError,ySysError));
  }

  void DataPoints::addPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  for (const auto & p : dataPoints) addPoint(p);
  }

  void DataPoints::addPoints(const std::vector<double> & x,
                               const std::vector<double> & y)
  {
  static const std::vector<double> none;
  addPoints(x,none,y,none,none);
  }

  void DataPoints::addPoints(const std::vector<double> & x,
                               const std::vector<double> & y,
                               const std::vector<double> & yStatError)
  {
  static const std::vector<double> none;
  addPoints(x,none,y,yStatError,none);
  }

  void DataPoints::addPoints(const std::vector<double> & x,
                               const std::vector<double> & xError,
                               const std::vector<double> & y,
                               const std::vector<double> & yStatError,
                               const std::vector<double> & ySysError)
  {
  unsigned int n = x.size();
  if (y.size()!=n)                                throw MathException("x and y have different sizes",__FUNCTION__);
  if (!xError.empty()     && xError.size()!=n)     throw MathException("x and xError have different sizes",__FUNCTION__);
  if (!yStatError.empty() && yStatError.size()!=n) throw MathException("x and yStatError have different sizes",__FUNCTION__);
  if (!ySysError.empty()  && ySysError.size()!=n)  throw MathException("x and ySysError have different sizes",__FUNCTION__);
  for (unsigned int k=0; k<n; k++)
    {
    addPoint(DataPoint2D(x[k],
                         xError.empty()     ? 0.0f : float(xError[k]),
                         y[k],
                         yStatError.empty() ? 0.0f : float(yStatError[k]),
                         ySysError.empty()  ? 0.0f : float(ySysError[k])));
    }
  }

  unsigned int DataPoints::extremumIndex(bool useY, bool findMaximum) const
  {
  unsigned int n = size();
  if (n==0) throw MathException("No points in container",__FUNCTION__);
  unsigned int best = 0;
  DataPoint2D p0 = point(0);
  float bestValue = useY ? p0.y() : p0.x();
  for (unsigned int k=1; k<n; k++)
    {
    DataPoint2D p = point(k);
    float v = useY ? p.y() : p.x();
    if (findMaximum ? (v>bestValue) : (v<bestValue)) { best = k; bestValue = v; }
    }
  return best;
  }

  float DataPoints::xMinimum()    const { return point(extremumIndex(false,false)).x(); }
  float DataPoints::xMaximum()    const { return point(extremumIndex(false,true)).x();  }
  float DataPoints::xAtYMaximum() const { return point(extremumIndex(true,true)).x();   }
  float DataPoints::xAtYMinimum() const { return point(extremumIndex(true,false)).x();  }
  float DataPoints::yMinimum()    const { return point(extremumIndex(true,false)).y();  }
  float DataPoints::yMaximum()    const { return point(extremumIndex(true,true)).y();   }
  float DataPoints::yAtXMaximum() const { return point(extremumIndex(false,true)).y();  }
  float DataPoints::yAtXMinimum() const { return point(extremumIndex(false,false)).y(); }

  void DataPoints::print() const
  {
  for (unsigned int k=0; k<size(); k++) std::cout << k << ": " << point(k) << std::endl;
  }

  std::vector<double> DataPoints::coordinatesAt(unsigned int index) const      { return std::vector<double>{point(index).x()}; }
  std::vector<double> DataPoints::coordinateErrorsAt(unsigned int index) const { return std::vector<double>{point(index).xError()}; }
  double DataPoints::valueAt(unsigned int index) const     { return point(index).y(); }
  double DataPoints::statErrorAt(unsigned int index) const { return point(index).yStatError(); }
  double DataPoints::sysErrorAt(unsigned int index) const  { return point(index).ySysError(); }

  } // namespace MATH
} // namespace CAP

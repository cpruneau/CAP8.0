#include "MathBasicIntegration.hpp"
#include "CubicSpline.hpp"

namespace CAP
{

namespace MATH
{

//!
//! Integration of a function based on values obtained from that function and computed at regular
//! intervals of size 'stepSize'.
//!
double integrationSimpsonRule(double* values, int nValues, double stepSize)
{
  if (!values)     throw MathException("First argument is a null pointer","integrationSimpsonRule()");
  if (nValues<3)   throw MathException("nValues<3","integrationSimpsonRule()");
  if (stepSize<=0) throw MathException("stepSize<=0","integrationSimpsonRule()");
  double sum = 0;
  for (int j=0; j<nValues-2; j+=2)
    sum += values[j]+4.0*values[j+1]+values[j+2];
  sum *= stepSize/3.0;
  return sum;
}

//!
//! Integration of a function based on values obtained from that function and computed at regular
//! intervals of size 'stepSize'.
//!
double integrationSimpsonRule(const vector<double> & values, double stepSize)
{
  int nValues = values.size();
  if (nValues<3)   throw MathException("values.size()<3","integrationSimpsonRule()");
  if (stepSize<=0) throw MathException("stepSize<=0","integrationSimpsonRule()");
  double sum = 0;
  for (int j=0; j<nValues-2; j+=2)
    sum += values[j]+4.0*values[j+1]+values[j+2];
  sum *= stepSize/3.0;
  return sum;
}

double integrationTrapezoidRule(const vector<double> & values, double stepSize)
{
  if (values.size()<2) throw MathException("values.size()<2","integrationTrapezoidRule()");
  double sum = 0.5*(values.front()+values.back());
  for (size_t k=1; k+1<values.size(); k++) sum += values[k];
  return sum*stepSize;
}

double integrationTrapezoidRule(const vector<double> & x, const vector<double> & y)
{
  if (x.size()!=y.size()) throw MathException("The sizes of x and y differ","integrationTrapezoidRule()");
  if (x.size()<2)         throw MathException("Fewer than 2 points","integrationTrapezoidRule()");
  double sum = 0;
  for (size_t k=0; k+1<x.size(); k++) sum += 0.5*(y[k]+y[k+1])*(x[k+1]-x[k]);
  return sum;
}

namespace
{
// integral over [x0, x2] of the parabola through (x0,y0), (x1,y1), (x2,y2)
double parabolaIntegral(double x0, double x1, double x2, double y0, double y1, double y2)
{
  double h0 = x1-x0, h1 = x2-x1, h = h0+h1;
  return h/6.0*((2.0-h1/h0)*y0 + h*h/(h0*h1)*y1 + (2.0-h0/h1)*y2);
}
// integral over [x1, x2] of the parabola through (x0,y0), (x1,y1), (x2,y2): y1 + b t + c t^2, t = x - x1
double parabolaLastInterval(double x0, double x1, double x2, double y0, double y1, double y2)
{
  double h0 = x1-x0, h1 = x2-x1;
  double c = ((y2-y1)/h1 + (y0-y1)/h0)/(h0+h1);
  double b = (y2-y1)/h1 - c*h1;
  return y1*h1 + 0.5*b*h1*h1 + c*h1*h1*h1/3.0;
}

void checkGrid(const vector<double> & values, const vector<unsigned int> & nPoints, const vector<double> & steps,
               const char * source)
{
  if (nPoints.empty() || nPoints.size()!=steps.size()) throw MathException("The sizes of nPoints and steps differ, or are 0",source);
  size_t n = 1;
  for (unsigned int p : nPoints) n *= p;
  if (n!=values.size()) throw MathException("The number of values differs from the number of nodes",source);
}

// product rule on a grid: weight(k, i) is the weight of node i of axis k (times the step)
template <class Weight>
double productRule(const vector<double> & values, const vector<unsigned int> & nPoints, Weight weight)
{
  const size_t rank = nPoints.size();
  vector<unsigned int> index(rank,0);
  double sum = 0;
  for (size_t g=0; g<values.size(); g++)
    {
    double w = 1.0;
    for (size_t k=0; k<rank; k++) w *= weight(k,index[k]);
    sum += w*values[g];
    size_t k = 0;
    while (k<rank && ++index[k]==nPoints[k]) { index[k] = 0; k++; }
    }
  return sum;
}
} // namespace

double integrationSimpsonRule(const vector<double> & x, const vector<double> & y)
{
  if (x.size()!=y.size()) throw MathException("The sizes of x and y differ","integrationSimpsonRule()");
  if (x.size()<3)         throw MathException("Fewer than 3 points","integrationSimpsonRule()");
  double sum = 0;
  size_t k = 0;
  for (; k+2<x.size(); k+=2) sum += parabolaIntegral(x[k],x[k+1],x[k+2],y[k],y[k+1],y[k+2]);
  if (k+1<x.size()) sum += parabolaLastInterval(x[k-1],x[k],x[k+1],y[k-1],y[k],y[k+1]);   // a last odd interval
  return sum;
}

double integrationCubicRule(const vector<double> & x, const vector<double> & y)
{
  if (x.size()!=y.size()) throw MathException("The sizes of x and y differ","integrationCubicRule()");
  if (x.size()<2)         throw MathException("Fewer than 2 points","integrationCubicRule()");
  CubicSpline spline(x,y,CubicSpline::NotAKnot);
  return spline.integralInRange(x.front(),x.back());
}

double integrationCubicRule(const vector<double> & values, double stepSize)
{
  vector<double> x(values.size());
  for (size_t k=0; k<x.size(); k++) x[k] = k*stepSize;
  return integrationCubicRule(x,values);
}

vector<double> cumulativeTrapezoidRule(const vector<double> & values, double stepSize)
{
  vector<double> c(values.size(),0.0);
  for (size_t k=1; k<values.size(); k++) c[k] = c[k-1] + 0.5*(values[k-1]+values[k])*stepSize;
  return c;
}

vector<double> cumulativeSimpsonRule(const vector<double> & values, double stepSize)
{
  if (values.size()<3 || values.size()%2==0) throw MathException("The number of values must be odd and 3 or more","cumulativeSimpsonRule()");
  size_t n = (values.size()-1)/2;
  double width = 2.0*stepSize;
  vector<double> c(n+1,0.0);
  for (size_t k=0; k<n; k++) c[k+1] = c[k] + width*(values[2*k] + 4.0*values[2*k+1] + values[2*k+2])/6.0;
  return c;
}

double integrationTrapezoidRuleND(const vector<double> & values, const vector<unsigned int> & nPoints, const vector<double> & steps)
{
  checkGrid(values,nPoints,steps,"integrationTrapezoidRuleND()");
  for (unsigned int p : nPoints) if (p<2) throw MathException("Fewer than 2 nodes along an axis","integrationTrapezoidRuleND()");
  return productRule(values,nPoints,[&](size_t k, unsigned int i)
    { return (i==0 || i+1==nPoints[k]) ? 0.5*steps[k] : steps[k]; });
}

double integrationSimpsonRuleND(const vector<double> & values, const vector<unsigned int> & nPoints, const vector<double> & steps)
{
  checkGrid(values,nPoints,steps,"integrationSimpsonRuleND()");
  for (unsigned int p : nPoints) if (p<3 || p%2==0) throw MathException("The number of nodes along an axis must be odd and 3 or more","integrationSimpsonRuleND()");
  return productRule(values,nPoints,[&](size_t k, unsigned int i)
    { return steps[k]/3.0*((i==0 || i+1==nPoints[k]) ? 1.0 : (i%2==1 ? 4.0 : 2.0)); });
}

double integrationTrapezoidRule2D(const vector<double> & values, unsigned int nX, unsigned int nY, double stepX, double stepY)
{
  return integrationTrapezoidRuleND(values,{nX,nY},{stepX,stepY});
}

double integrationSimpsonRule2D(const vector<double> & values, unsigned int nX, unsigned int nY, double stepX, double stepY)
{
  return integrationSimpsonRuleND(values,{nX,nY},{stepX,stepY});
}

} // namespace MATH

} // namespace CAP

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "FlowCumulants.hpp"
#include <cmath>
#include <functional>
#include "Exceptions.hpp"

namespace CAP
{

  namespace
  {
  //! Value and derivative (forward differentiation).
  struct Dual
  {
  double v, d;
  Dual(double value=0.0, double derivative=0.0) : v(value), d(derivative) { }
  Dual operator+(const Dual & o) const { return Dual(v+o.v,d+o.d); }
  Dual operator-(const Dual & o) const { return Dual(v-o.v,d-o.d); }
  Dual operator*(const Dual & o) const { return Dual(v*o.v,v*o.d+d*o.v); }
  Dual operator*(double a) const { return Dual(a*v,a*d); }
  };

  double factorial(unsigned int k) { double f = 1.0; for (unsigned int j=2; j<=k; j++) f *= double(j); return f; }

  //! b_1..b_K of ln(1 + sum a_k z^k) (index 0 unused).
  std::vector<Dual> logSeries(const std::vector<Dual> & a)
  {
  std::vector<Dual> b(a.size());
  for (unsigned int k=1; k<a.size(); k++)
    {
    Dual s;
    for (unsigned int j=1; j<k; j++) s = s + b[j]*a[k-j]*double(j);
    b[k] = a[k] - s*(1.0/double(k));
    }
  return b;
  }

  //! Linear uncertainty of f(x) from those of x (central differences, steps a fraction of the uncertainties).
  double propagate(const std::function<double(const std::vector<double> &)> & f, std::vector<double> x,
                   const std::vector<double> & ex)
  {
  double sum = 0.0;
  for (unsigned int j=0; j<x.size(); j++)
    {
    if (!(ex[j]>0.0)) continue;
    double h = 1.0E-3*ex[j];
    double x0 = x[j];
    x[j] = x0+h; double fp = f(x);
    x[j] = x0-h; double fm = f(x);
    x[j] = x0;
    double g = (fp-fm)/(2.0*h);
    if (std::isfinite(g)) sum += g*g*ex[j]*ex[j];
    }
  return std::sqrt(sum);
  }
  } // namespace

  double FlowCumulants::cumulant(const std::vector<double> & m, unsigned int k)
  {
  if (k<1 || k>m.size()) throw Exception("Order out of range","FlowCumulants::cumulant()");
  std::vector<Dual> a(k+1);
  for (unsigned int j=1; j<=k; j++) a[j] = Dual(m[j-1]/(factorial(j)*factorial(j)));
  return logSeries(a)[k].v*factorial(k)*factorial(k);
  }

  std::vector<double> FlowCumulants::cumulants(const std::vector<double> & m)
  {
  std::vector<Dual> a(m.size()+1);
  for (unsigned int j=1; j<=m.size(); j++) a[j] = Dual(m[j-1]/(factorial(j)*factorial(j)));
  std::vector<Dual> b = logSeries(a);
  std::vector<double> c(m.size());
  for (unsigned int k=1; k<=m.size(); k++) c[k-1] = b[k].v*factorial(k)*factorial(k);
  return c;
  }

  double FlowCumulants::differentialCumulant(const std::vector<double> & m, const std::vector<double> & mp, unsigned int k)
  {
  if (k<1 || k>m.size() || k>mp.size()) throw Exception("Order out of range","FlowCumulants::differentialCumulant()");
  std::vector<Dual> a(k+1);
  for (unsigned int j=1; j<=k; j++)
    {
    double f = factorial(j)*factorial(j);
    a[j] = Dual(m[j-1]/f, (double(j)/double(k))*mp[j-1]/f);
    }
  return logSeries(a)[k].d*factorial(k)*factorial(k);
  }

  double FlowCumulants::coefficient(unsigned int k)
  {
  return cumulant(std::vector<double>(k,1.0),k);
  }

  double FlowCumulants::flow(double c, unsigned int k, bool & defined)
  {
  double r = c/coefficient(k);
  defined = std::isfinite(r) && r>0.0;
  return defined ? std::pow(r,1.0/(2.0*k)) : 0.0;
  }

  double FlowCumulants::differentialFlow(double d, double v, unsigned int k)
  {
  return d/(coefficient(k)*std::pow(v,2.0*k-1.0));
  }

  bool FlowCumulants::flowWithError(const std::vector<double> & m, const std::vector<double> & em, unsigned int k,
                                    double & c, double & ec, double & v, double & ev)
  {
  std::vector<double> mk(m.begin(),m.begin()+k), emk(em.begin(),em.begin()+k);
  c  = cumulant(mk,k);
  ec = propagate([k](const std::vector<double> & x) { return cumulant(x,k); },mk,emk);
  bool defined;
  v  = flow(c,k,defined);
  ev = defined ? v*ec/(2.0*k*std::fabs(c)) : 0.0;
  return defined;
  }

  bool FlowCumulants::differentialFlowWithError(const std::vector<double> & m, const std::vector<double> & em,
                                                const std::vector<double> & mp, const std::vector<double> & emp,
                                                unsigned int k, double & d, double & ed, double & vp, double & evp)
  {
  std::vector<double> x(m.begin(),m.begin()+k), ex(em.begin(),em.begin()+k);
  x.insert(x.end(),mp.begin(),mp.begin()+k);
  ex.insert(ex.end(),emp.begin(),emp.begin()+k);
  auto split = [k](const std::vector<double> & y, std::vector<double> & a, std::vector<double> & b)
    { a.assign(y.begin(),y.begin()+k); b.assign(y.begin()+k,y.end()); };
  auto dOf = [k,split](const std::vector<double> & y)
    { std::vector<double> a, b; split(y,a,b); return differentialCumulant(a,b,k); };
  auto vpOf = [k,split](const std::vector<double> & y)
    {
    std::vector<double> a, b; split(y,a,b);
    bool ok; double v = flow(cumulant(a,k),k,ok);
    return ok && v>0.0 ? differentialFlow(differentialCumulant(a,b,k),v,k) : 0.0;
    };
  d  = dOf(x);
  ed = propagate(dOf,x,ex);
  bool defined;
  double v = flow(cumulant(std::vector<double>(m.begin(),m.begin()+k),k),k,defined);
  if (!defined || !(v>0.0)) { vp = 0.0; evp = 0.0; return false; }
  vp  = differentialFlow(d,v,k);
  evp = propagate(vpOf,x,ex);
  return true;
  }

} // namespace CAP

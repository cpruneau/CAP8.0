/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "FlowCorrelator.hpp"
#include <cmath>
#include <string>
#include "Exceptions.hpp"

namespace CAP
{

  FlowVector::FlowVector(unsigned int maxHarmonic, unsigned int maxPower)
  :
  _maxHarmonic(maxHarmonic),
  _maxPower(maxPower),
  _count(0),
  _q((maxHarmonic+1)*(maxPower+1))
  { }

  void FlowVector::resize(unsigned int maxHarmonic, unsigned int maxPower)
  {
  _maxHarmonic = maxHarmonic;
  _maxPower    = maxPower;
  _q.assign((maxHarmonic+1)*(maxPower+1),Complex(0.0,0.0));
  _count = 0;
  }

  void FlowVector::clear()
  {
  std::fill(_q.begin(),_q.end(),Complex(0.0,0.0));
  _count = 0;
  }

  void FlowVector::add(double phi, double weight)
  {
  const Complex z(std::cos(phi),std::sin(phi));
  Complex zh(1.0,0.0);
  const unsigned int np = _maxPower+1;
  for (unsigned int h=0; h<=_maxHarmonic; h++)
    {
    double wp = 1.0;
    Complex * row = &_q[h*np];
    for (unsigned int p=0; p<np; p++)
      {
      row[p] += wp*zh;
      wp *= weight;
      }
    zh *= z;
    }
  _count++;
  }

  FlowVector & FlowVector::operator+=(const FlowVector & other)
  {
  if (other._q.size()!=_q.size()) throw Exception("Flow vectors of different sizes","FlowVector::operator+=()");
  for (unsigned int k=0; k<_q.size(); k++) _q[k] += other._q[k];
  _count += other._count;
  return *this;
  }

  FlowVector::Complex FlowVector::q(int h, unsigned int p) const
  {
  unsigned int a = h<0 ? (unsigned int)(-h) : (unsigned int)(h);
  if (a>_maxHarmonic || p>_maxPower)
    throw Exception(String("Flow vector Q_{")+String(std::to_string(h).c_str())+","+String(std::to_string(p).c_str())
                    +"} beyond the highest harmonic "+String(std::to_string(_maxHarmonic).c_str())+" or power "
                    +String(std::to_string(_maxPower).c_str()),"FlowVector::q()");
  const Complex & v = _q[a*(_maxPower+1)+p];
  return h<0 ? std::conj(v) : v;
  }

  double FlowCorrelator::blockCoefficient(unsigned int b)
  {
  double c = 1.0;
  for (unsigned int k=2; k<b; k++) c *= double(k);     // (b-1)!
  return (b%2==1) ? c : -c;                            // (-1)^{b-1}
  }

  std::vector<int> FlowCorrelator::alternating(int n, unsigned int m)
  {
  std::vector<int> h(m);
  for (unsigned int k=0; k<m; k++) h[k] = (k%2==0) ? n : -n;
  return h;
  }

  void FlowCorrelator::subsetSums(const FlowVector & q, const std::vector<int> & h, std::vector<Complex> & n)
  {
  subsetSums([&q](int harmonic, unsigned int power) { return q.q(harmonic,power); },h,n);
  }

  void FlowCorrelator::subsetSums(const BlockVector & block, const std::vector<int> & h, std::vector<Complex> & n)
  {
  const unsigned int m = h.size();
  if (m>20) throw Exception("More than 20 slots","FlowCorrelator::subsetSums()");
  const unsigned int size = 1u<<m;
  std::vector<int> harmonic(size,0);
  std::vector<unsigned int> slots(size,0);
  for (unsigned int s=1; s<size; s++)
    {
    unsigned int low = s & (~s+1u);
    unsigned int k = 0;
    while ((1u<<k)!=low) k++;
    harmonic[s] = harmonic[s^low] + h[k];
    slots[s]    = slots[s^low] + 1;
    }
  std::vector<double> coefficient(m+1,0.0);
  for (unsigned int b=1; b<=m; b++) coefficient[b] = blockCoefficient(b);
  n.assign(size,Complex(0.0,0.0));
  n[0] = Complex(1.0,0.0);
  for (unsigned int s=1; s<size; s++)
    {
    unsigned int low  = s & (~s+1u);
    unsigned int rest = s ^ low;
    Complex sum(0.0,0.0);
    for (unsigned int t=rest; ; t=(t-1u)&rest)           // blocks low|t, t a subset of rest
      {
      unsigned int b = low | t;
      sum += coefficient[slots[b]]*block(harmonic[b],slots[b])*n[s^b];
      if (t==0) break;
      }
    n[s] = sum;
    }
  }

  FlowCorrelator::Complex FlowCorrelator::differential(const FlowVector & p, const FlowVector & q, int h0,
                                                       const std::vector<int> & h, unsigned int others,
                                                       const std::vector<Complex> & n)
  {
  Complex sum(0.0,0.0);
  for (unsigned int t=others; ; t=(t-1u)&others)        // the slots t join slot 0
    {
    int harmonic = h0;
    unsigned int b = 1;
    for (unsigned int k=0; k<h.size(); k++) if (t & (1u<<k)) { harmonic += h[k]; b++; }
    Complex x = (b==1) ? p.q(harmonic,1) : q.q(harmonic,b);
    sum += blockCoefficient(b)*x*n[others^t];
    if (t==0) break;
    }
  return sum;
  }

} // namespace CAP

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "Profile.hpp"
#include "BinaryIO.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

ClassImp(CAP::MATH::Profile);

namespace CAP
{
namespace MATH
{

  Profile::Profile()
  :
  Histogram(),
  _counts(), _sumW(), _sumW2(), _sumWY(), _sumWY2()
  {
  setClassName("Profile");
  }

  Profile::Profile(const Partition & partition)
  :
  Histogram(partition),
  _counts(), _sumW(), _sumW2(), _sumWY(), _sumWY2()
  {
  if (rank()!=1 && rank()!=2) throw Exception("A Profile must have rank 1 or 2",__FUNCTION__);
  setClassName("Profile");
  setPrecision(kDouble);
  resizeSums();
  }

  Profile::Profile(const String & name, const String & title, unsigned int n, double minimum, double maximum)
  :
  Histogram(name,title,n,minimum,maximum),
  _counts(), _sumW(), _sumW2(), _sumWY(), _sumWY2()
  {
  setClassName("Profile");
  setPrecision(kDouble);
  resizeSums();
  }

  Profile::Profile(const String & name, const String & title,
                   unsigned int n0, double minimum0, double maximum0,
                   unsigned int n1, double minimum1, double maximum1)
  :
  Histogram(name,title,n0,minimum0,maximum0,n1,minimum1,maximum1),
  _counts(), _sumW(), _sumW2(), _sumWY(), _sumWY2()
  {
  setClassName("Profile");
  setPrecision(kDouble);
  resizeSums();
  }

  Profile::Profile(const Profile & source)
  :
  Histogram(source),
  _counts(source._counts),
  _sumW(source._sumW),
  _sumW2(source._sumW2),
  _sumWY(source._sumWY),
  _sumWY2(source._sumWY2)
  { }

  Profile & Profile::operator=(const Profile & rhs)
  {
  if (this!=&rhs)
    {
    Histogram::operator=(rhs);
    _counts = rhs._counts;
    _sumW   = rhs._sumW;
    _sumW2  = rhs._sumW2;
    _sumWY  = rhs._sumWY;
    _sumWY2 = rhs._sumWY2;
    }
  return *this;
  }

  void Profile::resizeSums()
  {
  unsigned int n = nBinsTotal();
  _counts.assign(n,0);
  _sumW.assign(n,0.0);
  _sumW2.assign(n,0.0);
  _sumWY.assign(n,0.0);
  _sumWY2.assign(n,0.0);
  }

  // Mean = sum(w*y)/sum(w); uncertainty = standard error of the mean = spread/sqrt(effective entries),
  // with effective entries sum(w)^2/sum(w^2) (ROOT's default error option for profiles).
  void Profile::updateBin(unsigned int k)
  {
  double sw = _sumW[k];
  if (_counts[k]==0 || sw==0.0)
    {
    setEmpty(k);
    return;
    }
  double mean     = _sumWY[k]/sw;
  double variance = _sumWY2[k]/sw - mean*mean;
  if (variance<0.0) variance = 0.0;
  double nEff     = _sumW2[k]>0.0 ? sw*sw/_sumW2[k] : 0.0;
  double error    = nEff>0.0 ? std::sqrt(variance/nEff) : 0.0;
  setValue(k,1,mean,error);
  }

  void Profile::setSums(unsigned int k, long entries, double sumW, double sumW2, double sumWY, double sumWY2)
  {
  checkGlobal(k,__FUNCTION__);
  _counts[k] = entries;
  _sumW[k]   = sumW;
  _sumW2[k]  = sumW2;
  _sumWY[k]  = sumWY;
  _sumWY2[k] = sumWY2;
  updateBin(k);
  }

  void Profile::fill(double x, double value, double weight)
  {
  int k = binIndex(x);
  if (k<0) return;
  _counts[k]++;
  _sumW[k]   += weight;
  _sumW2[k]  += weight*weight;
  _sumWY[k]  += weight*value;
  _sumWY2[k] += weight*value*value;
  updateBin(k);
  }

  void Profile::fill(double x0, double x1, double value, double weight)
  {
  int k = binIndex(x0,x1);
  if (k<0) return;
  _counts[k]++;
  _sumW[k]   += weight;
  _sumW2[k]  += weight*weight;
  _sumWY[k]  += weight*value;
  _sumWY2[k] += weight*value*value;
  updateBin(k);
  }

  void Profile::reset()
  {
  Histogram::reset();
  resizeSums();
  }

  void Profile::clear()
  {
  Histogram::clear();
  resizeSums();
  }

  void Profile::scale(double coefficient)
  {
  for (unsigned int k=0; k<_sumWY.size(); k++)
    {
    _sumWY[k]  *= coefficient;
    _sumWY2[k] *= coefficient*coefficient;
    updateBin(k);
    }
  }

  const Profile & Profile::checkProfile(const Histogram & h, const char * source) const
  {
  const Profile * p = dynamic_cast<const Profile *>(&h);
  if (!p) throw Exception(String("Histogram ")+h.name()+" is not a Profile",source);
  if (!compatibleWith(*p)) throw Exception(String("Profile ")+h.name()+" has a different binning",source);
  return *p;
  }

  void Profile::add(const Histogram & h, double coefficient)
  {
  const Profile & p = checkProfile(h,__FUNCTION__);
  for (unsigned int k=0; k<_sumW.size(); k++)
    {
    // a negative coefficient removes the entries of p
    _counts[k] += coefficient<0.0 ? -p._counts[k] : p._counts[k];
    if (_counts[k]<0) _counts[k] = 0;
    _sumW[k]   += coefficient*p._sumW[k];
    _sumW2[k]  += coefficient*std::fabs(coefficient)*p._sumW2[k];
    _sumWY[k]  += coefficient*p._sumWY[k];
    _sumWY2[k] += coefficient*p._sumWY2[k];
    updateBin(k);
    }
  }

  void Profile::setAsSumOf(const Histogram & h1, const Histogram & h2, double coefficient1, double coefficient2)
  {
  setAsSumOf(std::vector<const Histogram *>{&h1,&h2},std::vector<double>{coefficient1,coefficient2});
  }

  void Profile::setAsSumOf(const std::vector<const Histogram *> & histograms, const std::vector<double> & coefficients)
  {
  if (histograms.empty()) throw Exception("No profiles given",__FUNCTION__);
  if (histograms.size()!=coefficients.size()) throw Exception("histograms and coefficients sizes differ",__FUNCTION__);
  for (auto h : histograms)
    {
    if (!h) throw NullPointerException("histograms",__FUNCTION__);
    checkProfile(*h,__FUNCTION__);
    }
  Profile result(*static_cast<const Profile *>(histograms[0]));
  result.reset();
  for (unsigned int i=0; i<histograms.size(); i++) result.add(*histograms[i],coefficients[i]);
  String keepName = name(), keepTitle = title();
  *this = result;
  setName(keepName);
  setTitle(keepTitle);
  }

  void Profile::print() const
  {
  std::cout << "Profile ";
  Histogram::print();
  }

  // ---------------------------------------------------------------- binary I/O

  namespace
  {
  void writeVector(BinaryWriter & w, const std::vector<double> & v, const std::vector<long> & counts)
  {
  for (std::size_t k=0; k<v.size(); k++) if (counts[k]!=0) w.writeFloat64(v[k]);
  }
  }

  void Profile::encodeBinary(BinaryWriter & w) const
  {
  Histogram::encodeBinary(w);
  // sums of the non-empty bins (bins with at least one entry), in increasing global index
  uint64_t nNonEmpty = 0;
  for (long c : _counts) if (c!=0) nNonEmpty++;
  w.writeVarUInt(nNonEmpty);
  std::size_t next = 0;
  for (std::size_t k=0; k<_counts.size(); k++)
    {
    if (_counts[k]==0) continue;
    w.writeVarUInt(k-next);
    w.writeVarInt(_counts[k]);
    next = k+1;
    }
  writeVector(w,_sumW,_counts);
  writeVector(w,_sumW2,_counts);
  writeVector(w,_sumWY,_counts);
  writeVector(w,_sumWY2,_counts);
  }

  void Profile::decodeBinary(BinaryReader & r, const String & profileName, unsigned int version)
  {
  // the histogram part: format version 3 of Histogram in version 1, version 4 (with the fits) since version 2
  Histogram::decodeBinary(r,profileName,version>=2 ? 4 : 3);
  if (rank()!=1 && rank()!=2) throw Exception("A Profile must have rank 1 or 2","Profile::decodeBinary()");
  setClassName("Profile");
  setPrecision(kDouble);
  resizeSums();
  uint64_t n = nBinsTotal();
  uint64_t nNonEmpty = r.readVarUInt();
  if (nNonEmpty > n) throw Exception("More non-empty bins than bins","Profile::decodeBinary()");
  if (8*4*nNonEmpty > r.remaining()) throw Exception("More sums than data","Profile::decodeBinary()");
  std::vector<std::size_t> indices;
  indices.reserve(nNonEmpty);
  uint64_t k = 0;
  for (uint64_t i=0; i<nNonEmpty; i++)
    {
    k += r.readVarUInt();
    if (k>=n) throw Exception("Bin index out of range","Profile::decodeBinary()");
    long count = long(r.readVarInt());
    if (count<=0) throw Exception("Invalid number of entries","Profile::decodeBinary()");
    _counts[k] = count;
    indices.push_back(k);
    k++;
    }
  for (std::size_t i : indices) _sumW[i]   = r.readFloat64();
  for (std::size_t i : indices) _sumW2[i]  = r.readFloat64();
  for (std::size_t i : indices) _sumWY[i]  = r.readFloat64();
  for (std::size_t i : indices) _sumWY2[i] = r.readFloat64();
  }

  Profile * safeClone(const Profile * p)
  {
  return p ? new Profile(*p) : nullptr;
  }

  Profile * loadProfile(const CapFile & inputFile, const String & name)
  {
  const Profile * p = inputFile.get<Profile>(name);
  if (!p) throw FileException(inputFile.path(),String("No profile named ")+name,__FUNCTION__);
  return new Profile(*p);
  }

  Profile operator+(const Profile & lhs, const Profile & rhs)
  {
  Profile result(lhs);
  result.add(rhs,1.0);
  return result;
  }

  Profile operator-(const Profile & lhs, const Profile & rhs)
  {
  Profile result(lhs);
  result.add(rhs,-1.0);
  return result;
  }

} // namespace MATH
} // namespace CAP

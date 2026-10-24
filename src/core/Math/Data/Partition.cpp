#include "Partition.hpp"
#include "Exceptions.hpp"



namespace CAP
{
  namespace MATH
  {

  Partition::Partition()
  :
  _rank(0),
  _type(0),
  _nBinsTotal(0),
  _nBins(),
  _minima(),
  _maxima(),
  _widths(),
  _titles()
  { }

  Partition::Partition(const Partition & src)
  :
  _rank(src._rank),
  _type(src._type),
  _nBinsTotal(src._nBinsTotal),
  _nBins(src._nBins),
  _minima(src._minima),
  _maxima(src._maxima),
  _widths(src._widths),
  _titles(src._titles),
  _binLabels(src._binLabels)
  { }

  Partition::~Partition()
  {
  clear();
  }

  Partition & Partition::operator=(const Partition & rhs)
  {
  if (this!=&rhs)
    {
    _rank       = rhs._rank;
    _type       = rhs._type;
    _nBinsTotal = rhs._nBinsTotal;
    _nBins      = rhs._nBins;
    _minima     = rhs._minima;
    _maxima     = rhs._maxima;
    _widths     = rhs._widths;
    _titles     = rhs._titles;
    _binLabels  = rhs._binLabels;
    }
  return *this;
  }

  void Partition::checkDimIndex(unsigned int dimIndex, const char * source) const
  {
  if (dimIndex>=_rank)
    throw IndexOutOfBoundException(dimIndex,_rank,"Dimension index out of bound",source);
  }

  void Partition::addDimension(const Partition & partition)
  {
  if (&partition==this) throw Exception("Cannot add a partition to itself",__FUNCTION__);
  unsigned int addedRank = partition.rank();
  for (unsigned int k=0; k<addedRank; k++)
    {
    addDimension(partition._nBins[k],partition._minima[k],partition._maxima[k],partition._titles[k]);
    }
  }

  void Partition::addDimension(unsigned int nBin, float minimum, float maximum, String title)
  {
  if (nBin<1) throw Exception("Invalid dimension size nBins<1",__FUNCTION__);
  if (minimum>=maximum) throw Exception("Invalid dimension range minimum>=maximum",__FUNCTION__);
  _nBinsTotal = (_rank==0) ? nBin : _nBinsTotal*nBin;
  _nBins.push_back(nBin);
  _minima.push_back(minimum);
  _maxima.push_back(maximum);
  _widths.push_back((maximum-minimum)/double(nBin));
  _titles.push_back(title);
  _binLabels.push_back(std::map<unsigned int,String>());
  _rank++;
  }

  bool Partition::compatibleWith(const Partition & rhs, bool sizeAndLimits) const
  {
  if (_rank!=rhs._rank) return false;
  for (unsigned int k=0; k<_rank; k++)
    {
    if (_nBins[k]!=rhs._nBins[k])  return false;
    if (sizeAndLimits)
      {
      if (_minima[k]!=rhs._minima[k])  return false;
      if (_maxima[k]!=rhs._maxima[k])  return false;
      }
    }
  return true;
  }

  void Partition::clear()
  {
  _rank = 0;
  _type = 0;
  _nBinsTotal = 0;
  _nBins.clear();
  _minima.clear();
  _maxima.clear();
  _widths.clear();
  _titles.clear();
  _binLabels.clear();
  }

  void Partition::createPartition(const Partition & partition1,const Partition & partition2)
  {
  // copy first in case partition1 or partition2 is *this
  Partition p1(partition1);
  Partition p2(partition2);
  clear();
  addDimension(p1);
  addDimension(p2);
  }

  unsigned int Partition::nBins(unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _nBins[dimIndex];
  }

  unsigned int Partition::nBinsTotal() const
  {
  return _nBinsTotal;
  }

  int Partition::binIndex(unsigned int index, bool isStrict) const
  {
  if (_rank!=1) throw Exception("Partition has rank!=1",__FUNCTION__);
  if (index >= _nBins[0])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index,_nBins[0],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  return index;
  }

  int Partition::binIndex(unsigned int index0,unsigned int index1, bool isStrict) const
  {
  if (_rank!=2) throw Exception("Partition has rank!=2",__FUNCTION__);
  if (index0 >= _nBins[0])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index0,_nBins[0],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  if (index1 >= _nBins[1])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index1,_nBins[1],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  return index1*_nBins[0] + index0;
  }

  int Partition::binIndex(unsigned int index0,unsigned int index1,unsigned int index2, bool isStrict) const
  {
  if (_rank!=3) throw Exception("Partition has rank!=3",__FUNCTION__);
  if (index0 >= _nBins[0])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index0,_nBins[0],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  if (index1 >= _nBins[1])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index1,_nBins[1],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  if (index2 >= _nBins[2])
    {
    if (isStrict)
      throw IndexOutOfBoundException(index2,_nBins[2],"Out of bound index",__FUNCTION__);
    else
      return -1;
    }
  return (index2*_nBins[1] + index1)*_nBins[0] + index0;
  }

  int Partition::binIndex(const std::vector<unsigned int> & indices, bool isStrict) const
  {
  if (indices.size()!=_rank) throw Exception("Number of indices does not match rank",__FUNCTION__);
  unsigned int index = 0;
  unsigned int mult  = 1;
  for (unsigned int k=0; k<_rank; k++)
    {
    if (indices[k] >= _nBins[k])
      {
      if (isStrict)
        throw IndexOutOfBoundException(indices[k],_nBins[k],"Out of bound index",__FUNCTION__);
      else
        return -1;
      }
    index += mult*indices[k];
    mult  *= _nBins[k];
    }
  return index;
  }

  int Partition::axisBinIndex(double x, unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  if (x<_minima[dimIndex] || x>=_maxima[dimIndex]) return -1;
  unsigned int index = (unsigned int) ((x-_minima[dimIndex])/_widths[dimIndex]);
  if (index>=_nBins[dimIndex]) index = _nBins[dimIndex]-1; // guard float rounding at the upper edge
  return index;
  }

  int Partition::binIndex(double x) const
  {
  if (_rank!=1) throw Exception("Partition has rank!=1",__FUNCTION__);
  return axisBinIndex(x,0);
  }

  int Partition::binIndex(double x0, double x1) const
  {
  if (_rank!=2) throw Exception("Partition has rank!=2",__FUNCTION__);
  int index0 = axisBinIndex(x0,0);
  int index1 = axisBinIndex(x1,1);
  if (index0<0 || index1<0) return -1;
  return binIndex((unsigned int)index0,(unsigned int)index1);
  }

  int Partition::binIndex(double x0, double x1, double x2) const
  {
  if (_rank!=3) throw Exception("Partition has rank!=3",__FUNCTION__);
  int index0 = axisBinIndex(x0,0);
  int index1 = axisBinIndex(x1,1);
  int index2 = axisBinIndex(x2,2);
  if (index0<0 || index1<0 || index2<0) return -1;
  return binIndex((unsigned int)index0,(unsigned int)index1,(unsigned int)index2);
  }

  int Partition::binIndex(const std::vector<double> & x) const
  {
  if (x.size()!=_rank) throw Exception("Rank mismatch for given x",__FUNCTION__);
  std::vector<unsigned int> indices;
  for (unsigned int k=0; k<_rank; k++)
    {
    int index = axisBinIndex(x[k],k);
    if (index<0) return -1;
    indices.push_back(index);
    }
  return binIndex(indices);
  }

  double Partition::minimum(unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _minima[dimIndex];
  }

  double Partition::maximum(unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _maxima[dimIndex];
  }

  // return the bin width -- for now use fixed widths...
  double Partition::binWidth(unsigned int binIndex __attribute__((unused)), unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _widths[dimIndex];
  }

  double Partition::binLowEdge(unsigned int binIndex, unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _minima[dimIndex] + double(binIndex)*_widths[dimIndex];
  }

  double Partition::binHighEdge(unsigned int binIndex, unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _minima[dimIndex] + double(binIndex+1)*_widths[dimIndex];
  }

  double Partition::binCenter(unsigned int binIndex, unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _minima[dimIndex] + (0.5+double(binIndex))*_widths[dimIndex];
  }

  Partition Partition::extractPartition(unsigned int k)  const
  {
  checkDimIndex(k,__FUNCTION__);
  Partition newPartition;
  newPartition.addDimension(_nBins[k], _minima[k], _maxima[k], _titles[k]);
  return newPartition;
  }

  Partition Partition::extractPartition(unsigned int k0, unsigned int k1) const
  {
  checkDimIndex(k0,__FUNCTION__);
  checkDimIndex(k1,__FUNCTION__);
  if (k0==k1)  throw Exception("Dimension k0 and k1 are the same",__FUNCTION__);
  Partition newPartition;
  newPartition.addDimension(_nBins[k0], _minima[k0], _maxima[k0], _titles[k0]);
  newPartition.addDimension(_nBins[k1], _minima[k1], _maxima[k1], _titles[k1]);
  return newPartition;
  }

  Partition Partition::extractPartition(unsigned int k0, unsigned int k1, unsigned int k2) const
  {
  checkDimIndex(k0,__FUNCTION__);
  checkDimIndex(k1,__FUNCTION__);
  checkDimIndex(k2,__FUNCTION__);
  if (k0==k1)  throw Exception("Dimension k0, k1 are the same",__FUNCTION__);
  if (k0==k2)  throw Exception("Dimension k0, k2 are the same",__FUNCTION__);
  if (k1==k2)  throw Exception("Dimension k1, k2 are the same",__FUNCTION__);
  Partition newPartition;
  newPartition.addDimension(_nBins[k0], _minima[k0], _maxima[k0], _titles[k0]);
  newPartition.addDimension(_nBins[k1], _minima[k1], _maxima[k1], _titles[k1]);
  newPartition.addDimension(_nBins[k2], _minima[k2], _maxima[k2], _titles[k2]);
  return newPartition;
  }
  void Partition::setDimensionTitle(unsigned int k, const String & title)
  {
  checkDimIndex(k,__FUNCTION__);
  _titles[k] = title;
  }


  const String & Partition::dimensionTitle(unsigned int k) const
  {
  checkDimIndex(k,__FUNCTION__);
  return _titles[k];
  }

  
  void Partition::setBinLabel(unsigned int binIndex, const String & label, unsigned int dimIndex)
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  if (binIndex>=_nBins[dimIndex]) throw IndexOutOfBoundException(binIndex,_nBins[dimIndex],"Invalid bin index",__FUNCTION__);
  if (label.length()==0) _binLabels[dimIndex].erase(binIndex);
  else                   _binLabels[dimIndex][binIndex] = label;
  }

  String Partition::binLabel(unsigned int binIndex, unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  auto it = _binLabels[dimIndex].find(binIndex);
  return it==_binLabels[dimIndex].end() ? String("") : it->second;
  }

  const std::map<unsigned int,String> & Partition::binLabels(unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  return _binLabels[dimIndex];
  }

} // namespace MATH
} // namespace CAP

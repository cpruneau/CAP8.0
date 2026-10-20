#include "DataPoints.hpp"
#include "MathExceptions.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include <iostream>

ClassImp(CAP::MATH::DataPoints);

namespace CAP
{
  namespace MATH
  {

  DataPoints::DataPoints()
  :
  Object("DataPoints","DataPoints","DataPoints")
  { }

  DataPoints::DataPoints(const String & name, const String & title, const String & className)
  :
  Object(name,title,className)
  { }

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

  double DataPoints::extremum(unsigned int index, bool findMaximum, const char * source) const
  {
  unsigned int n = size();
  if (n==0) throw MathException("No points in container",source);
  unsigned int r = rank();
  if (index>r) throw MathException("Independent variable index>=rank()",source);
  bool useValue = (index==r);
  double best = 0.0;
  for (unsigned int k=0; k<n; k++)
    {
    double v;
    if (useValue)
      v = valueAt(k);
    else
      {
      std::vector<double> coordinates = coordinatesAt(k);
      if (index>=coordinates.size()) throw MathException("Point has fewer coordinates than rank()",source);
      v = coordinates[index];
      }
    if (k==0 || (findMaximum ? (v>best) : (v<best))) best = v;
    }
  return best;
  }

  double DataPoints::minimum(unsigned int index) const
  {
  if (index>=rank()) throw MathException("Independent variable index>=rank()",__FUNCTION__);
  return extremum(index,false,__FUNCTION__);
  }

  double DataPoints::maximum(unsigned int index) const
  {
  if (index>=rank()) throw MathException("Independent variable index>=rank()",__FUNCTION__);
  return extremum(index,true,__FUNCTION__);
  }

  double DataPoints::minimumValue() const { return extremum(rank(),false,__FUNCTION__); }
  double DataPoints::maximumValue() const { return extremum(rank(),true,__FUNCTION__);  }

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


  namespace
  {
  String numberText(double v)
  {
  char text[32];
  std::snprintf(text, sizeof(text), "%.6g", v);
  return String(text);
  }
  } // namespace

  String DataPoints::description() const
  {
  char text[128];
  const Histogram * histogram = dynamic_cast<const Histogram *>(this);
  if (histogram)
    std::snprintf(text, sizeof(text), "%s (%uD), %u bins", dynamic_cast<const Profile *>(this) ? "Profile" : "Histogram", rank(), size());
  else
    std::snprintf(text, sizeof(text), "Data points (%uD), %u points", rank(), size());
  return String(text);
  }

  std::vector<String> DataPoints::columnNames() const
  {
  static const char * axes[] = { "x", "y", "z" };
  const Histogram * histogram = dynamic_cast<const Histogram *>(this);
  unsigned int n = rank();
  std::vector<String> names{ "#" };
  for (unsigned int k=0; k<n; k++)
    {
    std::string name;
    if (histogram && !histogram->dimensionTitle(k).isNull()) name = histogram->dimensionTitle(k).data();
    else if (n <= 3) name = axes[k];
    else name = "x" + std::to_string(k);
    names.push_back(String(name.c_str()));
    names.push_back(String(("\xC2\xB1 " + name).c_str()));
    }
  if (histogram && !histogram->title().isNull()) names.push_back(histogram->title());
  else names.push_back(n == 1 ? "y" : "value");
  names.push_back("\xC2\xB1 stat.");
  names.push_back("\xC2\xB1 syst.");
  return names;
  }

  std::vector<bool> DataPoints::columnsShown() const
  {
  unsigned int n = rank();
  std::vector<bool> nonZero(n + 2, false);
  for (unsigned int k=0; k<size(); k++)
    {
    std::vector<double> ex = coordinateErrorsAt(k);
    for (unsigned int i=0; i<n && i<ex.size(); i++) if (ex[i] != 0.0) nonZero[i] = true;
    if (statErrorAt(k) != 0.0) nonZero[n]   = true;
    if (sysErrorAt(k)  != 0.0) nonZero[n+1] = true;
    }
  std::vector<bool> shown{ true };                                  // index
  for (unsigned int i=0; i<n; i++) { shown.push_back(true); shown.push_back(nonZero[i]); }
  shown.push_back(true);                                            // value
  shown.push_back(nonZero[n]);
  shown.push_back(nonZero[n+1]);
  return shown;
  }

  std::vector<String> DataPoints::rowValues(unsigned int k) const
  {
  std::vector<String> cells;
  std::vector<double> x = coordinatesAt(k), ex = coordinateErrorsAt(k);
  for (std::size_t i=0; i<x.size(); i++)
    {
    cells.push_back(numberText(x[i]));
    cells.push_back(numberText(i < ex.size() ? ex[i] : 0.0));
    }
  cells.push_back(numberText(valueAt(k)));
  cells.push_back(numberText(statErrorAt(k)));
  cells.push_back(numberText(sysErrorAt(k)));
  return cells;
  }

  std::vector<String> DataPoints::fitColumnNames(bool pulls) const
  {
  std::vector<String> names;
  for (unsigned int f=0; f<nFits(); f++)
    {
    std::string label = fit(f).label.isNull() ? "fit " + std::to_string(f + 1) : fit(f).label.data();
    names.push_back(String(("f(x) " + label).c_str()));
    if (pulls) names.push_back(String(("pull " + label).c_str()));
    }
  return names;
  }

  double DataPoints::pullAt(unsigned int k, const FitRecord & fit) const
  {
  double value = fit.valueAt(coordinatesAt(k)), error = statErrorAt(k);
  if (!std::isfinite(value) || !(error > 0)) return std::numeric_limits<double>::quiet_NaN();
  return (valueAt(k) - value)/error;
  }

  std::vector<String> DataPoints::fitValues(unsigned int k, bool pulls, std::vector<bool> * outside) const
  {
  std::vector<String> cells;
  std::vector<double> x = coordinatesAt(k);
  for (unsigned int f=0; f<nFits(); f++)
    {
    const FitRecord & record = fit(f);
    double value = record.valueAt(x);
    bool out = !record.inRange(x);
    cells.push_back(std::isfinite(value) ? numberText(value) : String());
    if (outside) outside->push_back(out);
    if (!pulls) continue;
    double pull = pullAt(k, record);
    cells.push_back(std::isfinite(pull) ? numberText(pull) : String());
    if (outside) outside->push_back(out);
    }
  return cells;
  }

  String DataPoints::table(bool fits, bool pulls) const
  {
  std::string text;
  std::vector<String> names = columnNames();
  std::vector<bool> shown = columnsShown();
  bool first = true;
  for (std::size_t c=0; c<names.size(); c++)
    if (shown[c]) { text += std::string(first ? "" : "\t") + names[c].data(); first = false; }
  if (fits) for (const auto & name : fitColumnNames(pulls)) text += std::string("\t") + name.data();
  text += "\n";
  for (unsigned int k=0; k<size(); k++)
    {
    text += std::to_string(k);
    std::vector<String> cells = rowValues(k);
    for (std::size_t c=0; c<cells.size(); c++) if (shown[c+1]) text += std::string("\t") + cells[c].data();
    if (fits) for (const auto & cell : fitValues(k, pulls)) text += std::string("\t") + cell.data();
    text += "\n";
    }
  return String(text.c_str());
  }

  } // namespace MATH
} // namespace CAP

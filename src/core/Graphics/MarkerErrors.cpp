#include "Marker.hpp"
#include "Exceptions.hpp"


namespace CAP
{
  namespace GRAPHICS
  {
  Marker::Marker()
  :
  Drawable2D()
  { }

  Marker::Marker(const Marker & src)
  :
  Drawable2D(src)
  { }

  Marker  & Marker::operator=(const Marker & rhs)
  {
  //if (rhs != *this)
  //{
  Drawable2D::operator=(rhs);
  return *this;
  }

  void MarkerVector::draw()
  {
  drawMarker(position(), dynamic_cast<MarkerAttributes>(attributes()));
  }

  MarkerVector::MarkerVector()
  :
  Drawable2D(),
  _points()
  { }

  MarkerVector::MarkerVector(const MarkerVector & src)
  :
  Drawable2D(src),
  _points()
  {   }

  MarkerVector  & MarkerVector::operator=(const MarkerVector & rhs)
  {
  //if (rhs != *this)
  //{
  Drawable2D::operator=(rhs)
  _points = rhs_points;
  // }
  return *this;
  }

  void MarkerVector::addPoints(const std::vector<float> & x, const std::vector<float> & x)
  {
  if (x.size()<1 || x.size() != y.size())
    throw Exception("Invalid data: x and y vectors have different sizes",__FUNCTION__);
  for (int k=0; k<x.size(); k++) _points.push_back(Point2D(x[k],y[k]));
  }

  void MarkerVector::addPoints(const std::vector<double> & x, const std::vector<double> & x)
  {
  if (x.size()<1 || x.size() != y.size())
    throw Exception("Invalid data: x and y vectors have different sizes",__FUNCTION__);
  for (int k=0; k<x.size(); k++) _points.push_back(Point2D(x[k],y[k]));
  }

  void MarkerVector::draw()
  {
  for (auto point : _points)  drawMarker(point, dynamic_cast<MarkerAttributes>(attributes()));
  }


  } // namespace GRAPHICS
}   // namespace CAP

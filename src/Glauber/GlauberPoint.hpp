#ifndef CAP__GlauberPoint
#define CAP__GlauberPoint
#include <cmath>
#include "TObject.h"
#include <cmath>

namespace CAP
{
using namespace std;


//!
//! @brief Point of the transverse plane (x, y), e.g. the position of a nucleon-nucleon interaction.
//!
class GlauberPoint
{
public:
  //! @brief Default constructor.
  GlauberPoint();
  //! @brief Constructor. @param x x @param y y
  GlauberPoint(float x, float y);
  //! @brief Copy constructor. @param GlauberPoint object to copy
  GlauberPoint(const GlauberPoint & GlauberPoint);
  //! @brief Copy assignment. @param GlauberPoint object to copy @return this object
  GlauberPoint operator=(const GlauberPoint & GlauberPoint);
  //! @brief Destructor.
  virtual ~GlauberPoint(){}
  //! @brief x coordinate. @return x
  float x() const  { return _x; }
  //! @brief y coordinate. @return y
  float y() const  { return _y; }
  //! @brief Distance to the origin. @return r
  float r() const  { return std::sqrt(_x*_x+_y*_y); }
  //! @brief Squared distance to the origin. @return r^2
  float r2() const { return _x*_x+_y*_y; }
  //! @brief Set the coordinates. @param x x @param y y
  void set(float x, float y)
  {
  _x = x;
  _y = y;
  }
  //! @brief Squared distance to another point. @return squared distance
  float  distanceSquare(const GlauberPoint & GlauberPoint)  const;
  //! @brief Distance to another point. @return distance
  float  distance(const GlauberPoint & GlauberPoint)  const;
  //! @brief Whether another point has the same coordinates. @return true if so
  bool   equalTo(const GlauberPoint & GlauberPoint)  const;
  //! @brief Whether another point is closer than a distance. @return true if so
  bool   closeTo(const GlauberPoint & GlauberPoint, float maxDistance)  const;
  //! @brief Rotate the point about the origin. @param phi angle
  void   rotateXY(float phi);
    //!
    //! determine the azimuthal angle of the point relative to the x axis.
    //!
  float  phi() const { return std::atan2(_y,_x);}
  
protected:

  //! x coordinate.
  float _x;
  //! y coordinate.
  float _y;

  ClassDef(GlauberPoint,1) // GlauberPoint class

};

} // namespace CAP

#endif




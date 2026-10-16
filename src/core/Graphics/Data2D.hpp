#ifndef CAP__Data2D
#define CAP__Data2D
#include "TObject.h"
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "DataPoint2D.hpp"   // CAP::MATH::DataPoint2D (Math)
#include "DataPoints.hpp"  // CAP::MATH::DataPoints (Math), abstract base of Data2D

namespace CAP
{
  namespace GRAPHICS
  {

  //! Graphics uses the data point class of the Math package.
  using DataPoint2D = CAP::MATH::DataPoint2D;

  //!
  //! Abstract interface for a data (x, dx, y, dy) container
  //! It is designed to give access to either column style data e.g., vectors x, dx, y, and dy, or
  //! point based data points (Data2DPoint)
  //!
  class Data2D : public CAP::MATH::DataPoints
  {
  public:

  enum DataType { kColumn, kRow};

  Data2D();
  Data2D(DataType type);
  Data2D(const Data2D & src);
  Data2D  & operator=(const Data2D & rhs);
  virtual ~Data2D() { }

  //! Name and title: Object::setName(), name(), setTitle(), title().
  void setType(DataType type)      { _type = type; }
  void setXName(const String & s)  { _xName = s; }
  void setYName(const String & s)  { _yName = s; }
  void setXTitle(const String & s) { _xTitle = s; }
  void setYTitle(const String & s) { _yTitle = s; }
  void setLabels(const std::vector<String> & labels) { _labels = labels; }
  void addLabel(const String & s)  { _labels.push_back(s); }
  void setLabel(unsigned int index, const String & s);
  void clearLabels()               { _labels.clear(); }

  DataType type() const  { return _type; }
  const String & xName()  const  { return _xName; }
  const String & yName()  const  { return _yName; }
  const String & xTitle() const  { return _xTitle; }
  const String & yTitle() const  { return _yTitle; }
  unsigned int nLabels() const   { return _labels.size(); }
  const String & label(unsigned int index) const;
  std::vector<String> & labels()  { return _labels; }
  const std::vector<String> & labels() const { return _labels; }

  //!
  //! Data2D is abstract: concrete classes (Data2DColumns, Data2DPoints) implement the
  //! interface of CAP::MATH::DataPoints (size(), point(), addPoint(), extrema, ...).
  //!

  //! Append points from vectors of values and uncertainties (single precision).
  virtual void addPoints(std::vector<float> & x,
                 std::vector<float> & xError,
                 std::vector<float> & y,
                 std::vector<float> & yStatError,
                 std::vector<float> & ySystError);
  using CAP::MATH::DataPoints::addPoints; // keep the DataPoint2D and double-vector versions visible

  protected:

  //! @brief Write the description (type, x and y names, title, x and y titles, labels) to a binary payload; the name is
  //! written by the record itself. Used by the binary files of the derived classes. @param w writer
  void encodeBinaryDescription(CAP::BinaryWriter & w) const;
  //! @brief Read the description written by encodeBinaryDescription(). @param r reader @param name stored name
  //! @throws Exception if the data is invalid
  void decodeBinaryDescription(CAP::BinaryReader & r, const String & name);

  DataType _type;  //! type of data held by this object (colum like "kColumn" or row like "kRow")
  String  _xName;  //! Name of the x-axis
  String  _yName;  //! Name of the y-axis
  String  _xTitle; //! Title of the x-axis (to be shown in graphs)
  String  _yTitle; //! Title of the y-axis (to be shown in graphs)
  std::vector<String> _labels; //! Labels for this object, to be shown in multi-graphs

  ClassDef(Data2D,0)
  };

  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Data2D

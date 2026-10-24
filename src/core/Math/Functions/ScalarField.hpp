#ifndef CAP__ScalarField
#define CAP__ScalarField
#include <vector>
#include "String.hpp"
#include "Object.hpp"
#include "Function.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Scalar field of rank n: a function of n real variables (see Function), either constant or defined on a mesh.
  //!
  //! <b>Constant field.</b> The field has the same value everywhere (ScalarField(name, value, rank), setValue(double)).
  //!
  //! <b>Mesh field.</b> The field is defined by its values at the nodes of a regular mesh of any rank n:
  //! along axis k, nPoints(k) nodes are equally spaced from minimum(k) to maximum(k), both included
  //! (a single node, at minimum(k), if nPoints(k) is 1). The mesh is set with setMesh(minima, maxima, nPoints),
  //! and its boundaries can be changed with setMinima(), setMaxima(), setMinimum() and setMaximum().
  //! Node values are accessed with operator()(indices) and valueAt(indices), and stored with axis 0 varying fastest.
  //!
  //! <b>Evaluation.</b> evaluateAt() (or interpolate()) returns the constant value, or the multilinear
  //! interpolation of the node values (linear for rank 1, bilinear for rank 2, trilinear for rank 3, ...).
  //! Axes with a single node are constant along that axis. Outside the mesh, the field is extrapolated
  //! linearly from the nearest cell.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   ScalarField c("c", 2.5, 3);                                   // constant field of rank 3
  //!   ScalarField f("f", {0.0, 0.0}, {1.0, 2.0}, {11, 21});       // rank 2 mesh, 11 x 21 nodes
  //!   for (unsigned int i=0; i<11; i++)
  //!     for (unsigned int j=0; j<21; j++)
  //!       f(i,j) = f.coordinate(0,i) + f.coordinate(1,j);
  //!   double v = f.evaluateAt(0.55, 1.25);                         // 1.8 (interpolated)
  //! @endcode
  //!
  class ScalarField : public Function
  {
  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: constant field of value 0 and rank 1, named "Null".
  ScalarField();
  //!
  //! @brief Constant field.
  //! @param name name of the field @param value value of the field @param rank number of variables (>= 1)
  //! @throws MathException if rank is 0
  //!
  ScalarField(const String & name, double value, unsigned int rank=1);
  //!
  //! @brief Mesh field of any rank, with all node values set to initialValue.
  //! @param name name of the field
  //! @param minima lower boundary of each axis @param maxima upper boundary of each axis
  //! @param nPoints number of nodes along each axis (>= 1) @param initialValue value of all nodes
  //! @throws MathException if the vectors are empty or differ in size, or an nPoints is 0
  //! @throws InvalidBoundsException if maximum <= minimum on an axis with more than one node
  //!
  ScalarField(const String & name,
              const std::vector<double> & minima,
              const std::vector<double> & maxima,
              const std::vector<unsigned int> & nPoints,
              double initialValue=0.0);
  //!
  //! @brief Mesh field of rank 3 (kept for compatibility). Numbers of points below 1 are set to 1.
  //! @throws InvalidBoundsException if a maximum is <= the corresponding minimum
  //!
  ScalarField(const String & name,
              double xMin, double xMax, unsigned int xNPts,
              double yMin, double yMax, unsigned int yNPts,
              double zMin, double zMax, unsigned int zNPts,
              double initialValue=0.0);
  //! @brief Copy constructor. @param source field to copy
  ScalarField(const ScalarField & source);
  //! @brief Assignment operator: copies name, rank, mesh and values. @param rhs field to copy @return reference to this field
  ScalarField & operator=(const ScalarField & rhs);
  //! @brief Destructor.
  virtual ~ScalarField() {}

  //! @}

  //! @name Setting the field
  //! @{

  //! @brief Make this a constant field, keeping its rank (the mesh is removed). @param value value of the field
  void setValue(double value);
  //! @brief Copy the rank, mesh and values of source (the name is kept). @param source field to copy
  void setValue(const ScalarField & source);
  //!
  //! @brief Make this a mesh field of rank 1, with all node values set to initialValue. Numbers of points below 1 are set to 1.
  //! @param name name of the field
  //! @param xMin lower boundary @param xMax upper boundary @param xNPts number of nodes
  //! @param initialValue value of all nodes
  //! @throws InvalidBoundsException if xMax <= xMin
  //!
  void setValue(const String & name,
                double xMin, double xMax, unsigned int xNPts,
                double initialValue=0.0);
  //!
  //! @brief Make this a mesh field of rank 2, with all node values set to initialValue. Numbers of points below 1 are set to 1.
  //! @param name name of the field
  //! @param xMin lower boundary of x @param xMax upper boundary of x @param xNPts number of nodes along x
  //! @param yMin lower boundary of y @param yMax upper boundary of y @param yNPts number of nodes along y
  //! @param initialValue value of all nodes
  //! @throws InvalidBoundsException if a maximum is <= the corresponding minimum
  //!
  void setValue(const String & name,
                double xMin, double xMax, unsigned int xNPts,
                double yMin, double yMax, unsigned int yNPts,
                double initialValue=0.0);
  //!
  //! @brief Make this a mesh field of rank 3, with all node values set to initialValue. Numbers of points below 1 are set to 1.
  //! @param name name of the field
  //! @param xMin lower boundary of x @param xMax upper boundary of x @param xNPts number of nodes along x
  //! @param yMin lower boundary of y @param yMax upper boundary of y @param yNPts number of nodes along y
  //! @param zMin lower boundary of z @param zMax upper boundary of z @param zNPts number of nodes along z
  //! @param initialValue value of all nodes
  //! @throws InvalidBoundsException if a maximum is <= the corresponding minimum
  //!
  void setValue(const String & name,
                double xMin, double xMax, unsigned int xNPts,
                double yMin, double yMax, unsigned int yNPts,
                double zMin, double zMax, unsigned int zNPts,
                double initialValue=0.0);
  //!
  //! @brief Make this a mesh field; its rank becomes the size of the vectors, and all node values are set to initialValue.
  //! @param minima lower boundary of each axis @param maxima upper boundary of each axis
  //! @param nPoints number of nodes along each axis (>= 1) @param initialValue value of all nodes
  //! @throws MathException if the vectors are empty or differ in size, or an nPoints is 0
  //! @throws InvalidBoundsException if maximum <= minimum on an axis with more than one node
  //!
  void setMesh(const std::vector<double> & minima,
               const std::vector<double> & maxima,
               const std::vector<unsigned int> & nPoints,
               double initialValue=0.0);
  //!
  //! @brief Set the rank of a constant field.
  //! @param rank new rank (>= 1)
  //! @throws MathException if rank is 0, or if this is a mesh field and rank differs from the mesh rank (use setMesh())
  //!
  void setRank(unsigned int rank) override;
  //! @brief Set all node values (or the constant value) to zero.
  virtual void reset() override;

  //! @}

  //! @name Mesh boundaries and nodes
  //! Boundaries can be changed without changing the node values: only the node coordinates change.
  //! For a constant field the vectors are empty.
  //! @{

  //! @brief Whether the field is constant (no mesh). @return true if constant
  bool isConstant() const { return _values.empty(); }
  //! @brief Lower boundary of each axis. @return minima
  const std::vector<double> & minima() const { return _minima; }
  //! @brief Upper boundary of each axis. @return maxima
  const std::vector<double> & maxima() const { return _maxima; }
  //! @brief Number of nodes along each axis. @return numbers of nodes
  const std::vector<unsigned int> & nPoints() const { return _nPoints; }
  //! @brief Lower boundary of an axis. @param axis axis index @return minimum @throws MathException if axis >= mesh rank
  double minimum(unsigned int axis) const;
  //! @brief Upper boundary of an axis. @param axis axis index @return maximum @throws MathException if axis >= mesh rank
  double maximum(unsigned int axis) const;
  //! @brief Number of nodes along an axis. @param axis axis index @return number of nodes @throws MathException if axis >= mesh rank
  unsigned int nPoints(unsigned int axis) const;
  //! @brief Total number of nodes (0 for a constant field). @return number of nodes
  unsigned int nPointsTotal() const { return _values.size(); }
  //! @brief Distance between consecutive nodes along an axis (0 if the axis has a single node). @param axis axis index @return spacing
  //! @throws MathException if axis >= mesh rank
  double spacing(unsigned int axis) const;
  //! @brief Coordinate of node index along an axis. @param axis axis index @param index node index @return coordinate
  //! @throws MathException if axis >= mesh rank; IndexOutOfBoundException if index >= nPoints(axis)
  double coordinate(unsigned int axis, unsigned int index) const;

  //! @brief Set the lower boundaries of all axes. @param minima new minima, one per axis
  //! @throws MathException if this is a constant field or the size differs from the mesh rank; InvalidBoundsException if a maximum would be <= its minimum
  void setMinima(const std::vector<double> & minima);
  //! @brief Set the upper boundaries of all axes. @param maxima new maxima, one per axis
  //! @throws MathException if this is a constant field or the size differs from the mesh rank; InvalidBoundsException if a maximum would be <= its minimum
  void setMaxima(const std::vector<double> & maxima);
  //! @brief Set the lower boundary of one axis. @param axis axis index @param minimum new minimum
  //! @throws MathException if axis >= mesh rank; InvalidBoundsException if the maximum would be <= the minimum
  void setMinimum(unsigned int axis, double minimum);
  //! @brief Set the upper boundary of one axis. @param axis axis index @param maximum new maximum
  //! @throws MathException if axis >= mesh rank; InvalidBoundsException if the maximum would be <= the minimum
  void setMaximum(unsigned int axis, double maximum);

  //! @brief Product of the axis ranges (maximum - minimum); 1 for a constant field. @return volume of the mesh
  double volume() const;
  //! @brief Coordinate uniformly distributed along an axis. @param axis axis index @param r number in [0,1] @return minimum + (maximum - minimum) r
  double random(unsigned int axis, double r) const;

  //! @}

  //! @name Node values
  //! For a constant field these return (a reference to) the constant value, whatever the indices.
  //! @{

  //! @brief Node value (rank 1). @throws MathException if the rank is not 1; IndexOutOfBoundException if out of range
  double & operator()(unsigned int i);
  //! @brief Node value (rank 2). @throws MathException if the rank is not 2; IndexOutOfBoundException if out of range
  double & operator()(unsigned int i, unsigned int j);
  //! @brief Node value (rank 3). @throws MathException if the rank is not 3; IndexOutOfBoundException if out of range
  double & operator()(unsigned int i, unsigned int j, unsigned int k);
  //! @brief Node value (any rank). @param indices node index along each axis
  //! @throws MathException if indices.size() differs from the rank; IndexOutOfBoundException if out of range
  double & operator()(const std::vector<unsigned int> & indices);
  //! @brief Node value (any rank). @param indices node index along each axis @return node value
  double valueAt(const std::vector<unsigned int> & indices) const;
  //! @brief Node value (rank 3, kept for compatibility).
  double valueAt(unsigned int i, unsigned int j, unsigned int k) const;
  //! @brief Interpolated value (rank 3, kept for compatibility; same as evaluateAt(x,y,z)).
  double valueAt(double x, double y, double z) const;
  //! @brief Value of a constant field. @return constant value @throws MathException if this is a mesh field
  double value() const;

  //! @}

  //! @name Evaluation and interpolation
  //! The constant value, or the multilinear interpolation of the mesh. All throw MathException if the number
  //! of coordinates differs from the rank, or if a coordinate is not finite.
  //! @{

  double evaluateAt(double x) const override;
  double evaluateAt(double x1, double x2) const override;
  double evaluateAt(double x1, double x2, double x3) const override;
  double evaluateAt(const std::vector<double> & x) const override;

  //! @brief Interpolated value (rank 1). @param x coordinate @return f(x)
  double interpolate(double x) const;
  //! @brief Interpolated value (rank 2). @param x1 first coordinate @param x2 second coordinate @return f(x1, x2)
  double interpolate(double x1, double x2) const;
  //! @brief Interpolated value (rank 3). @param x1 first coordinate @param x2 second coordinate @param x3 third coordinate @return f(x1, x2, x3)
  double interpolate(double x1, double x2, double x3) const;
  //! @brief Interpolated value (any rank). @param x coordinates @return f(x)
  double interpolate(const std::vector<double> & x) const;

  //! @}

  //! @name Derivatives
  //! @{

  //!
  //! @brief Derivative of the field along an axis, as a field on the same mesh.
  //!
  //! Computed at each node from three neighboring nodes: central difference inside, second-order one-sided
  //! differences at the edges (first-order difference if the axis has only two nodes).
  //! @param axis axis index @param name name of the result
  //! @return derivative field
  //! @throws MathException if this is a constant field or axis >= mesh rank; DivByZeroException if the axis has a single node
  //!
  ScalarField derivative(unsigned int axis, const String & name) const;

  //! @}

  //! @name Rank 1 to 3 conveniences (kept for compatibility): axis 0, 1 and 2
  //! @{
  double xMin() const { return minimum(0); }
  double xMax() const { return maximum(0); }
  unsigned int xNPts() const { return nPoints(0); }
  double yMin() const { return minimum(1); }
  double yMax() const { return maximum(1); }
  unsigned int yNPts() const { return nPoints(1); }
  double zMin() const { return minimum(2); }
  double zMax() const { return maximum(2); }
  unsigned int zNPts() const { return nPoints(2); }
  double xRange() const { return maximum(0) - minimum(0); }
  double yRange() const { return maximum(1) - minimum(1); }
  double zRange() const { return maximum(2) - minimum(2); }
  double randomX(double r) const { return random(0,r); }
  double randomY(double r) const { return random(1,r); }
  double randomZ(double r) const { return random(2,r); }
  ScalarField derivativeX(const String & name) const { return derivative(0,name); }
  ScalarField derivativeY(const String & name) const { return derivative(1,name); }
  ScalarField derivativeZ(const String & name) const { return derivative(2,name); }
  //! @}


  //! @name Arithmetic
  //! Two constant fields give a constant field; two mesh fields must have the same mesh (rank, boundaries and numbers
  //! of points) and are added node by node; a constant field added to (or subtracted from) a mesh field shifts all its nodes.
  //! The rank of both fields must be the same. The result keeps the name and title of the left operand.
  //! @{
  //! @brief Add rhs. @param rhs field @return this field @throws MathException if the ranks or meshes differ (unchanged)
  ScalarField & operator+=(const ScalarField & rhs);
  //! @brief Subtract rhs. @param rhs field @return this field @throws MathException if the ranks or meshes differ (unchanged)
  ScalarField & operator-=(const ScalarField & rhs);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPF" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly, with the fits attached since format version 2 (see FitList::encodeBinary()). Records of other classes in
  //! the same file are skipped.
  //! @{

  //! @brief Tag of ScalarField records in binary files. @return "CAPF"
  static const char * binaryTag() { return "CAPF"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 2; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this field as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next ScalarField record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this field to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the first field, or the first one with the given name.
  //! @param fileName file name @param name name to look for (empty: the first one)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no such object (this one is then unchanged)
  void loadBinary(const String & fileName, const String & name="");
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<ScalarField> & objects);
  //! @brief Load all the ScalarField objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<ScalarField> loadAllBinary(const String & fileName);

  //! @}

  protected:

  //! @brief Throw if axis is not an axis of the mesh. @param axis axis index @param source name of the calling function
  void checkAxis(unsigned int axis, const char * source) const;
  //! @brief Throw if a maximum is not above its minimum on an axis with more than one node. @param source name of the calling function
  void checkBounds(const std::vector<double> & minima, const std::vector<double> & maxima, const char * source) const;
  //! @brief Position of a node in _values. @param indices node index along each axis @param source name of the calling function @return position
  unsigned int nodeIndex(const std::vector<unsigned int> & indices, const char * source) const;
  //! @brief Multilinear interpolation at x (x has rank() values). @param x coordinates @param source name of the calling function @return interpolated value
  double interpolateMesh(const double * x, const char * source) const;

  double _constValue;                  //!< value of a constant field
  std::vector<double> _minima;         //!< lower boundary of each axis (empty for a constant field)
  std::vector<double> _maxima;         //!< upper boundary of each axis (empty for a constant field)
  std::vector<unsigned int> _nPoints;  //!< number of nodes along each axis (empty for a constant field)
  std::vector<double> _values;         //!< node values, axis 0 varying fastest (empty for a constant field)


  private:
  //! @brief Add sign*rhs. @param rhs field @param sign +1 or -1 @param source name of the calling function
  void combineWith(const ScalarField & rhs, double sign, const char * source);
  };

  //! @relates ScalarField
  //! @brief Sum of two fields. @param left first field @param right second field @return left+right @throws MathException as operator+=
  ScalarField operator+(const ScalarField & left, const ScalarField & right);
  //! @relates ScalarField
  //! @brief Difference of two fields. @param left first field @param right second field @return left-right @throws MathException as operator-=
  ScalarField operator-(const ScalarField & left, const ScalarField & right);

  } //namespace MATH
} //  namespace CAP

#endif

#ifndef CAP_MATH_Parameter
#define CAP_MATH_Parameter
#include "Aliases.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Named parameter with a value, a type, and an allowed range [minimum, maximum].
  //!
  //! The type indicates how the parameter may vary (e.g., in a fit):
  //!  - Free        : free to vary
  //!  - Fixed       : held at its current value
  //!  - LowHigh     : constrained to [minimum, maximum]
  //!  - Low         : bounded from below by minimum
  //!  - High        : bounded from above by maximum
  //!
  //! Setting a minimum (maximum) on a Free parameter makes it Low (High); setting the other bound as well
  //! makes it LowHigh. setValue() throws MathDomainException if the new value violates a bound.
  //! Any parameter, including a bounded one, can be declared Fixed with setType(Fixed); its bounds are kept,
  //! but setMinimum() and setMaximum() then throw MathException.
  //!
  //! Comparison functions and operators compare parameters by value only.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Parameter p("mu", 1.5, CAP::MATH::Parameter::LowHigh, 0.0, 3.0);
  //!   p.setValue(2.0);
  //!   double v = p.value();   // 2.0
  //! @endcode
  //!
  class Parameter
  {
  public:

  //! @brief Parameter type.
  enum ParameterType
  {
    Free,         //!< free to vary
    Fixed,        //!< held at its current value
    LowHigh,      //!< constrained to [minimum, maximum]
    Low,          //!< bounded from below by minimum
    High          //!< bounded from above by maximum
  };

  //! @brief Default minimum: large negative value, safe in arithmetic (e.g. maximum - minimum does not overflow).
  static constexpr double defaultMinimum = -1.0E300;
  //! @brief Default maximum: large positive value, safe in arithmetic (e.g. maximum - minimum does not overflow).
  static constexpr double defaultMaximum =  1.0E300;

  //! @brief Default constructor: empty name, zero value, type Free, bounds defaultMinimum and defaultMaximum.
  Parameter();
  //! @brief Detailed constructor.
  //!
  //! Bounds not given default to defaultMinimum and defaultMaximum, so that e.g. Parameter("x", -5.0, Parameter::Low)
  //! is bounded from below by defaultMinimum only, until setMinimum() is called.
  //! @throws InvalidBoundsException if type is LowHigh and minimum > maximum
  //! @throws MathDomainException if type is Low, High or LowHigh and value is outside the bounds
  //! @param name name of the parameter
  //! @param value value of the parameter
  //! @param type type of the parameter
  //! @param minimum minimum allowed value
  //! @param maximum maximum allowed value
  Parameter(const String & name,
            double value,
            ParameterType type    = Free,
            double minimum = defaultMinimum,
            double maximum = defaultMaximum);
  //! @brief Copy constructor. @param source parameter to copy
  Parameter(const Parameter & source);
  //! @brief Assignment operator. @param rhs parameter to copy @return reference to this parameter
  Parameter & operator=(const Parameter & rhs);
  //! @brief Destructor.
  virtual ~Parameter() {}

  //! @brief Set the name. @param name new name
  void setName(const String & name) { _name = name; }
  //!
  //! @brief Set the value.
  //! @param value new value
  //! @throws MathDomainException if the parameter is bounded (Low, High, LowHigh) and value is outside its bounds
  //!
  void setValue(double value);
  //! @brief Set the type. Any parameter, bounded or not, may be declared Fixed; its bounds are kept. @param type new type
  void setType(ParameterType type) { _type = type; }
  //!
  //! @brief Set the minimum and update the type: Free becomes Low, High becomes LowHigh (Low and LowHigh are unchanged).
  //! @param minimum new minimum
  //! @throws InvalidBoundsException if the parameter has a maximum and minimum > maximum
  //! @throws MathException if the parameter is Fixed
  //! @throws MathDomainException if the current value is below minimum
  //!
  void setMinimum(double minimum);
  //!
  //! @brief Set the maximum and update the type: Free becomes High, Low becomes LowHigh (High and LowHigh are unchanged).
  //! @param maximum new maximum
  //! @throws InvalidBoundsException if the parameter has a minimum and maximum < minimum
  //! @throws MathException if the parameter is Fixed
  //! @throws MathDomainException if the current value is above maximum
  //!
  void setMaximum(double maximum);

  //! @brief Name of the parameter. @return name
  const String & name() const { return _name; }
  //! @brief Value of the parameter. @return value
  double value() const        { return _value; }
  //! @brief Type of the parameter. @return type
  ParameterType type() const  { return _type; }
  //! @brief Minimum allowed value. @return minimum
  double minimum() const      { return _minimum; }
  //! @brief Maximum allowed value. @return maximum
  double maximum() const      { return _maximum; }

  //! @brief Whether the parameter is free (type Free). @return true if free
  bool isFree() const            { return _type==Free; }
  //! @brief Whether the parameter is bounded from below (type Low or LowHigh). @return true if it has a minimum
  bool isLowConstrained() const  { return _type==Low  || _type==LowHigh; }
  //! @brief Whether the parameter is bounded from above (type High or LowHigh). @return true if it has a maximum
  bool isHighConstrained() const { return _type==High || _type==LowHigh; }
  //! @brief Whether the parameter is bounded from below and/or above (type Low, High or LowHigh). @return true if bounded
  bool isConstrained() const     { return isLowConstrained() || isHighConstrained(); }

  //! @brief Whether this value is smaller than that of other. @param other parameter to compare to
  bool smaller(const Parameter & other) const;
  //! @brief Whether this value is greater than that of other. @param other parameter to compare to
  bool greater(const Parameter & other) const;
  //! @brief Whether this value equals that of other. @param other parameter to compare to
  bool equal(const Parameter & other) const;
  //! @brief Whether this value differs from that of other. @param other parameter to compare to
  bool notEqual(const Parameter & other) const;

  //! @brief Same as smaller(). @param other parameter to compare to
  bool operator<(const Parameter & other) const  { return smaller(other); }
  //! @brief Same as greater(). @param other parameter to compare to
  bool operator>(const Parameter & other) const  { return greater(other); }
  //! @brief Same as equal(). @param other parameter to compare to
  bool operator==(const Parameter & other) const { return equal(other); }
  //! @brief Same as notEqual(). @param other parameter to compare to
  bool operator!=(const Parameter & other) const { return notEqual(other); }

  //! @name Binary files (independent of ROOT)
  //! Stored: name, value, type, minimum and maximum, restored exactly (without the checks of the setters, which the
  //! saved parameter already satisfied).

  //! Saved as "CAPR" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of Parameter records in binary files. @return "CAPR"
  static const char * binaryTag() { return "CAPR"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this parameter as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next Parameter record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this parameter to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the first parameter, or the first one with the given name.
  //! @param fileName file name @param name name to look for (empty: the first one)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no such object (this one is then unchanged)
  void loadBinary(const String & fileName, const String & name="");
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<Parameter> & objects);
  //! @brief Load all the Parameter objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Parameter> loadAllBinary(const String & fileName);

  //! @}

  protected:

  String _name;     //!< name of the parameter
  double _value;    //!< value of the parameter
  ParameterType _type;   //!< type of the parameter
  double _minimum;  //!< minimum allowed value
  double _maximum;  //!< maximum allowed value

  ClassDef(Parameter,1)
  };

  } // namespace MATH
} // namespace CAP

#endif

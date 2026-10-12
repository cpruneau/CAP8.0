/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Condition
#define CAP__Condition
#include "Aliases.hpp"
#include "Exceptions.hpp"

using CAP::String;

namespace CAP
{

//!
//! @brief Base class of the conditions applied by filters (see Filter).
//!
//! A condition tests one value: accept() returns whether the value fulfills the condition. The type of a condition
//! (ConditionType) tells which derived class it is and which accept() overload applies; its subtype is defined by the
//! filter using it and tells which quantity is tested (e.g., in EventFilter, the event property compared).
//!
//! The base class accepts nothing: its accept() functions throw. Each derived class overrides the overload for its
//! value type; the other overloads keep throwing (they are made visible with using-declarations, so that an int is not
//! silently converted to a bool, for instance).
//!
//! Conditions are copied polymorphically with clone(): every derived class must override it.
//!
class Condition
{
protected:

  int  _type;     //!< type of the condition (a ConditionType)
  int  _subtype;  //!< subtype, defined by the filter using the condition

public:

  //! @brief Types of conditions, one per derived class.
  enum ConditionType   { kCondition, kConditionBool, kConditionInteger, kConditionIntegerRange, kConditionDoubleRange, kConditionDoubleDualRange};

  //! @brief Constructor. @param type type of the condition (a ConditionType) @param subtype subtype (defined by the filter)
  Condition(int type, int subtype);
  //! @brief Copy constructor. @param source condition to copy
  Condition(const Condition & source);
  //! @brief Destructor.
  virtual ~Condition() {}
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const Condition & operator=(const Condition & source);
  //! @brief Type of the condition. @return a ConditionType
  virtual int type() const { return _type; }
  //! @brief Subtype of the condition. @return subtype
  virtual int subtype() const  { return _subtype; }
  //! @brief Name of the class of the condition. @return class name
  virtual String typeName() const  {  return String("Condition");  }
  //!
  //! Polymorphic copy: returns a heap-allocated copy of the dynamic type (caller owns it).
  //! Every class derived from Condition must override clone().
  //!
  virtual Condition * clone() const { return new Condition(*this); }
  //virtual String subtypeName() const;
  //! @brief Test a boolean value. @param value value @return true if accepted @throws Exception unless overridden
  virtual bool accept(bool value);
  //! @brief Test an integer value. @param value value @return true if accepted @throws Exception unless overridden
  virtual bool accept(int  value);
  //! @brief Test a floating-point value. @param value value @return true if accepted @throws Exception unless overridden
  virtual bool accept(double  value);
  //! @brief Print the type, subtype and parameters of the condition.
  virtual void print() const;
};

//!
//! @brief Condition on a boolean: accepts the requested value.
//!
class ConditionBool : public Condition
{
protected:

  bool _requested; //!< accepted value

public:

  //! @brief Constructor. @param subtype subtype (defined by the filter) @param requestedValue accepted value
  ConditionBool(int subtype,
                bool requestedValue);
  //! @brief Copy constructor. @param source condition to copy
  ConditionBool(const ConditionBool & source);
  //! @brief Destructor.
  virtual ~ConditionBool() {};
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const ConditionBool & operator=(const ConditionBool & source);
  //! @brief Name of the class. @return "ConditionBool"
  virtual String typeName() const { return String("ConditionBool");}
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  virtual ConditionBool * clone() const { return new ConditionBool(*this); }
  using Condition::accept; // keep the int/double overloads visible (they throw) instead of silently converting to bool
  //! @brief Test a value. @param value value @return true if value is the requested value
  virtual bool accept(bool value) { return (value == _requested);  }
  //! @brief Print the condition.
  virtual void print() const;
};

//!
//! @brief Condition on an integer: accepts the requested value.
//!
class ConditionInteger : public Condition
{
protected:

  int _requested; //!< accepted value

public:

  //! @brief Constructor. @param subtype subtype (defined by the filter) @param requestedValue accepted value
  ConditionInteger(int subtype,
                   int requestedValue);
  //! @brief Copy constructor. @param source condition to copy
  ConditionInteger(const ConditionInteger & source);
  //! @brief Destructor.
  virtual ~ConditionInteger() {}
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const ConditionInteger & operator=(const ConditionInteger & source);
  //! @brief Name of the class. @return "ConditionInteger"
  virtual String typeName() const { return String("ConditionInteger");  }
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  virtual ConditionInteger * clone() const { return new ConditionInteger(*this); }
  using Condition::accept;
  //! @brief Test a value. @param value value @return true if value is the requested value
  virtual bool accept(int value) {  return (_requested == value); }
  //! @brief Print the condition.
  virtual void print() const;
};

//!
//! @brief Condition on an integer: accepts values in the closed range [minimum, maximum].
//!
class ConditionIntegerRange : public Condition
{
public:

  int _minimum; //!< smallest accepted value
  int _maximum; //!< largest accepted value

  //! @brief Constructor. @param subtype subtype (defined by the filter) @param minimum smallest accepted value
  //! @param maximum largest accepted value
  ConditionIntegerRange(int subtype,
                        int minimum,
                        int maximum);
  //! @brief Copy constructor. @param source condition to copy
  ConditionIntegerRange(const ConditionIntegerRange & source);
  //! @brief Destructor.
  virtual ~ConditionIntegerRange() {}
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const ConditionIntegerRange & operator=(const ConditionIntegerRange & source);
  //! @brief Name of the class. @return "ConditionIntegerRange"
  virtual String typeName() const {  return String("ConditionIntegerRange");  }
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  virtual ConditionIntegerRange * clone() const { return new ConditionIntegerRange(*this); }
  using Condition::accept;
  //! @brief Test a value. @param value value @return true if minimum <= value <= maximum
  virtual bool accept(int value) {  return (value>=_minimum  &&  value<=_maximum);  }
  //! @brief Print the condition.
  virtual void print() const;
};

//!
//! @brief Condition on a floating-point value: accepts values in the half-open range [minimum, maximum).
//!
class ConditionDoubleRange : public Condition
{
protected:

  double _minimum; //!< lower edge of the accepted range (included)
  double _maximum; //!< upper edge of the accepted range (excluded)

public:

  //! @brief Constructor. @param subtype subtype (defined by the filter) @param minimum lower edge (included)
  //! @param maximum upper edge (excluded)
  ConditionDoubleRange(int subtype,
                       double minimum,
                       double maximum);
  //! @brief Copy constructor. @param source condition to copy
  ConditionDoubleRange(const ConditionDoubleRange & source);
  //! @brief Destructor.
  virtual ~ConditionDoubleRange() {}
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const ConditionDoubleRange & operator=(const ConditionDoubleRange & source);
  //! @brief Name of the class. @return "ConditionDoubleRange"
  virtual String typeName() const  { return String("ConditionDoubleRange");  }
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  virtual ConditionDoubleRange * clone() const { return new ConditionDoubleRange(*this); }
  using Condition::accept;
  //! @brief Test a value. @param value value @return true if minimum <= value < maximum
  virtual bool accept(double value)  {  return (value>=_minimum  &&  value<_maximum);  }
  //! @brief Print the condition.
  virtual void print() const;
};

//!
//! @brief Condition on a floating-point value: accepts values in either of two half-open ranges,
//! [minimum, maximum) or [minimum2, maximum2).
//!
class ConditionDoubleDualRange : public ConditionDoubleRange
{
public:

  double _minimum2; //!< lower edge of the second range (included)
  double _maximum2; //!< upper edge of the second range (excluded)

  //! @brief Constructor. @param subtype subtype (defined by the filter) @param minimum lower edge of the first range
  //! @param maximum upper edge of the first range @param minimum2 lower edge of the second range
  //! @param maximum2 upper edge of the second range
  ConditionDoubleDualRange(int subtype,
                           double minimum,
                           double maximum,
                           double minimum2,
                           double maximum2);
  //! @brief Copy constructor. @param source condition to copy
  ConditionDoubleDualRange(const ConditionDoubleDualRange & source);
  //! @brief Destructor.
  virtual ~ConditionDoubleDualRange() {}
  //! @brief Copy assignment. @param source condition to copy @return this condition
  const ConditionDoubleDualRange & operator=(const ConditionDoubleDualRange & source);
  //! @brief Name of the class. @return "ConditionDoubleDualRange"
  virtual String typeName() const { return String("ConditionDoubleDualRange"); }
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  virtual ConditionDoubleDualRange * clone() const { return new ConditionDoubleDualRange(*this); }
  using ConditionDoubleRange::accept;
  //! @brief Test a value. @param value value @return true if value is in either range
  virtual bool accept(double value) { return (value>=_minimum  &&  value<_maximum) || (value>=_minimum2  &&  value<_maximum2); }
  //! @brief Print the condition.
  virtual void print() const;
};

} // namespace CAP

#endif /* CAP__Condition */



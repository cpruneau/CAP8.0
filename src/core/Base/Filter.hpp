/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Filter
#define CAP__Filter
#include "Exceptions.hpp"
#include "Object.hpp"
#include "Condition.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

//!
//! @brief Exception thrown by filters (e.g., Filter::accept() called on the base class).
//!
class FilterException : public Exception
{
public:
  //! @brief Constructor. @param _message description of the problem @param _source function where it happened
  FilterException(const String & _message,
                  const String & _source);
  //! @brief Destructor.
  virtual ~FilterException() {}
};

//!
//! @brief Base class of the filters selecting objects of type T (events, particles, ...).
//!
//! A filter holds a list of conditions (see Condition) and decides whether an object, or a pair of objects, is
//! accepted. Derived classes (EventFilter, ParticleFilter, ...) override accept(): typically, for each condition, they
//! compute the quantity selected by the condition's subtype and test it with Condition::accept().
//!
//! The filter owns its conditions: they are created by addCondition(), deep-copied with the filter and deleted with it.
//!
//! Example:
//! @code
//!   class PtFilter : public CAP::Filter<Particle>
//!   {
//!   public:
//!     bool accept(const Particle & p) const override { return _conditions[0]->accept(p.pt()); }
//!   };
//!   PtFilter f;
//!   f.addCondition(0, 0.2, 2.0);   // 0.2 <= pT < 2.0
//! @endcode
//!
//! @tparam T type of the filtered objects
//!
template < class T >
class Filter
{
protected:

  //! @brief Conditions of the filter (owned).
  std::vector<Condition*> _conditions;

public:

  //! @brief Default constructor: no conditions.
  Filter()
  :
  _conditions()
  { /* no ops */ }

  //!
  //! Deep copy: the filter owns its Condition* objects, so each one is duplicated
  //! polymorphically via Condition::clone() rather than sharing the source's pointers.
  //!
  Filter(const Filter & source)
  :
  _conditions(cloneConditions(source._conditions))
  { /* no ops */ }

  //!
  //! Deep-copy assignment. The new conditions are cloned before the old ones are released,
  //! so if a clone throws, this filter is left unchanged.
  //!
  Filter & operator=(const Filter & rhs)
  {
  if (this!=&rhs)
    {
    std::vector<Condition*> copies = cloneConditions(rhs._conditions);
    deleteConditions();
    _conditions.swap(copies);
    }
  return *this;
  }

  //! @brief Destructor: the conditions are deleted.
  virtual ~Filter()
  {
  deleteConditions();
  }

  //! @brief Number of conditions. @return number of conditions
  unsigned int nConditions() const {  return _conditions.size(); }
  //! @brief Conditions (owned by the filter). @return conditions
  std::vector<Condition*> & conditions() { return _conditions; }
  //! @brief Conditions (owned by the filter). @return conditions
  const std::vector<Condition*> & conditions() const { return _conditions; }

  //!
  //! Test whether the given object is acceptable based on this filter's criteria.
  //!  The base class throws: derived filters must override accept().
  //!
  virtual bool accept(const T & object __attribute__((unused))) const
  {
  throw FilterException("Improper call to accept of base class","Filter::accept(const T & object __attribute__((unused)))");
  }

  //! Test whether the given objects are acceptable based on this filter's criteria.
  //!  The base class throws: derived filters must override accept().
  virtual bool accept(const T & object1 __attribute__((unused)), const T & object2 __attribute__((unused))) const
  {
  throw FilterException("Improper call to accept of base class","Filter::accept(const T & object1 __attribute__((unused)), const T & object2 __attribute__((unused)))");
  }

//  virtual void print(  ) const
//  {
//  for (auto & condition : _conditions) condition->print();
//  }

  //! @brief Add a ConditionBool. @param _subtype subtype @param _requested accepted value
  void addCondition(int _subtype,
                    bool _requested)
  {
  _conditions.push_back(new ConditionBool(_subtype,_requested));
  }

  //! @brief Add a ConditionInteger. @param _subtype subtype @param _requested accepted value
  void addCondition(int _subtype,
                    int _requested)
  {
  _conditions.push_back(new ConditionInteger(_subtype,_requested));
  }

  //! @brief Add a ConditionIntegerRange accepting [_minimum, _maximum]. @param _subtype subtype
  //! @param _minimum smallest accepted value @param _maximum largest accepted value
  void addCondition(int _subtype,
                    int _minimum,
                    int _maximum)
  {
  _conditions.push_back(new ConditionIntegerRange(_subtype,_minimum,_maximum));
  }

  //! @brief Add a ConditionDoubleRange accepting [_minimum, _maximum). @param _subtype subtype
  //! @param _minimum lower edge (included) @param _maximum upper edge (excluded)
  void addCondition(int _subtype,
                    double _minimum,
                    double _maximum)
  {
  _conditions.push_back(new ConditionDoubleRange(_subtype,_minimum,_maximum));
  }

  //! @brief Add a ConditionDoubleDualRange accepting [_minimum, _maximum) or [_minimum2, _maximum2).
  //! @param _subtype subtype @param _minimum lower edge of the first range @param _maximum upper edge of the first range
  //! @param _minimum2 lower edge of the second range @param _maximum2 upper edge of the second range
  void addCondition(int _subtype,
                    double _minimum,
                    double _maximum,
                    double _minimum2,
                    double _maximum2)
  {
  _conditions.push_back(new ConditionDoubleDualRange(_subtype,_minimum,_maximum,_minimum2,_maximum2));
  }


protected:

  //!
  //! Clone every condition of the given list. If any clone throws, the copies already made are
  //! deleted before the exception propagates (no leak).
  //!
  static std::vector<Condition*> cloneConditions(const std::vector<Condition*> & source)
  {
  std::vector<Condition*> copies;
  copies.reserve(source.size());
  try
    {
    for (auto & condition : source) copies.push_back(condition ? condition->clone() : nullptr);
    }
  catch (...)
    {
    for (auto & copy : copies) delete copy;
    throw;
    }
  return copies;
  }

  //! @brief Delete all conditions.
  void deleteConditions()
  {
  for (auto & condition : _conditions) delete condition;
  _conditions.clear();
  }

}; // Filter<T>

}// namespace CAP


#endif /* CAP__Filter  */


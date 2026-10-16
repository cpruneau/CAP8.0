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
#ifndef CAP__Accountant
#define CAP__Accountant
#include "Aliases.hpp"
#include "Object.hpp"

namespace CAP
{

//!
//! @brief Counter of executed tasks or fulfilled conditions.
//!
//! An Accountant keeps one or several counts (type long). With a single count it tracks how many times a task is
//! completed or a condition is fulfilled (e.g., Task uses one to count calls to execute()); with several counts it keeps
//! track of disjoint conditions, such as the outcome of each of a list of event filters.
//!
//! Counts are indexed from 0 to size()-1. Accountants can be saved and loaded with CapFile::exportAccountant() and
//! CapFile::importAccountant().
//!
//! Example:
//! @code
//!   CAP::Accountant accepted("eventAccepted",3);   // three filters
//!   accepted.increment(1);                          // filter 1 accepted an event
//!   long n = accepted.valueAt(1);
//! @endcode
//!
class Accountant
:
public Object
{
public:

  //! @brief Constructor. @param name name of the accountant @param size number of counts (all set to 0)
  Accountant(const String & name="Executed", unsigned int size=1);
  //! @brief Copy constructor. @param accountant accountant to copy
  Accountant(const Accountant & accountant);
  //! @brief Copy assignment. @param accountant accountant to copy @return this accountant
  Accountant & operator=(const Accountant & accountant);
  //! @brief Destructor.
  virtual ~Accountant() { }
  //! @brief Set all counts to 0, keeping their number.
  virtual void reset();
  //! @brief Remove all counts (size() becomes 0).
  virtual void clear();
  //! @brief Rename the accountant and set its size, with all counts set to 0.
  //! @param name new name @param size new number of counts
  virtual void initialize(const String & name, unsigned );
  //! @brief Add one to a count. @param index index of the count @throws IndexOutOfBoundException if index>=size()
  virtual void increment(unsigned int index=0);
  //! @brief Add the counts of another accountant, count by count. @param other accountant to add
  //! @throws Exception if the two accountants have different sizes
  virtual void increment(const Accountant & other);
  //! @brief Value of a count. @param index index of the count @return value
  //! @throws IndexOutOfBoundException if index>=size()
  virtual long valueAt(unsigned int index=0) const;
  //! @brief Set the value of a count. @param index index of the count @param value new value
  //! @throws IndexOutOfBoundException if index>=size()
  virtual void setValueAt(unsigned int index, long value);
  //! @brief Print the name, the number of counts and each count.
  virtual void print() const;
  
  //! @brief Counts (modifiable). @return vector of counts
  std::vector<long> & values()
  {
  return _counts;
  }
  //! @brief Counts. @return vector of counts
  const std::vector<long> & values() const
  {
  return _counts;
  }

  //! @brief Number of counts. @return size
  unsigned int size() const
  {
  return _counts.size();
  }

protected:
  
  std::vector<long> _counts; //!< counts

  ClassDef(Accountant,0)
};

} // namespace CAP

#endif /* CAP__Accountant */

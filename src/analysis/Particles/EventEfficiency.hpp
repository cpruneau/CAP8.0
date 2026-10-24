/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventEfficiency
#define CAP__EventEfficiency
#include "HistogramFolder.hpp"
#include "Event.hpp"

namespace CAP
{

//!
//! @brief Event detection efficiency, as a function of the multiplicity, used to simulate event losses and to weight
//! events.
//! @note efficiencyAt() currently returns 1 and accept() always returns true.
//!
class EventEfficiency
:
public HistogramFolder
{
public:

  //! @brief Default constructor.
  EventEfficiency();
  //! @brief Copy constructor. @param otherFilter efficiency to copy
  EventEfficiency(const EventEfficiency & otherFilter);
  //! @brief Copy assignment. @param otherFilter efficiency to copy @return this efficiency
  EventEfficiency & operator=(const EventEfficiency & otherFilter);
  //! @brief Destructor.
  virtual ~EventEfficiency() { delete h_weight_mult; }
  //! @brief Configure the efficiency (binning of the multiplicity).
  //! @param ownerName name of the owner (task) @param typeName object type (EVENT_EFFICIENCY)
  //! @param configuration configuration @param index index of the efficiency
  virtual void configure(const String & ownerName,
                               const String & typeName,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the efficiency histogram.
  virtual void create();
  //! @brief Load the efficiency histogram from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Efficiency for an event. @param event event @return efficiency (currently 1)
  virtual double efficiencyAt(const Event & event);
  //! @brief Weight (inverse efficiency) of an event. @param event event @return weight
  virtual double weightAt(const Event & event);
  //! @brief Whether an event is detected (currently always true). @param event event @return true if detected
  virtual bool   accept(const Event & event);
  //! @brief Compute the weight histogram (inverse of the efficiency).
  virtual void   calculateWeights();
protected:

  //! @brief Copy the content of another efficiency. @param source efficiency to copy
  virtual void cloneB(const EventEfficiency & source);

  //! Efficiency vs multiplicity.
  Histogram * h_efficiency_mult;
  //! Weight (inverse efficiency) vs multiplicity.
  Histogram * h_weight_mult;

  //! Number of multiplicity bins.
  int nBins_mult;
  //! Lower edge of the multiplicity range.
  double min_mult;
  //! Upper edge of the multiplicity range.
  double max_mult;


};

}

#endif /* CAP__EventEfficiency */


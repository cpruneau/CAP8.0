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
#ifndef CAP__ManagedObject
#define CAP__ManagedObject
#include "Object.hpp"
#include "Configuration.hpp"
#include "CapFile.hpp"
#include <fstream>

namespace CAP
{

  //!
  //! @brief Base class of the objects managed by tasks through ManagedObjects: events, filters, efficiencies, smearers,
  //! particle databases.
  //!
  //! A managed object is configured from keys \<task\>:\<TYPE\>_\<index\>:..., created or loaded (from a CAP file or ASCII
  //! files), and can be saved, scaled, reset and printed. All functions do nothing here; derived classes override those
  //! they need.
  //!
  class ManagedObject
  :
  public Object
  {
  public:

  //! @brief Default constructor.
  ManagedObject();
  //! @brief Copy constructor. @param source object to copy
  ManagedObject(const ManagedObject & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  ManagedObject & operator=(const ManagedObject & rhs);
  //! @brief Destructor.
  virtual ~ManagedObject() {}
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>_\<index\>:... (nothing by default).
  //! @param taskName name of the owning task @param objectType object type (e.g. PARTICLE_FILTER)
  //! @param configuration configuration @param index index of the object
  virtual void configure(const String & taskName  __attribute__((unused)),
                               const String & objectType  __attribute__((unused)),
                               const Configuration & configuration  __attribute__((unused)),
                               unsigned int index  __attribute__((unused))){}

  //! @brief Initialize the object (nothing by default).
  virtual void initialize() {}
  //! @brief Create the object's content, e.g. histograms (nothing by default).
  virtual void create() {}
  //! @brief Load the object from a CAP file (nothing by default). @param file file
  virtual void loadFrom(CapFile & file __attribute__((unused))) {}
  //! @brief Load the object from an ASCII file (nothing by default). @param file file
  virtual void loadFromAscii(std::ifstream & file __attribute__((unused))) {}
  //! @brief Load from an XML file (nothing here; ParticleDb overrides it). @param fileName file name
  virtual void loadFromXml(const String & fileName __attribute__((unused))) {}
  //! @brief Save the object in a CAP file (nothing by default). @param file file
  virtual void saveTo(CapFile & file __attribute__((unused))) const {}
  //! @brief Save the object to an ASCII file (nothing by default). @param file file
  virtual void saveToAscii(std::ofstream & file __attribute__((unused))) {}
  //! @brief Save the object to two ASCII files (nothing by default). @param file1 first file @param file2 second file
  virtual void saveToAscii2(std::ofstream & file1 __attribute__((unused)),
                                  std::ofstream & file2 __attribute__((unused))) {}

  //! @brief Scale the object, e.g. its histograms (nothing by default). @param factor factor
  virtual void scaleObject(double factor __attribute__((unused))) { }
  //! @brief Reset the object (nothing by default).
  virtual void reset(){ }
  //! @brief Clear the object (nothing by default).
  virtual void clear(){ }
  //! @brief Print the object (nothing by default).
  virtual void print() const {}

  protected:

  ClassDef(ManagedObject,0)
  };

}

#endif /* CAP__ManagedObject */


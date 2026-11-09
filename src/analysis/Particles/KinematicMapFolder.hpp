/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__KinematicMapFolder
#define CAP__KinematicMapFolder
#include <limits>
#include <memory>
#include <vector>
#include "HistogramFolder.hpp"
#include "KinematicMap.hpp"
#include "Configuration.hpp"

namespace CAP
{

//!
//! @brief Base class of the objects made of kinematic maps (KinematicMap): ParticleEfficiency (one map) and
//! ParticleSmear (resolutions and biases). It configures the maps, creates or imports their histograms, exports
//! their histograms and functions, and keeps them right through copies.
//!
//! Each map has a key (e.g. EFFICIENCY, PT, ETA_BIAS) and a range of allowed values (e.g. [0, 1] for an efficiency):
//! histograms with a value outside it are refused, and values of functions are clamped to it.
//!
//! Configuration of object k of type \<TYPE\> of task \<task\> (see ManagedObjects), for each map \<KEY\>:
//! @code
//!   <task>:<TYPE>_<k>:<KEY>:MODEL       NONE, CONSTANT, FUNCTION or HISTOGRAM
//!   <task>:<TYPE>_<k>:<KEY>:VALUE       the constant, or the initial content of a created histogram
//!   <task>:<TYPE>_<k>:<KEY>:FUNCTION    name of the function in MATH::FunctionRegistry (e.g. Polynomial, Normal)
//!   <task>:<TYPE>_<k>:<KEY>:PARAMETERS  values of its parameters (e.g. "0.9, 0.01"; empty: its defaults)
//!   <task>:<TYPE>_<k>:<KEY>:VARIABLES   variables of the function or histogram, in order (e.g. "pt,eta")
//!   <task>:<TYPE>_<k>:pt_nbins, pt_min, pt_max, eta_..., phi_...   binning of the created histograms
//! @endcode
//! A HISTOGRAM map gets its histogram from create() (CREATE: filled with VALUE, to be set later) or loadFrom()
//! (IMPORT).
//!
//! Map files: saveTo() (EXPORT) writes the object in a sub-folder named folderName() (BASE_NAME, or the name of the
//! object when BASE_NAME is empty or NONE), so that the efficiencies and smearers of a workflow can share one file. The
//! sub-folder is self-describing: KIND (kindName()), and for each map its settings (StringParameter records
//! \<KEY\>_MODEL, \<KEY\>_VARIABLES, \<KEY\>_VALUE), its histogram \<KEY\> or its function \<KEY\>_FUNCTION, the binning
//! used to create histograms (BINNING: nbins, min, max of pT, eta and phi), and the settings of the derived class
//! (saveSettings()). loadFrom() (IMPORT) takes everything from that sub-folder: the
//! configuration only says which file. A file written before (no sub-folder: histograms \<BASE_NAME\>_\<KEY\> at the top,
//! no settings) is read as configured: the histograms of the HISTOGRAM maps, and the functions of the FUNCTION maps
//! when the file has them.
//!
class KinematicMapFolder : public HistogramFolder
{
public:

  //! @brief Default constructor (no map: derived classes add theirs with addMap()).
  KinematicMapFolder();
  //! @brief Copy constructor: the histograms copied, the maps pointing to the copies. @param source object to copy
  KinematicMapFolder(const KinematicMapFolder & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  KinematicMapFolder & operator=(const KinematicMapFolder & rhs);
  //! @brief Destructor.
  virtual ~KinematicMapFolder() {}

  //! @brief Configure the maps and the binning (see the class description).
  //! @param taskName name of the task @param typeName object type @param configuration configuration @param index index
  //! @throws Exception for an unknown model, function or variable, or parameters that do not fit the function
  virtual void configure(const String & taskName, const String & typeName, const Configuration & configuration,
                         unsigned int index);
  //! @brief Create the histograms of the HISTOGRAM maps (binning of the configuration, filled with VALUE).
  virtual void create();
  //! @brief Import the maps (see the class description) and the settings of the derived class (loadSettings()).
  //! @param inputFile folder @throws Exception if a histogram or function is missing, a setting is invalid, or a
  //! histogram has values outside the allowed range
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Export the maps (settings, histograms, functions that can be saved: MATH::ParameterizedFunction) and the
  //! settings of the derived class (saveSettings()). @param outputFile folder
  virtual void saveTo(Folder & outputFile) const;
  //! @brief Name of a setting of map k in the files (e.g. PT_MODEL). @param k index @param field field @return name
  String mapSettingName(unsigned int k, const String & field) const { return _maps[k].key+"_"+field; }
  //! @brief Name of a setting of the object in the files (the field itself, e.g. SPECIES). @param field field
  //! @return name
  String settingName(const String & field) const { return field; }

  //! @brief Number of maps. @return number
  unsigned int nMaps() const { return _maps.size(); }
  //! @brief Key of map k. @param k index @return key
  const String & mapKey(unsigned int k) const { return _maps[k].key; }
  //! @brief Map k. @param k index @return map
  const KinematicMap & map(unsigned int k) const { return _maps[k].map; }
  //! @brief Index of the map of a key. @param key key @return index @throws Exception if there is none
  unsigned int mapIndex(const String & key) const;
  //! @brief Name of the histogram of map k (its key). @param k index @return name
  String mapHistogramName(unsigned int k) const { return _maps[k].key; }
  //! @brief Name of the function of map k in the files (\<KEY\>_FUNCTION). @param k index @return name
  String mapFunctionName(unsigned int k) const { return _maps[k].key+"_FUNCTION"; }
  //! @brief Name of the sub-folder of the object in the files: BASE_NAME, or the name of the object when BASE_NAME is
  //! empty or NONE. @return name
  String folderName() const;
  //! @brief Kind of the object, written in its files (KIND): PARTICLE_EFFICIENCY, PARTICLE_SMEARER. @return kind
  virtual String kindName() const = 0;
  //! @brief Names of the objects of a kind in a folder of a file: its sub-folders whose KIND is that kind.
  //! @param file folder @param kind kind (e.g. PARTICLE_EFFICIENCY) @return names
  static std::vector<String> objectNames(const Folder & file, const String & kind);

  //! @brief Value of map k at a point, clamped to its range (not a number: the lowest value, or 0 if unbounded).
  //! @param k index @param pt pT @param eta pseudorapidity @param phi azimuth @return value
  double mapValue(unsigned int k, double pt, double eta, double phi) const;
  //! @brief Map k: no source. @param k index
  void setMapNone(unsigned int k);
  //! @brief Map k: constant. @param k index @param value value @throws Exception if it is outside the allowed range
  void setMapConstant(unsigned int k, double value);
  //! @brief Map k: function. @param k index @param function function (shared) @param variables its variables
  //! @throws Exception (see KinematicMap::setFunction())
  void setMapFunction(unsigned int k, std::shared_ptr<const MATH::Function> function,
                      const std::vector<KinematicMap::Variable> & variables);
  //! @brief Map k: a copy of a histogram, kept in this folder. @param k index @param histogram histogram
  //! @param variables its variables @throws Exception if a value is outside the allowed range, or see
  //! KinematicMap::setHistogram()
  void setMapHistogram(unsigned int k, const MATH::Histogram & histogram,
                       const std::vector<KinematicMap::Variable> & variables);
  //! @brief Check the values of a histogram against the range of map k. @param k index @param histogram histogram
  //! @throws Exception if a value is outside
  void checkRange(unsigned int k, const MATH::Histogram & histogram) const;
  //! @brief Lowest allowed value of map k. @param k index @return value
  double mapLowest(unsigned int k) const { return _maps[k].lowest; }
  //! @brief Highest allowed value of map k. @param k index @return value
  double mapHighest(unsigned int k) const { return _maps[k].highest; }
  //! @brief Number of bins of the histograms created, for a variable. @param v variable @return number
  int nBins(KinematicMap::Variable v) const { return _nBins[v]; }
  //! @brief Lower edge of the histograms created, for a variable. @param v variable @return edge
  double binMinimum(KinematicMap::Variable v) const { return _minimum[v]; }
  //! @brief Upper edge of the histograms created, for a variable. @param v variable @return edge
  double binMaximum(KinematicMap::Variable v) const { return _maximum[v]; }
  //! @brief The histogram of map k, to change its bin values (null if it has none); check it with checkRange() after.
  //! @param k index @return histogram, owned by this folder
  MATH::Histogram * mutableMapHistogram(unsigned int k) { return _maps[k].map.histogram() ? histogram(mapHistogramName(k)) : nullptr; }
  //! @brief Value with which created histograms of map k are filled. @param k index @return value
  double mapCreateValue(unsigned int k) const { return _maps[k].createValue; }
  //! @brief Binning of the histograms created for a variable. @param v variable @param nBins number of bins
  //! @param minimum lower edge @param maximum upper edge @throws Exception if the binning is empty
  void setBinning(KinematicMap::Variable v, int nBins, double minimum, double maximum);

protected:

  //! A map, with its key and allowed range.
  struct MapEntry
  {
    String key;            //!< key in the configuration
    KinematicMap map;      //!< the map
    double lowest;         //!< lowest allowed value
    double highest;        //!< highest allowed value
    double createValue;    //!< content of a created histogram
  };

  //! @brief Add a map. @param key key @param initial initial map (e.g. constant 1, or none) @param lowest lowest allowed
  //! value @param highest highest allowed value @return its index
  unsigned int addMap(const String & key, const KinematicMap & initial,
                      double lowest = -std::numeric_limits<double>::infinity(),
                      double highest = std::numeric_limits<double>::infinity());
  //! @brief Point the histograms of the maps to those of this folder (after a copy). @param source object copied
  void repoint(const KinematicMapFolder & source);
  //! @brief Import from a folder of the old layout (top of the file, names \<BASE_NAME\>_\<KEY\>), as configured.
  //! @param inputFile folder
  void loadLegacy(const Folder & inputFile);
  //! @brief Save the settings of the derived class (nothing here). @param outputFile folder
  virtual void saveSettings(Folder & outputFile) const { (void) outputFile; }
  //! @brief Load the settings of the derived class (nothing here). @param inputFile folder
  virtual void loadSettings(const Folder & inputFile) { (void) inputFile; }
  //! @brief Write a text setting (replacing one of that name). @param outputFile folder @param name name @param value value
  static void writeSetting(Folder & outputFile, const String & name, const String & value);
  //! @brief Text setting of a folder. @param inputFile folder @param name name @param value set to its value if found
  //! @return whether the folder has it
  static bool readSetting(const Folder & inputFile, const String & name, String & value);

  std::vector<MapEntry> _maps;      //!< the maps
  int    _nBins[3];                 //!< binning of the created histograms: number of bins for pt, eta, phi
  double _minimum[3];               //!< lower edges for pt, eta, phi
  double _maximum[3];               //!< upper edges for pt, eta, phi
};

} // namespace CAP

#endif /* CAP__KinematicMapFolder */

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__CalibrationProducer
#define CAP__CalibrationProducer
#include "AnalysisHistos.hpp"
#include "ParticlePerformanceHistos.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmear.hpp"

namespace CAP
{

//!
//! @brief Task writing the detector maps measured by a performance analysis: a map file of particle efficiencies and
//! smearers (ParticleEfficiency, ParticleSmear), read by the measurement simulators (IMPORT of PARTICLE_EFFICIENCY and
//! PARTICLE_SMEARER) and edited in CapGui (Detector Maps).
//!
//! The task reads the performance histograms (ParticlePerformanceHistos) of a source task (SOURCE_TASK, by default
//! PerformanceAnalyzer) from the input file of its job, one folder per combination of filters, computes their derived
//! histograms if needed, and makes for each combination:
//! - an efficiency \<name\>_Efficiency: the efficiency map, clamped to [0, 1] (migration near the edges of a filter
//!   can give more than 1), as a histogram of the generator-level kinematics;
//! - a smearer \<name\>_Smearer: the resolution maps of pT, eta and phi (rms of reco - gen) and, with MAPS:BIASES, the
//!   bias maps (average of reco - gen), Gaussian deviates.
//! \<name\> is the base name of the histograms of the combination. The maps are functions of (pT, eta) (the maps
//! vs (Pt, Eta) of the folders, ptEta_fill) or of pT (the histograms vs Pt): MAPS:VARIABLES = AUTO (default: (pT,
//! eta) when the folders have them), PT_ETA or PT. A bin with no generated particle has efficiency 0; a bin with
//! fewer than two detected particles has resolution 0 (no smearing): choose the binning for the statistics.
//!
//! Configuration (keys \<task\>:...): SOURCE_TASK, MAPS:PATH (directory; DEFAULT, absolute, "./...", or relative to
//! $CAP_CALIB_EXPORT_PATH), MAPS:FILE_NAME (default DetectorMaps.cap), MAPS:VARIABLES, MAPS:BIASES (default true), and
//! the binning and fill options of the performance histograms (see ParticlePerformanceHistos).
//!
class CalibrationProducer : public CalculatorHistos<ParticlePerformanceHistos,1>
{
public:

  typedef CalculatorHistos<ParticlePerformanceHistos,1> Base;

  //! @brief Default constructor.
  CalibrationProducer();
  //! @brief Copy constructor. @param source task to copy
  CalibrationProducer(const CalibrationProducer & source) = default;
  //! @brief Copy assignment. @param rhs task to copy @return this task
  CalibrationProducer & operator=(const CalibrationProducer & rhs) = default;
  //! @brief Destructor.
  virtual ~CalibrationProducer() {}
  //! @brief Copy of this task. @return copy
  virtual Task * clone() override { return new CalibrationProducer(*this); }
  //! @brief Add the default configuration of the task (see the class description).
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task. @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Make the maps of each combination of filters and write them to the map file.
  virtual void execute() override;

  //! @brief Variables of the maps.
  enum MapVariables { kAuto=0, kPtEta, kPtOnly };

  //!
  //! @brief Efficiency of one folder of performance histograms (its derived histograms computed).
  //! @param folder performance histograms @param variables variables of the maps (kAuto: (pT, eta) if the folder has
  //! them) @return efficiency, named \<base name\>_Efficiency @throws Exception if the folder has no efficiency histogram
  //!
  static ParticleEfficiency makeEfficiency(const ParticlePerformanceHistos & folder, MapVariables variables=kAuto);
  //!
  //! @brief Smearer of one folder of performance histograms (its derived histograms computed).
  //! @param folder performance histograms @param variables variables of the maps @param biases whether the biases are
  //! included @return smearer, named \<base name\>_Smearer @throws Exception if the folder lacks a resolution histogram
  //!
  static ParticleSmear makeSmearer(const ParticlePerformanceHistos & folder, MapVariables variables=kAuto,
                                   bool biases=true);
  //! @brief Parse MAPS:VARIABLES (AUTO, PT_ETA, PT; case ignored). @param text text @return variables
  //! @throws Exception if unknown
  static MapVariables parseMapVariables(const String & text);

protected:

  String       _mapsPath;       //!< directory of the map file (resolved)
  String       _mapsFileName;   //!< name of the map file
  MapVariables _variables;      //!< variables of the maps
  bool         _biases;         //!< whether the smearers include the biases

};

} // namespace CAP

#endif /* CAP__CalibrationProducer */

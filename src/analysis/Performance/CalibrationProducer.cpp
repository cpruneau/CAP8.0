/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "CalibrationProducer.hpp"
#include "CapFile.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "PrintHelpers.hpp"
#include <algorithm>
#include <memory>

namespace CAP
{

  namespace
  {
    //! Name of the maps of a folder: the base name of its histograms.
    String mapsName(const ParticlePerformanceHistos & folder)
    {
    const String & base = folder.histogramBaseName();
    return base.isEmpty() ? String("Detector") : base;
    }

    //! Whether the maps of a folder are functions of (pT, eta).
    bool usePtEta(const ParticlePerformanceHistos & folder, CalibrationProducer::MapVariables variables)
    {
    switch (variables)
      {
      case CalibrationProducer::kPtEta:  return true;
      case CalibrationProducer::kPtOnly: return false;
      default:                           return folder.efficiencyPtEta()!=nullptr;
      }
    }

    //! A plain histogram with the bin values of h (a profile: its averages), clamped to [lowest, highest].
    MATH::Histogram copyOf(const MATH::Histogram & h, double lowest, double highest)
    {
    MATH::Histogram copy(static_cast<const MATH::Partition &>(h));
    for (unsigned int k=0; k<h.nBinsTotal(); k++)
      copy.setBinValueAtGlobal(k,std::min(highest,std::max(lowest,h.binValueAtGlobal(k))));
    return copy;
    }

    const std::vector<KinematicMap::Variable> ptEta = { KinematicMap::kPt, KinematicMap::kEta };
    const std::vector<KinematicMap::Variable> ptOnly = { KinematicMap::kPt };
  }

  CalibrationProducer::CalibrationProducer()
  :
  Base(ParticlePerformanceHistos::analyzerName()),
  _mapsPath(""),
  _mapsFileName("DetectorMaps.cap"),
  _variables(kAuto),
  _biases(true)
  {
  appendClassName("CalibrationProducer");
  setMinimumReportLevel(Object::kInfo);
  setName("CalibrationProducer");
  setTitle("CalibrationProducer");
  }

  void CalibrationProducer::setDefaultConfiguration()
  {
  Base::setDefaultConfiguration();
  ParticlePerformanceHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"MAPS:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(name(),"MAPS:FILE_NAME"),"DetectorMaps.cap");
  _configuration.addProperty(createKey(name(),"MAPS:VARIABLES"),"AUTO");
  _configuration.addProperty(createKey(name(),"MAPS:BIASES"),true);
  }

  void CalibrationProducer::configure(const Configuration & configuration)
  {
  Base::configure(configuration);
  _mapsPath     = FileSystem::configuredPath(configuration.valueString(createKey(name(),"MAPS:PATH")),"CAP_CALIB_EXPORT_PATH");
  _mapsFileName = configuration.valueString(createKey(name(),"MAPS:FILE_NAME"));
  _variables    = parseMapVariables(configuration.valueString(createKey(name(),"MAPS:VARIABLES")));
  _biases       = configuration.valueBool(createKey(name(),"MAPS:BIASES"));
  if (_mapsFileName.isEmpty() || _mapsFileName.equalTo("NONE"))
    throw ConfigurationException(name(),"MAPS:FILE_NAME: a map file is required",__FUNCTION__);
  if (reportInfo(__FUNCTION__))
    {
    printValue(createKey(name(),"MAPS:PATH"),_mapsPath);
    printValue(createKey(name(),"MAPS:FILE_NAME"),_mapsFileName);
    printValue(createKey(name(),"MAPS:VARIABLES"),configuration.valueString(createKey(name(),"MAPS:VARIABLES")));
    printValue(createKey(name(),"MAPS:BIASES"),_biases);
    }
  }

  CalibrationProducer::MapVariables CalibrationProducer::parseMapVariables(const String & text)
  {
  if (text.equalTo("AUTO",true))   return kAuto;
  if (text.equalTo("PT_ETA",true)) return kPtEta;
  if (text.equalTo("PT",true))     return kPtOnly;
  throw Exception(String("Unknown MAPS:VARIABLES ")+text+" (AUTO, PT_ETA or PT)","CalibrationProducer::parseMapVariables()");
  }

  ParticleEfficiency CalibrationProducer::makeEfficiency(const ParticlePerformanceHistos & folder, MapVariables variables)
  {
  bool twoD = usePtEta(folder,variables);
  const MATH::Histogram * h = twoD ? folder.efficiencyPtEta() : folder.efficiency(ParticlePerformanceHistos::kPt);
  if (!h) throw Exception(String("No efficiency ")+(twoD ? "vs (pT, eta)" : "vs pT")+" in the performance histograms "
                          +mapsName(folder)+(twoD ? " (ptEta_fill)" : ""),"CalibrationProducer::makeEfficiency()");
  ParticleEfficiency efficiency(copyOf(*h,0.0,1.0),twoD ? ptEta : ptOnly);
  efficiency.setName(mapsName(folder)+"_Efficiency");
  return efficiency;
  }

  ParticleSmear CalibrationProducer::makeSmearer(const ParticlePerformanceHistos & folder, MapVariables variables,
                                                 bool biases)
  {
  bool twoD = usePtEta(folder,variables);
  const std::vector<KinematicMap::Variable> & v = twoD ? ptEta : ptOnly;
  static const ParticlePerformanceHistos::Quantity quantities[3] =
    { ParticlePerformanceHistos::kDpt, ParticlePerformanceHistos::kDeta, ParticlePerformanceHistos::kDphi };
  static const ParticleSmear::Observable observables[3] = { ParticleSmear::kPt, ParticleSmear::kEta, ParticleSmear::kPhi };
  ParticleSmear smearer;
  for (int k=0; k<3; k++)
    {
    ParticlePerformanceHistos::Quantity q = quantities[k];
    const MATH::Histogram * sigma = twoD ? folder.resolutionPtEta(q) : folder.resolution(q,ParticlePerformanceHistos::kPt);
    if (!sigma) throw Exception(String("No resolution ")+(twoD ? "vs (pT, eta)" : "vs pT")+" in the performance "
                                "histograms "+mapsName(folder),"CalibrationProducer::makeSmearer()");
    smearer.setResolution(observables[k],copyOf(*sigma,0.0,1.0e300),v);
    if (!biases) continue;
    const MATH::Histogram * bias = twoD ? static_cast<const MATH::Histogram *>(folder.profilePtEta(q))
                                        : static_cast<const MATH::Histogram *>(folder.profile(q,ParticlePerformanceHistos::kPt));
    if (bias) smearer.setBias(observables[k],copyOf(*bias,-1.0e300,1.0e300),v);
    }
  smearer.setName(mapsName(folder)+"_Smearer");
  return smearer;
  }

  void CalibrationProducer::execute()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  std::vector<ParticlePerformanceHistos*> & folders = histograms();
  if (folders.empty()) throw Exception("No performance histograms (HISTOGRAM:IMPORT from the input file of the job)",__FUNCTION__);
  std::unique_ptr<CapFile> file(CapFile::openRecreateCapFile(_mapsPath,_mapsFileName));
  for (ParticlePerformanceHistos * folder : folders)
    {
    if (!folder->hasDerived()) { folder->createDerived(); folder->calculateDerived(); }
    makeEfficiency(*folder,_variables).saveTo(*file);
    makeSmearer(*folder,_variables,_biases).saveTo(*file);
    if (reportInfo(__FUNCTION__)) printValue("Detector maps",mapsName(*folder));
    }
  file->write();
  if (reportInfo(__FUNCTION__)) printValue("Map file",CapFile::makeFileName(_mapsPath,_mapsFileName));
  }

} // namespace CAP

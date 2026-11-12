/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticleEfficiency.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include <algorithm>
#include <string>

namespace CAP
{

  ParticleEfficiency::ParticleEfficiency(double efficiency)
  :
  KinematicMapFolder(),
  _species(),
  _random()
  {
  appendClassName("ParticleEfficiency");
  setName("ParticleEfficiency");
  setTitle("ParticleEfficiency");
  addMap("EFFICIENCY",KinematicMap(1.0),0.0,1.0);
  setConstant(efficiency);
  }

  ParticleEfficiency::ParticleEfficiency(std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables)
  :
  ParticleEfficiency(1.0)
  {
  setFunction(function,variables);
  }

  ParticleEfficiency::ParticleEfficiency(const MATH::Histogram & histogram, const std::vector<Variable> & variables)
  :
  ParticleEfficiency(1.0)
  {
  setHistogram(histogram,variables);
  }

  void ParticleEfficiency::configure(const String & taskName, const String & typeName,
                                     const Configuration & configuration, unsigned int index)
  {
  KinematicMapFolder::configure(taskName,typeName,configuration,index);
  String species = configuration.valueString(createKey(taskName,typeName,index,"SPECIES"),true,"");
  _species.clear();
  for (double code : KinematicMap::parseNumbers(species)) _species.push_back(int(code));
  }

  void ParticleEfficiency::saveSettings(Folder & outputFile) const
  {
  std::string text;
  for (unsigned int k=0; k<_species.size(); k++) { if (k) text += ","; text += std::to_string(_species[k]); }
  writeSetting(outputFile,settingName("SPECIES"),String(text.c_str()));
  }

  void ParticleEfficiency::loadSettings(const Folder & inputFile)
  {
  String text;
  if (!readSetting(inputFile,settingName("SPECIES"),text)) return;
  _species.clear();
  for (double code : KinematicMap::parseNumbers(text)) _species.push_back(int(code));
  }

  bool ParticleEfficiency::appliesTo(const Particle & particle) const
  {
  if (_species.empty()) return true;
  const ParticleType * type = particle.typePtr();
  if (!type) return false;
  return std::find(_species.begin(),_species.end(),type->pdgCode())!=_species.end();
  }

  double ParticleEfficiency::efficiency(const Particle & particle) const
  {
  if (!appliesTo(particle)) return 0.0;
  const VectorLorentz & p = particle.momentum();
  return efficiency(p.perp(),p.pseudorapidity(),p.phi());
  }

  double ParticleEfficiency::weight(const Particle & particle) const
  {
  double e = efficiency(particle);
  return e>0.0 ? 1.0/e : 0.0;
  }

  bool ParticleEfficiency::accept(const Particle & particle, MATH::RandomGenerator & random) const
  {
  double e = efficiency(particle);
  return random.random()<e;           // r in [0, 1): always accepted for e = 1, never for e = 0
  }

} // namespace CAP

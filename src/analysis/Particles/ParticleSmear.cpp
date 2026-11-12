/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticleSmear.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include <cctype>
#include <cmath>
#include <string>

namespace CAP
{

  namespace
  {
    const char * const observableKey[3] = { "PT", "ETA", "PHI" };
  }

  ParticleSmear::ParticleSmear()
  :
  KinematicMapFolder(),
  _random()
  {
  appendClassName("ParticleSmear");
  setName("ParticleSmear");
  setTitle("ParticleSmear");
  for (int o=0; o<3; o++) addMap(observableKey[o],KinematicMap(),0.0);                     // resolutions: >= 0
  for (int o=0; o<3; o++) addMap(String(observableKey[o])+"_BIAS",KinematicMap());         // biases
  for (int o=0; o<3; o++) _distribution[o] = kGaussian;
  }

  void ParticleSmear::setDistribution(Observable o, Distribution d)
  {
  if (d==kCustom) throw Exception("Give the deviates of a custom distribution with setDeviate()","ParticleSmear::setDistribution()");
  _distribution[o] = d;
  _deviate[o] = Deviate();
  }

  void ParticleSmear::setDeviate(Observable o, Deviate deviate)
  {
  if (!deviate) throw Exception("No deviate function","ParticleSmear::setDeviate()");
  _distribution[o] = kCustom;
  _deviate[o] = deviate;
  }

  double ParticleSmear::deviate(Distribution d, MATH::RandomGenerator & random)
  {
  switch (d)
    {
    case kGaussian: return random.gauss(0.0,1.0);
    case kUniform:  return std::sqrt(3.0)*(2.0*random.random()-1.0);
    case kLaplace:
      {
      double u = random.positive()-0.5;                     // in (-1/2, 1/2): b = 1/sqrt 2 gives rms 1
      double s = u<0.0 ? -1.0 : 1.0;
      return -s*std::log(1.0-2.0*std::fabs(u))/std::sqrt(2.0);
      }
    case kCauchy:   return std::tan(M_PI*(random.positive()-0.5));
    default: throw Exception("A custom distribution draws with its own function","ParticleSmear::deviate()");
    }
  }

  double ParticleSmear::draw(Observable o, MATH::RandomGenerator & random) const
  {
  return _distribution[o]==kCustom ? _deviate[o](random) : deviate(_distribution[o],random);
  }

  ParticleSmear::Distribution ParticleSmear::parseDistribution(const String & text)
  {
  std::string name = text.data();
  for (char & c : name) c = char(std::toupper(static_cast<unsigned char>(c)));
  if (name=="GAUSSIAN" || name=="GAUSS" || name.empty()) return kGaussian;
  if (name=="UNIFORM")  return kUniform;
  if (name=="LAPLACE")  return kLaplace;
  if (name=="CAUCHY")   return kCauchy;
  throw Exception(String("Unknown distribution ")+text+" (GAUSSIAN, UNIFORM, LAPLACE or CAUCHY)",
                  "ParticleSmear::parseDistribution()");
  }

  const char * ParticleSmear::distributionName(Distribution d)
  {
  static const char * const names[5] = { "GAUSSIAN", "UNIFORM", "LAPLACE", "CAUCHY", "CUSTOM" };
  return names[d];
  }

  void ParticleSmear::configure(const String & taskName, const String & typeName,
                                const Configuration & configuration, unsigned int index)
  {
  KinematicMapFolder::configure(taskName,typeName,configuration,index);
  for (int o=0; o<3; o++)
    {
    String text = configuration.valueString(createKey(taskName,typeName,index,String(observableKey[o])+":DISTRIBUTION"),
                                            true,"GAUSSIAN");
    setDistribution(Observable(o),parseDistribution(text));
    }
  }

  void ParticleSmear::saveSettings(Folder & outputFile) const
  {
  for (int o=0; o<3; o++)
    writeSetting(outputFile,mapSettingName(o,"DISTRIBUTION"),distributionName(_distribution[o]));
  }

  void ParticleSmear::loadSettings(const Folder & inputFile)
  {
  for (int o=0; o<3; o++)
    {
    String text;
    if (!readSetting(inputFile,mapSettingName(o,"DISTRIBUTION"),text) || text=="CUSTOM") continue;
    setDistribution(Observable(o),parseDistribution(text));
    }
  }

  void ParticleSmear::smear(Particle & generated, Particle & smeared, MATH::RandomGenerator & random) const
  {
  const VectorLorentz & in = generated.momentum();
  double pt   = in.perp();
  double eta  = in.pseudorapidity();
  double phi  = in.phi();
  double mass = in.modulus();
  double value[3] = { pt, eta, phi };
  double result[3] = { pt, eta, phi };
  for (int io=0; io<3; io++)
    {
    Observable o = Observable(io);
    if (!smears(o) && biasMap(o).isNone()) continue;
    double sigma = smears(o) ? resolution(o,pt,eta,phi) : 0.0;
    double shift = bias(o,pt,eta,phi);
    // a deviate is drawn for each smeared observable, whatever its resolution here: the random sequence does not
    // depend on the values of the maps (except for the pT drawn again)
    if (o!=kPt)
      {
      result[o] = value[o] + shift + (smears(o) ? sigma*draw(o,random) : 0.0);
      continue;
      }
    // pT: drawn again while not positive (truncated at 0); the generator-level pT kept if it never is
    result[o] = value[o];
    for (int attempt=0; attempt<100; attempt++)
      {
      double candidate = value[o] + shift + (smears(o) ? sigma*draw(o,random) : 0.0);
      if (candidate>0.0) { result[o] = candidate; break; }
      }
    }
  // smeared: a copy of generated (type, position, live, pid, source index), then the detector-level momentum
  smeared = generated;
  smeared.setParent(nullptr);          // a detector-level particle has no decay history of its own
  smeared.clearChildren();
  smeared._ixEtaPhi = -1;              // bin indices of the generator-level momentum: set again by the analyzers
  smeared._ixYPhi   = -1;
  // smeared (pT, eta, phi) at the same mass: pz = pT sinh(eta) (eta is the pseudorapidity, not the rapidity)
  smeared.momentum().setMPhiRhoZ(mass,result[kPhi],result[kPt],result[kPt]*std::sinh(result[kEta]));
  smeared.setTruth(&generated);
  }

} // namespace CAP

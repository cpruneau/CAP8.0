/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include "BasicEventGen.hpp"
#include "Exceptions.hpp"
#include "FunctionRegistry.hpp"
#include "ParameterizedComposite.hpp"
#include "MathConstants.hpp"
#include "MathBasicIntegration.hpp"
#include "InterpolationTools.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

namespace
{
  //! Number of intervals of the tabulated distributions.
  const unsigned int kTableSize = 4000;
  //! Maximum number of trials for one particle (cylindrical style with a momentum range).
  const unsigned int kMaxTrials = 100000;
}

BasicEventGen::BasicEventGen()
:
EventProcessor(),
_eventGenType(kCylindrical),
_pdgSelected({211,-211,321,-321}),
_multMin(2),
_multMax(20),
_rapidityMin(-2.0),
_rapidityMax(2.0),
_useMomentumRange(false),
_ptMin(0.0),
_ptMax(10.0),
_pMin(0.0),
_pMax(10.0),
_thetaMin(0.0),
_thetaMax(MATH::pi()),
_phiMin(0.0),
_phiMax(MATH::twoPi()),
_distribution(kExponential),
_slope(0.5),
_sigma(0.5),
_function(),
_tableX(),
_tableCdf(),
_setSeed(true),
_seedValue(14316431),
_random(),
_mass(),
_types()
{
  appendClassName("BasicEventGen");
  setMinimumReportLevel(Object::kInfo);
  setName("BasicEventGen");
  setTitle("BasicEventGen");
}

BasicEventGen::BasicEventGen(const BasicEventGen & source) = default;

BasicEventGen & BasicEventGen::operator=(const BasicEventGen & rhs) = default;

void BasicEventGen::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  _configuration.addProperty(createKey(taskName,"EventGenType"),       int(kCylindrical));
  _configuration.addProperty(createKey(taskName,"PdgSelected"),        String("211, -211, 321, -321"));
  _configuration.addProperty(createKey(taskName,"MultMin"),            2);
  _configuration.addProperty(createKey(taskName,"MultMax"),            20);
  _configuration.addProperty(createKey(taskName,"RapidityMin"),        -2.0);
  _configuration.addProperty(createKey(taskName,"RapidityMax"),         2.0);
  _configuration.addProperty(createKey(taskName,"MomentumRange"),      String("PT"));
  _configuration.addProperty(createKey(taskName,"PtMin"),               0.0);
  _configuration.addProperty(createKey(taskName,"PtMax"),              10.0);
  _configuration.addProperty(createKey(taskName,"PMin"),                0.0);
  _configuration.addProperty(createKey(taskName,"PMax"),               10.0);
  _configuration.addProperty(createKey(taskName,"ThetaMin"),            0.0);
  _configuration.addProperty(createKey(taskName,"ThetaMax"),           MATH::pi());
  _configuration.addProperty(createKey(taskName,"PhiMin"),              0.0);
  _configuration.addProperty(createKey(taskName,"PhiMax"),             MATH::twoPi());
  _configuration.addProperty(createKey(taskName,"Distribution"),       String("EXPONENTIAL"));
  _configuration.addProperty(createKey(taskName,"Slope"),               0.5);
  _configuration.addProperty(createKey(taskName,"Sigma"),               0.5);
  _configuration.addProperty(createKey(taskName,"Function:Name"),      String("NONE"));
  _configuration.addProperty(createKey(taskName,"Function:Parameters"),String(""));
  _configuration.addProperty(createKey(taskName,"Random:SetSeed"),     true);
  _configuration.addProperty(createKey(taskName,"Random:Seed"),        14316431);
}

std::vector<double> BasicEventGen::parseNumbers(const String & text, const String & key)
{
  std::vector<double> values;
  std::string list(text.c_str());
  for (char & c : list) if (c==',' || c==';' || c=='\t' || c=='\n') c = ' ';
  std::istringstream in(list);
  std::string entry;
  while (in >> entry)
    {
    std::size_t used = 0;
    double value = 0.0;
    try { value = std::stod(entry,&used); }
    catch (...) { used = 0; }
    if (used!=entry.size()) throw ConfigurationException(key,String("'")+entry.c_str()+"' is not a number",__FUNCTION__);
    values.push_back(value);
    }
  return values;
}

void BasicEventGen::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  readSettings();
  selectTypes(EventProcessor::db());
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("EventGenType",_eventGenType);
    printValue("PdgSelected",_configuration.valueString(createKey(name(),"PdgSelected")));
    printValue("MultMin",_multMin);
    printValue("MultMax",_multMax);
    printValue("Distribution",_configuration.valueString(createKey(name(),"Distribution")));
    printValue("Random:Seed",long(_random.seed()));
    }
}

void BasicEventGen::readSettings()
{
  const String taskName = name();
  auto key = [&](const char * k) { return createKey(taskName,k); };

  _eventGenType = _configuration.valueInt(   key("EventGenType"));
  _multMin      = _configuration.valueInt(   key("MultMin"));
  _multMax      = _configuration.valueInt(   key("MultMax"));
  _rapidityMin  = _configuration.valueDouble(key("RapidityMin"));
  _rapidityMax  = _configuration.valueDouble(key("RapidityMax"));
  _ptMin        = _configuration.valueDouble(key("PtMin"));
  _ptMax        = _configuration.valueDouble(key("PtMax"));
  _pMin         = _configuration.valueDouble(key("PMin"));
  _pMax         = _configuration.valueDouble(key("PMax"));
  _thetaMin     = _configuration.valueDouble(key("ThetaMin"));
  _thetaMax     = _configuration.valueDouble(key("ThetaMax"));
  _phiMin       = _configuration.valueDouble(key("PhiMin"));
  _phiMax       = _configuration.valueDouble(key("PhiMax"));
  _slope        = _configuration.valueDouble(key("Slope"));
  _sigma        = _configuration.valueDouble(key("Sigma"));
  _setSeed      = _configuration.valueBool(  key("Random:SetSeed"));
  _seedValue    = _configuration.valueLong(  key("Random:Seed"));
  String range        = _configuration.valueString(key("MomentumRange"));
  String distribution = _configuration.valueString(key("Distribution"));
  String functionName = _configuration.valueString(key("Function:Name"));
  String parameters   = _configuration.valueString(key("Function:Parameters"),true,"");
  range.toUpper();
  distribution.toUpper();

  // style and ranges
  if (_eventGenType!=kCylindrical && _eventGenType!=kSpherical)
    throw ConfigurationException(key("EventGenType"),"must be 0 (cylindrical: y, pT, phi) or 1 (spherical: p, theta, phi)",__FUNCTION__);
  if (_multMin<0 || _multMax<_multMin)
    throw ConfigurationException(key("MultMax"),"the multiplicity range must satisfy 0 <= MultMin <= MultMax",__FUNCTION__);
  if (range=="PT")     _useMomentumRange = false;
  else if (range=="P") _useMomentumRange = true;
  else throw ConfigurationException(key("MomentumRange"),"must be PT (range PtMin, PtMax) or P (range PMin, PMax)",__FUNCTION__);
  if (!(_phiMax>_phiMin))       throw ConfigurationException(key("PhiMax"),"PhiMax must exceed PhiMin",__FUNCTION__);
  if (_eventGenType==kCylindrical)
    {
    if (!(_rapidityMax>=_rapidityMin)) throw ConfigurationException(key("RapidityMax"),"RapidityMax must not be below RapidityMin",__FUNCTION__);
    if (!_useMomentumRange && !(_ptMin>=0.0 && _ptMax>_ptMin))
      throw ConfigurationException(key("PtMax"),"the transverse momentum range must satisfy 0 <= PtMin < PtMax",__FUNCTION__);
    }
  if ((_eventGenType==kSpherical || _useMomentumRange) && !(_pMin>=0.0 && _pMax>_pMin))
    throw ConfigurationException(key("PMax"),"the momentum range must satisfy 0 <= PMin < PMax",__FUNCTION__);
  if (_eventGenType==kSpherical && !(_thetaMin>=0.0 && _thetaMax<=MATH::pi()+1e-12 && _thetaMax>_thetaMin))
    throw ConfigurationException(key("ThetaMax"),"the polar angle range must satisfy 0 <= ThetaMin < ThetaMax <= pi",__FUNCTION__);

  // momentum distribution
  _function.reset();
  if (distribution=="EXPONENTIAL")
    {
    _distribution = kExponential;
    if (!(_slope>0.0)) throw ConfigurationException(key("Slope"),"must be positive",__FUNCTION__);
    }
  else if (distribution=="GAUSSIAN" || distribution=="GAUSS")
    {
    _distribution = kGaussian;
    if (!(_sigma>0.0)) throw ConfigurationException(key("Sigma"),"must be positive",__FUNCTION__);
    }
  else if (distribution=="FUNCTION")
    {
    _distribution = kFunction;
    if (!MATH::FunctionRegistry::isRegistered(functionName) && !MATH::FunctionRegistry::isComposite(functionName))
      MATH::FunctionRegistry::readComposites(MATH::FunctionRegistry::defaultCompositesFile());
    std::vector<double> values = parseNumbers(parameters,key("Function:Parameters"));
    MATH::ParameterizedFunction * function = nullptr;
    if (MATH::FunctionRegistry::isComposite(functionName))
      function = MATH::FunctionRegistry::composite(functionName)->clone();
    else if (!values.empty())
      function = MATH::FunctionRegistry::create(functionName,values.size());
    else
      function = MATH::FunctionRegistry::create(functionName);
    if (!function)
      throw ConfigurationException(key("Function:Name"),String("no registered function ")+functionName
                                   +" (with "+String(std::to_string(values.size()))+" parameters)",__FUNCTION__);
    _function.reset(function);
    if (!values.empty())
      {
      if (values.size()!=_function->nParameters())
        throw ConfigurationException(key("Function:Parameters"),String("the function has ")
                                     +String(std::to_string(_function->nParameters()))+" parameters",__FUNCTION__);
      _function->setParameterValues(values);
      }
    if (_function->rank()!=1)
      throw ConfigurationException(key("Function:Name"),"the function must be a function of one variable",__FUNCTION__);
    }
  else
    throw ConfigurationException(key("Distribution"),"must be EXPONENTIAL, GAUSSIAN or FUNCTION",__FUNCTION__);

  _tableX.clear();
  _tableCdf.clear();
  if (_distribution!=kExponential)
    {
    if (_eventGenType==kSpherical)   tabulate(_pMin,_pMax);
    else if (_useMomentumRange)      tabulate(0.0,_pMax);
    else                             tabulate(_ptMin,_ptMax);
    }

  _random.setSeed(_setSeed ? std::uint64_t(_seedValue) : 0);
}

void BasicEventGen::selectTypes(const ParticleDb & particleDb)
{
  const String key = createKey(name(),"PdgSelected");
  std::vector<double> codes = parseNumbers(_configuration.valueString(key),key);
  if (codes.empty()) throw ConfigurationException(key,"no particle type selected",__FUNCTION__);
  _pdgSelected.clear();
  _types.clear();
  _mass.clear();
  for (double code : codes)
    {
    int pdg = int(std::lround(code));
    ParticleType * type = nullptr;
    try { type = particleDb.findPdgCode(pdg); }
    catch (...) { type = nullptr; }
    if (!type) throw ConfigurationException(key,String("PDG code ")+String(std::to_string(pdg))+" is not in the particle database",__FUNCTION__);
    _pdgSelected.push_back(pdg);
    _types.push_back(type);
    _mass.push_back(type->mass());
    }
}

double BasicEventGen::density(double x) const
{
  if (_distribution==kGaussian) return std::exp(-0.5*x*x/(_sigma*_sigma));
  double f = _function->evaluateAt(x);
  return (std::isfinite(f) && f>0.0) ? f : 0.0;
}

void BasicEventGen::tabulate(double low, double high)
{
  _tableX.resize(kTableSize+1);
  _tableCdf.resize(kTableSize+1);
  const double step = (high-low)/kTableSize;
  std::vector<double> values(kTableSize+1);
  for (unsigned int k=0; k<=kTableSize; k++)
    {
    _tableX[k] = low + k*step;
    values[k]  = density(_tableX[k]);
    }
  _tableCdf = MATH::cumulativeTrapezoidRule(values,step);   // trapezoidal rule
  if (!(_tableCdf.back()>0.0))
    throw ConfigurationException(createKey(name(),"Distribution"),"the momentum distribution is zero on the range",__FUNCTION__);
}

double BasicEventGen::drawMomentum(double low, double high)
{
  if (_distribution==kExponential) return _random.exp(_slope,low,high);
  // inverse of the tabulated cumulative (the table covers [low, high])
  double u = _random.random()*_tableCdf.back();
  return MATH::inverseLinearInterpolation(_tableX,_tableCdf,u);
}

void BasicEventGen::generateCylindrical(double m, MATH::VectorLorentz & momentum)
{
  const double m2 = m*m;
  for (unsigned int trial=0; trial<kMaxTrials; trial++)
    {
    double phi = _random.uniform(_phiMin,_phiMax);
    double y   = _random.uniform(_rapidityMin,_rapidityMax);
    double pt  = _useMomentumRange ? drawMomentum(0.0,_pMax) : drawMomentum(_ptMin,_ptMax);
    double mt  = std::sqrt(m2 + pt*pt);
    double pz  = mt*std::sinh(y);
    if (_useMomentumRange)
      {
      double p = std::sqrt(pt*pt + pz*pz);
      if (p<_pMin || p>_pMax) continue;
      }
    momentum.setTXYZ(mt*std::cosh(y),pt*std::cos(phi),pt*std::sin(phi),pz);
    return;
    }
  throw Exception("No momentum in the range [PMin, PMax] after many trials: check the rapidity and momentum ranges",__FUNCTION__);
}

void BasicEventGen::generateSpherical(double m, MATH::VectorLorentz & momentum)
{
  double phi      = _random.uniform(_phiMin,_phiMax);
  double cosTheta = _random.uniform(std::cos(_thetaMax),std::cos(_thetaMin));   // isotropic
  double sinTheta = std::sqrt(std::max(0.0,1.0-cosTheta*cosTheta));
  double p        = drawMomentum(_pMin,_pMax);
  momentum.setTXYZ(std::sqrt(m*m+p*p),p*sinTheta*std::cos(phi),p*sinTheta*std::sin(phi),p*cosTheta);
}

void BasicEventGen::execute()
{
  Event & theEvent = event();
  MATH::VectorLorentz momentum;
  MATH::VectorLorentz position;
  position.setTXYZ(0.0, 0.0, 0.0, 0.0);
  const auto & filters = particleFilters();
  for (unsigned int iType = 0; iType<_types.size(); iType++)
    {
    long nParticles = _random.uniformInteger(_multMin,_multMax);
    for (long i = 0; i < nParticles; i++)
      {
      if (_eventGenType==kSpherical) generateSpherical(_mass[iType],momentum);
      else                           generateCylindrical(_mass[iType],momentum);
      Particle & particle = Particle::factory().nextObject();
      particle.reset();
      particle.setType(_types[iType]);
      particle.setLive(1);
      particle.setMomentum(momentum);
      particle.setPosition(position);
      bool accepted = filters.empty();
      for (auto & particleFilter : filters)
        if (particleFilter->accept(particle)) { accepted = true; break; }
      if (accepted) theEvent.addParticle(&particle);
      }
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

} // namespace CAP

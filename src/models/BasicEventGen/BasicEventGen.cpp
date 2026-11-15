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
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "HistogramFolder.hpp"
#include "ManagedObjects.hpp"
#include <memory>

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
_multDistribution(kMultUniform),
_multValues(),
_multCdf(),
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
_types(),
_vn(),
_flowPtScale(0.0),
_psi(0.0)
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
  _configuration.addProperty(createKey(taskName,"Flow:Vn"),            String(""));
  _configuration.addProperty(createKey(taskName,"Mult:Distribution"),        String("UNIFORM"));
  _configuration.addProperty(createKey(taskName,"Mult:Function:Name"),       String("NONE"));
  _configuration.addProperty(createKey(taskName,"Mult:Function:Parameters"), String(""));
  _configuration.addProperty(createKey(taskName,"Mult:Histogram:Path"),      String("DEFAULT"));
  _configuration.addProperty(createKey(taskName,"Mult:Histogram:File"),      String("NONE"));
  _configuration.addProperty(createKey(taskName,"Mult:Histogram:Name"),      String("NONE"));
  _configuration.addProperty(createKey(taskName,"Flow:PtScale"),        0.0);
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
    printValue("Mult:Distribution",_configuration.valueString(createKey(name(),"Mult:Distribution"),true,"UNIFORM"));
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
  String flow   = _configuration.valueString(key("Flow:Vn"),true,"");
  _vn           = flow.isEmpty() ? std::vector<double>() : parseNumbers(flow,key("Flow:Vn"));
  _flowPtScale  = _configuration.valueDouble(key("Flow:PtScale"));
  double vnSum  = 0.0;
  for (double v : _vn) vnSum += std::fabs(v);
  if (!(vnSum<0.5)) throw ConfigurationException(key("Flow:Vn"),"the sum of the |v_n| must be below 1/2",__FUNCTION__);
  if (_flowPtScale<0.0) throw ConfigurationException(key("Flow:PtScale"),"must not be negative",__FUNCTION__);

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
    _function.reset(makeFunction(functionName,parameters,key("Function:Name"),key("Function:Parameters")));
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

  readMultiplicity();
  _random.setSeed(_setSeed ? std::uint64_t(_seedValue) : 0);
}
MATH::ParameterizedFunction * BasicEventGen::makeFunction(const String & functionName, const String & parameters,
                                                          const String & nameKey, const String & parametersKey)
{
  if (!MATH::FunctionRegistry::isRegistered(functionName) && !MATH::FunctionRegistry::isComposite(functionName))
    MATH::FunctionRegistry::readComposites(MATH::FunctionRegistry::defaultCompositesFile());
  std::vector<double> values = parseNumbers(parameters,parametersKey);
  MATH::ParameterizedFunction * function = nullptr;
  if (MATH::FunctionRegistry::isComposite(functionName))
    function = MATH::FunctionRegistry::composite(functionName)->clone();
  else if (!values.empty())
    function = MATH::FunctionRegistry::create(functionName,values.size());
  else
    function = MATH::FunctionRegistry::create(functionName);
  if (!function)
    throw ConfigurationException(nameKey,String("no registered function ")+functionName
                                 +" (with "+String(std::to_string(values.size()))+" parameters)","makeFunction");
  std::unique_ptr<MATH::ParameterizedFunction> owned(function);
  if (!values.empty())
    {
    if (values.size()!=owned->nParameters())
      throw ConfigurationException(parametersKey,String("the function has ")
                                   +String(std::to_string(owned->nParameters()))+" parameters","makeFunction");
    owned->setParameterValues(values);
    }
  if (owned->rank()!=1)
    throw ConfigurationException(nameKey,"the function must be a function of one variable","makeFunction");
  return owned.release();
}

void BasicEventGen::readMultiplicity()
{
  const String taskName = name();
  auto key = [&](const char * k) { return createKey(taskName,k); };
  String type = _configuration.valueString(key("Mult:Distribution"),true,"UNIFORM");
  type.toUpper();
  _multValues.clear();
  _multCdf.clear();
  if (type=="UNIFORM")
    {
    _multDistribution = kMultUniform;
    return;
    }
  if (type=="FUNCTION")
    {
    _multDistribution = kMultFunction;
    std::unique_ptr<MATH::ParameterizedFunction> f(makeFunction(
        _configuration.valueString(key("Mult:Function:Name"),true,"NONE"),
        _configuration.valueString(key("Mult:Function:Parameters"),true,""),
        key("Mult:Function:Name"),key("Mult:Function:Parameters")));
    std::vector<long> values;
    std::vector<double> weights;
    for (long n=_multMin; n<=_multMax; n++)
      {
      double w = f->evaluateAt(double(n));
      values.push_back(n);
      weights.push_back(std::isfinite(w) && w>0.0 ? w : 0.0);
      }
    setMultiplicityTable(values,weights,key("Mult:Function:Name"));
    return;
    }
  if (type=="HISTOGRAM")
    {
    _multDistribution = kMultHistogram;
    String path = FileSystem::configuredPath(_configuration.valueString(key("Mult:Histogram:Path"),true,"DEFAULT"),
                                             "CAP_HISTOS_IMPORT_PATH");
    String file = _configuration.valueString(key("Mult:Histogram:File"),true,"NONE");
    String histogramName = _configuration.valueString(key("Mult:Histogram:Name"),true,"NONE");
    if (file.isEmpty() || file=="NONE")
      throw ConfigurationException(key("Mult:Histogram:File"),"no file named",__FUNCTION__);
    if (histogramName.isEmpty() || histogramName=="NONE")
      throw ConfigurationException(key("Mult:Histogram:Name"),"no histogram named",__FUNCTION__);
    std::unique_ptr<CapFile> input;
    try { input.reset(CapFile::openOldCapFile(path,ManagedObjects<HistogramFolder>::capFileName(file))); }
    catch (const std::exception & e)
      {
      throw ConfigurationException(key("Mult:Histogram:File"),String("cannot open ")+file+" in "+path+": "+e.what(),
                                   __FUNCTION__);
      }
    const MATH::Histogram * h = input->folder().get<MATH::Histogram>(histogramName);
    if (!h)
      throw ConfigurationException(key("Mult:Histogram:Name"),String("no histogram ")+histogramName+" in "+file,
                                   __FUNCTION__);
    if (h->rank()!=1)
      throw ConfigurationException(key("Mult:Histogram:Name"),String("histogram ")+histogramName
                                   +" is not of one variable",__FUNCTION__);
    const std::vector<double> contents = h->binValues();
    std::vector<long> values;
    std::vector<double> weights;
    for (unsigned int k=0; k<h->nBins(0) && k<contents.size(); k++)
      {
      double w = contents[k];
      if (!(std::isfinite(w) && w>0.0)) continue;                 // empty bins (flagged) and negative contents
      double low = h->binLowEdge(k), high = h->binHighEdge(k);
      long first = long(std::ceil(low)), last = long(std::ceil(high))-1;   // the integers in [low, high)
      if (first<0) first = 0;
      if (last<first)                                              // none: the integer nearest to the centre
        {
        long n = std::lround(0.5*(low+high));
        if (n<0) continue;
        first = last = n;
        }
      double share = w/double(last-first+1);
      for (long n=first; n<=last; n++) { values.push_back(n); weights.push_back(share); }
      }
    setMultiplicityTable(values,weights,key("Mult:Histogram:Name"));
    return;
    }
  throw ConfigurationException(key("Mult:Distribution"),"must be UNIFORM, FUNCTION or HISTOGRAM",__FUNCTION__);
}

void BasicEventGen::setMultiplicityTable(const std::vector<long> & values, const std::vector<double> & weights,
                                         const String & key)
{
  _multValues.clear();
  _multCdf.clear();
  double sum = 0.0;
  for (unsigned int k=0; k<values.size(); k++)
    {
    if (!(weights[k]>0.0)) continue;
    sum += weights[k];
    _multValues.push_back(values[k]);
    _multCdf.push_back(sum);
    }
  if (!(sum>0.0)) throw ConfigurationException(key,"the multiplicity distribution is zero",__FUNCTION__);
}

long BasicEventGen::drawMultiplicity()
{
  if (_multDistribution==kMultUniform) return _random.uniformInteger(_multMin,_multMax);
  double u = _random.random()*_multCdf.back();
  unsigned int k = std::upper_bound(_multCdf.begin(),_multCdf.end(),u) - _multCdf.begin();
  if (k>=_multValues.size()) k = _multValues.size()-1;
  return _multValues[k];
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

double BasicEventGen::drawPhi(double pt)
{
  if (_vn.empty()) return _random.uniform(_phiMin,_phiMax);
  double scale = (_flowPtScale>0.0) ? std::min(pt/_flowPtScale,1.0) : 1.0;
  double maximum = 1.0;
  for (double v : _vn) maximum += 2.0*std::fabs(v)*scale;
  for (;;)
    {
    double phi = _random.uniform(_phiMin,_phiMax);
    double f = 1.0;
    for (unsigned int n=1; n<=_vn.size(); n++) f += 2.0*_vn[n-1]*scale*std::cos(n*(phi-_psi));
    if (_random.random()*maximum<f) return phi;
    }
}

void BasicEventGen::generateCylindrical(double m, MATH::VectorLorentz & momentum)
{
  const double m2 = m*m;
  for (unsigned int trial=0; trial<kMaxTrials; trial++)
    {
    double phi = _vn.empty() ? _random.uniform(_phiMin,_phiMax) : 0.0;
    double y   = _random.uniform(_rapidityMin,_rapidityMax);
    double pt  = _useMomentumRange ? drawMomentum(0.0,_pMax) : drawMomentum(_ptMin,_ptMax);
    if (!_vn.empty()) phi = drawPhi(pt);
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
  double phi      = _vn.empty() ? _random.uniform(_phiMin,_phiMax) : 0.0;
  double cosTheta = _random.uniform(std::cos(_thetaMax),std::cos(_thetaMin));   // isotropic
  double sinTheta = std::sqrt(std::max(0.0,1.0-cosTheta*cosTheta));
  double p        = drawMomentum(_pMin,_pMax);
  if (!_vn.empty()) phi = drawPhi(p*sinTheta);
  momentum.setTXYZ(std::sqrt(m*m+p*p),p*sinTheta*std::cos(phi),p*sinTheta*std::sin(phi),p*cosTheta);
}

void BasicEventGen::execute()
{
  Event & theEvent = event();
  MATH::VectorLorentz momentum;
  MATH::VectorLorentz position;
  position.setTXYZ(0.0, 0.0, 0.0, 0.0);
  const auto & filters = particleFilters();
  if (!_vn.empty()) _psi = _random.uniform(0.0,MATH::twoPi());
  for (unsigned int iType = 0; iType<_types.size(); iType++)
    {
    long nParticles = drawMultiplicity();
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

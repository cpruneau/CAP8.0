#include "Model.hpp"
#include "PhysicsConstants.hpp"
#include "MathBasicFunctions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "ParticleTypeFilter.hpp"
#include "ParticleFilter.hpp"
#include <fstream>
#include "TMath.h"

using namespace std;
using CAP::MATH::twoPiCube;
using CAP::MATH::twoPi;


namespace CAP
{

  Model::Model()
  :
  ModelSettings(),
  Thermodynamics(),
  Hypersurface(),
  _maxIntegrandSafety(1.2),
  _nOverflows(0)
  {    }

  void Model::configure(const String & taskName,
                        const String & typeName,
                        const Configuration & configuration)
  {
  _modelType      = configuration.valueInt(  createKey(taskName,typeName,"Type"));
  _modelSubtype   = configuration.valueInt(  createKey(taskName,typeName,"Subtype"));
  setTemperature(configuration.valueDouble(  createKey(taskName,typeName,"Thermodynamics:Temperature")));
  setMuB(        configuration.valueDouble(  createKey(taskName,typeName,"Thermodynamics:MuB")));
  setMuI(        configuration.valueDouble(  createKey(taskName,typeName,"Thermodynamics:MuI")));
  setMuS(        configuration.valueDouble(  createKey(taskName,typeName,"Thermodynamics:MuS")));
  setMuC(        configuration.valueDouble(  createKey(taskName,typeName,"Thermodynamics:MuC")));
  _pRapidityRange = configuration.valueDouble(createKey(taskName,typeName,"pRapidityRange"));
  _sRapidityRange = configuration.valueDouble(createKey(taskName,typeName,"sRapidityRange"));
  _rhoMax         = configuration.valueDouble(createKey(taskName,typeName,"rhoMax"));
  _tauI           = configuration.valueDouble(createKey(taskName,typeName,"TauI"));
  _tauC           = configuration.valueDouble(createKey(taskName,typeName,"TauC"));
  _tVelocity      = configuration.valueDouble(createKey(taskName,typeName,"tVelocity"));
  _amp            = configuration.valueDouble(createKey(taskName,typeName,"Amplitude"));
  _delay          = configuration.valueDouble(createKey(taskName,typeName,"Delay"));
  _backFlow       = configuration.valueBool(  createKey(taskName,typeName,"BackFlow"));
  _tGamma         = gammaAtBeta(_tVelocity);
  _nSamples       = configuration.valueInt(   createKey(taskName,typeName,"NSamples"),true,100000);
  _maxIntegrandSafety = configuration.valueDouble(createKey(taskName,typeName,"MaxIntegrandSafety"),true,1.2);

  setSystemName(      configuration.valueString(createKey(taskName,typeName,"SystemName")));
  setSystemEnergy(    configuration.valueDouble(createKey(taskName,typeName,"SystemEnergy")));
  setCentrality(      configuration.valueDouble(createKey(taskName,typeName,"Centrality")));
  double min = configuration.valueDouble(createKey(taskName,typeName,"CentralityMin"));
  double max = configuration.valueDouble(createKey(taskName,typeName,"CentralityMax"));
  setCentralityRange(min,max);
  min = configuration.valueDouble(createKey(taskName,typeName,"ImpactParameter"));
  max = configuration.valueDouble(createKey(taskName,typeName,"ImpactParameterMin"));
  setImpactParameter( configuration.valueDouble(createKey(taskName,typeName,"ImpactParameterMax")));
  setImpactParameterRange(min,max);
   _thermalHadronGas.configure(taskName,typeName,configuration);
  }

  void   Model::print() const
  {
  printCR();
  printLine();
  printString("Therminator 3");
  printString("Release, 1.0.0, 6 February 2026 ");
  printCR();
  printString("Authors");
  printString("    Claude A Pruneau   (aa7526@wayne.edu)");
  printCR();
  printCR();
  printString("Based on Therminator 2");
  printString("Release, 2.0.3, 1 February 2011 ");
  printCR();
  printString("Authors");
  printString("    Mikolaj Chojnacki   (Mikolaj.Chojnacki@ifj.edu.pl)");
  printString("    Adam Kisiel         (kisiel@if.pw.edu.pl)");
  printString("    Wojciech Broniowski (Wojciech.Broniowski@ifj.edu.pl)");
  printString("    Wojciech Florkowski (Wojciech.Florkowski@ifj.edu.pl)");
  printLine();
  printCR();
  }


  void Model::setHypercubeVolume()
  {
  // spatial rapidity x azimuth x radius, and momentum rapidity x zeta in [0,1) x azimuth (Therminator 2 mHyperCube)
  _sVolume = _sRapidityRange * twoPi() * _rhoMax;
  _pVolume = _pRapidityRange * 1.0 * twoPi();
  }

  void Model::calculateMultiplicities()
  {
  setHypercubeVolume();
  _volume = _sVolume * _pVolume;
  for (auto & hadron : _thermalHadronGas.hadrons())
    {
    double maxIntegrand = 0.0;
    double sum = 0.0;
    for (unsigned int iSample = 0; iSample < _nSamples; iSample++)
      {
      double integrand = calculateIntegrand(hadron);
      if (integrand>maxIntegrand) maxIntegrand = integrand;
      sum += integrand;
      }
    hadron.setIntegrand(maxIntegrand);
    hadron.setAvgMultiplicity(_volume*sum/_nSamples);
    }
  }

  double Model::calculateIntegrand(ThermalHadron & hadron __attribute__((unused)))
  {
  // each model (ModelHadronGas, ModelBlastWave, ...) has its own integrand
  throw Exception("Model has no integrand: use one of the Model* classes",__FUNCTION__);
  }


  void Model::generateEvent(Event & event,const ParticleFilter & particleFilter)
  {
  std::vector<ThermalHadron> & hadrons = _thermalHadronGas.hadrons();

  // choose (scaled down) _mult of this event
  double multFraction = 1.0;
  if (_multFractionRange>0)
    multFraction = _multFractionMin + _multFractionRange*gRandom->Rndm();
  else
    multFraction = _multFractionMin;
  double mean  = 0;
  
  for (auto & hadron : hadrons)
    {
    ParticleType & type = hadron.type();
    if (type.isPhoton()) continue; // no primordial photons (as in Therminator 2)
    mean  = multFraction * hadron.avgMultiplicity();
    switch (_fluctType)
      {
        case 0: // Poisson fluctuations
        hadron.setMultiplicity(gRandom->Poisson(mean));
        break;

        case 1: // Negative Binomial  fluctuations
        hadron.setMultiplicity(0.0); // Fix me!
        break;

        default:
        case 2: // Gaussian fluctuations
        hadron.setMultiplicity(TMath::Max(0, int(gRandom->Gaus(mean,std::sqrt(mean)))));
        break;

        case 3:  // Poisson or Gaussian fluctuations
        if (mean>20)
          hadron.setMultiplicity(TMath::Max(0, int(gRandom->Gaus(mean,std::sqrt(mean)))));
        else
          hadron.setMultiplicity(TMath::Max(0, int(gRandom->Poisson(mean))));
        break;
      }

    // now generate the particles
    double   value;
    double   valueTest;
    // the maximum was sampled in calculateMultiplicities(); the safety factor covers values the sampling missed
    double   maxIntegrand = _maxIntegrandSafety * hadron.integrand();
    unsigned int mult = hadron.multiplicity();
    unsigned int iParticle = 0;
    while (iParticle < mult)
      {
      value = calculateIntegrand(hadron);
      if (value>maxIntegrand) _nOverflows++;
      valueTest = maxIntegrand * gRandom->Rndm();
      if (valueTest<value)
        {
        Particle & particle = Particle::factory().nextObject();
        particle.setType(&type);
        particle.setLive(true);
        particle.setPosition(_position);
        particle.setMomentum(_momentum);
        iParticle++;
        if (particleFilter.accept(particle)) event.addParticle(&particle);
        }
      }
    }
  }

} // namespace CAP

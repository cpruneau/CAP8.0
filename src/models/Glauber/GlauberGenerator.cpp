/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
#include "GlauberGenerator.hpp"
#include "GlauberEvent.hpp"
#include "GlauberEventMoments.hpp"
#include "Exceptions.hpp"
#include "MersenneTwister.hpp"



namespace CAP
{

  using CAP::MATH::pi;
  using CAP::MATH::twoPi;

  //TF1 *getNNProf(Double_t snn=67.6, Double_t omega=0.4, Double_t G=1);
//  Double_t      fXSectOmega;     //StdDev of Nucleon-nucleon cross section
//  Double_t      fXSectLambda;    //Jacobian from tot to inelastic (Strikman)
//  Double_t      fXSectEvent;     //Event value of Nucleon-nucleon cross section
//  Double_t      fHardFrac;       //Fraction of cross section used for Nhard (def=0.65)
//  TF1          *fPTot;           //Cross section distribution
//  TF1          *fNNProf;         //NN profile (hard-sphere == 0 by default)

  GlauberGenerator::GlauberGenerator()
  :
  Task(),
  _managedDb(),
  _managedEvents(),
  _managedEventFilters(),
  _nnCollisionOption(1),
  _randomizeEP(false),
  _xSectOption(1),
  _xSectVsSOption(1),
  _sqrtS(13000.0),
  _xSection(63.0),
  _xSectionOmega(0.0),
  _xSectionG(1.0),
  _xSectionSigma(0.0),
  _minImpact(0.0),
  _maxImpact(20.0),
  _intDistanceMax(4.0),
  _hardIntDistMax(-1.0),
  _hardFraction(0.65),
  _minImpactSq(0.0),
  _maxImpactSq(400.0),
  _intDistanceMaxSq(16.0),
  _hardIntDistMaxSq(0.0),
  _nnXSectVsSqrtS(),
  _nnXSectProfile(),
  _ptotProfile()
  {
  // no ops
  }

  GlauberGenerator::GlauberGenerator(const GlauberGenerator & src)
  :
  Task(src),
  _managedDb(src._managedDb),
  _managedEvents(src._managedEvents),
  _managedEventFilters(src._managedEventFilters),
  _nnCollisionOption(src._nnCollisionOption),
  _randomizeEP(src._randomizeEP),
  _xSectOption(src._xSectOption),
  _xSectVsSOption(src._xSectVsSOption),
  _sqrtS(src._sqrtS),
  _xSection(src._xSection),
  _xSectionOmega(src._xSectionOmega),
  _xSectionG(src._xSectionG),
  _xSectionSigma(src._xSectionSigma),
  _minImpact(src._minImpact),
  _maxImpact(src._maxImpact),
  _intDistanceMax(src._intDistanceMax),
  _hardIntDistMax(src._hardIntDistMax),
  _hardFraction(src._hardFraction),
  _minImpactSq(src._minImpactSq),
  _maxImpactSq(src._maxImpactSq),
  _intDistanceMaxSq(src._intDistanceMaxSq),
  _hardIntDistMaxSq(src._hardIntDistMaxSq),
  _nnXSectVsSqrtS(src._nnXSectVsSqrtS),
  _nnXSectProfile(src._nnXSectProfile),
  _ptotProfile(src._ptotProfile)
  {
  // no ops
  }

  GlauberGenerator & GlauberGenerator::operator=(const GlauberGenerator & rhs)
  {
  if (this!=&rhs)
    {
    Task::operator=(rhs);
    _managedDb              = rhs._managedDb;
    _managedEvents          = rhs._managedEvents;
    _managedEventFilters    = rhs._managedEventFilters;
    _nnCollisionOption      = rhs._nnCollisionOption;
    _randomizeEP            = rhs._randomizeEP;
    _xSectOption            = rhs._xSectOption;
    _xSectVsSOption         = rhs._xSectVsSOption;
    _sqrtS                  = rhs._sqrtS;
    _xSection               = rhs._xSection;
    _xSectionOmega          = rhs._xSectionOmega;
    _xSectionG              = rhs._xSectionG;
    _xSectionSigma          = rhs._xSectionSigma;
    _minImpact              = rhs._minImpact;
    _maxImpact              = rhs._maxImpact;
    _intDistanceMax         = rhs._intDistanceMax;
    _hardIntDistMax         = rhs._hardIntDistMax;
    _hardFraction           = rhs._hardFraction;
    _minImpactSq            = rhs._minImpactSq;
    _maxImpactSq            = rhs._maxImpactSq;
    _intDistanceMaxSq       = rhs._intDistanceMaxSq;
    _hardIntDistMaxSq       = rhs._hardIntDistMaxSq;
    _nnXSectVsSqrtS         = rhs._nnXSectVsSqrtS;
    _nnXSectProfile         = rhs._nnXSectProfile;
    _ptotProfile            = rhs._ptotProfile;
    }
  return *this;
  }

  void GlauberGenerator::setDefaultConfiguration()
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  Task::setDefaultConfiguration();
  String taskName = name();
  String objectType;
  // the defaults of the lists first: the values below (EVENT:USE, EVENT:N, DB:USE, DB:N, ...) replace theirs
  _managedDb.setDefaultObjectConfiguration(taskName,"DB", _configuration);
  _managedEvents.setDefaultObjectConfiguration(taskName,"EVENT", _configuration);
  _managedEventFilters.setDefaultObjectConfiguration(taskName,"EVENT_FILTER", _configuration);

  _configuration.addProperty(createKey(taskName,"SEVERITY"),         "TRACE");
  _configuration.addProperty(createKey(taskName,"SUBTASK:N"),        0);
  _configuration.addProperty(createKey(taskName,"EVENT:USE"),        true);
  _configuration.addProperty(createKey(taskName,"EVENT:REQUESTED:N"),1000);
  _configuration.addProperty(createKey(taskName,"EVENT:REPORT:N"),   100);
  _configuration.addProperty(createKey(taskName,"EVENT:N"),          1);
  _configuration.addProperty(createKey(taskName,"EVENT_0:NAME"),     "Glauber");
  _configuration.addProperty(createKey(taskName,"EVENT_0:OWNER"),    true);

  _configuration.addProperty(createKey(taskName,"DB","USE"),true);
  _configuration.addProperty(createKey(taskName,"DB","IMPORT"),false);
  _configuration.addProperty(createKey(taskName,"DB","EXPORT"),false);
  _configuration.addProperty(createKey(taskName,"DB","IMPORT:PATH"),"NONE");
  _configuration.addProperty(createKey(taskName,"DB","IMPORT:FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,"DB","EXPORT:PATH"),"NONE");
  _configuration.addProperty(createKey(taskName,"DB","EXPORT:FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,"DB","N"),1);
  _configuration.addProperty(createKey(taskName,"DB_0:NAME"),"DefaultDb");
  _configuration.addProperty(createKey(taskName,"DB_0:TITLE"),"DefaultDb");
  _configuration.addProperty(createKey(taskName,"DB_0:OWNER"),true);
  _configuration.addProperty(createKey(taskName,"DB_0:FILE"),"Glauber/NucleusDb.xml");   // nuclei (in $CAP_DATABASE_PATH)

  objectType = "GENERATION";
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:NAME"),          "Pb");
  // defaults of TGlauberMC: 0.4 fm hard core, recentered (mode 1)
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:NTRIALS"),          1000);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:RECENTERING"),      true);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:RECENTERING_MODE"), 1);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:ALLOW_OVERLAP"),    false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:MIN_INTER"),        0.4);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:USE_USERSIGMA"),    false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:LATTICE"),          false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:LATTICE_TYPE"),     0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:NODE_DISTANCE"),    0.0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:SMEARING"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_A:MAX_CENTER_SHIFT"), -1.0);


  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:NAME"),          "Pb");
  // defaults of TGlauberMC: 0.4 fm hard core, recentered (mode 1)
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:NTRIALS"),          1000);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:RECENTERING"),      true);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:RECENTERING_MODE"), 1);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:ALLOW_OVERLAP"),    false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:MIN_INTER"),        0.4);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:USE_USERSIGMA"),    false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:LATTICE"),          false);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:LATTICE_TYPE"),     0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:NODE_DISTANCE"),    0.0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:SMEARING"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"NUCLEUS_B:MAX_CENTER_SHIFT"), -1.0);

  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:NNCOLLISION_OPTION"),_nnCollisionOption);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:xSECT_OPTION"),   _xSectOption);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:IMPACT_MIN"),    _minImpact);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:IMPACT_MAX"),    _maxImpact);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:RANDOMIZE_EP"),  _randomizeEP);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:SQRT_S"),        _sqrtS);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:SQRT_S_OPTION"), _xSectVsSOption);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:xSECTION"),      _xSection);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:xSECTION_OMEGA"),_xSectionOmega);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:xSECTION_G"),    _xSectionG);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:xSECTION_SIGMA"),_xSectionSigma);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:DISTANCE_MAX"),  _intDistanceMax);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:HARD_DISTANCE_MAX"),_hardIntDistMax);
  _configuration.addProperty(createKey(taskName,objectType,"INTERACTION:HARD_FRACTION"),   _hardFraction);
  //_configuration.addProperty(createKey(taskName,objectType,"INTERACTION:SMEARINGPROFILE:NUCLEONSIGMA"),1.0);
//  _configuration.addProperty(createKey(taskName,objectType,"PTOTPROFILE:NNXSECT"),         1.0);
//  _configuration.addProperty(createKey(taskName,objectType,"PTOTPROFILE:NNXSECTOMEGA"),    1.0);
//  _configuration.addProperty(createKey(taskName,objectType,"PTOTPROFILE:NNXSECTLAMBDA"),   1.0);

  }


  void GlauberGenerator::configure(const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  Task::configure(configuration);
  _managedDb.configureObjects(configuration);
  _managedEvents.configureObjects(configuration);
  _managedEventFilters.configureObjects(configuration);
  }

  void GlauberGenerator::initialize()
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  GlauberDb & glauberDb = db();
  GlauberEvent & glauberEvent = event();

  String taskName = name();
  String objectType = "GENERATION";
  bool verbose = reportInfo(__FUNCTION__);
  if (verbose)
    {
    printCR();
    printValue("taskName",taskName);
    printValue("objectType",objectType);
    }

  // the two nuclei: type from the database, then the placement of their nucleons
  auto setUpNucleus = [&](GlauberNucleus & nucleus, const String & which)
    {
    String nucleusName = _configuration.valueString(createKey(taskName,objectType,which+":NAME"));
    glauberDb.findNucleus(nucleusName, nucleus.type());
    nucleus.setNucleus();
    bool   recenter      = _configuration.valueBool(  createKey(taskName,objectType,which+":RECENTERING"));
    int    recenterMode  = _configuration.valueInt(   createKey(taskName,objectType,which+":RECENTERING_MODE"));
    bool   allowOverlap  = _configuration.valueBool(  createKey(taskName,objectType,which+":ALLOW_OVERLAP"));
    double minDistance   = _configuration.valueDouble(createKey(taskName,objectType,which+":MIN_INTER"));
    bool   lattice       = _configuration.valueBool(  createKey(taskName,objectType,which+":LATTICE"));
    double maxShift      = _configuration.valueDouble(createKey(taskName,objectType,which+":MAX_CENTER_SHIFT"));
    nucleus.setRecenteringMode(recenter ? recenterMode : 0);
    nucleus.setAllowOverlap(allowOverlap);
    nucleus.setMinInterNucleonDistance(minDistance);
    nucleus.setMaxNumberTrials(  _configuration.valueInt(   createKey(taskName,objectType,which+":NTRIALS")));
    nucleus.setUserSigmaMode(    _configuration.valueBool(  createKey(taskName,objectType,which+":USE_USERSIGMA")));
    nucleus.setUseLattice(lattice);
    nucleus.setLatticeType(      _configuration.valueInt(   createKey(taskName,objectType,which+":LATTICE_TYPE")));
    nucleus.setNodeDistance(     _configuration.valueDouble(createKey(taskName,objectType,which+":NODE_DISTANCE")));
    nucleus.setSmearing(         _configuration.valueDouble(createKey(taskName,objectType,which+":SMEARING")));
    if (maxShift>0.0) nucleus.setMaxCenterShift(maxShift);   // otherwise the default of the nucleus type
    if (lattice && !(nucleus.nodeDistance()>0.0))
      throw InvalidValueGlauberException(nucleus.nodeDistance(),which+":LATTICE needs "+which+":NODE_DISTANCE > 0",__FUNCTION__);
    nucleus.generateNucleons();
    if (verbose)
      {
      printValue(which+" name",nucleusName);
      printValue(which+" recentering mode",nucleus.recenteringMode());
      printValue(which+" allow overlap",allowOverlap);
      printValue(which+" minimum distance",minDistance);
      printValue(which+" lattice",lattice);
      }
    };
  setUpNucleus(glauberEvent.nucleusA(), "NUCLEUS_A");
  setUpNucleus(glauberEvent.nucleusB(), "NUCLEUS_B");

  _nnCollisionOption  = _configuration.valueInt(   createKey(taskName,objectType,"INTERACTION:NNCOLLISION_OPTION"));
  _minImpact          = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:IMPACT_MIN"));
  _maxImpact          = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:IMPACT_MAX"));
  _randomizeEP        = _configuration.valueBool(  createKey(taskName,objectType,"INTERACTION:RANDOMIZE_EP"));
  _intDistanceMax     = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:DISTANCE_MAX"));
  _hardIntDistMax     = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:HARD_DISTANCE_MAX"));
  _hardFraction       = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:HARD_FRACTION"));
  _minImpactSq        = _minImpact*_minImpact;
  _maxImpactSq        = _maxImpact*_maxImpact;
  _intDistanceMaxSq   = _intDistanceMax*_intDistanceMax;
  _hardIntDistMaxSq   = _hardIntDistMax*_hardIntDistMax;

  // nucleon-nucleon inelastic cross section: from sqrt(s) (xSECT_OPTION 1) or given (2)
  _xSectOption = _configuration.valueInt(createKey(taskName,objectType,"INTERACTION:xSECT_OPTION"));
  switch (_xSectOption)
    {
      case 1:
      {
      _sqrtS           = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:SQRT_S"));
      _xSectVsSOption  = _configuration.valueInt(   createKey(taskName,objectType,"INTERACTION:SQRT_S_OPTION"));
      delete _nnXSectVsSqrtS;
      _nnXSectVsSqrtS = new NNXSectVsSqrtS();
      _nnXSectVsSqrtS->setOption(_xSectVsSOption);
      _xSection = _nnXSectVsSqrtS->evaluate(_sqrtS);
      break;
      }
      case 2:
      _xSection = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:xSECTION"));
      break;
      default:
      throw InvalidValueGlauberException(_xSectOption,"INTERACTION:xSECT_OPTION must be 1 or 2",__FUNCTION__);
    }
  // collision profile: hard sphere (omega = 0) or gamma profile (0 < omega <= 1)
  _xSectionOmega = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:xSECTION_OMEGA"));
  _xSectionG     = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:xSECTION_G"));
  _xSectionSigma = _configuration.valueDouble(createKey(taskName,objectType,"INTERACTION:xSECTION_SIGMA"));
  delete _nnXSectProfile;
  _nnXSectProfile = new NNProfile();
  _nnXSectProfile->setParameters(_xSection,_xSectionOmega,_xSectionG);
  delete _ptotProfile;
  _ptotProfile = nullptr;
  switch (_nnCollisionOption)
    {
      case 1: // fixed cross section
      break;
      case 2: // cross section fluctuating event by event (the same for all the nucleon pairs of an event)
      case 3: // cross section of each nucleon fluctuating independently (TGlauberMC AAGG)
      if (!(_xSectionSigma>0.0))
        throw InvalidValueGlauberException(_xSectionSigma,"INTERACTION:NNCOLLISION_OPTION 2 and 3 need INTERACTION:xSECTION_SIGMA > 0",
                                           __FUNCTION__);
      _ptotProfile = new PTotProfile();
      _ptotProfile->setParametersWithMean(_xSection,_xSectionSigma);
      break;
      default:
      throw InvalidValueGlauberException(_nnCollisionOption,"INTERACTION:NNCOLLISION_OPTION must be 1, 2 or 3",__FUNCTION__);
    }
  if (verbose)
    {
    printValue("nnCollisionOption",_nnCollisionOption);
    printValue("xSection (mb)",_xSection);
    printValue("xSectionOmega",_xSectionOmega);
    printValue("xSectionG",_xSectionG);
    printValue("xSectionSigma",_xSectionSigma);
    printValue("minImpact",_minImpact);
    printValue("maxImpact",_maxImpact);
    printValue("randomizeEP",_randomizeEP);
    printValue("intDistanceMax",_intDistanceMax);
    printValue("hardIntDistMax",_hardIntDistMax);
    printValue("hardFraction",_hardFraction);
    }
  }

  //!
  //! One event (TGlauberMC::NextEvent and CalcEvent): the nucleons of both nuclei placed, the impact parameter drawn
  //! uniformly in b^2, nucleus A shifted by -b/2 and B by +b/2 along x (the impact vector rotated randomly with
  //! RANDOMIZE_EP), then each pair of nucleons closer than DISTANCE_MAX in the transverse plane collides with the
  //! probability of the collision profile at their distance. Option 2 draws the cross section of the event, option 3
  //! that of each nucleon (a pair collides within the disk of their mean cross section, then with the probability
  //! of the profile when omega > 0). A collision is hard when the pair is closer than HARD_DISTANCE_MAX, or, when
  //! that is not positive, than sqrt(HARD_FRACTION sigma/(10 pi)) (TGlauberMC).
  //!
  void GlauberGenerator::execute()
  {
  GlauberEvent & ev = event();
  GlauberNucleus & nucleusA = ev.nucleusA();
  GlauberNucleus & nucleusB = ev.nucleusB();
  CAP::MATH::RandomGenerator & generator = CAP::MATH::randomGenerator();
  ev.reset();
  nucleusA.positionNucleons();
  nucleusB.positionNucleons();
  std::vector<GlauberPoint> & interactionPositions = ev.interactionPositions();
  interactionPositions.clear();
  double impact = std::sqrt(_minImpactSq + (_maxImpactSq-_minImpactSq)*generator.random());
  ev.setImpactParameter(impact);

  VectorLorentz shiftA(0.0,-impact/2.0,0.0,0.0);
  VectorLorentz shiftB(0.0,impact/2.0,0.0,0.0);
  if (_randomizeEP)
    {
    double eventPlaneAngle = twoPi()*generator.random();
    shiftA.rotateZ(eventPlaneAngle);
    shiftB.rotateZ(eventPlaneAngle);
    }
  nucleusA.shiftNucleus(shiftA);
  nucleusB.shiftNucleus(shiftB);

  double eventXSection = _xSection;
  NNProfile * profile = _nnXSectProfile;
  if (_nnCollisionOption==2)
    {
    eventXSection = _ptotProfile->random();
    _eventProfile.setParameters(eventXSection,_xSectionOmega,_xSectionG);
    profile = &_eventProfile;
    }
  else if (_nnCollisionOption==3)
    {
    eventXSection = _ptotProfile->random();
    for (auto & nucleonA : nucleusA.allNucleons()) nucleonA.setXSect(_ptotProfile->random());
    for (auto & nucleonB : nucleusB.allNucleons()) nucleonB.setXSect(_ptotProfile->random());
    }
  ev.setEventXSection(eventXSection);
  const double hardDistance = (_hardIntDistMax>0.0) ? _hardIntDistMax : std::sqrt(_hardFraction*eventXSection/(10.0*pi()));

  for (auto & nucleonA : nucleusA.allNucleons())
    {
    for (auto & nucleonB : nucleusB.allNucleons())
      {
      double distanceAB = nucleonA.distanceTo2D(nucleonB);
      if (distanceAB>_intDistanceMax) continue;
      if (_nnCollisionOption==3)
        {
        double d = std::sqrt(0.05*(nucleonA.xSect()+nucleonB.xSect())/pi()); // disk of the mean cross section (fm)
        if (distanceAB>d) continue;
        if (_xSectionOmega>0.0 && generator.random()>profile->evaluate(distanceAB)) continue;
        }
      else
        {
        double probability = profile->evaluate(distanceAB);
        if (probability<=0.0) continue;
        if (probability<1.0 && generator.random()>probability) continue;
        }
      bool isHard = distanceAB < hardDistance;
      ev.incrementColls(nucleonA,nucleonB,isHard);
      const VectorLorentz & positionA = nucleonA.position();
      const VectorLorentz & positionB = nucleonB.position();
      interactionPositions.push_back(GlauberPoint(0.5*(positionA.x()+positionB.x()),0.5*(positionA.y()+positionB.y())));
      }
    }
  }

  void GlauberGenerator::finalize()
  {
  // fix me...
  }

  void GlauberGenerator::reset()
  {
  // fix me...
  }

  void GlauberGenerator::clear()
  {
  // fix me...
  }

  void GlauberGenerator::print() const
  {
  // fix me...
  }

  const GlauberDb & GlauberGenerator::db() const
  {
  //printString("GlauberGenerator::db() const");
  return *_managedDb.objectAt(0);
  }

  const std::vector<GlauberDb*>   & GlauberGenerator::dbs() const
  {
  //printString("GlauberGenerator::dbs() const");
  return _managedDb.getObjects();
  }

  const GlauberEvent & GlauberGenerator::event() const
  {
  //printString("GlauberGenerator::event() const");
  return *_managedEvents.objectAt(0);
  }

  const std::vector<GlauberEvent*> & GlauberGenerator::events() const
  {
  //printString("GlauberGenerator::events() const");
  return _managedEvents.getObjects();
  }

  const std::vector<GlauberEventFilter*> & GlauberGenerator::eventFilters() const
  {
  //printString("GlauberGenerator::eventFilters() const");
  return _managedEventFilters.getObjects();
  }

  GlauberDb & GlauberGenerator::db()
  {
  //printString("GlauberGenerator::db()");
  return *_managedDb.objectAt(0);
  }

  std::vector<GlauberDb*> & GlauberGenerator::dbs()
  {
  //printString("GlauberGenerator::dbs()");
  return _managedDb.getObjects();
  }

  GlauberEvent & GlauberGenerator::event()
  {
  //printString("GlauberGenerator::event()");
  return *_managedEvents.objectAt(0);
  }

  std::vector<GlauberEvent*> & GlauberGenerator::events()
  {
  //printString("GlauberGenerator::events()");
  return _managedEvents.getObjects();
  }


} // namespace CAP

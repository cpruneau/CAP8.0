#include "ModelHydro3D.hpp"
#include "XmlDocument.hpp"
#include "XmlScalarField.hpp"
#include "FileSystem.hpp"
#include "NameHelpers.hpp"


namespace CAP
{

  ModelHydro3D::ModelHydro3D()
  :
  Model()
  {   }

  void ModelHydro3D::configure(const String & taskName,
                         const String & typeName,
                         const Configuration & configuration)
  {
  Model::configure(taskName,typeName,configuration);
  String path = FileSystem::configuredPath(configuration.valueString(createKey(taskName,typeName,"HYPERS:IMPORT:PATH")),
                                           "CAP_HYPERSURFACE_PATH");
  String fileName = configuration.valueString(createKey(taskName,typeName,"HYPERS:IMPORT:FILE_NAME"));
  readFromXmlFile(path.c_str(),fileName.c_str());
  }

  double ModelHydro3D::calculateIntegrand(ThermalHadron & hadron)
  {
  // Type of _statistics Bose-Einstein or Fermi-Dirac
  ParticleType & type = hadron.type();
  double sf    = type.spinFactor();
  double stat  = type.statistics();
  double mass  = type.mass();
  double mass2 = mass*mass;
  double chemPotential = chemicalPotential(type);
  double zeta    = _distanceField.randomX( gRandom->Rndm() );
  double sPhi    = _distanceField.randomY( gRandom->Rndm() );
  double theta   = _distanceField.randomZ( gRandom->Rndm() );;
  double dhs     = _distanceField.interpolate(zeta,sPhi,theta);
  double dDdZeta = _distanceFieldDZeta.interpolate(zeta,sPhi,theta);
  double dDdPhi  = _distanceFieldDPhi.interpolate(zeta,sPhi,theta);
  double dDdTheta= _distanceFieldDTheta.interpolate(zeta,sPhi,theta);
  double uxFluidVelocity       = uxFluidVelocityField.interpolate(zeta,sPhi,theta);
  double uyFluidVelocity       = uyFluidVelocityField.interpolate(zeta,sPhi,theta);
  double rapidityFluidVelocity = rapidityFluidField.interpolate(zeta,sPhi,theta);
  double tau       = _tauI + dhs*std::sin(theta)*std::sin(zeta);
  double rho       = dhs * std::sin(theta)*std::cos(zeta);
  double sRapidity = dhs * std::cos(theta)/_lambda;
  _position.setTXYZ(tau *std::cosh(sRapidity),rho*std::cos(sPhi),rho*std::sin(sPhi),tau *std::sinh(sRapidity));

  // pT = u/(1-u): its own random number, zeta must stay the hypersurface angle used by dSigmaP below
  double u         = gRandom->Rndm();
  double zetac     = (u>0.9999999) ? 0.00000001 : 1.00-u;
  double pT        = u/zetac;
  double dPt       = 1.0/(zetac*zetac);
  double pPhi      = twoPi() * gRandom->Rndm();
  double pRapidity = _pRapidityRange * (gRandom->Rndm() - 0.5);
  double mT        = std::sqrt(mass2+pT*pT);
  double e         = mT*std::cosh(pRapidity);
  _momentum.setTXYZ(e,pT*std::cos(pPhi),pT*std::sin(pPhi),mT*std::sinh(pRapidity));

  double pDotU = std::sqrt(1.0 + uxFluidVelocity * uxFluidVelocity + uyFluidVelocity*uyFluidVelocity)*mT*std::cosh(rapidityFluidVelocity - pRapidity) - pT*(uxFluidVelocity*std::cos(pPhi) + uyFluidVelocity*std::sin(pPhi));
  double dSigmaP =  dhs*dhs*std::sin(theta) *
  ( tau  / _lambda * (
                       dDdZeta * std::cos(zeta)* (-mT * std::cos(zeta) * std::cosh(pRapidity-sRapidity) + pT*std::sin(zeta)*std::cos(sPhi-pPhi))
                       + (dhs*std::sin(theta) - dDdTheta*std::cos(theta))*std::cos(zeta)*std::sin(theta)*(mT*std::sin(zeta)*std::cosh(pRapidity-sRapidity) + pT*std::cos(zeta)*std::cos(sPhi-pPhi))
                       + dDdPhi*pT*std::sin(sPhi-pPhi)
                       )
   +   (dhs * std::cos(theta) + dDdTheta*std::sin(theta))*std::sin(theta)*mT*std::cos(zeta)*std::sinh(pRapidity-sRapidity)
   );
  if (_backFlow)
    {
    // only particle emission directed back to the hydro-region
    dSigmaP = (dSigmaP >= 0.0) ? 0.0 : -dSigmaP;
    }
  else
    {
    // disable particle emission back to the hydro-region
    if (dSigmaP < 0.0) dSigmaP = 0.0;
    }
  double denom = stat + std::exp( (pDotU-chemPotential)/_temperature.value() );
  double integrand = sf*pT*dPt*dSigmaP/(denom*twoPiCube());
  return integrand; }

  void ModelHydro3D::setHypercubeVolume()
  {
  // zeta x phi x theta ranges of the hypersurface grid (Therminator 2 Hypersurface_Lhyquid3D)
  _sVolume = (_distanceField.maximum(0)-_distanceField.minimum(0)) * (_distanceField.maximum(1)-_distanceField.minimum(1))
           * (_distanceField.maximum(2)-_distanceField.minimum(2));
  _pVolume = _pRapidityRange * 1.0 * twoPi();
  }

  void ModelHydro3D::readFromXmlFile(const char * _inputPath,
                                      const char * _inputFileName)
  {
  double temperature, muB, muI, muS, muC;
  String systemName;
  double systemEnergy;
  double centralityMin;
  double centralityMax;
  double impactParameter;
  //double _centerT;

  printCR();
  printValue("Reading from xml file named",_inputFileName);
  printValue("from path named",_inputPath);
  XmlDocument xmlDoc = XmlDocument::read(_inputPath,_inputFileName);
  const XmlElement & root = xmlDoc.root();
  // text of the PARAMETER element with the given name attribute
  auto parameter = [&root](const char * name) -> String
    {
    const XmlElement * e = root.findFirst("PARAMETER","name",name);
    if (!e) throw XmlException(String("No PARAMETER named ")+name,"readFromXmlFile()");
    return e->text();
    };
  _tauI            = parameter("Tau_i").atof() / CAP::Physics::hBarC();       // [GeV^-1]
  _lambda          = parameter("Lambda").atof() / CAP::Physics::hBarC();      // [GeV^-1]
  temperature     = parameter("Temperature").atof() * 0.001;  // [GeV]
  muB             = parameter("Mu_B").atof() * 0.001;
  muI             = parameter("Mu_I").atof() * 0.001;
  muS             = parameter("Mu_S").atof() * 0.001;
  muC             = parameter("Mu_C").atof() * 0.001;        // [GeV]
  systemEnergy = parameter("colliding_energy").atof();    // [GeV]
  centralityMin   = parameter("centrality_min").atof();      // [%]
  centralityMax   = parameter("centrality_max").atof();     // [%]
  impactParameter = parameter("impact_parameter").atof();    // [fm]
  //_centerT = xmlDoc.getXmlContent().atof();  // [MeV]
  //deviceName      = xmlDoc.getXmlContent().data();
  systemName = parameter("colliding_system");
  _distanceField       = XmlScalarField::read(root,"Distance");      // [GeV^-1]
  uxFluidVelocityField        = XmlScalarField::read(root,"FluidUx");      // [1]
  uyFluidVelocityField        = XmlScalarField::read(root,"FluidUy");      // [1]
  rapidityFluidField  = XmlScalarField::read(root,"FluidRap");      // [1]
  setTemperature(temperature);
  setMuB(muB);
  setMuI(muI);
  setMuS(muS);
  setMuC(muC);
  setSystemName(systemName);
  setSystemEnergy(systemEnergy);
  setCentralityRange(centralityMin,centralityMax);
  setImpactParameter(impactParameter);
  // derivatives: from the file when given, otherwise by finite differences (as Therminator 2 does)
  if (root.findFirst("VECTOR3D","name","DistanceDZeta"))
    _distanceFieldDZeta = XmlScalarField::read(root,"DistanceDZeta");    // [GeV^-1/rad]
  else
    _distanceFieldDZeta = _distanceField.derivativeX("DistanceDZeta");

  if (root.findFirst("VECTOR3D","name","DistanceDPhi"))
    _distanceFieldDPhi = XmlScalarField::read(root,"DistanceDPhi");    // [GeV^-1/rad]
  else
    _distanceFieldDPhi = _distanceField.derivativeY("DistanceDPhi");

  if (root.findFirst("VECTOR3D","name","DistanceDTheta"))
    _distanceFieldDTheta = XmlScalarField::read(root,"DistanceDTheta");    // [GeV^-1/rad]
  else
    _distanceFieldDTheta = _distanceField.derivativeZ("DistanceDTheta");
  }

} // namespace CAP


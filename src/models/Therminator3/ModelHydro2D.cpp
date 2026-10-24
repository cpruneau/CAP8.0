#include "ModelHydro2D.hpp"
#include "XmlDocument.hpp"
#include "XmlScalarField.hpp"

using CAP::MATH::twoPiCube;
using CAP::MATH::twoPi;
using CAP::Physics::hBarC;

namespace CAP
{
  ModelHydro2D::ModelHydro2D()
  :
  Model()
  {    }

  double ModelHydro2D::calculateIntegrand(ThermalHadron & hadron)
  {
  // Type of _statistics Bose-Einstein or Fermi-Dirac
  ParticleType & type = hadron.type();
  double sf    = type.spinFactor();
  double stat  = type.statistics();
  double mass  = type.mass();
  double mass2 = mass*mass;
  double chemPotential = chemicalPotential(type);

  double zeta      = _distanceField.randomX(gRandom->Rndm());
  double sPhi      = _distanceField.randomY(gRandom->Rndm());
  double sRapidity = _sRapidityRange * (gRandom->Rndm() - 0.5); // * _sRapidityRange;
  double dhs       = _distanceField.interpolate(zeta,sPhi,0.0);
  double dDdPhi    = _distanceFieldDPhi.interpolate(zeta,sPhi, 0.0);
  double dDdZeta   = _distanceFieldDZeta.interpolate(zeta,sPhi, 0.0);
  double vT        = _vtFluidField.interpolate(zeta,sPhi,0.0);
  double gammaT    = gammaAtBeta(vT);
  double phiF      = _phiFluidField.interpolate(zeta,sPhi,0.0);
  double tau       = _tauI + dhs*std::sin(zeta);
  double rho       = dhs*std::cos(zeta);
  _position.setTXYZ(tau*std::cosh(sRapidity),rho*std::cos(sPhi),rho*std::sin(sPhi),tau*std::sinh(sRapidity));

  zeta      = gRandom->Rndm();
  double zetac     = (zeta>0.9999999) ? 0.00000001 : 1.00-zeta;
  double pT        = zeta/zetac;
  double dPt       = 1.0/(zetac*zetac);
  double pPhi      = twoPi() * gRandom->Rndm();
  double pRapidity = _pRapidityRange * (gRandom->Rndm() - 0.5);
  double mT        = std::sqrt(mass2+pT*pT);
  double e         = mT * std::cosh(pRapidity);
  _momentum.setTXYZ(e,pT*std::cos(pPhi),pT*std::sin(pPhi),mT*std::sinh(pRapidity));

  double pDotU   = gammaT * (mT*std::cosh(pRapidity-sRapidity) - vT*pT*std::cos(sPhi-pPhi+phiF));
  double dSigmaP =  dhs*(_tauI+dhs*std::sin(zeta))
  * (
     dhs*std::cos(zeta)*(mT*std::sin(zeta)*std::cosh(pRapidity-sRapidity)+pT*std::cos(zeta)*std::cos(sPhi-pPhi))
     + dDdZeta*std::cos(zeta)*(-mT*std::cos(zeta)*std::cosh(pRapidity-sRapidity) + pT*std::sin(zeta)*std::cos(sPhi-pPhi))
     + dDdPhi*pT*std::sin(sPhi-pPhi)
     );
  if (_backFlow)
    {
    // only particle emission directed back to the hydro-region
    if(dSigmaP >= 0.0)
      dSigmaP = 0.0;
    else
      dSigmaP = -dSigmaP;
    }
  else
    {
    // disable particle emission back to the hydro-region
    if(dSigmaP < 0.0) dSigmaP = 0.0;
    }
  double denom = stat + std::exp( (pDotU-chemPotential)/_temperature.value() );
  double integrand = sf*pT*dPt*dSigmaP/(denom*twoPiCube());
  hadron.setIntegrand(integrand);
  return integrand;
  }

  void   ModelHydro2D::readFromXmlFile(const char * inputPath,
                                        const char * inputFileName)
  {
  double temperature, muB, muI, muS, muC;
  String systemName;
  String deviceName;
  double systemEnergy;
  double centralityMin;
  double centralityMax;
  double impactParameter;
  //double _centerT;
  printCR();
  printValue("Reading from xml file named",inputFileName);
  printValue("from path named",inputPath);
  XmlDocument xmlDoc = XmlDocument::read(inputPath,inputFileName);
  const XmlElement & root = xmlDoc.root();
  // text of the PARAMETER element with the given name attribute
  auto parameter = [&root](const char * name) -> String
    {
    const XmlElement * e = root.findFirst("PARAMETER","name",name);
    if (!e) throw XmlException(String("No PARAMETER named ")+name,"readFromXmlFile()");
    return e->text();
    };
  // files name this parameter Tau_i (as for ModelHydro3D); "_tau _i" is the name used by the previous reader
  _tauI            = (root.findFirst("PARAMETER","name","Tau_i") ? parameter("Tau_i") : parameter("_tau _i")).atof()/hBarC();       // [GeV^-1]
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
  deviceName      = parameter("device");
  systemName = parameter("colliding_system");
  _distanceField       = XmlScalarField::read(root,"Distance");      // [GeV^-1]
  _vtFluidField        = XmlScalarField::read(root,"FluidVt");
  _phiFluidField       = XmlScalarField::read(root,"FluidPhi");      // [c]

  setTemperature(temperature);
  setMuB(muB);
  setMuI(muI);
  setMuS(muS);
  setMuC(muC);
  setSystemName(systemName);
  setSystemEnergy(systemEnergy);
  setCentralityRange(centralityMin,centralityMax);
  setImpactParameter(impactParameter);
  // _distanceField derivatives
  if (root.findFirst("VECTOR3D","name","DistanceDZeta"))
    _distanceFieldDZeta = XmlScalarField::read(root,"DistanceDZeta");      // [GeV^-1/rad]
  else
    _distanceFieldDZeta = _distanceField.derivativeX("DistanceDZeta");
  if (root.findFirst("VECTOR3D","name","DistanceDPhi"))
    _distanceFieldDPhi = XmlScalarField::read(root,"DistanceDPhi");      // [GeV^-1/rad]
  else
    _distanceFieldDPhi = _distanceField.derivativeY("DistanceDPhi");
  }

} // namespace CAP

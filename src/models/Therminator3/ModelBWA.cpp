#include "ModelBWA.hpp"


using namespace std;
using CAP::MATH::twoPi;
using CAP::MATH::twoPiCube;

namespace CAP
{

  ModelBWA::ModelBWA()
  :
  Model()
  {    }

  double ModelBWA::calculateIntegrand(ThermalHadron & hadron)
  {
  // Type of _statistics Bose-Einstein or Fermi-Dirac
  ParticleType & type = hadron.type();
  double sf    = type.spinFactor();
  double stat  = type.statistics();
  double mass  = type.mass();
  double mass2 = mass*mass;
  double chemPotential = chemicalPotential(type);

  double rho       = _rhoMax * gRandom->Rndm();
  double sPhi      = twoPi() * gRandom->Rndm();
  double sRapidity = _sRapidityRange * (gRandom->Rndm() - 0.5);;
  double tau       = _tauI + _amp*rho;

  double zeta      = gRandom->Rndm();
  double zetac     = (zeta>0.9999999) ? 0.0000001 : 1.00-zeta;
  double pT        = zeta/zetac;
  double dPt       = 1.0/(zetac*zetac);
  double pPhi      = twoPi() * gRandom->Rndm();
  double pRapidity = _pRapidityRange * (gRandom->Rndm() - 0.5);
  double mT        = std::sqrt(mass2+pT*pT);
  //double e         = mT*std::cosh(pRapidity);

  double vT;
  switch (_modelSubtype)
    {
      default:
      case 2:
      case 3:
      vT = _tVelocity; break;
      case 4:
      case 5:
      case 6:
      // "linear" profile of the Therminator 2 paper (arXiv:1102.0273, Eq. 42): VelT is in the denominator
      vT = (rho/_rhoMax)/(_tVelocity+rho/_rhoMax); break;
    }
  // Invariants
  double gammaT     = gammaAtBeta(vT);
  double mTcoshDiff = mT*(std::cosh(sRapidity-pRapidity));
  double cosDphi    = std::cos(sPhi-pPhi);

  // Return values
  double Xt = tau * std::cosh(sRapidity);
  double Xx = rho * std::cos(sPhi);
  double Xy = rho * std::sin(sPhi);
  double Xz = tau * std::sinh(sRapidity);

  double Pe = mT  * std::cosh(pRapidity);
  double Px = pT  * std::cos(pPhi);
  double Py = pT  * std::sin(pPhi);
  double Pz = mT  * std::sinh(pRapidity);

  switch (_modelSubtype)
    {
      default: break;
      case 3:
      case 5:
      Xt += -_delay * std::log(gRandom->Rndm()); break;
      case 6:
        {
        // formation: the particle streams freely with velocity p/E during the delay only, from its point
        // on the hypersurface (Therminator 2 used the full time Xt, as if it streamed from t = 0)
        double energy = std::sqrt(mT*mT + Pz*Pz);
        double delay  = -_delay * std::log(gRandom->Rndm());
        Xt += delay;
        Xx += delay * Px / energy;
        Xy += delay * Py / energy;
        Xz += delay * Pz / energy;
        }
      break;
    }
  _position.setTXYZ(Xt,Xx,Xy,Xz);
  _momentum.setTXYZ(Pe,Px,Py,Pz);
  double pDotU   = gammaT*(mTcoshDiff - vT*pT*cosDphi);
  double dSigmaP = tau*rho*(mTcoshDiff - _amp*pT*cosDphi);
  double denom   = stat + std::exp( (pDotU-chemPotential)/_temperature.value() );
  double integrand = sf*pT*dPt*dSigmaP/(denom*twoPiCube());
  return integrand;
  }

} // namespace CAP

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
#include "GlauberNucleus.hpp"
#include "PrintHelpers.hpp"
#include "MersenneTwister.hpp"
#include "FileSystem.hpp"
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>


namespace CAP
{
  using CAP::MATH::pi;
  using CAP::MATH::twoPi;

  GlauberNucleus::GlauberNucleus()
  :
  _nucleusType(),
  _nucleons(),
  _allowOverlap(true),
  _useLattice(false),
  _latticeType(0),
  _nTrialsMax(1000),
  _nnDistanceMin(-1.0),
  _nnDistanceMinSq(-1.0),
  _nodeDistance(0.0),
  _recenteringMode(1),
  _overlapMode(0),
  _userSigmaMode(0),
  _smearing(0.0),
  _maxCenterShift(99.0),
  _orientationPhi(0.0),
  _orientationTheta(0.0),
  _nNonSmeared(0),
  _protonDistribution(),
  _neutronDistribution(),
  _configurations(),
  _configurationIndex(0),
  _usedNodes()
  {   }

  GlauberNucleus::GlauberNucleus(const GlauberNucleus & src)
  :
  _nucleusType(src._nucleusType),
  _nucleons(src._nucleons),
  _allowOverlap(src._allowOverlap),
  _useLattice(src._useLattice),
  _latticeType(src._latticeType),
  _nTrialsMax(src._nTrialsMax),
  _nnDistanceMin(src._nnDistanceMin),
  _nnDistanceMinSq(src._nnDistanceMinSq),
  _nodeDistance(src._nodeDistance),
  _recenteringMode(src._recenteringMode),
  _overlapMode(src._overlapMode),
  _userSigmaMode(src._userSigmaMode),
  _smearing(src._smearing),
  _maxCenterShift(src._maxCenterShift),
  _orientationPhi(src._orientationPhi),
  _orientationTheta(src._orientationTheta),
  _nNonSmeared(src._nNonSmeared),
  _protonDistribution(src._protonDistribution),
  _neutronDistribution(src._neutronDistribution),
  _configurations(src._configurations),
  _configurationIndex(src._configurationIndex),
  _usedNodes()
  {   }

  GlauberNucleus GlauberNucleus::operator=(const GlauberNucleus & rhs)
  {
  if (this!=&rhs)
    {
    _nucleusType        = rhs._nucleusType;
    _nucleons           = rhs._nucleons;
    _allowOverlap       = rhs._allowOverlap;
    _useLattice         = rhs._useLattice;
    _latticeType        = rhs._latticeType;
    _nTrialsMax         = rhs._nTrialsMax;
    _nnDistanceMin      = rhs._nnDistanceMin;
    _nnDistanceMinSq    = rhs._nnDistanceMinSq;
    _nodeDistance       = rhs._nodeDistance;
    _recenteringMode    = rhs._recenteringMode;
    _overlapMode        = rhs._overlapMode;
    _userSigmaMode      = rhs._userSigmaMode;
    _smearing           = rhs._smearing;
    _maxCenterShift     = rhs._maxCenterShift;
    _orientationPhi     = rhs._orientationPhi;
    _orientationTheta   = rhs._orientationTheta;
    _nNonSmeared        = rhs._nNonSmeared;
    _protonDistribution = rhs._protonDistribution;
    _neutronDistribution = rhs._neutronDistribution;
    _configurations     = rhs._configurations;
    _configurationIndex = rhs._configurationIndex;
    }
  return *this;
  }

  GlauberNucleus::~GlauberNucleus()
  {
  clear();
  }
  
  //!
  //!Returns the number of wounded nucleons in this nucleus
  //!
  int GlauberNucleus::nWoundedNucleons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWounded()) n++;
    }
  return n;
  }

  //!
  //!Returns the number of wounded protons in this nucleus
  //!
  int GlauberNucleus::nWoundedProtons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWounded()  &&  nucleon.isProton()) n++;
    }
  return n;
  }

  //!
  //!Returns the number of wounded neutrons in this nucleus
  //!
  int GlauberNucleus::nWoundedNeutrons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWounded()  &&  nucleon.isNeutron()) n++;
    }
  return n;
  }


  //!
  //!Returns the number of hard wounded nucleons in this nucleus
  //!
  int GlauberNucleus::nWoundedHardNucleons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWoundedHard()) n++;
    }
  return n;
  }

  //!
  //!Returns the number of hard wounded protons in this nucleus
  //!
  int GlauberNucleus::nWoundedHardProtons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWoundedHard()  &&  nucleon.isProton() ) n++;
    }
  return n;
  }

  //!
  //!Returns the number of hard wounded neutrons in this nucleus
  //!
  int GlauberNucleus::nWoundedHardNeutrons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isWoundedHard()  &&  nucleon.isNeutron() ) n++;
    }
  return n;
  }

  int GlauberNucleus::nCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons) n += nucleon.nCollisions();
  return n;
  }

  int GlauberNucleus::nPCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    if (nucleon.isProton()) n += nucleon.nCollisions();
  return n;
  }

  int GlauberNucleus::nNCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    if (nucleon.isNeutron()) n += nucleon.nCollisions();
  return n;
  }

  int GlauberNucleus::nHardCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons) n += nucleon.nHardCollisions();
  return n;
  }

  int GlauberNucleus::nPHardCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    if (nucleon.isProton()) n += nucleon.nHardCollisions();
  return n;
  }

  int GlauberNucleus::nNHardCollisions() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    if (nucleon.isNeutron()) n += nucleon.nHardCollisions();
  return n;
  }

  //!
  //!Returns the number of spectator nucleons in this nucleus
  //!
  int GlauberNucleus::nSpectatorNucleons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isSpectator()) n++;
    }
  return n;
  }

  //!
  //!Returns the number of spectator protons in this nucleus
  //!
  int GlauberNucleus::nSpectatorProtons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isSpectator()  &&  nucleon.isProton() ) n++;
    }
  return n;
  }

  //!
  //!Returns the number of spectator neutrons in this nucleus
  //!
  int GlauberNucleus::nSpectatorNeutrons() const
  {
  int n = 0;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isSpectator()  &&  nucleon.isNeutron() ) n++;
    }
  return n;
  }

  //!
  //! Create the required array of nucleons
  //! which will be used and reused.
  //!
  void GlauberNucleus::generateNucleons()
  {
  _nucleons.clear();
  int nProtons  = _nucleusType.nProtons();
  int nNeutrons = _nucleusType.nNeutrons();
  for (int i=0; i<nProtons; ++i)
    {
    GlauberNucleon nucleon;
    nucleon.setTypeToProton();
    _nucleons.push_back(nucleon);
    }
  for (int i=0; i<nNeutrons; ++i)
    {
    GlauberNucleon nucleon;
    nucleon.setTypeToNeutron();
    _nucleons.push_back(nucleon);
    }
  }

  //!
  //! Rotate (x,y,z) so that the z axis goes to the unit vector u (TVector3::RotateUz of ROOT).
  //!
  void GlauberNucleus::rotateUz(double ux, double uy, double uz, double & x, double & y, double & z)
  {
  double up = ux*ux + uy*uy;
  if (up>0.0)
    {
    up = std::sqrt(up);
    double px = x, py = y, pz = z;
    x = (ux*uz*px - uy*py)/up + ux*pz;
    y = (uy*uz*px + ux*py)/up + uy*pz;
    z = -up*px + uz*pz;
    }
  else if (uz<0.0)
    {
    x = -x;
    z = -z;
    }
  }

  //!
  //! Rotate (x,y,z) about x, then y, then z (TGlauNucleon::RotateXYZ_3D of TGlauberMC).
  //!
  void GlauberNucleus::rotateXYZ3D(double psiX, double psiY, double psiZ, double & x, double & y, double & z)
  {
  double c = std::cos(psiX), s = std::sin(psiX);
  double t = y;
  y = c*t - s*z;
  z = s*t + c*z;
  c = std::cos(psiY); s = std::sin(psiY);
  t = z;
  z = c*t - s*x;
  x = s*t + c*x;
  c = std::cos(psiZ); s = std::sin(psiZ);
  t = x;
  x = c*t - s*y;
  y = s*t + c*y;
  }

  //!
  //! Whether (x,y,z) is closer than the minimum distance to one of the first n nucleons (those placed already).
  //!
  bool GlauberNucleus::isCloseToPlaced(int n, double x, double y, double z) const
  {
  if (_allowOverlap || _nnDistanceMinSq<=0.0) return false;
  for (int j=0; j<n; ++j) if (_nucleons[j].isCloseTo(x,y,z,_nnDistanceMinSq)) return true;
  return false;
  }

  //!
  //! Generate random positions for all the nucleons (TGlauNucleus::ThrowNucleons of TGlauberMC v3.2): protons and
  //! neutrons in a random order; the deuteron (types 3, 4), the configurations read from a file (type 6), a lattice
  //! (setUseLattice() with a node distance), or each nucleon drawn from the density, at the minimum distance from
  //! those placed before unless overlaps are allowed; deformed nuclei and configurations randomly oriented. The
  //! nucleus is thrown again while its center is farther than maxCenterShift() from the origin, then recentered.
  //!
  void GlauberNucleus::positionNucleons()
  {
  const int n = int(_nucleons.size());
  if (n<1) throw InvalidValueGlauberException(n,"Nucleus has nNucleons<1",__FUNCTION__);
  CAP::MATH::RandomGenerator & generator = CAP::MATH::randomGenerator();
  const int nZ = _nucleusType.nProtons();
  for (int i=0, iz=0; i<n; ++i)
    {
    double fraction = double(nZ-iz)/double(n-i);
    if (generator.random()<fraction) { _nucleons[i].setTypeToProton(); ++iz; }
    else _nucleons[i].setTypeToNeutron();
    }
  const int type = _nucleusType.radialDistType();
  const long maxThrows = 1000000;
  for (long iThrow=0; iThrow<maxThrows; ++iThrow)
    {
    for (auto & nucleon : _nucleons) nucleon.reset();
    _nNonSmeared = 0;
    _orientationPhi   = twoPi()*generator.random();
    _orientationTheta = std::acos(2.0*generator.random()-1.0);
    if (n==2 && (type==3 || type==4))
      positionDeuteron();
    else if (type==6)
      positionFromConfiguration();
    else if (_useLattice && _nodeDistance>0.0)
      positionOnLattice();
    else
      positionContinuous();
    VectorLorentz center;
    findNucleusCenter(center);
    if (center.modulus3() > _maxCenterShift) continue;
    recenterNucleus();
    return;
    }
  throw IterationsGlauberException(maxThrows,maxThrows,"Too many throws: the center of the nucleus stays too far from the origin",
                                   __FUNCTION__);
  }

  //!
  //! Deuteron (Hulthen): the distance between the nucleons drawn from the distribution, each at half of it from the
  //! center; type 4 puts the second nucleon opposite to the first, type 3 draws it independently.
  //!
  void GlauberNucleus::positionDeuteron()
  {
  CAP::MATH::RandomGenerator & generator = CAP::MATH::randomGenerator();
  for (int i=0; i<2; ++i)
    {
    if (i==1 && _nucleusType.radialDistType()==4)
      {
      const VectorLorentz & first = _nucleons[0].position();
      _nucleons[1].position().setTXYZ(0.0, -first.x(), -first.y(), -first.z());
      break;
      }
    double r      = 0.5*_protonDistribution.random();
    double phi    = twoPi()*generator.random();
    double cosTh  = 2.0*generator.random()-1.0;
    double sinTh  = std::sqrt(std::max(0.0,1.0-cosTh*cosTh));
    _nucleons[i].position().setTXYZ(0.0, r*sinTh*std::cos(phi), r*sinTh*std::sin(phi), r*cosTh);
    }
  }

  //!
  //! Each nucleon drawn from the density of its kind (protons and neutrons differ for the types 11 and 13), rotated
  //! with the nucleus when it is deformed, and drawn again while it is closer than the minimum distance to one of the
  //! nucleons placed before (unless overlaps are allowed).
  //!
  void GlauberNucleus::positionContinuous()
  {
  const bool deformed = _protonDistribution.isDeformed();
  const double ux = std::sin(_orientationTheta)*std::cos(_orientationPhi);
  const double uy = std::sin(_orientationTheta)*std::sin(_orientationPhi);
  const double uz = std::cos(_orientationTheta);
  const int n = int(_nucleons.size());
  for (int i=0; i<n; ++i)
    {
    GlauberNucleon & nucleon = _nucleons[i];
    NuclearDistribution & distribution = nucleon.isProton() ? _protonDistribution : _neutronDistribution;
    for (int trial=1; ; ++trial)
      {
      if (trial>_nTrialsMax)
        throw IterationsGlauberException(_nTrialsMax,trial,"Too Many Iterations In Overlap Test",__FUNCTION__);
      double x, y, z;
      distribution.randomPosition(x,y,z);
      if (deformed) rotateUz(ux,uy,uz,x,y,z);
      if (isCloseToPlaced(i,x,y,z)) continue;
      nucleon.position().setTXYZ(0.0,x,y,z);
      break;
      }
    }
  }

  //!
  //! Nucleons on the nodes of a lattice (TGlauberMC, grid mode): a free node drawn at random in a cube of 40 fm, kept
  //! with a probability equal to the density at its radius, optionally smeared (each coordinate multiplied by a
  //! Gaussian number of mean 1 and rms smearing(), kept if at the minimum distance from the nucleons placed before,
  //! 99 attempts), then the whole nucleus rotated about x, y and z by random angles. Lattice types: 1 primitive cubic,
  //! 2 body-centered cubic, 3 face-centered cubic, other values hexagonal close packing.
  //!
  void GlauberNucleus::positionOnLattice()
  {
  if (_protonDistribution.isDeformed())
    throw GlauberException("The lattice placement is not available for deformed nuclei",__FUNCTION__);
  if (!_allowOverlap && _nnDistanceMin>_nodeDistance)
    throw GlauberException("The minimum distance between nucleons is larger than the distance between the nodes",__FUNCTION__);
  CAP::MATH::RandomGenerator & generator = CAP::MATH::randomGenerator();
  const double d = _nodeDistance;
  const double edge = 20.0;
  const double edgeX = edge + d*generator.random() - 0.5*d;
  const double edgeY = edge + d*generator.random() - 0.5*d;
  const double edgeZ = edge + d*generator.random() - 0.5*d;
  const long nSlots = long(2.0*edge/d) + 1;
  if (nSlots*nSlots*nSlots > 200000000L)
    throw GlauberException("The distance between the lattice nodes is too small",__FUNCTION__);
  _usedNodes.assign(size_t(nSlots*nSlots*nSlots),0);
  const double rotX = twoPi()*generator.random();
  const double rotY = twoPi()*generator.random();
  const double rotZ = twoPi()*generator.random();
  const double sqrt3 = std::sqrt(3.0), sqrt6 = std::sqrt(6.0);
  const int n = int(_nucleons.size());
  for (int i=0; i<n; ++i)
    {
    GlauberNucleon & nucleon = _nucleons[i];
    NuclearDistribution & distribution = nucleon.isProton() ? _protonDistribution : _neutronDistribution;
    for (long trial=1; ; ++trial)
      {
      if (trial>100000000L) throw IterationsGlauberException(100000000L,trial,"No free lattice node accepted",__FUNCTION__);
      long iNode = long((2.0*edge/d)*generator.random());
      long jNode = long((2.0*edge/d)*generator.random());
      long kNode = long((2.0*edge/d)*generator.random());
      size_t index = size_t(iNode*nSlots*nSlots + jNode*nSlots + kNode);
      if (_usedNodes[index]) continue;
      double x, y, z;
      switch (_latticeType)
        {
          case 1: // primitive cubic
          x = d*iNode - edgeX;
          y = d*jNode - edgeY;
          z = d*kNode - edgeZ;
          break;
          case 2: // body-centered cubic
          x = 0.5*d*(-iNode+jNode+kNode) - 0.5*edgeX;
          y = 0.5*d*(+iNode-jNode+kNode) - 0.5*edgeY;
          z = 0.5*d*(+iNode+jNode-kNode) - 0.5*edgeZ;
          break;
          case 3: // face-centered cubic
          x = 0.5*d*(jNode+kNode) - edgeX;
          y = 0.5*d*(iNode+kNode) - edgeY;
          z = 0.5*d*(iNode+jNode) - edgeZ;
          break;
          default: // hexagonal close packing (the (k mod 2)/3 of TGlauberMC was an integer division, always 0)
          x = 0.5*d*(2*iNode + ((jNode+kNode)%2)) - edgeX;
          y = 0.5*d*(sqrt3*(jNode + (kNode%2)/3.0)) - edgeY;
          z = 0.5*d*(kNode*2.0*sqrt6/3.0) - edgeZ;
          break;
        }
      double r = std::sqrt(x*x+y*y+z*z);
      if (r>distribution.rangeMax() || generator.random()>distribution.density(r)) continue;
      if (_smearing>0.0)
        {
        bool smeared = false;
        for (int attempt=0; attempt<99 && !smeared; ++attempt)
          {
          double xs = x*generator.gauss(1.0,_smearing);
          double ys = y*generator.gauss(1.0,_smearing);
          double zs = z*generator.gauss(1.0,_smearing);
          if (!isCloseToPlaced(i,xs,ys,zs)) { x = xs; y = ys; z = zs; smeared = true; }
          }
        if (!smeared) ++_nNonSmeared;
        }
      _usedNodes[index] = 1;
      nucleon.position().setTXYZ(0.0,x,y,z);
      break;
      }
    }
  // the nucleus rotated once all its nucleons are placed (TGlauberMC rotated each nucleon as it was placed, so that the
  // distance test of the smearing compared positions of different frames)
  for (auto & nucleon : _nucleons)
    {
    VectorLorentz & p = nucleon.position();
    double x = p.x(), y = p.y(), z = p.z();
    rotateXYZ3D(rotX,rotY,rotZ,x,y,z);
    p.setTXYZ(0.0,x,y,z);
    }
  }

  //!
  //! Name of the file of nucleon configurations of a nucleus (type 6), in $CAP_DATABASE_PATH/Glauber/NucleonConfigurations.
  //!
  String GlauberNucleus::configurationFileName() const
  {
  const String & name = _nucleusType.name();
  if (name=="He3") return "he3_plaintext.dat";
  if (name=="H3")  return "h3_plaintext.dat";
  if (name=="He4") return "he4_plaintext.dat";
  if (name=="C")   return "carbon_plaintext.dat";
  if (name=="O")   return "oxygen_plaintext.dat";
  throw GlauberException(String("No file of nucleon configurations for the nucleus ")+name,__FUNCTION__);
  }

  //!
  //! Read the nucleon configurations of the nucleus (once per file for the whole program): one per line, the x, y, z
  //! of each nucleon (after two numbers for carbon; the numbers after them are ignored).
  //!
  void GlauberNucleus::readConfigurations()
  {
  static std::map<std::string, std::shared_ptr<const std::vector<double>>> cache;
  String relative = String("Glauber/NucleonConfigurations/") + configurationFileName();
  String path = FileSystem::configuredPath(relative, "CAP_DATABASE_PATH");
  std::string key = path.str();
  auto found = cache.find(key);
  if (found==cache.end())
    {
    std::ifstream file(key);
    if (!file) throw GlauberException(String("Cannot read the nucleon configurations ")+path+" (is CAP_DATABASE_PATH set?)",
                                      __FUNCTION__);
    const int n = int(_nucleons.size());
    const int skip = (n==12) ? 2 : 0;
    auto values = std::make_shared<std::vector<double>>();
    std::string line;
    while (std::getline(file,line))
      {
      std::istringstream words(line);
      std::vector<double> numbers;
      double v;
      while (words >> v) numbers.push_back(v);
      if (int(numbers.size()) < skip + 3*n) continue;
      values->insert(values->end(), numbers.begin()+skip, numbers.begin()+skip+3*n);
      }
    if (values->empty()) throw GlauberException(String("No nucleon configuration in ")+path,__FUNCTION__);
    found = cache.emplace(key, values).first;
    }
  _configurations = found->second;
  // a random first configuration, so that two nuclei of the same kind do not use the same sequence
  const long nConfigurations = long(_configurations->size()/(3*_nucleons.size()));
  _configurationIndex = long(CAP::MATH::randomGenerator().random()*nConfigurations) % nConfigurations;
  }

  //!
  //! Nucleons at the positions of the next configuration of the file, the configuration randomly oriented (type 6).
  //!
  void GlauberNucleus::positionFromConfiguration()
  {
  if (!_configurations) readConfigurations();
  const int n = int(_nucleons.size());
  const long nConfigurations = long(_configurations->size()/(3*n));
  if (_configurationIndex>=nConfigurations) _configurationIndex = 0;
  const double * xyz = _configurations->data() + 3*n*_configurationIndex;
  const double ux = std::sin(_orientationTheta)*std::cos(_orientationPhi);
  const double uy = std::sin(_orientationTheta)*std::sin(_orientationPhi);
  const double uz = std::cos(_orientationTheta);
  for (int i=0; i<n; ++i)
    {
    double x = xyz[3*i], y = xyz[3*i+1], z = xyz[3*i+2];
    rotateUz(ux,uy,uz,x,y,z);
    _nucleons[i].position().setTXYZ(0.0,x,y,z);
    }
  ++_configurationIndex;
  }

  //!
  //! Recenter the nucleus (TGlauberMC fRecenter): 0 not at all; 1 all the nucleons shifted so that their center is at
  //! the origin; 2 the last nucleon shifted so that the center is at the origin; 3 the nucleus rotated so that its
  //! center is on the z axis, then shifted along z to the origin; 4 rotated only.
  //!
  void GlauberNucleus::recenterNucleus()
  {
  if (_recenteringMode<=0) return;
  VectorLorentz center;
  findNucleusCenter(center);
  const double cx = center.x(), cy = center.y(), cz = center.z();
  const int n = int(_nucleons.size());
  switch (_recenteringMode)
    {
      default:
      case 1:
      for (auto & nucleon : _nucleons)
        {
        VectorLorentz & p = nucleon.position();
        p.setTXYZ(0.0, p.x()-cx, p.y()-cy, p.z()-cz);
        }
      break;
      case 2:
      {
      VectorLorentz & p = _nucleons[n-1].position();
      p.setTXYZ(0.0, p.x()-n*cx, p.y()-n*cy, p.z()-n*cz);
      break;
      }
      case 3:
      case 4:
      {
      // rotation about (center x z) by the angle between the center and z: the center goes onto the z axis
      double length = std::sqrt(cx*cx+cy*cy+cz*cz);
      double kx = cy, ky = -cx;                   // center x (0,0,1)
      double kLength = std::sqrt(kx*kx+ky*ky);
      if (length>0.0 && kLength>0.0)
        {
        kx /= kLength; ky /= kLength;
        double angle = std::acos(std::max(-1.0,std::min(1.0,cz/length)));
        double c = std::cos(angle), s = std::sin(angle);
        for (auto & nucleon : _nucleons)
          {
          VectorLorentz & p = nucleon.position();
          double x = p.x(), y = p.y(), z = p.z();
          double kDotV = kx*x + ky*y;
          // Rodrigues: v c + (k x v) s + k (k.v)(1-c), with k = (kx, ky, 0)
          double rx = x*c + (ky*z)*s + kx*kDotV*(1-c);
          double ry = y*c + (-kx*z)*s + ky*kDotV*(1-c);
          double rz = z*c + (kx*y - ky*x)*s;
          p.setTXYZ(0.0, rx, ry, rz);
          }
        }
      if (_recenteringMode==3)
        for (auto & nucleon : _nucleons)
          {
          VectorLorentz & p = nucleon.position();
          p.setTXYZ(0.0, p.x(), p.y(), p.z()-length);
          }
      break;
      }
    }
  }

  //!
  //!Find the center (mean position) of all the nucleons of this nucleus
  //!
  void GlauberNucleus::findNucleusCenter(VectorLorentz & center)
  {
  double x = 0.0, y = 0.0, z = 0.0;
  for (auto & nucleon : _nucleons)
    {
    const VectorLorentz & p = nucleon.position();
    x += p.x();
    y += p.y();
    z += p.z();
    }
  const double n = _nucleons.empty() ? 1.0 : double(_nucleons.size());
  center.setTXYZ(0.0, x/n, y/n, z/n);
  }

  //!
  //! Shift all the nucleons of the given nucleus in the given direction
  //!
  void GlauberNucleus::shiftNucleus(VectorLorentz & shift)
  {
  for (auto & nucleon : _nucleons)
    {
    nucleon.shiftPosition(shift);
    }
  }

  //!
  //! rotate the position of all the nucleons of this nucleus
  //! by the given azimuthal angle phi relative to the x axis.
  //!
  void GlauberNucleus::rotateX(double phi)
  {
  for (auto & nucleon : _nucleons)  nucleon.rotateX(phi);
  }

  //!
  //! rotate the position of all the nucleons of this nucleus
  //! by the given azimuthal angle phi relative to the z axis.
  //!
  void GlauberNucleus::rotateY(double phi)
  {
  for (auto & nucleon : _nucleons)  nucleon.rotateY(phi);
  }

  //!
  //! rotate the position of all the nucleons of this nucleus
  //! by the given azimuthal angle phi relative to the z axis.
  //!
  void GlauberNucleus::rotateZ(double phi)
  {
  for (auto & nucleon : _nucleons)  nucleon.rotateZ(phi);
  }

  //!
  //! rotate the position of all the nucleons of this nucleus
  //! by the given three Euler angles.
  //!
  void GlauberNucleus::rotateXYZ(double psiX, double psiY, double psiZ)
  {
  for (auto & nucleon : _nucleons)  nucleon.rotateXYZ(psiX,psiY,psiZ);
  }

  //!
  //! Reset the states wounded, hardWounded of the nucleons
  //!
  void GlauberNucleus::resetNucleons()
  {
  for (auto & nucleon : _nucleons) nucleon.reset();
  }

  //!
  //! Delete all  nucleons, and clear the nucleon container
  //!
  void GlauberNucleus::clearNucleons()
  {
  _nucleons.clear();
  }


  void GlauberNucleus::reset()
  {
  resetNucleons();
  }

  void GlauberNucleus::clear()
  {
  clearNucleons();
  }

  void GlauberNucleus::print() const
  {
  printValue("Type",         _nucleusType.name());
  printValue("nProtons",     _nucleusType.nProtons());
  printValue("nNeutrons",    _nucleusType.nNeutrons());
  printValue("nNucleons",    _nucleusType.nNucleons());
  printValue("nWoundedNucleons",     nWoundedNucleons());
  printValue("nWoundedProtons",      nWoundedProtons());
  printValue("nWoundedNeutrons",     nWoundedNeutrons());
  printValue("nWoundedHardNucleons", nWoundedHardNucleons());
  printValue("nWoundedHardProtons",  nWoundedHardProtons());
  printValue("nWoundedHardNeutrons", nWoundedHardNeutrons());
  printValue("nSpectatorNucleons",   nSpectatorNucleons());
  printValue("nSpectatorProtons",    nSpectatorProtons());
  printValue("nSpectatorNeutrons",   nSpectatorNeutrons());
  }

  //!
  //!Determine whether this nucleus has a nucleons with dSqMax of  to the given position (x,y,z)
  //!
  bool GlauberNucleus::hasNucleonCloseTo(double x, double y, double z, double dSqMax) const
  {
  if (dSqMax<=0) return false;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isCloseTo(x,y,z,dSqMax)) return true;
    }
  return false;
  }

  //!
  //!Determine whether this nucleus has a nucleons with dSqMax of  to the given position reference.
  //!
  bool GlauberNucleus::hasNucleonCloseTo(const VectorLorentz & reference, double dSqMax) const
  {
  if (dSqMax<=0) return false;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isCloseTo(reference,dSqMax)) return true;
    }
  return false;
  }

  //!
  //!Determine whether this nucleus has a nucleons with dSqMax of  to the given position reference.
  //!
  bool GlauberNucleus::hasNucleonCloseTo(const GlauberNucleon & reference, double dSqMax) const
  {
  if (dSqMax<=0) return false;
  for (auto & nucleon : _nucleons)
    {
    if (nucleon.isCloseTo(reference,dSqMax)) return true;
    }
  return false;
  }


  //!
  //!Returns a function that computes the probability of finding a proton/neutron a
  //!given distance from the nucleus center using the radial distribution type "radialDistType"
  //!
  void GlauberNucleus::setNucleus()
  {
  _protonDistribution.setType(  _nucleusType.radialDistType());
  _protonDistribution.setRadius(_nucleusType.radiusProton());
  _protonDistribution.setSkin(  _nucleusType.skinProton());
  _protonDistribution.setW(     _nucleusType.wProton());
  _protonDistribution.setA(     _nucleusType.a());
  _protonDistribution.setB(     _nucleusType.b());
  _protonDistribution.setBeta2( _nucleusType.beta2());
  _protonDistribution.setBeta4( _nucleusType.beta4());
  _protonDistribution.setR0(    _nucleusType.r0());
  _protonDistribution.setR1(    _nucleusType.r1());
  _protonDistribution.setR2(    _nucleusType.r2());
  _protonDistribution.setMaxRadius(_nucleusType.maxRadius());

  _neutronDistribution.setType(  _nucleusType.radialDistType());
  _neutronDistribution.setRadius(_nucleusType.radiusNeutron());
  _neutronDistribution.setSkin(  _nucleusType.skinNeutron());
  _neutronDistribution.setW(     _nucleusType.wNeutron());
  _neutronDistribution.setA(     _nucleusType.a());
  _neutronDistribution.setB(     _nucleusType.b());
  _neutronDistribution.setBeta2( _nucleusType.beta2());
  _neutronDistribution.setBeta4( _nucleusType.beta4());
  _neutronDistribution.setR0(    _nucleusType.r0());
  _neutronDistribution.setR1(    _nucleusType.r1());
  _neutronDistribution.setR2(    _nucleusType.r2());
  _neutronDistribution.setMaxRadius(_nucleusType.maxRadius());
  // TGlauberMC throws the reweighted nuclei again until their center is within 0.1 fm of the origin
  int type = _nucleusType.radialDistType();
  _maxCenterShift = (type==12 || type==13 || type==14) ? 0.1 : 99.0;
  _configurations.reset();
  }

} // namespace CAP

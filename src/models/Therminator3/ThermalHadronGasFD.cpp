#include "ThermalHadronGasFD.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include "ParticleDecayMode.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"


using namespace std;

namespace CAP
{

  ThermalHadronGasFD::ThermalHadronGasFD()
  :
  ThermalHadronGas(),
  _stableHadrons(),
  _stableHadronPairs()
  {  }

  ThermalHadronGasFD::ThermalHadronGasFD(const ThermalHadronGasFD & source)
  :
  ThermalHadronGas(source),
  _stableHadrons(source._stableHadrons),
  _stableHadronPairs(source._stableHadronPairs)
  {  }

  ThermalHadronGasFD & ThermalHadronGasFD::operator=(const ThermalHadronGasFD & rhs)
  {
  if (this!=&rhs)
    {
    ThermalHadronGas::operator=(rhs);
    _stableHadrons     = rhs._stableHadrons;
    _stableHadronPairs = rhs._stableHadronPairs;
    }
  return *this;
  }

  void ThermalHadronGasFD::reset()
  {
    ThermalHadronGas::reset();
    for (auto & value : _stableHadronPairs) value = 0.0;
  }

  void ThermalHadronGasFD::clear()
  {
  ThermalHadronGas::clear();
  _stableHadronPairs.clear();
  }

  //!
  //! Probabilities that a hadron of the gas yields each stable hadron (singles) and each pair of stable hadrons (pairs)
  //! after its decays. They follow the decay chains through the database, also through the types the gas does not hold
  //! (K0 -> K0S, K0L -> pions), with the branching ratios normalized to their sum, as the decayer draws them
  //! (ParticleType::setupDecayGenerator()). Photons and the stable types that are not in the gas (leptons) feed nothing.
  //!
  void ThermalHadronGasFD::calculateParticleDecayProbability()
  {
  _stableHadrons.clear();
  for (auto & hadron : _hadrons)
    if (hadron.type().isStable()) _stableHadrons.push_back(&hadron);
  unsigned int nStableHadrons = _stableHadrons.size();
  if (nStableHadrons<1) throw Exception("_stableHadrons.size() < 1",__FUNCTION__);
  printValue("nStableHadrons",nStableHadrons);

  std::map<const ParticleType*,unsigned int> stableIndex;
  for (unsigned int k=0; k<nStableHadrons; k++) stableIndex[&_stableHadrons[k]->type()] = k;
  std::map<const ParticleType*,std::vector<double>> singles, pairs;
  const unsigned int n = nStableHadrons;
  std::function<void(const ParticleType &)> compute = [&](const ParticleType & type)
    {
    if (singles.count(&type)) return;
    std::vector<double> single(n,0.0), pair(n*n,0.0);
    singles[&type] = single;   // guards against a loop in the decay tables
    pairs[&type]   = pair;
    auto stable = stableIndex.find(&type);
    if (stable!=stableIndex.end())
      single[stable->second] = 1.0;
    else if (!type.isStable())
      {
      double brSum = 0.0;
      for (auto & decayMode : type.decayModes()) brSum += decayMode.branchingRatio();
      for (auto & decayMode : type.decayModes())
        {
        if (!(brSum>0.0)) break;
        double br = decayMode.branchingRatio()/brSum;
        std::vector<const ParticleType*> children;
        for (unsigned int c=0; c<unsigned(decayMode.nChildren()); c++)
          {
          const ParticleType & child = decayMode.childType(c);
          if (child.pdgCode()==22) continue;   // photons
          compute(child);
          children.push_back(&child);
          }
        for (auto child : children)
          {
          const std::vector<double> & s = singles[child];
          const std::vector<double> & p = pairs[child];
          for (unsigned int k=0; k<n; k++)   single[k] += br*s[k];
          for (unsigned int k=0; k<n*n; k++) pair[k]   += br*p[k];
          }
        // pairs from two different products
        for (unsigned int c1=0; c1<children.size(); c1++)
          for (unsigned int c2=c1+1; c2<children.size(); c2++)
            {
            const std::vector<double> & s1 = singles[children[c1]];
            const std::vector<double> & s2 = singles[children[c2]];
            for (unsigned int k1=0; k1<n; k1++)
              for (unsigned int k2=0; k2<n; k2++)
                pair[k1*n+k2] += br*s1[k1]*s2[k2];
            }
        }
      }
    singles[&type] = single;
    pairs[&type]   = pair;
    };

  for (auto & hadron : _hadrons)
    {
    compute(hadron.type());
    hadron.createDecayProbabilityArray(n);
    const std::vector<double> & single = singles[&hadron.type()];
    const std::vector<double> & pair   = pairs[&hadron.type()];
    for (unsigned int k=0; k<n; k++) hadron.setDecayProbability(k,single[k]);
    for (unsigned int k1=0; k1<n; k1++)
      for (unsigned int k2=0; k2<n; k2++)
        hadron.setDecayProbabilityPair(k1,k2,pair[k1*n+k2]);
    }
  }

  //!
  //!Compute the densities of _stable hadrons based on the thermal yields of more
  //!massive hadrons and their respective decay probabilities into the give _stable hadrons
  //!
  void ThermalHadronGasFD::calculateStableDensities()
  {
  int nStables = _stableHadrons.size();
  // singles
  for (int iStable = 0; iStable < nStables; iStable++)
    {
    double _stableDensity = 0.0;
    for (auto hadron : _hadrons)
      {
      _stableDensity += hadron.decayProbability(iStable) * hadron.numberDensity();
      }
    _stableHadrons[iStable]->setFeedDownNumberDensity(_stableDensity);
    }

  // pairs
  _stableHadronPairs.clear();
  for (int iStable1 = 0; iStable1 < nStables; iStable1++)
    {
    for (int iStable2 = 0; iStable2 < nStables; iStable2++)
      {
      double _stableDensity = 0.0;
      for (auto & hadron : _hadrons)
        {
        _stableDensity += hadron.decayProbabilityPair(iStable1,iStable2) * hadron.numberDensity();
        }
      _stableHadronPairs.push_back(_stableDensity);
      }
    }

  // symmetrize
  for (int k1 = 0; k1 < nStables; k1++)
    {
    for (int k2 = 0; k2 < k1; k2++)
      {
      double v1 = _stableHadronPairs[k1*nStables+k2];
      double v2 = _stableHadronPairs[k2*nStables+k1];
      double v = v1+v2;
      //std::cout << "k1,k2=" << k1 << "," << k2 << "   " << v1 << "," << v2 << " === " << v << endl;
      _stableHadronPairs[k1*nStables+k2] =v;
      _stableHadronPairs[k2*nStables+k1] =v;
      }
    }
  }

  // this needs to be "fixed"
  void ThermalHadronGasFD::setupDecayGenerator()
  {
  //hadronRndmSelector = new SelectionGenerator(particleDensities);
  }

  // this needs to be "fixed"
  ParticleType * ThermalHadronGasFD::generateRandomHadron()
  {
  //int index = 0; // hadronRndmSelector->generate();
  return nullptr;// _particleTypes->particleType(index);
  }

  // this needs to be "fixed"
  int ThermalHadronGasFD::generateRandomHadronByIndex()
  {
  return 0; // hadronRndmSelector->generate();
  }


  void ThermalHadronGasFD::print() const
  {
    // fix me...
  }

  void ThermalHadronGasFD::findStableHadrons()
  {
  _stableHadrons.clear();
  for (auto & hadron : _hadrons)
    {
    printValue("name",hadron.type().name());
    printValue("stable",hadron.type().isStable());
    if (hadron.type().isStable())
      _stableHadrons.push_back(&hadron);
    }
  }

  ThermalHadron & ThermalHadronGasFD::findStableHadronFor(int pdgCode)
  {
  for (unsigned int iHadron=0; iHadron<_stableHadrons.size(); iHadron++)
    {
    if (_stableHadrons[iHadron]->type().pdgCode() == pdgCode) return *(_stableHadrons[iHadron]);
    }
  throw Exception("Unknown PDG Code",__FUNCTION__);
  }

  int ThermalHadronGasFD::findStableHadronIndexFor(int pdgCode)
  {
  for (unsigned int iHadron=0; iHadron<_stableHadrons.size(); iHadron++)
    {
    if (_stableHadrons[iHadron]->type().pdgCode() == pdgCode)
      {
      //      printValue("findStableHadronIndexFor() pdgCode",pdgCode);
      //      printValue("findStableHadronIndexFor() iHadron",iHadron);
      return iHadron;
      }
    }
  return -1; //not found
  }

  void ThermalHadronGasFD::calculateAllProperties()
  {
  ThermalHadronGas::calculateAllProperties();
  calculateParticleDecayProbability();
  calculateStableDensities();
  }

}   // namespace CAP

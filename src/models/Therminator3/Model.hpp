#ifndef CAP__MODEL
#define CAP__MODEL
#include "String.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "Hypersurface.hpp"
#include "ModelSettings.hpp"
#include "ParticleFilter.hpp"
#include "ScalarField.hpp"
#include "ThermalHadronGas.hpp"
#include "Thermodynamics.hpp"
#include "VectorLorentz.hpp"
//#include <iostream>

using CAP::MATH::ScalarField;
using CAP::MATH::VectorLorentz;


namespace CAP
{

  class Model
  :
  public ModelSettings,
  public Thermodynamics,
  public Hypersurface
  {
  protected:
  ScalarField _distanceField;
  ScalarField _distanceFieldDPhi;
  ScalarField _distanceFieldDZeta;
  VectorLorentz _position;
  VectorLorentz _momentum;
  ThermalHadronGas _thermalHadronGas;
  double _maxIntegrandSafety;     //!< factor applied to the sampled maximum of the integrand in the accept/reject step
  unsigned long _nOverflows;      //!< accepted draws whose integrand exceeded the (scaled) maximum

  public:
  Model();
  virtual ~Model() {};
  //! @brief Model parameters from the configuration: the lengths and times (rhoMax, TauI, TauC, Delay) in fm,
  //! the temperature and the chemical potentials in GeV. The models work in GeV and GeV^-1, and the particles
  //! they emit have positions in fm.
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration);

  virtual void loadFrom(const Folder & file __attribute__((unused))) {}
  virtual void loadFromAscii(std::ifstream & file __attribute__((unused))) {}
  //! @brief Integrated hadron gas (integrand maxima and average multiplicities) written by saveToAscii().
  virtual void loadFromAscii(std::ifstream & file, const ParticleDb & particleDb) { _thermalHadronGas.loadFromAscii(file,particleDb); }
  virtual void saveTo(Folder & file __attribute__((unused))) {}
  //! @brief Write the integrated hadron gas (see ThermalHadronGas::saveToAscii()).
  virtual void saveToAscii(std::ofstream & file) { _thermalHadronGas.saveToAscii(file); }
  virtual void saveToAscii2(std::ofstream & file1 __attribute__((unused)),
                            std::ofstream & file2 __attribute__((unused))) {}

  virtual void print() const;

  //! @brief Volume of the hypercube the integrand is sampled in: _sVolume (space-time) and _pVolume (momentum).
  virtual void setHypercubeVolume();
  //! @brief Monte Carlo integration of the Cooper-Frye integrand (Therminator 2 Integrator::Integrate): for each
  //! hadron, the maximum of the integrand over _nSamples draws (stored with setIntegrand()) and the average
  //! multiplicity, volume times the mean integrand (stored with setAvgMultiplicity()).
  virtual void calculateMultiplicities();
  //! @brief Hadron gas of the model. @return hadron gas
  ThermalHadronGas & thermalHadronGas() { return _thermalHadronGas; }
  //! @brief Number of draws whose integrand exceeded the maximum used by the accept/reject step. @return count
  unsigned long nOverflows() const { return _nOverflows; }
  virtual double calculateIntegrand(ThermalHadron& hadron);
  virtual void generateEvent(Event & event,const ParticleFilter & particleFilter);

  inline double gammaAtBeta(double beta)
  {
  return 1.0/std::sqrt(1-beta*beta);
  }

  inline double gammaAtBetaSq(double betaSq)
  {
  return 1.0/std::sqrt(1-betaSq);
  }

  };

} // namespace CAP

#endif // !CAP__ModelH_

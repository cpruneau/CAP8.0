#ifndef DATUM_ThermalDerivedHistos
#define DATUM_ThermalDerivedHistos
#include "ThermalHistos.hpp"

namespace Datum
{

  class ThermalDerivedHistos : public ThermalHistos
  {
  public:

  ThermalDerivedHistos();
  ThermalDerivedHistos(const ThermalDerivedHistos & source);
  ThermalDerivedHistos operator=(const ThermalDerivedHistos & rhs);
  virtual ~ThermalDerivedHistos() {}
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration,
                         unsigned int index);
  virtual void create();
  virtual void loadFrom(const Folder & inputFile);

  virtual void calculateDerivedHistograms(const ThermalHistos & h);
  
  protected:

  virtual void cloneD(const ThermalDerivedHistos & source);


  };

} // namespace Datum

#endif /* DATUM_thermalHadronGasVsTempHistograms  */




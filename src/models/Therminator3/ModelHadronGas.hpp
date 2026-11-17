#ifndef DATUM__ModelHadronGas
#define DATUM__ModelHadronGas
#include "Model.hpp"

namespace Datum
{
  class ModelHadronGas
  :
  public Model
  {
  public:
  ModelHadronGas();
  virtual ~ModelHadronGas() {}
  virtual Model * clone() const { return new ModelHadronGas(*this); }
  virtual double calculateIntegrand(ThermalHadron & hadron);
  };

} // namespace Datum

#endif // !DATUM__ModelthermalHadronGas




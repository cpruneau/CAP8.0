#ifndef CAP__ModelHadronGas
#define CAP__ModelHadronGas
#include "Model.hpp"

namespace CAP
{
  class ModelHadronGas
  :
  public Model
  {
  public:
  ModelHadronGas();
  virtual ~ModelHadronGas() {}
  virtual double calculateIntegrand(ThermalHadron & hadron);
  };

} // namespace CAP

#endif // !CAP__ModelthermalHadronGas




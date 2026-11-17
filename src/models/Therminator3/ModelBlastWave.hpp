#ifndef DATUM__ModelBlastWave
#define DATUM__ModelBlastWave
#include "Model.hpp"

namespace Datum
{

  class ModelBlastWave
  :
  public Model
  {
  public:
  ModelBlastWave();
  virtual ~ModelBlastWave() {}
  virtual Model * clone() const { return new ModelBlastWave(*this); }
  virtual double calculateIntegrand(ThermalHadron & hadron);
  };

} // namespace Datum


#endif // !DATUM__ModelBlastWave

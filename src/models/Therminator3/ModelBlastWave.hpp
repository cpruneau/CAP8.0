#ifndef CAP__ModelBlastWave
#define CAP__ModelBlastWave
#include "Model.hpp"

namespace CAP
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

} // namespace CAP


#endif // !CAP__ModelBlastWave

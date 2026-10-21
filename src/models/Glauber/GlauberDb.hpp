#ifndef CAP__GlauberDb
#define CAP__GlauberDb
#include <ostream>
#include "Configuration.hpp"
#include "GlauberNucleus.hpp"
#include "GlauberNucleusType.hpp"
#include "ManagedObject.hpp"
#include "MathConstants.hpp"
#include "NNXSectVsSqrtS.hpp"
#include "NNProfile.hpp"
#include "NucleonProfile.hpp"
#include "PTotProfile.hpp"
#include "SmearingProfile.hpp"

using namespace std;
using CAP::MATH::pi;
using CAP::MATH::twoPi;

namespace CAP
{
  //!
  //! @brief Database of nucleus types for the Glauber simulations (numbers of nucleons and density parameters of the
  //! common nuclei).
  //!
  class GlauberDb
  :
  public ManagedObject
  {
  public:

  //! @brief Default constructor.
  GlauberDb();
  //! @brief Copy constructor. @param src object to copy
  GlauberDb(const GlauberDb & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  GlauberDb operator=(const GlauberDb & rhs);
  //! @brief Destructor.
  virtual ~GlauberDb() {}
  //! @brief Set a nucleus type from its name. @param name name (e.g., Pb, Au, Cu, p, d) @param nucleus type set
  void findNucleus(const String & name, GlauberNucleusType & nucleus);

  //! @brief Configure the database (nothing).
  virtual void configure(const String & taskName __attribute__((unused)),
                         const String & objectType __attribute__((unused)),
                         const Configuration & configuration __attribute__((unused)),
                         unsigned int index __attribute__((unused)) )
  { }




  ClassDef(GlauberDb,0)
  };

} // namespace CAP

#endif




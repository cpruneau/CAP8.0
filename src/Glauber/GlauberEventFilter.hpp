#ifndef CAP__GlauberEventFilter
#define CAP__GlauberEventFilter
#include "Filter.hpp"
#include "GlauberEvent.hpp"
#include "ManagedObject.hpp"

namespace CAP
{
  //!
  //! @brief Filter selecting Glauber events on the impact parameter, number of participants, number of binary
  //! collisions or number of spectators (all conditions must hold).
  //!
  class GlauberEventFilter
  :
  public Filter<GlauberEvent>,
  public ManagedObject
  {
  public:

  //!
  //! Filter types
  //!  kAny : all Glauber accepted.
  //!  kImpact: filtering based on impact parameter.
  //!  kNParts:  filtering based on number of participants.
  //!  kNColls:  filtering based on number of binary collisions.
  //!  kNSpecs: filtering based on number of spectators..
  //!
  enum GlauberEventFilterType { kAny, kImpact, kNParts, kNColls, kNSpecs};

  //! @brief Default constructor.
  GlauberEventFilter();
  //! @brief Copy constructor. @param source object to copy
  GlauberEventFilter(const GlauberEventFilter & source);
  //! @brief Copy assignment. @param source object to copy @return this object
  GlauberEventFilter & operator=(const GlauberEventFilter & source);
  //! @brief Destructor.
  virtual ~GlauberEventFilter() {}
  //! @brief Whether an event satisfies all the conditions. @param event event @return true if accepted
  virtual bool accept(const GlauberEvent & event) const;
  //! @brief Configure the object from keys \<ownerName\>:\<objectType\>:... (binning, options).
  //! @param ownerName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & ownerName,
                         const String & typeName,
                         const Configuration & configuration,
                         unsigned int index);
  
  ClassDef(GlauberEventFilter,0)
  };

} // namespace CAP

#endif /* CAP__GlauberEventFilter */

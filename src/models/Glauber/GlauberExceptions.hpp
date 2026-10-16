#ifndef CAP_GlauberExceptions
#define CAP_GlauberExceptions
#include "Exceptions.hpp"

namespace CAP
{
  
  //! @brief Base class of the exceptions of the Glauber simulations.
  class GlauberException : public Exception
  {
  public:
  //! @brief Constructor.
  GlauberException(const String & message,
                   const String & source,
                   const String & ec="GlauberException");
  //! @brief Destructor.
  virtual ~GlauberException() {}
  };
  
  //! @brief Exception thrown for an invalid parameter value.
  class InvalidValueGlauberException  : public GlauberException
  {
  public:
  //! @brief Constructor.
  InvalidValueGlauberException(double value,
                               const String & message,
                               const String & source,
                               const String & ec="InvalidValueGlauberException");
  //! @brief Destructor.
  virtual ~InvalidValueGlauberException() {}
  //! @brief Print the object.
  virtual void print() const;
  protected:
  double _value;  ///< invalid value
  };
  
  //! @brief Exception thrown when a nucleus is not in the Glauber database.
  class NucleusNotFoundGlauberException  : public GlauberException
  {
  public:
  //! @brief Constructor.
  NucleusNotFoundGlauberException(const String name,
                                  const String & message,
                                  const String & source,
                                  const String & ec="NucleusNotFoundGlauberException");
  //! @brief Destructor.
  virtual ~NucleusNotFoundGlauberException() {}
  //! @brief Print the object.
  virtual void print() const;
  protected:
  String _name;  ///< name of the nucleus
  };
  
  //! @brief Exception thrown for an unknown radial distribution type.
  class RadialTypeGlauberException  : public GlauberException
  {
  public:
  //! @brief Constructor.
  RadialTypeGlauberException(int type,
                             const String & message,
                             const String & source,
                             const String & ec="RadialTypeGlauberException");
  //! @brief Destructor.
  virtual ~RadialTypeGlauberException() {}
  //! @brief Print the object.
  virtual void print() const;
  protected:
  int _type;  ///< radial distribution type
  };
  
  //! @brief Exception thrown when nucleons cannot be placed within the maximum number of iterations.
  class IterationsGlauberException  : public GlauberException
  {
  public:
  //! @brief Constructor.
  IterationsGlauberException(int maxIterations,
                             int iterations,
                             const String & message,
                             const String & source,
                             const String & ec="OverlapGlauberException");
  //! @brief Destructor.
  virtual ~IterationsGlauberException() {}
  //! @brief Print the object.
  virtual void print() const;
  
  protected:
  int _maxIterations;  ///< maximum number of iterations
  int _iterations;  ///< number of iterations done
  };
  
} // namespace CAP

#endif

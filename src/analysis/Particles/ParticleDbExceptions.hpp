#ifndef CAP_ParticleDbExceptions
#define CAP_ParticleDbExceptions
#include "String.hpp"
#include "Exceptions.hpp"
namespace CAP
{

//! @brief Exception thrown for an invalid decay (e.g., a decay mode with an unsupported number of products).
class ParticleDecayException : public Exception
{
public:
  //! @brief Constructor.
  ParticleDecayException(const String  & _message, const String & _source);
  //! @brief Constructor.
  ParticleDecayException(int count, const String  & _message, const String & _source);
  //! @brief Destructor.
  virtual ~ParticleDecayException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  int _count;  ///< number of products
};

//! @brief Exception thrown for an invalid particle type index.
class InvalidIndexParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  InvalidIndexParticleTypeException(int index, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~InvalidIndexParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  int _index;  ///< invalid index
};

//! @brief Exception thrown when a particle type name is not in the database.
class UndefinedParticleTypeException : public Exception
  {
  public:
  //! @brief Constructor.
  UndefinedParticleTypeException(const String & name, const String & _source);
  //! @brief Destructor.
  virtual ~UndefinedParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

  protected:
  String _name;  ///< name of the particle type
  };


//! @brief Exception thrown when no particle type is selected.
class NoSelectedParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  NoSelectedParticleTypeException(const String & _source);
  //! @brief Destructor.
  virtual ~NoSelectedParticleTypeException() {}
};

//! @brief Exception thrown when a particle type does not exist.
class NonExistingParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  NonExistingParticleTypeException(const String & name, const String & _source);
  //! @brief Destructor.
  virtual ~NonExistingParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  String _name;  ///< name of the particle type
};

//! @brief Exception thrown when adding a particle type that already exists.
class ExistingParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  ExistingParticleTypeException(const String & name, const String & _source);
  //! @brief Destructor.
  virtual ~ExistingParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  String _name;  ///< name of the particle type
};

//! @brief Exception thrown for a null particle type pointer.
class NullPtrParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  NullPtrParticleTypeException(const String & _source);
  //! @brief Destructor.
  virtual ~NullPtrParticleTypeException() {}
};

//! @brief Exception thrown for an unknown particle type.
class UnknownParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  UnknownParticleTypeException(const String & _name,const String & _source);
  //! @brief Destructor.
  virtual ~UnknownParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  String _name;  ///< name of the particle type
};

//! @brief Exception thrown when a particle database file is not found.
class NotFoundFileParticleTypeException : public Exception
{
public:
  //! @brief Constructor.
  NotFoundFileParticleTypeException(const String & _pathName,
                                  const String & _fileName,
                                  const String & _source);
  //! @brief Destructor.
  virtual ~NotFoundFileParticleTypeException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  String _pathName;  ///< directory of the file
  String _fileName;  ///< name of the file
};

//! @brief Exception thrown when the particle database is empty.
class EmptyDbParticleDbException : public Exception
{
public:
  //! @brief Constructor.
  EmptyDbParticleDbException(const String & _message,
                             const String & _source);
  //! @brief Destructor.
  virtual ~EmptyDbParticleDbException() {}
  //! @brief Print the exception.
  virtual void print() const;

protected:
  String _dbName;  ///< name of the database

};

//! @brief Exception thrown for an invalid particle database file.
class InvalidFileParticleDbException : public Exception
{
public:
  //! @brief Constructor.
  InvalidFileParticleDbException(const String & _message,
                             const String & _source);
  //! @brief Destructor.
  virtual ~InvalidFileParticleDbException() {}
  //! @brief Print the exception.
  virtual void print() const;
};


} // namespace CAP

#endif

#ifndef CAP_Exceptions
#define CAP_Exceptions
#include <iostream>
#include <exception>
#include "Aliases.hpp"

//!
//! @file Exceptions.hpp
//! @brief Exception classes of CAP.
//!
//! All CAP exceptions derive from Exception, itself a std::exception: catch CAP::Exception (or std::exception) to
//! handle any of them, and call print() to report one. Each exception records a message, the function where it was
//! thrown (source), and the name of its class.
//!
//! Example:
//! @code
//!   try { task->initialize(); }
//!   catch (const CAP::Exception & e) { e.print(); }
//! @endcode
//!

namespace CAP
{

//!
//! @brief Base class of the CAP exceptions.
//!
class Exception : public std::exception
{
public:
  //! @brief Constructor. @param message description of the problem @param source function where it happened
  //! @param exceptionClass name of the exception class
  Exception(const String & message,
            const String & source,
            const String & exceptionClass="Exception");
  //! @brief Destructor.
  virtual ~Exception() {}
  //! @brief Print the exception class, source and message.
  virtual void print() const;
  //! Allows catching CAP exceptions through std::exception as well.
  virtual const char * what() const noexcept override { return _message.data(); }

protected:
  //! @name Data
  //! @{
  const String  _exception;  //!< name of the exception class
  const String  _source;     //!< function where the exception was thrown
  const String  _message;    //!< description of the problem
  //! @}
};

//!
//! @brief Exception thrown when a required pointer is null.
//!
class NullPointerException : public Exception
{
public:
  //! @brief Constructor (message "Null Pointer"). @param source function where it happened
  NullPointerException(const String & source);
  //! @brief Constructor. @param message description @param source function where it happened
  NullPointerException(const String & message, const String & source);
  //! @brief Destructor.
  virtual ~NullPointerException() {}
};

//!
//! @brief Exception thrown when an index is out of the valid range [0, bound).
//!
class IndexOutOfBoundException : public Exception
{
public:
  //! @brief Constructor. @param index invalid index @param bound size of the indexed range @param message description
  //! @param source function where it happened
  IndexOutOfBoundException(int index, int bound, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~IndexOutOfBoundException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;

protected:
  int _index;  //!< invalid index
  int _bound;  //!< size of the indexed range
};

//!
//! @brief Exception thrown when a name is invalid (e.g., empty or containing a forbidden character).
//!
class InvalidNameException : public Exception
{
public:
  //! @brief Constructor. @param name invalid name @param message description @param source function where it happened
  InvalidNameException(const String & name, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~InvalidNameException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;

protected:
  String _name;  //!< invalid name
};


//!
//! @brief Exception thrown when memory cannot be allocated or a memory constraint is violated.
//!
class MemoryException : public Exception
{
public:
  //! @brief Constructor. @param message description @param source function where it happened
  MemoryException(const String & message, const String & source);
  //! @brief Destructor.
  virtual ~MemoryException() {}
};

//!
//! @brief Exception thrown when a Factory cannot serve the requested number of objects.
//!
class FactoryException : public Exception
{
public:
  //! @brief Constructor. @param currentSize current size of the factory @param requestedSize requested size
  //! @param message description @param source function where it happened
  FactoryException(long currentSize,
                   long requestedSize,
                   const String & message,
                   const String & source);
  //! @brief Destructor.
  virtual ~FactoryException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
protected:
  long _currentSize;  //!< current size of the factory
  long _requestedSize;  //!< requested size
};



//!
//! @brief Exception thrown when a file cannot be opened, read or written.
//!
class FileException : public Exception
{
public:
  //! @brief Constructor. @param fileName name of the file @param message description @param source function where it happened
  FileException(const String & fileName, const String & message, const String & source);
  //! @brief Constructor. @param pathName directory of the file @param fileName name of the file @param message description
  //! @param source function where it happened
  FileException(const String & pathName, const String & fileName, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~FileException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
protected:
  const String  _pathName;  //!< directory of the file
  const String  _fileName;  //!< name of the file
};


//!
//! @brief Exception thrown when a function is not implemented by a class (e.g., Task::clone() not overridden).
//!
class NoImplementationException : public Exception
{
public:
  //! @brief Constructor. @param className class lacking the implementation @param source function concerned
  NoImplementationException(const String & className, const String & source);
  //! @brief Destructor.
  virtual ~NoImplementationException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
protected:
  const String _className;  //!< class lacking the implementation
};

//!
//! @brief Exception thrown when the end of a file is reached unexpectedly.
//!
class EndOfFileException : public FileException
{
public:
  //! @brief Constructor. @param fileName name of the file @param message description @param source function where it happened
  EndOfFileException(const String & fileName, const String & message, const String & source);
  //! @brief Constructor. @param pathName directory @param fileName name of the file @param message description
  //! @param source function where it happened
  EndOfFileException(const String & pathName, const String & fileName, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~EndOfFileException() {}
};

//!
//! @brief Exception thrown when no more data are available from a file or data source.
//!
class EndOfDataException : public FileException
{
public:
  //! @brief Constructor. @param fileName name of the file @param message description @param source function where it happened
  EndOfDataException(const String & fileName, const String & message, const String & source);
  //! @brief Constructor. @param pathName directory @param fileName name of the file @param message description
  //! @param source function where it happened
  EndOfDataException(const String & pathName, const String & fileName, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~EndOfDataException() {}
};

//!
//! @brief Exception thrown when a configuration parameter is missing or invalid.
//!
class ConfigurationException : public Exception
{
public:
  //! @brief Constructor. @param name key of the parameter @param message description @param source function where it happened
  ConfigurationException(const String & name, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~ConfigurationException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
protected:
  const String  _keyword;  //!< key of the parameter
};

//!
//! @brief Exception thrown when a histogram is missing or invalid.
//!
class HistogramException : public Exception
{
public:
  //! @brief Constructor. @param histogramName name of the histogram @param message description
  //! @param source function where it happened
  HistogramException(const String & histogramName, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~HistogramException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
protected:
  const String  _name;  //!< name of the histogram, class or resource
};


//!
//! @brief Exception thrown when histograms combined in an operation have incompatible binnings.
//!
class IncompatibleHistogramException : public Exception
{
public:
  //! @brief Constructor. @param source function where it happened
  IncompatibleHistogramException(const String & source);
  //! @brief Destructor.
  virtual ~IncompatibleHistogramException() {}
};

//!
//! @brief Exception thrown when histogram groups combined in an operation are incompatible.
//!
class IncompatibleHistogramGroupException : public Exception
{
public:
  //! @brief Constructor. @param message description @param source function where it happened
  IncompatibleHistogramGroupException(const String & message, const String & source);
  //! @brief Destructor.
  virtual ~IncompatibleHistogramGroupException() {}
};

  //!
  //! @brief Exception thrown when a class name is unknown (e.g., to ROOT's dictionary in Task::createTask()).
  //!
  class UnknownClassException : public Exception
  {
  public:
  //! @brief Constructor. @param name unknown class name @param source function where it happened
  UnknownClassException(const String & name, const String & source);
  //! @brief Destructor.
  virtual ~UnknownClassException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
  protected:
  const String  _name;  //!< name of the histogram, class or resource
  };

  //!
  //! @brief Exception thrown when a named resource (object, file, ...) cannot be found.
  //!
  class ResourceNotFoundException : public Exception
  {
  public:
  //! @brief Constructor. @param name name of the resource @param source function where it happened
  ResourceNotFoundException(const String & name, const String & source);
  //! @brief Destructor.
  virtual ~ResourceNotFoundException() {}
  //! @brief Print the exception and its details.
  virtual void print() const;
  protected:
  const String  _name;  //!< name of the histogram, class or resource
  };

} // namespace CAP

#endif

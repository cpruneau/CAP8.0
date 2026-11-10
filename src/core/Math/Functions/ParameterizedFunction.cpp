#include "ParameterizedFunction.hpp"

#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include "MathExceptions.hpp"
#include "BinaryIO.hpp"
#include "Exceptions.hpp"
#include "FunctionRegistry.hpp"
#include "ParameterizedComposite.hpp"


namespace CAP
{
  namespace MATH
  {

  ParameterizedFunction::ParameterizedFunction(unsigned int rank)
  :
  Function(rank),
  _parameters()
  {}

  ParameterizedFunction::ParameterizedFunction(const std::vector<Parameter> & parameters, unsigned int rank)
  :
  Function(rank),
  _parameters(parameters)
  {}

  ParameterizedFunction::ParameterizedFunction(const ParameterizedFunction & source)
  :
  Function(source),
  _parameters(source._parameters)
  {}

  ParameterizedFunction::~ParameterizedFunction()
  {}

  ParameterizedFunction & ParameterizedFunction::operator=(const ParameterizedFunction & rhs)
  {
  if (this != &rhs)
    {
    Function::operator=(rhs);
    _parameters = rhs._parameters;
    }
  return *this;
  }

  void ParameterizedFunction::addParameter(const Parameter & parameter)
  {
  _parameters.push_back(parameter);
  }

  const Parameter & ParameterizedFunction::parameter(unsigned int index) const
  {
  if (index >= _parameters.size())
    throw MathException("Parameter index out of range",__FUNCTION__);
  return _parameters[index];
  }

  ParameterizedFunction * ParameterizedFunction::clone() const
  {
  throw MathException(String("clone() is not implemented by the class ") + className(), __FUNCTION__);
  }

  void ParameterizedFunction::setParameterValue(unsigned int index, double value)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  _parameters[index].setValue(value);
  }

  void ParameterizedFunction::setParameterValues(const std::vector<double> & values)
  {
  if (values.size() != _parameters.size()) throw MathException("One value per parameter is needed",__FUNCTION__);
  for (unsigned int k=0; k<values.size(); k++) _parameters[k].setValue(values[k]);
  }

  std::vector<double> ParameterizedFunction::parameterValues() const
  {
  std::vector<double> values;
  for (const auto & p : _parameters) values.push_back(p.value());
  return values;
  }

  std::vector<unsigned int> ParameterizedFunction::freeParameters() const
  {
  std::vector<unsigned int> indices;
  for (unsigned int k=0; k<_parameters.size(); k++) if (_parameters[k].type() != Parameter::Fixed) indices.push_back(k);
  return indices;
  }

  void ParameterizedFunction::fixParameter(unsigned int index, bool fixed)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  Parameter & p = _parameters[index];
  if (fixed) { p.setType(Parameter::Fixed); return; }
  bool low = p.minimum() > Parameter::defaultMinimum, high = p.maximum() < Parameter::defaultMaximum;
  p.setType(low ? (high ? Parameter::LowHigh : Parameter::Low) : (high ? Parameter::High : Parameter::Free));
  }

  void ParameterizedFunction::setParameterLimits(unsigned int index, double minimum, double maximum)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  if (minimum > maximum) throw MathException("The minimum is above the maximum",__FUNCTION__);
  Parameter & p = _parameters[index];
  bool fixed = p.type() == Parameter::Fixed;
  p.setType(Parameter::Free);                         // the value is moved within the new bounds first
  double v = std::min(std::max(p.value(), minimum), maximum);
  p.setValue(v);
  p.setMinimum(minimum);
  p.setMaximum(maximum);
  if (fixed) p.setType(Parameter::Fixed);
  }

  void ParameterizedFunction::setParameter(unsigned int index, const Parameter & parameter)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  _parameters[index] = parameter;
  }

  namespace
  {
  // Function currently being evaluated by the vector version of evaluateAt() (per thread). The vector version
  // forwards to the 1, 2 or 3 argument versions, and these forward back to the vector version: this flag
  // detects the loop when a derived class implements neither, and turns it into an exception.
  thread_local const ParameterizedFunction * evaluatingVector = nullptr;

  struct EvaluatingVectorGuard
  {
  const ParameterizedFunction * saved;
  EvaluatingVectorGuard(const ParameterizedFunction * f) : saved(evaluatingVector) { evaluatingVector = f; }
  ~EvaluatingVectorGuard() { evaluatingVector = saved; }
  };

  const char * notImplementedMessage = "evaluateAt() is not implemented by the derived class";
  }

  double ParameterizedFunction::evaluateAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x});
  }

  double ParameterizedFunction::evaluateAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2});
  }

  double ParameterizedFunction::evaluateAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2,x3});
  }

  double ParameterizedFunction::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  EvaluatingVectorGuard guard(this);
  switch (_rank)
    {
    case 1: return evaluateAt(x[0]);
    case 2: return evaluateAt(x[0],x[1]);
    case 3: return evaluateAt(x[0],x[1],x[2]);
    default: throw MathException(notImplementedMessage,__FUNCTION__);
    }
  }

  // ---------------------------------------------------------------- binary files

  namespace
  {
  //! Name of the class of a composite in the records (its components are recorded, so it needs no registration).
  const char * kCompositeClass = "ParameterizedComposite";
  } // namespace

  void ParameterizedFunction::encodeBinary(BinaryWriter & w) const
  {
  String type;
  // a composite of no registered class of its own (e.g. one registered with registerComposite()): its components
  // are recorded; a registered class derived from ParameterizedComposite (e.g. FourierDecomposition) is made by its name
  if (dynamic_cast<const ParameterizedComposite *>(this) && !FunctionRegistry::isRegistered(*this)) type = kCompositeClass;
  else
    {
    if (!FunctionRegistry::isRegistered(*this))
      throw MathException(String("The class of function ") + name() + " (" + FunctionRegistry::typeName(*this)
                          + ") is not registered in FunctionRegistry: it cannot be saved", __FUNCTION__);
    type = FunctionRegistry::typeName(*this);
    }
  w.writeString(type);
  w.writeVarUInt(nParameters());
  w.writeVarUInt(rank());
  w.writeString(String(title()));
  for (const Parameter & p : parameters()) { w.writeString(p.name()); p.encodeBinary(w); }
  BinaryWriter state;
  encodeState(state);
  w.writeVarUInt(state.buffer().size());
  w.writeBytes(state.buffer().data(), state.buffer().size());
  _fits.encodeBinary(w);
  }

  ParameterizedFunction * ParameterizedFunction::decodeNewBinary(BinaryReader & r, const String & name, unsigned int version)
  {
  if (version == 0 || version > binaryVersion()) throw Exception("Function record of an unsupported format version", __FUNCTION__);
  String type = r.readString();
  uint64_t nParameters = r.readVarUInt();
  uint64_t rank = r.readVarUInt();
  if (nParameters > r.remaining() || rank == 0 || rank > 64) throw Exception("Invalid function record", __FUNCTION__);
  std::unique_ptr<ParameterizedFunction> f(type == kCompositeClass ? new ParameterizedComposite()
                                                                   : FunctionRegistry::create(type, (unsigned int)nParameters));
  if (!f) throw Exception(String("Functions of class ") + type + " with " + String(std::to_string(nParameters).c_str())
                          + " parameters cannot be made: is the class registered in FunctionRegistry?", __FUNCTION__);
  String title = r.readString();
  std::vector<Parameter> parameters;
  for (uint64_t k=0; k<nParameters; k++)
    {
    Parameter p;
    String parameterName = r.readString();
    p.decodeBinary(r, parameterName, Parameter::binaryVersion());
    parameters.push_back(p);
    }
  uint64_t stateLength = r.readVarUInt();
  if (stateLength > r.remaining()) throw Exception("Function state larger than the record", __FUNCTION__);
  BinaryReader state(r.readSpan(std::size_t(stateLength)), std::size_t(stateLength));
  f->decodeState(state, version);
  if (f->nParameters() != nParameters || f->rank() != rank)
    throw Exception(String("Function ") + name + " of class " + type + ": the parameters or the rank differ from those recorded", __FUNCTION__);
  for (unsigned int k=0; k<parameters.size(); k++) f->setParameter(k, parameters[k]);
  f->setName(name);
  f->setTitle(title);
  f->_fits.decodeBinary(r);
  return f.release();
  }

  ParameterizedFunction * ParameterizedFunction::readBinary(std::istream & in)
  {
  std::string tag, payload;
  unsigned int version;
  while (readBinaryRecord(in, tag, version, payload, "functions"))
    {
    if (tag != binaryTag()) continue;
    BinaryReader r(payload.data(), payload.size());
    String functionName = r.readString();
    std::unique_ptr<ParameterizedFunction> f(decodeNewBinary(r, functionName, version));
    if (r.remaining() != 0) throw Exception("Unexpected data at the end of a function record", __FUNCTION__);
    return f.release();
    }
  return nullptr;
  }

  } // namespace MATH
} // namespace CAP

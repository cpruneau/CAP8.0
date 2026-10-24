/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Reflection.hpp
//! @brief Run-time reflection independent of ROOT: classes registered by name, with their constructors, data
//! members and member functions, used through Value objects.
#ifndef CAP__Reflection
#define CAP__Reflection
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <variant>
#include <vector>
#include "String.hpp"
#include "Exceptions.hpp"

namespace CAP
{
  //!
  //! @brief Run-time reflection: what ROOT's dictionaries give (classes found by name, their members and functions,
  //! objects created by class name), without ROOT, rootcling or LinkDef files.
  //!
  //! A class is described once, in C++, by a ClassBuilder; its description (ClassInfo) is then found by name in the
  //! Registry. Arguments and results are Value objects: nothing, a boolean, an integer, a real, a text, or an object
  //! of a registered class.
  //! @code
  //!   CAP_REFLECTION(Histogram)                                      // in a .cpp file of the library of the class
  //!   {
  //!   REFLECTION::ClassBuilder<MATH::Histogram>("CAP::MATH::Histogram")
  //!     .constructor<String, String, unsigned int, double, double>()
  //!     .method("fill", static_cast<void (MATH::Histogram::*)(double, double)>(&MATH::Histogram::fill))
  //!     .method("nBins", &MATH::Histogram::nBins)
  //!     .property("name", &MATH::Histogram::name, &MATH::Histogram::setName);
  //!   }
  //!
  //!   const REFLECTION::ClassInfo * c = REFLECTION::Registry::instance().find("CAP::MATH::Histogram");
  //!   REFLECTION::Value h = c->create({ "h", "a histogram", 10, 0.0, 1.0 });
  //!   h.call("fill", { 0.25, 1.0 });
  //!   long n = h.call("nBins", { 0 }).toInteger();
  //! @endcode
  //!
  //! Limitations of this version: the functions are found in the class given and its registered bases (not in the
  //! classes derived from the static type of a returned pointer); a function with an argument or a result of an
  //! unregistered class type cannot be registered.
  //!
  namespace REFLECTION
  {

  class ClassInfo;

  //! @brief Exception of the reflection (unknown class or member, wrong arguments).
  class ReflectionException : public Exception
  {
  public:
  //! @brief Exception. @param message what happened @param source where
  ReflectionException(const String & message, const String & source) : Exception(message, source) { }
  };

  //! @brief An object of a registered class: its address, its class, and its owner if the reflection made it.
  struct Instance
  {
  void * pointer = nullptr;   //!< address of the object
  const ClassInfo * type = nullptr;   //!< class of the object (as known: the static type)
  std::shared_ptr<void> owner;   //!< owner of the object, if it was created by the reflection (null: owned elsewhere)
  };

  //!
  //! @brief Value passed to or returned by a function: nothing, a boolean, an integer, a real, a text, an object, or a
  //! list of values.
  //!
  //! Numbers convert to each other (a real to an integer only if it is a whole number), and to booleans. Lists
  //! convert to and from std::vector of numbers or texts.
  //!
  class Value
  {
  public:
  //! Kinds of values
  enum Kind { kNone, kBool, kInteger, kReal, kText, kInstance, kList };

  Value() : _value() { }   //!< Nothing
  Value(bool v) : _value(v) { }   //!< Boolean
  //! Integer (any integer type)
  template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
  Value(T v) : _value(long(v)) { }
  Value(double v) : _value(v) { }   //!< Real
  Value(float v) : _value(double(v)) { }   //!< Real
  Value(const char * v) : _value(std::string(v ? v : "")) { }   //!< Text
  Value(const std::string & v) : _value(v) { }   //!< Text
  Value(const String & v) : _value(v.str()) { }   //!< Text
  Value(const Instance & v) : _value(v) { }   //!< Instance
  Value(const std::vector<Value> & v) : _value(v) { }   //!< List

  Kind kind() const { return Kind(_value.index()); }   //!< Kind of the value
  bool isNone() const { return kind() == kNone; }   //!< Whether it holds nothing
  //! @brief The value as a boolean. @return value @throws ReflectionException if it is not a boolean or a number
  bool toBool() const;
  //! @brief The value as an integer. @return value @throws ReflectionException if it is not a whole number
  long toInteger() const;
  //! @brief The value as a real. @return value @throws ReflectionException if it is not a number
  double toReal() const;
  //! @brief The value as a text: a text as it is. @return value @throws ReflectionException if it is not a text
  const std::string & toText() const;
  //! @brief The object. @return object @throws ReflectionException if it is not an object
  const Instance & toInstance() const;
  //! @brief The list. @return values @throws ReflectionException if it is not a list
  const std::vector<Value> & toList() const;
  //! @brief The value written for a person: numbers, texts in quotes, objects as "<class at address>". @return text
  std::string print() const;
  //! @brief Name of the kind of the value ("integer", "text", the class of an object...). @return name
  std::string typeName() const;

  //! @name For objects
  //! @{
  //! @brief Call a member function. @param name function @param arguments arguments @return result
  Value call(const std::string & name, const std::vector<Value> & arguments={}) const;
  //! @brief Value of a data member or property. @param name member @return value
  Value get(const std::string & name) const;
  //! @brief Set a data member or property. @param name member @param value value
  void set(const std::string & name, const Value & value) const;
  //! @}

  private:
  std::variant<std::monostate, bool, long, double, std::string, Instance, std::vector<Value>> _value;   //!< the value
  };

  //! @brief A data member or a property (pair of functions): its name, its type, and how to read and write it.
  struct Field
  {
  std::string name;   //!< name
  std::string typeName;   //!< type, as written
  std::function<Value(void *)> get;   //!< value of the member of an object
  std::function<void(void *, const Value &)> set;   //!< set the member of an object (empty: read only)
  };

  //! @brief A member function (or a constructor): its name, its signature, and how to call it.
  struct Method
  {
  std::string name;   //!< name
  std::string signature;   //!< signature, as "double (int, double) const"
  int nArguments = 0;   //!< number of arguments
  std::function<Value(void *, const std::vector<Value> &)> invoke;   //!< call it on an object (null for constructors)
  };

  //!
  //! @brief Description of a registered class: name, bases, constructors, data members (and properties) and member
  //! functions. Made by a ClassBuilder, kept by the Registry.
  //!
  class ClassInfo
  {
  public:
  //! @brief Class of a name and a C++ type. @param name full name (e.g. "CAP::MATH::Histogram") @param type its type
  ClassInfo(const std::string & name, const std::type_info & type) : _name(name), _type(type) { }

  const std::string & name() const { return _name; }   //!< Full name of the class
  const std::type_info & type() const { return _type; }   //!< C++ type of the class
  //! @brief Registered direct bases of the class. @return bases (those not registered are left out)
  std::vector<const ClassInfo *> bases() const;
  //! @brief Whether the class is base or derives from it. @param base class @return true if so
  bool inheritsFrom(const ClassInfo & base) const;
  //! @brief Address of the base part of an object. @param pointer object of this class @param base class
  //! @return address of its base part, or null if base is not a base
  void * upcast(void * pointer, const ClassInfo & base) const;

  const std::vector<Method> & constructors() const { return _constructors; }   //!< Constructors
  const std::vector<Field> & fields() const { return _fields; }   //!< Data members and properties of this class
  const std::vector<Method> & methods() const { return _methods; }   //!< Member functions of this class
  //! @brief Data member or property, of this class or a base. @param name name @return field, or null
  const Field * field(const std::string & name) const;
  //! @brief Member functions of a name, of this class and its bases. @param name name @return functions
  std::vector<const Method *> methods(const std::string & name) const;
  //! @brief All the member functions, fields, of this class and its bases. @return list
  std::vector<const Method *> allMethods() const;
  std::vector<const Field *> allFields() const;   //!< All the fields, of this class and its bases

  //! @brief New object, made by the constructor whose arguments match. @param arguments arguments
  //! @return the object, owned by the value (and its copies) @throws ReflectionException if none matches
  Value create(const std::vector<Value> & arguments={}) const;
  //! @brief New object made with new by the constructor whose arguments match, owned by the caller (see create()
  //! for an object owned by a Value). @param arguments arguments @return address of the object
  //! @throws ReflectionException if no constructor matches
  void * construct(const std::vector<Value> & arguments={}) const;
  //! @brief Call a member function on an object (the first of that name whose arguments match).
  //! @param object object of this class @param name function @param arguments arguments @return result
  //! @throws ReflectionException if there is no such function, or no overload accepts the arguments
  Value call(void * object, const std::string & name, const std::vector<Value> & arguments) const;
  //! @brief Value of a data member or property of an object (of this class or a base). @param object object of this
  //! class @param name member @return value @throws ReflectionException if there is no such member
  Value get(void * object, const std::string & name) const;
  //! @brief Set a data member or property of an object. @param object object of this class @param name member
  //! @param value value @throws ReflectionException if there is no such member, or it is read only
  void set(void * object, const std::string & name, const Value & value) const;
  //! @brief Text describing the class: bases, constructors, fields, functions. @return text
  std::string describe() const;

  //! @name For ClassBuilder
  //! @{
  //! @brief Add a base. @param type its C++ type @param upcast address of the base part of an object
  void addBase(const std::type_info & type, std::function<void *(void *)> upcast) { _bases.push_back({ &type, upcast }); }
  void addConstructor(Method m) { _constructors.push_back(std::move(m)); }   //!< Add a constructor
  void addField(Field f) { _fields.push_back(std::move(f)); }   //!< Add a field
  Field & lastField() { return _fields.back(); }   //!< The field added last
  void addMethod(Method m) { _methods.push_back(std::move(m)); }   //!< Add a member function
  //! @brief Add how to make an object from arguments, with new (one function per constructor). @param f function
  void addConstruct(std::function<void *(const std::vector<Value> &)> f) { _constructs.push_back(std::move(f)); }
  //! @brief Set how to delete an object of the class. @param f function
  void setDestroy(std::function<void(void *)> f) { _destroy = std::move(f); }
  //! @}

  private:
  struct Base { const std::type_info * type; std::function<void *(void *)> upcast; };   //!< a direct base
  std::string _name;   //!< full name
  const std::type_info & _type;   //!< C++ type
  std::vector<Base> _bases;   //!< direct bases
  std::vector<Method> _constructors;   //!< constructors (signatures)
  std::vector<std::function<void *(const std::vector<Value> &)>> _constructs;   //!< one per constructor
  std::function<void(void *)> _destroy;   //!< delete an object of the class
  std::vector<Field> _fields;   //!< fields
  std::vector<Method> _methods;   //!< member functions
  };

  //!
  //! @brief All the registered classes, by name and by C++ type.
  //!
  class Registry
  {
  public:
  //! @brief The registry. @return registry
  static Registry & instance();
  //! @brief Class of a name. @param name full name, or the name without namespaces if it is unique @return class, or null
  const ClassInfo * find(const std::string & name) const;
  //! @brief Class of a C++ type. @param type type @return class, or null
  const ClassInfo * find(const std::type_info & type) const;
  //! @brief Names of the registered classes (sorted). @param base if not null: only the classes deriving from it
  //! @return names
  std::vector<std::string> names(const ClassInfo * base=nullptr) const;
  //! @brief Add (or replace) a class. @param info description @return the class registered
  ClassInfo & add(std::unique_ptr<ClassInfo> info);

  private:
  Registry() { }
  std::map<std::string, std::unique_ptr<ClassInfo>> _byName;   //!< classes by full name
  std::map<std::type_index, ClassInfo *> _byType;   //!< classes by type
  };

  //! @brief Class registered for a C++ type. @return class, or null
  template <typename T> const ClassInfo * classOf() { return Registry::instance().find(typeid(T)); }

  //! @brief Address of an object as a T (its class, or a base). @param value object @param target class T
  //! @return address @throws ReflectionException if the value is not an object of T or of a class deriving from it
  void * objectPointer(const Value & value, const ClassInfo * target, const char * typeName);

  //! @cond DETAIL
  namespace Detail
  {
  //! Readable name of a C++ type.
  std::string typeName(const std::type_info & type);

  template <typename T> using Plain = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

  //! An argument of type A: how it is kept during the call (Stored), and how it is given to the function.
  template <typename A, typename Enable=void>
  struct Argument
  {
  // objects of registered classes, by reference or by value: kept as pointers
  using D = Plain<A>;
  using Stored = D *;
  static Stored make(const Value & v) { return static_cast<D *>(objectPointer(v, classOf<D>(), typeid(D).name())); }
  static D & get(Stored & s) { return *s; }
  };
  template <typename A>
  struct Argument<A, typename std::enable_if<std::is_arithmetic<Plain<A>>::value || std::is_enum<Plain<A>>::value>::type>
  {
  using D = Plain<A>;
  using Stored = D;
  static Stored make(const Value & v)
  {
  if constexpr (std::is_same<D, bool>::value) return v.toBool();
  else if constexpr (std::is_enum<D>::value || std::is_integral<D>::value) return D(v.toInteger());
  else return D(v.toReal());
  }
  static D & get(Stored & s) { return s; }
  };
  template <typename A>
  struct Argument<A, typename std::enable_if<std::is_same<Plain<A>, std::string>::value || std::is_same<Plain<A>, String>::value>::type>
  {
  using D = Plain<A>;
  using Stored = D;
  static Stored make(const Value & v) { return D(v.toText()); }
  static D & get(Stored & s) { return s; }
  };
  template <typename T> struct IsVector : std::false_type { };
  template <typename T> struct IsVector<std::vector<T>> : std::true_type { };
  template <typename A>
  struct Argument<A, typename std::enable_if<IsVector<Plain<A>>::value>::type>
  {
  // a list: each element converted
  using D = Plain<A>;
  using Stored = D;
  static Stored make(const Value & v)
  {
  D list;
  for (const Value & e : v.toList())
    {
    typename Argument<typename D::value_type>::Stored s = Argument<typename D::value_type>::make(e);
    list.push_back(Argument<typename D::value_type>::get(s));
    }
  return list;
  }
  static D & get(Stored & s) { return s; }
  };
  template <typename A>
  struct Argument<A, typename std::enable_if<std::is_same<Plain<A>, const char *>::value>::type>
  {
  using Stored = std::string;
  static Stored make(const Value & v) { return v.toText(); }
  static const char * get(Stored & s) { return s.c_str(); }
  };
  template <typename A>
  struct Argument<A, typename std::enable_if<std::is_pointer<Plain<A>>::value && !std::is_same<Plain<A>, const char *>::value>::type>
  {
  using P = typename std::remove_cv<typename std::remove_pointer<Plain<A>>::type>::type;
  using Stored = Plain<A>;
  static Stored make(const Value & v) { return v.isNone() ? nullptr : static_cast<Stored>(objectPointer(v, classOf<P>(), typeid(P).name())); }
  static Stored get(Stored & s) { return s; }
  };

  //! Instance of an object of static type P: of its dynamic type if P is polymorphic and that class is registered.
  template <typename P>
  Instance instance(const P * object)
  {
  Instance o;
  o.pointer = const_cast<void *>(static_cast<const void *>(object));
  o.type = classOf<P>();
  if constexpr (std::is_polymorphic<P>::value)
    if (const ClassInfo * dynamic = Registry::instance().find(typeid(*object)))
      if (dynamic != o.type) { o.type = dynamic; o.pointer = const_cast<void *>(dynamic_cast<const void *>(object)); }
  if (!o.type) throw ReflectionException(String("Unregistered class: ") + String(typeName(typeid(P))), "Reflection");
  return o;
  }

  //! A result of type R as a Value.
  template <typename R>
  Value result(R && r)
  {
  using D = Plain<R>;
  if constexpr (std::is_same<D, bool>::value || std::is_arithmetic<D>::value) return Value(r);
  else if constexpr (std::is_enum<D>::value) return Value(long(r));
  else if constexpr (std::is_same<D, std::string>::value || std::is_same<D, String>::value || std::is_same<D, const char *>::value
                     || std::is_same<D, char *>::value) return Value(r);
  else if constexpr (IsVector<D>::value)
    {
    std::vector<Value> list;
    for (const auto & e : r) list.push_back(result<const typename D::value_type &>(e));
    return Value(list);
    }
  else if constexpr (std::is_pointer<D>::value)
    {
    using P = typename std::remove_cv<typename std::remove_pointer<D>::type>::type;
    if (!r) return Value();
    return Value(instance<P>(r));
    }
  else
    {
    if constexpr (std::is_lvalue_reference<R>::value) return Value(instance<D>(&r));   // a reference: the object itself
    const ClassInfo * c = classOf<D>();
    if (!c) throw ReflectionException(String("Result of an unregistered class: ") + String(typeName(typeid(D))), "Reflection");
    Instance o;
    o.type = c;
    if constexpr (!std::is_lvalue_reference<R>::value)
      {
      std::shared_ptr<D> copy = std::make_shared<D>(std::forward<R>(r));  // a value: a copy owned by the Value
      o.pointer = copy.get();
      o.owner = copy;
      }
    return Value(o);
    }
  }

  //! Call f with the arguments converted.
  template <typename R, typename... A, typename F, std::size_t... I>
  Value apply(F && f, [[maybe_unused]] const std::vector<Value> & arguments, std::index_sequence<I...>)
  {
  std::tuple<typename Argument<A>::Stored...> stored{ Argument<A>::make(arguments[I])... };   // converted in order
  (void)stored;
  if constexpr (std::is_void<R>::value) { f(Argument<A>::get(std::get<I>(stored))...); return Value(); }
  else return result<R>(f(Argument<A>::get(std::get<I>(stored))...));
  }

  //! Signature of a function, as "R (A1, A2)".
  template <typename R, typename... A>
  std::string signature(const char * suffix="")
  {
  std::string s = typeName(typeid(R)) + " (";
  std::vector<std::string> names = { typeName(typeid(A))... };
  for (std::size_t k=0; k<names.size(); k++) s += (k ? ", " : "") + names[k];
  return s + ")" + suffix;
  }
  } // namespace Detail
  //! @endcond

  //! @brief Value referring to an object owned elsewhere (of its dynamic class if registered). @param object object
  //! @return value
  template <typename T> Value reference(T & object) { return Value(Detail::instance<T>(&object)); }
  //! @brief Value owning an object made with new (deleted with the last copy of the value). @param object object
  //! @return value
  template <typename T> Value owned(T * object)
  {
  Instance o = Detail::instance<T>(object);
  o.owner = std::shared_ptr<T>(object);
  return Value(o);
  }


  //!
  //! @brief Describe a class T and register it (when the builder is destroyed: at the end of the statement).
  //!
  //! Each function adds a part of the class and returns the builder, so that the parts can be chained. Overloaded
  //! functions are given with a static_cast choosing one of them.
  //!
  template <typename T>
  class ClassBuilder
  {
  public:
  //! @brief Builder of the class name. @param name full name of the class (e.g. "CAP::MATH::Histogram")
  explicit ClassBuilder(const std::string & name) : _info(new ClassInfo(name, typeid(T))) { }
  //! @brief Register the class.
  ~ClassBuilder() { if (_info) Registry::instance().add(std::move(_info)); }
  ClassBuilder(const ClassBuilder &) = delete;
  ClassBuilder & operator=(const ClassBuilder &) = delete;

  //! @brief A base class B of T (to be registered too, before or after T). @return builder
  template <typename B>
  ClassBuilder & base()
  {
  static_assert(std::is_base_of<B, T>::value, "not a base");
  _info->addBase(typeid(B), [](void * p) -> void * { return static_cast<B *>(static_cast<T *>(p)); });
  return *this;
  }

  //! @brief A constructor of T with arguments A... @return builder
  template <typename... A>
  ClassBuilder & constructor()
  {
  Method m;
  m.name = _info->name();
  m.signature = Detail::signature<void, A...>();
  m.signature = m.signature.substr(m.signature.find('('));
  m.nArguments = int(sizeof...(A));
  _info->addConstructor(m);
  _info->addConstruct([](const std::vector<Value> & arguments) -> void *
    {
    T * object = nullptr;
    Detail::apply<void, A...>([&object](A... a) { object = new T(a...); }, arguments, std::index_sequence_for<A...>());
    return object;
    });
  _info->setDestroy([](void * p) { delete static_cast<T *>(p); });
  return *this;
  }

  //! @brief A member function. @param name name @param f function @return builder
  template <typename R, typename C, typename... A>
  ClassBuilder & method(const std::string & name, R (C::*f)(A...))
  {
  static_assert(std::is_base_of<C, T>::value, "not a member function of the class or of a base");
  return add(name, Detail::signature<R, A...>(), int(sizeof...(A)), [f](void * p, const std::vector<Value> & arguments)
    { T * o = static_cast<T *>(p); return Detail::apply<R, A...>([o, f](A... a) -> R { return (o->*f)(std::forward<A>(a)...); }, arguments, std::index_sequence_for<A...>()); });
  }
  //! @brief A const member function. @param name name @param f function @return builder
  template <typename R, typename C, typename... A>
  ClassBuilder & method(const std::string & name, R (C::*f)(A...) const)
  {
  static_assert(std::is_base_of<C, T>::value, "not a member function of the class or of a base");
  return add(name, Detail::signature<R, A...>(" const"), int(sizeof...(A)), [f](void * p, const std::vector<Value> & arguments)
    { const T * o = static_cast<T *>(p); return Detail::apply<R, A...>([o, f](A... a) -> R { return (o->*f)(std::forward<A>(a)...); }, arguments, std::index_sequence_for<A...>()); });
  }
  //! @brief A function acting on objects of T, called as a member function (an extension, or a static function).
  //! @param name name @param f function whose first argument is the object (T & or const T &) @return builder
  template <typename R, typename O, typename... A>
  ClassBuilder & function(const std::string & name, R (*f)(O, A...))
  {
  return add(name, Detail::signature<R, A...>(), int(sizeof...(A)), [f](void * p, const std::vector<Value> & arguments)
    { T * o = static_cast<T *>(p); return Detail::apply<R, A...>([o, f](A... a) -> R { return f(*o, std::forward<A>(a)...); }, arguments, std::index_sequence_for<A...>()); });
  }

  //! @brief A public data member. @param name name @param member member @return builder
  template <typename M, typename C>
  ClassBuilder & field(const std::string & name, M C::*member)
  {
  Field f;
  f.name = name;
  f.typeName = Detail::typeName(typeid(M));
  f.get = [member](void * p) { return Detail::result<const M &>(static_cast<T *>(p)->*member); };
  if constexpr (std::is_copy_assignable<M>::value && !std::is_const<M>::value)
    f.set = [member](void * p, const Value & v)
      { typename Detail::Argument<M>::Stored s = Detail::Argument<M>::make(v); static_cast<T *>(p)->*member = Detail::Argument<M>::get(s); };
  _info->addField(f);
  return *this;
  }
  //! @brief A property read by a member function. @param name name @param getter function returning the value
  //! @return builder
  template <typename G, typename C1>
  ClassBuilder & property(const std::string & name, G (C1::*getter)() const)
  {
  Field f;
  f.name = name;
  f.typeName = Detail::typeName(typeid(Detail::Plain<G>));
  f.get = [getter](void * p) { return Detail::result<G>((static_cast<const T *>(p)->*getter)()); };
  _info->addField(f);
  return *this;
  }
  //! @brief A property: a value read and written by two member functions. @param name name @param getter function
  //! returning the value @param setter function setting it @return builder
  template <typename G, typename C1, typename S, typename C2>
  ClassBuilder & property(const std::string & name, G (C1::*getter)() const, void (C2::*setter)(S))
  {
  property(name, getter);
  _info->lastField().set = [setter](void * p, const Value & v)
    { typename Detail::Argument<S>::Stored s = Detail::Argument<S>::make(v); (static_cast<T *>(p)->*setter)(Detail::Argument<S>::get(s)); };
  return *this;
  }

  private:
  std::unique_ptr<ClassInfo> _info;   //!< class described

  ClassBuilder & add(const std::string & name, const std::string & signature, int n,
                     std::function<Value(void *, const std::vector<Value> &)> invoke)
  {
  Method m;
  m.name = name;
  m.signature = signature;
  m.nArguments = n;
  m.invoke = std::move(invoke);
  _info->addMethod(m);
  return *this;
  }
  };

  } // namespace REFLECTION
} // namespace CAP

//! @brief Register classes when a library is loaded: CAP_REFLECTION(Tag) { ClassBuilder<...>(...)...; }
#define CAP_REFLECTION(tag)                                                         \
  static void capReflection_##tag();                                                \
  namespace { const bool capReflectionDone_##tag = (capReflection_##tag(), true); } \
  static void capReflection_##tag()

#endif // !CAP__Reflection

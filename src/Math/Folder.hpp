#ifndef CAP_MATH_Folder
#define CAP_MATH_Folder
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <ostream>
#include "Aliases.hpp"
#include "Exceptions.hpp"
#include "BinaryIO.hpp"
#include "Object.hpp"
#include "Function.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace MATH
  {

  class Folder;

  //!
  //! @brief Type-erased entry of a Folder: owns one object of any type (internal to Folder).
  //!
  class FolderItem
  {
  public:
  virtual ~FolderItem() {}
  //! @brief Deep copy of the entry (and of its object). @throws Exception if the object type cannot be copied
  virtual FolderItem * clone() const = 0;
  //! @brief Type of the object. @return type information
  virtual const std::type_info & type() const = 0;
  //! @brief Readable name of the object type. @return type name
  virtual std::string typeName() const = 0;
  //! @brief Address of the object (of its stored type). @return pointer
  virtual void * pointer() const = 0;
  //! @brief The object as an Object, if it is one. @return pointer or nullptr
  virtual Object * asObject() const = 0;
  //! @brief The object as a Function, if it is one. @return pointer or nullptr
  virtual Function * asFunction() const = 0;
  //! @brief The object as DataPoints, if it is one. @return pointer or nullptr
  virtual DataPoints * asDataPoints() const = 0;
  //! @brief The object as a Folder, if it is one. @return pointer or nullptr
  virtual Folder * asFolder() const = 0;
  //! @brief Whether the object can be saved in a binary file. @return true if savable
  virtual bool savable() const = 0;
  //! @brief Write the object as one binary record. @param out stream @param skipUnsavable passed to sub-folders
  //! @throws Exception if the object cannot be saved
  virtual void writeRecord(std::ostream & out, bool skipUnsavable) const = 0;
  //! @brief Give up ownership of the object. @return pointer to the object (the entry no longer deletes it)
  virtual void * release() = 0;
  //! @brief Whether objects of this type can be added and subtracted. @return true if they can
  virtual bool combinable() const = 0;
  //!
  //! @brief New entry holding the sum (or difference) of this entry's object and other's.
  //! @param other entry of the same type @param subtract true for this - other @param path path of the entry, for messages
  //! @return new entry (owned by the caller)
  //! @throws Exception if the types differ, the type has no arithmetic, or the objects are incompatible
  //!
  virtual FolderItem * combine(const FolderItem & other, bool subtract, const std::string & path) const = 0;
  };

  //! @brief Whether T has T + T and T - T.
  template <typename T, typename = void>
  struct HasPlusMinus : std::false_type {};
  template <typename T>
  struct HasPlusMinus<T, std::void_t<decltype(T(std::declval<const T &>() + std::declval<const T &>())),
                                     decltype(T(std::declval<const T &>() - std::declval<const T &>()))>> : std::true_type {};

  //! @brief Whether T can be saved in CAP binary files (it provides binaryTag() and encodeBinary()).
  template <typename T, typename = void>
  struct IsBinarySavable : std::false_type {};
  template <typename T>
  struct IsBinarySavable<T, std::void_t<decltype(T::binaryTag()),
                                        decltype(std::declval<const T &>().encodeBinary(std::declval<BinaryWriter &>()))>> : std::true_type {};

  //! @brief Readable (demangled) name of a type. @param type type information @return name
  std::string demangledTypeName(const std::type_info & type);

  //!
  //! @brief Container of named objects of any type (histograms, data points, functions, fields, ...) and of
  //! sub-folders, forming a tree, with save and load of the whole tree to and from CAP binary files.
  //!
  //! <b>Ownership.</b> The folder owns its objects: add() takes an object created with new, and the folder deletes it
  //! when the object is removed, replaced or the folder destroyed. release() gives an object back to the caller.
  //! Copying a folder copies all its objects (deep copy).
  //!
  //! <b>Names and paths.</b> Each entry has a name, unique in its folder, that cannot contain '/'. Objects are found by
  //! name or by path through sub-folders, e.g. get<Histogram>("PtPt/pairs/n2"). Entries keep their insertion order.
  //!
  //! <b>Navigation.</b> Each folder knows its parent. cd() moves the current folder of this folder, like a shell:
  //! cd("name") or cd("a/b") descends, cd("..") or cd() goes up to the parent (if there is one), cd("/") goes to the
  //! top of the tree. The operations on entries (add, addFolder, get, folder, contains, typeName, remove, release,
  //! names, ls) act on the current folder; their paths can use ".." and "." and, with a leading "/", start at the top
  //! of the tree. Until cd() is called the current folder is this folder, so these operations act on it directly.
  //! Operations on the whole folder (copy, save, load, clear, size, paths, print) do not depend on the current folder.
  //! The current folder is remembered by its path: if that branch is removed, the current folder becomes this folder.
  //!
  //! <b>Types.</b> get<T>() returns nullptr if the entry does not exist or is not a T. It works through the base
  //! classes Object, Function and DataPoints and their derived classes: a Histogram can be obtained as Histogram,
  //! DataPoints or Object, a Polynomial as Polynomial, ParameterizedFunction or Function. Add objects with their
  //! concrete type (e.g. add(new Polynomial(...))), so that they can be copied and saved.
  //!
  //! <b>Binary files.</b> A folder is saved as a "CAPO" record holding the records of its entries, sub-folders included
  //! (see BinaryIO.hpp). Objects are rebuilt with their types when loading, through a registry of types: Histogram,
  //! DataPointsVector, DataPoint2D, ScalarField, Matrix, VectorND, Vector2D, Vector3D, VectorLorentz, Parameter and
  //! Folder are registered automatically; other classes with binary support are added with registerType<T>().
  //! Objects without binary support (e.g. functions) cannot be saved: saveBinary() throws unless skipUnsavable is true.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Folder run("run42");
  //!   Folder * pairs = run.addFolder("PtPt")->addFolder("pairs");
  //!   pairs->add(new Histogram("n2","pairs",100,0.0,10.0));
  //!   run.add(new Matrix(3,3,1.0),"covariance");
  //!   Histogram * h = run.get<Histogram>("PtPt/pairs/n2");
  //!   run.cd("PtPt/pairs");                 // like a shell
  //!   run.add(new Histogram("n1","singles",100,0.0,10.0));   // added to PtPt/pairs
  //!   run.cd();                             // up to PtPt
  //!   std::cout << run.pwd() << std::endl;  // "/PtPt"
  //!   run.saveBinary("run42.cap");
  //!   Folder copy;
  //!   copy.loadBinary("run42.cap");
  //! @endcode
  //!
  class Folder
  {
  public:

  //! @brief Constructor: empty folder. @param name name of the folder (no '/')
  //! @throws Exception if the name contains '/'
  Folder(const String & name="");
  //! @brief Copy constructor: deep copy of all objects and sub-folders. @param source folder to copy
  //! @throws Exception if an object cannot be copied
  Folder(const Folder & source);
  //! @brief Assignment: deep copy of all objects and sub-folders (the current content is deleted). @param rhs folder to copy @return this folder
  //! @throws Exception if an object cannot be copied (this folder is then unchanged)
  Folder & operator=(const Folder & rhs);
  //! @brief Destructor: deletes all objects and sub-folders.
  virtual ~Folder();

  //! @brief Name of the folder. @return name
  const String & name() const { return _name; }
  //! @brief Set the name of the folder. @param name new name (no '/') @throws Exception if the name contains '/'
  void setName(const String & name);

  //! @name Navigation
  //! @{

  //! @brief Folder that contains this one. @return parent, or nullptr for a top folder
  Folder * parent() const { return _parent; }
  //! @brief Top of the tree this folder belongs to. @return top folder (this one if it has no parent)
  Folder * top() const;
  //! @brief Path of this folder from the top of its tree, e.g. "/PtPt/pairs" ("/" for the top folder). @return path
  String path() const;
  //!
  //! @brief Change the current folder, like a shell.
  //! @param path sub-folder name, path "a/b", ".." for the parent, "." for the current folder, "/" or "/a/b" from the top
  //! @return the new current folder, or nullptr if there is no such folder (the current folder is then unchanged)
  //!
  Folder * cd(const String & path);
  //! @brief Go up to the parent of the current folder, if it has one. @return the new current folder, or nullptr (unchanged) if there is no parent
  Folder * cd();
  //! @brief Current folder (this folder until cd() is called). @return current folder
  Folder * current() const;
  //! @brief Path of the current folder from the top of the tree. @return path, e.g. "/PtPt"
  String pwd() const { return current()->path(); }
  //! @brief Print the entries of the current folder (or of the folder at path) with their types. @param out stream @param path folder (empty: the current one)
  void ls(std::ostream & out, const String & path="") const;

  //! @}

  //!
  //! @brief Add an object, of which the folder takes ownership.
  //! @param object object created with new (not nullptr)
  //! The object is added to the current folder (see cd()).
  //! @param name name of the entry; if empty, the object's own name is used (binaryName() or name(), when it has one)
  //! @return object
  //! @throws Exception if object is nullptr, the name is empty or contains '/', or an entry with that name exists
  //! (the object is then deleted, since the folder was given ownership)
  //!
  template <typename T>
  T * add(T * object, const String & name="");

  //!
  //! @brief Create a sub-folder of the current folder, or return the existing one with that name.
  //! @param name name of the sub-folder (no '/')
  //! @return sub-folder (owned by this folder)
  //! @throws Exception if an entry with that name exists and is not a folder
  //!
  Folder * addFolder(const String & name);

  //!
  //! @brief Object at a path, as a T.
  //! @param path name of the entry, or path through sub-folders ("a/b/name")
  //! @return the object, or nullptr if there is no such entry or it is not a T
  //!
  template <typename T>
  T * get(const String & path) const;

  //! @brief Sub-folder at a path. @param path name or path @return the folder, or nullptr if there is none
  Folder * folder(const String & path) const;
  //! @brief Whether an entry exists at a path. @param path name or path @return true if it exists
  bool contains(const String & path) const;
  //! @brief Readable type name of the entry at a path. @param path name or path @return type name, or "" if there is no such entry
  std::string typeName(const String & path) const;
  //! @brief Delete the entry at a path. @param path name or path @return true if an entry was deleted
  bool remove(const String & path);
  //!
  //! @brief Remove the entry at a path without deleting its object, which the caller then owns.
  //! @param path name or path
  //! @return the object, or nullptr (nothing removed) if there is no such entry or it is not a T
  //!
  template <typename T>
  T * release(const String & path);
  //! @brief Delete all entries of this folder (and its whole tree); the current folder becomes this folder.
  void clear();

  //! @brief Number of entries (objects and sub-folders) directly in this folder (not the current one). @return number of entries
  unsigned int size() const { return _entries.size(); }
  //! @brief Whether the folder has no entries. @return true if empty
  bool empty() const { return _entries.empty(); }
  //! @brief Names of the entries of the current folder, in insertion order. @return names
  std::vector<String> names() const;
  //! @brief Paths of all objects in this folder and its sub-folders (sub-folders themselves not included). @return paths
  std::vector<String> paths() const;
  //! @brief Paths of the objects that cannot be saved in a binary file. @return paths
  std::vector<String> unsavablePaths() const;
  //! @brief Print the tree of entries with their types to a stream. @param out stream @param indent indentation
  void print(std::ostream & out, unsigned int indent=0) const;

  //! @name Arithmetic
  //! Folders with the same structure (same entry names in every folder of the tree, entries of the same types) are
  //! added or subtracted entry by entry: the result has the structure, entry order and names of the left folder, and
  //! each object is the sum or difference of the corresponding objects (with their own rules: e.g. Histogram adds bin
  //! contents and propagates uncertainties, DataPointsVector adds points with compatible x values). Types without
  //! arithmetic (functions, Parameter) cannot be combined. Errors give the path of the offending entry.
  //! These operate on whole folders, not on the current folders.
  //! @{

  //!
  //! @brief Whether other has the same structure as this folder (same names and types throughout the tree).
  //! @param other folder to compare to @param reason output (optional): description of the first difference
  //! @return true if the structures are the same
  //!
  bool sameStructure(const Folder & other, std::string * reason=nullptr) const;
  //!
  //! @brief Sum of two folders with the same structure.
  //! @param left first folder @param right second folder @param name name of the result (empty: that of left)
  //! @return new folder holding the sums
  //! @throws Exception if the structures differ, an entry type has no arithmetic, or objects are incompatible
  //!
  static Folder add(const Folder & left, const Folder & right, const String & name="");
  //!
  //! @brief Difference of two folders with the same structure (left - right); see add().
  //! @param left first folder @param right second folder @param name name of the result (empty: that of left)
  //! @return new folder holding the differences
  //! @throws Exception if the structures differ, an entry type has no arithmetic, or objects are incompatible
  //!
  static Folder subtract(const Folder & left, const Folder & right, const String & name="");
  //! @brief Replace this folder's objects by their sums with those of rhs. @param rhs folder with the same structure @return this folder
  //! @throws Exception as add() (this folder is then unchanged)
  Folder & operator+=(const Folder & rhs);
  //! @brief Replace this folder's objects by their differences with those of rhs. @param rhs folder with the same structure @return this folder
  //! @throws Exception as subtract() (this folder is then unchanged)
  Folder & operator-=(const Folder & rhs);

  //! @}

  //! @name Binary files
  //! @{

  //! @brief Tag of folder records in binary files. @return "CAPO"
  static const char * binaryTag() { return "CAPO"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return _name; }
  //! @brief Write the binary payload after the name. @param w writer @throws Exception if an object cannot be saved
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Write the binary payload after the name. @param w writer @param skipUnsavable if true, objects without binary support are left out
  //! @throws Exception if an object cannot be saved and skipUnsavable is false
  void encodeBinary(BinaryWriter & w, bool skipUnsavable) const;
  //! @brief Rebuild this folder from a binary payload. @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid or holds a type that is not registered
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this folder (with its tree) as one binary record to a stream. @param out stream
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if writing fails or an object cannot be saved
  void writeBinary(std::ostream & out, bool skipUnsavable=false) const;
  //! @brief Read the next folder record of a stream into this folder (replacing its content), skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this folder (with its tree) to a binary file. @param fileName file name @param append add at the end of the file
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if the file cannot be written or an object cannot be saved
  void saveBinary(const String & fileName, bool append=false, bool skipUnsavable=false) const;
  //! @brief Load from a binary file the first folder, or the first one with the given name (replacing the content of this one).
  //! @param fileName file name @param name name of the folder (empty: the first one)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no such folder (this one is then unchanged)
  void loadBinary(const String & fileName, const String & name="");
  //! @brief Load all the folders of a binary file, in order. @param fileName file name @return folders
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Folder> loadAllBinary(const String & fileName);

  //!
  //! @brief Register a class with binary support so that folders can load its objects.
  //!
  //! Needed only for classes outside Math (the Math classes and Folder are registered automatically); registering a
  //! class twice is harmless.
  //!
  template <typename T>
  static void registerType();

  //! @}

  protected:

  //! @brief Entry: name and owned object.
  struct Entry
  {
    std::string name;
    std::unique_ptr<FolderItem> item;
  };
  //! @brief Factory rebuilding an object from a binary record payload.
  using Factory = std::function<FolderItem *(const std::string & payload, unsigned int version, const String & where)>;
  //! @brief Registry: tag -> factory.
  static std::map<std::string,Factory> & registry();

  //! @brief Throw if name is empty or contains '/'. @param name name @param source name of the calling function
  static void checkName(const String & name, const char * source);
  //! @brief Add an entry (ownership of item taken). @param name entry name @param item entry
  void addItem(const String & name, FolderItem * item);
  //! @brief Entry with that name directly in this folder. @param name name @return entry or nullptr
  const Entry * findEntry(const std::string & name) const;
  //! @brief Entry at a path. @param path path @return entry or nullptr
  const Entry * findPath(const String & path) const;
  //! @brief Folder holding the last component of a path (relative to the current folder, or from the top with a leading '/'),
  //! and that component. @param path path @param leaf output: last component @return folder or nullptr
  Folder * parentOf(const String & path, std::string & leaf) const;
  //! @brief Folder reached from start by the components of path ("..", "." and names). @param start start folder @param path path
  //! @return folder or nullptr
  static Folder * walk(const Folder * start, const std::string & path);
  //! @brief Set the parent of the sub-folders of this folder to this folder.
  void adoptSubFolders();
  //! @brief Combine two folders entry by entry. @param left first @param right second @param subtract left - right if true
  //! @param path path of left in the tree, for messages @return combined folder
  static Folder combineFolders(const Folder & left, const Folder & right, bool subtract, const std::string & path);
  //! @brief Compare structures. @param other folder @param path path, for messages @param reason output @return true if same
  bool compareStructure(const Folder & other, const std::string & path, std::string * reason) const;

  template <typename T> friend class FolderItemOf;
  //! @brief Collect object paths under prefix. @param prefix path of this folder @param out output @param unsavableOnly only unsavable objects
  void collectPaths(const std::string & prefix, std::vector<String> & out, bool unsavableOnly) const;

  String _name;                  //!< name of the folder
  std::vector<Entry> _entries;   //!<! entries, in insertion order (not streamed by ROOT: use the binary files)
  Folder * _parent;              //!<! folder containing this one (nullptr for a top folder)
  std::string _currentPath;      //!<! path of the current folder from the top ("" when it is this folder)


  ClassDef(Folder,1)
  };

  //!
  //! @brief Folder entry holding an object of type T (internal to Folder).
  //!
  template <typename T>
  class FolderItemOf : public FolderItem
  {
  public:
  explicit FolderItemOf(T * object) : _object(object) {}
  virtual ~FolderItemOf() { delete _object; }
  FolderItem * clone() const override
  {
    if constexpr (std::is_copy_constructible<T>::value && !std::is_abstract<T>::value)
      return new FolderItemOf<T>(new T(*_object));
    else
      throw Exception(String("Cannot copy an object of type ")+demangledTypeName(typeid(T)).c_str(),"Folder");
  }
  const std::type_info & type() const override { return typeid(T); }
  std::string typeName() const override { return demangledTypeName(typeid(T)); }
  void * pointer() const override { return const_cast<void *>(static_cast<const void *>(_object)); }
  Object * asObject() const override
  { if constexpr (std::is_base_of<Object,T>::value) return _object; else return nullptr; }
  Function * asFunction() const override
  { if constexpr (std::is_base_of<Function,T>::value) return _object; else return nullptr; }
  DataPoints * asDataPoints() const override
  { if constexpr (std::is_base_of<DataPoints,T>::value) return _object; else return nullptr; }
  Folder * asFolder() const override
  { if constexpr (std::is_same<Folder,T>::value) return _object; else return nullptr; }
  bool savable() const override { return IsBinarySavable<T>::value; }
  void writeRecord(std::ostream & out, bool skipUnsavable) const override
  {
    if constexpr (std::is_same<Folder,T>::value)
      _object->writeBinary(out,skipUnsavable);
    else if constexpr (IsBinarySavable<T>::value)
      writeBinaryObject(out,*_object);
    else
      throw Exception(String("Objects of type ")+demangledTypeName(typeid(T)).c_str()+" cannot be saved in binary files","Folder");
  }
  void * release() override { T * object = _object; _object = nullptr; return object; }
  bool combinable() const override { return std::is_same<Folder,T>::value || HasPlusMinus<T>::value; }
  FolderItem * combine(const FolderItem & other, bool subtract, const std::string & path) const override;
  T * object() const { return _object; }

  private:
  T * _object;
  };

  // ---------------------------------------------------------------- template implementations

  template <typename T>
  FolderItem * FolderItemOf<T>::combine(const FolderItem & other, bool subtract, const std::string & path) const
  {
    if (other.type()!=typeid(T))
      throw Exception(String("Entry ")+path.c_str()+": types differ ("+typeName().c_str()+" and "+other.typeName().c_str()+")","Folder");
    const T & rhs = *static_cast<const T *>(other.pointer());
    if constexpr (std::is_same<Folder,T>::value)
      return new FolderItemOf<T>(new Folder(Folder::combineFolders(*_object,rhs,subtract,path)));
    else if constexpr (HasPlusMinus<T>::value)
      {
      try
        {
        return new FolderItemOf<T>(new T(subtract ? T(*_object - rhs) : T(*_object + rhs)));
        }
      catch (std::exception & e)
        {
        throw Exception(String("Entry ")+path.c_str()+": "+e.what(),"Folder");
        }
      }
    else
      throw Exception(String("Entry ")+path.c_str()+": objects of type "+typeName().c_str()+" cannot be added or subtracted","Folder");
  }

  namespace FolderDetail
  {
  //! Name an object gives itself, if any (binaryName() or name()).
  template <typename T, typename = void> struct HasBinaryName : std::false_type {};
  template <typename T> struct HasBinaryName<T, std::void_t<decltype(std::declval<const T &>().binaryName())>> : std::true_type {};
  template <typename T, typename = void> struct HasName : std::false_type {};
  template <typename T> struct HasName<T, std::void_t<decltype(String(std::declval<const T &>().name()))>> : std::true_type {};
  template <typename T>
  String ownName(const T & object)
  {
    if constexpr (HasBinaryName<T>::value) return object.binaryName();
    else if constexpr (HasName<T>::value)  return String(object.name());
    else return String("");
  }

  //! Cast the object of an entry to T*, exactly or through the base classes Object, Function and DataPoints.
  template <typename T>
  T * cast(const FolderItem * item)
  {
    if (!item) return nullptr;
    if (item->type()==typeid(T)) return static_cast<T *>(item->pointer());
    if constexpr (std::is_polymorphic<T>::value)
      {
      if constexpr (std::is_base_of<Function,T>::value || std::is_same<Function,T>::value)
        { if (Function * f = item->asFunction()) return dynamic_cast<T *>(f); }
      if constexpr (std::is_base_of<DataPoints,T>::value || std::is_same<DataPoints,T>::value)
        { if (DataPoints * d = item->asDataPoints()) return dynamic_cast<T *>(d); }
      if constexpr (std::is_base_of<Object,T>::value || std::is_same<Object,T>::value)
        { if (Object * o = item->asObject()) return dynamic_cast<T *>(o); }
      }
    return nullptr;
  }
  }

  //! @relates Folder
  //! @brief Sum of two folders with the same structure (see Folder::add()). @param left first @param right second @return sum
  Folder operator+(const Folder & left, const Folder & right);
  //! @relates Folder
  //! @brief Difference of two folders with the same structure (see Folder::subtract()). @param left first @param right second @return difference
  Folder operator-(const Folder & left, const Folder & right);

  template <typename T>
  T * Folder::add(T * object, const String & name)
  {
    if (!object) throw Exception("Cannot add a null object","Folder::add()");
    std::unique_ptr<FolderItemOf<T>> item(new FolderItemOf<T>(object));   // owns the object from here on
    String entryName = (name.Length()>0) ? name : FolderDetail::ownName(*object);
    current()->addItem(entryName,item.release());   // addItem takes ownership, and deletes the entry if the name is rejected
    return object;
  }

  template <typename T>
  T * Folder::get(const String & path) const
  {
    const Entry * entry = findPath(path);
    return entry ? FolderDetail::cast<T>(entry->item.get()) : nullptr;
  }

  template <typename T>
  T * Folder::release(const String & path)
  {
    std::string leaf;
    Folder * parent = parentOf(path,leaf);
    if (!parent) return nullptr;
    for (auto it=parent->_entries.begin(); it!=parent->_entries.end(); ++it)
      {
      if (it->name!=leaf) continue;
      T * object = FolderDetail::cast<T>(it->item.get());
      if (!object) return nullptr;
      if (Folder * f = it->item->asFolder()) f->_parent = nullptr;   // the released folder becomes a top folder
      it->item->release();
      parent->_entries.erase(it);
      return object;
      }
    return nullptr;
  }

  template <typename T>
  void Folder::registerType()
  {
    registry()[T::binaryTag()] = [](const std::string & payload, unsigned int version, const String & where) -> FolderItem *
      {
      if (version==0 || version>T::binaryVersion())
        throw FileException(where,"Binary record of an unsupported format version","Folder");
      BinaryReader r(payload.data(),payload.size());
      String objectName = r.readString();
      std::unique_ptr<T> object(new T());
      object->decodeBinary(r,objectName,version);
      if (r.remaining()!=0) throw Exception("Unexpected data at the end of a record","Folder");
      return new FolderItemOf<T>(object.release());
      };
  }

  } // namespace MATH
} // namespace CAP

#endif

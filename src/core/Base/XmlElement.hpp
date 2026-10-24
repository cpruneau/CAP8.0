/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__XmlElement
#define CAP__XmlElement
#include <memory>
#include <utility>
#include <vector>
#include "Aliases.hpp"
#include "Exceptions.hpp"

namespace CAP
{

//!
//! @brief Exception thrown for malformed XML or a failed XML query; reports the line and column when known.
//!
class XmlException : public Exception
{
public:
  //! @brief Constructor. @param message description of the problem @param source function where it happened
  //! @param line line of the problem in the text (0 if unknown) @param column column of the problem (0 if unknown)
  XmlException(const String & message, const String & source, int line=0, int column=0);
  //! @brief Destructor.
  virtual ~XmlException() {}
  //! @brief Line of the problem (from 1; 0 if unknown). @return line
  int line() const { return _line; }
  //! @brief Column of the problem (from 1; 0 if unknown). @return column
  int column() const { return _column; }
  //! @brief Print the exception.
  virtual void print() const;
protected:
  int _line;   //!< line of the problem
  int _column; //!< column of the problem
};

//!
//! @brief Element of an XML document: a name, attributes, text and child elements.
//!
//! Elements are plain C++ objects (no ROOT): an element owns its children, and copying an element copies its whole
//! sub-tree. Attributes keep the order in which they were read or set.
//!
//! <b>Text.</b> text() is the character data of the element with entities decoded and surrounding white space removed;
//! rawText() keeps the white space. When text and child elements are mixed, the text pieces are concatenated.
//!
//! <b>Searching.</b> child(name) returns the first child with that name, find("A/B/C") follows a path of child names,
//! and findFirst(name, attribute, value) searches the whole sub-tree (this element included) in document order.
//! Child pointers and references stay valid until the child is removed or its parent destroyed (adding children does
//! not move existing ones).
//!
//! Example:
//! @code
//!   CAP::XmlElement model("MODEL");
//!   model.setAttribute("name","Hydro");
//!   model.addChild("PARAMETER").setAttribute("name","Temperature").setText(165.6);
//!   const CAP::XmlElement * t = model.findFirst("PARAMETER","name","Temperature");
//!   double temperature = t->textDouble();
//! @endcode
//!
class XmlElement
{
public:

  //! @brief Name/value pair of an attribute.
  using Attribute = std::pair<String,String>;

  //! @name Construction
  //! @{
  //! @brief Constructor. @param name name of the element (may be set later with setName())
  explicit XmlElement(const String & name="");
  //! @brief Copy constructor: deep copy of the sub-tree. @param source element to copy
  XmlElement(const XmlElement & source);
  //! @brief Move constructor.
  XmlElement(XmlElement &&) = default;
  //! @brief Copy assignment: deep copy of the sub-tree. @param rhs element to copy @return this element
  XmlElement & operator=(const XmlElement & rhs);
  //! @brief Move assignment.
  XmlElement & operator=(XmlElement &&) = default;
  //! @brief Destructor: the children are deleted.
  ~XmlElement() = default;
  //! @}

  //! @name Name
  //! @{
  //! @brief Name of the element. @return name
  const String & name() const { return _name; }
  //! @brief Set the name. @param name new name @return this element
  XmlElement & setName(const String & name) { _name = name; return *this; }
  //! @brief Whether the element has the given name. @param name name @return true if so
  bool isNamed(const String & name) const { return _name==name; }
  //! @}

  //! @name Attributes
  //! @{
  //! @brief All attributes, in order. @return attributes
  const std::vector<Attribute> & attributes() const { return _attributes; }
  //! @brief Whether the element has an attribute. @param name attribute name @return true if so
  bool hasAttribute(const String & name) const;
  //! @brief Value of an attribute. @param name attribute name @return value @throws XmlException if there is no such attribute
  const String & attribute(const String & name) const;
  //! @brief Value of an attribute, or a default. @param name attribute name @param defaultValue value returned if absent
  //! @return value
  String attribute(const String & name, const String & defaultValue) const;
  //! @brief Value of an attribute as a double. @param name attribute name @return value
  //! @throws XmlException if the attribute is absent or not a number
  double attributeDouble(const String & name) const;
  //! @brief Value of an attribute as a double, or a default if absent. @param name attribute name
  //! @param defaultValue value returned if absent @return value @throws XmlException if present but not a number
  double attributeDouble(const String & name, double defaultValue) const;
  //! @brief Value of an attribute as an integer. @param name attribute name @return value
  //! @throws XmlException if the attribute is absent or not an integer
  long attributeInt(const String & name) const;
  //! @brief Value of an attribute as an integer, or a default if absent. @param name attribute name
  //! @param defaultValue value returned if absent @return value @throws XmlException if present but not an integer
  long attributeInt(const String & name, long defaultValue) const;
  //! @brief Value of an attribute as a boolean (true/false, yes/no, 1/0, any case). @param name attribute name
  //! @return value @throws XmlException if the attribute is absent or not a boolean
  bool attributeBool(const String & name) const;
  //! @brief Value of an attribute as a boolean, or a default if absent. @param name attribute name
  //! @param defaultValue value returned if absent @return value @throws XmlException if present but not a boolean
  bool attributeBool(const String & name, bool defaultValue) const;
  //! @brief Set an attribute (replacing its value if it exists). @param name attribute name @param value value
  //! @return this element @throws XmlException if the name is not a valid XML name
  XmlElement & setAttribute(const String & name, const String & value);
  //! @brief Set an attribute. @param name attribute name @param value value @return this element
  XmlElement & setAttribute(const String & name, const char * value) { return setAttribute(name,String(value)); }
  //! @brief Set an attribute to a number (shortest text that reads back exactly). @param name attribute name
  //! @param value value @return this element
  XmlElement & setAttribute(const String & name, double value);
  //! @brief Set an attribute to an integer. @param name attribute name @param value value @return this element
  XmlElement & setAttribute(const String & name, long value);
  //! @brief Set an attribute to an integer. @param name attribute name @param value value @return this element
  XmlElement & setAttribute(const String & name, int value) { return setAttribute(name,long(value)); }
  //! @brief Set an attribute to a boolean ("true" or "false"). @param name attribute name @param value value @return this element
  XmlElement & setAttribute(const String & name, bool value);
  //! @brief Remove an attribute. @param name attribute name @return true if it existed
  bool removeAttribute(const String & name);
  //! @}

  //! @name Text
  //! @{
  //! @brief Text of the element, surrounding white space removed. @return text
  String text() const;
  //! @brief Text of the element as read or set, white space kept. @return text
  const String & rawText() const { return _text; }
  //! @brief Text as a double. @return value @throws XmlException if the text is not a number
  double textDouble() const;
  //! @brief Text as an integer. @return value @throws XmlException if the text is not an integer
  long textInt() const;
  //! @brief Text as a boolean (true/false, yes/no, 1/0, any case). @return value @throws XmlException otherwise
  bool textBool() const;
  //! @brief Text as a list of numbers separated by white space (e.g., a data array). @return values
  //! @throws XmlException if a word is not a number
  std::vector<double> textDoubles() const;
  //! @brief Set the text. @param text text @return this element
  XmlElement & setText(const String & text) { _text = text; return *this; }
  //! @brief Set the text. @param text text @return this element
  XmlElement & setText(const char * text) { _text = text; return *this; }
  //! @brief Set the text to a number (shortest text that reads back exactly). @param value value @return this element
  XmlElement & setText(double value);
  //! @brief Set the text to an integer. @param value value @return this element
  XmlElement & setText(long value);
  //! @brief Set the text to an integer. @param value value @return this element
  XmlElement & setText(int value) { return setText(long(value)); }
  //! @brief Set the text to numbers separated by spaces, perLine numbers per line. @param values values
  //! @param perLine numbers per line (all on one line if 0) @return this element
  XmlElement & setText(const std::vector<double> & values, unsigned int perLine=10);
  //! @brief Append text. @param text text @return this element
  XmlElement & appendText(const String & text) { _text += text; return *this; }
  //! @}

  //! @name Children
  //! @{
  //! @brief Number of child elements. @return number of children
  unsigned int nChildren() const { return (unsigned int)_children.size(); }
  //! @brief Whether the element has children. @return true if so
  bool hasChildren() const { return !_children.empty(); }
  //! @brief Child at an index. @param index index @return child @throws IndexOutOfBoundException if index>=nChildren()
  XmlElement & child(unsigned int index);
  //! @brief Child at an index. @param index index @return child @throws IndexOutOfBoundException if index>=nChildren()
  const XmlElement & child(unsigned int index) const;
  //! @brief First child with a name. @param name name @return child, or nullptr if none
  XmlElement * child(const String & name);
  //! @brief First child with a name. @param name name @return child, or nullptr if none
  const XmlElement * child(const String & name) const;
  //! @brief First child with a name, which must exist. @param name name @return child @throws XmlException if none
  const XmlElement & requiredChild(const String & name) const;
  //! @brief All children, in order. @return pointers to the children (owned by this element)
  std::vector<XmlElement *> children();
  //! @brief All children, in order. @return pointers to the children (owned by this element)
  std::vector<const XmlElement *> children() const;
  //! @brief Children with a name, in order. @param name name @return pointers to the children
  std::vector<const XmlElement *> children(const String & name) const;
  //! @brief Add a new empty child. @param name name of the child @return the child
  XmlElement & addChild(const String & name);
  //! @brief Add a copy of an element as last child. @param element element to copy @return the new child
  XmlElement & addChild(const XmlElement & element);
  //! @brief Add a child with text content. @param name name of the child @param text text @return the new child
  XmlElement & addChild(const String & name, const String & text) { return addChild(name).setText(text); }
  //! @brief Remove (and delete) the child at an index. @param index index
  //! @throws IndexOutOfBoundException if index>=nChildren()
  void removeChild(unsigned int index);
  //! @brief Remove all children, attributes and text (the name is kept).
  void clear();
  //! @}

  //! @name Searching
  //! @{
  //!
  //! @brief Element at the end of a path of child names, e.g. "MODEL/PARAMETERS/TEMPERATURE"; at each step the first
  //! child with the name is taken. An empty path designates this element.
  //! @param path names separated by '/' @return element, or nullptr if a step fails
  //!
  const XmlElement * find(const String & path) const;
  //! @brief Element at the end of a path of child names (see the const version). @param path path @return element or nullptr
  XmlElement * find(const String & path);
  //!
  //! @brief First element of the sub-tree (this element included, document order) with a name and, if attributeName is
  //! not empty, an attribute of that name equal to attributeValue.
  //! @param name element name @param attributeName attribute name (ignored if empty) @param attributeValue attribute value
  //! @return element, or nullptr if none
  //!
  const XmlElement * findFirst(const String & name, const String & attributeName="", const String & attributeValue="") const;
  //! @brief First matching element of the sub-tree (see the const version). @param name element name
  //! @param attributeName attribute name @param attributeValue attribute value @return element or nullptr
  XmlElement * findFirst(const String & name, const String & attributeName="", const String & attributeValue="");
  //! @brief All matching elements of the sub-tree, in document order (see findFirst()). @param name element name
  //! @param attributeName attribute name @param attributeValue attribute value @return elements
  std::vector<const XmlElement *> findAll(const String & name, const String & attributeName="", const String & attributeValue="") const;
  //! @}

  //! @name Text form
  //! @{
  //! @brief XML text of the element and its sub-tree. @param indent spaces per level (0: no line breaks) @return text
  String toString(unsigned int indent=2) const;
  //! @}

  //! @brief Whether a string is a valid XML name (letter, '_' or ':' first; then letters, digits, '_', ':', '.', '-';
  //! non-ASCII characters accepted). @param name name @return true if valid
  static bool isValidName(const String & name);
  //! @brief Shortest decimal text of a double that reads back to the same value. @param value value @return text
  static String formatNumber(double value);

protected:

  //! @brief Whether this element matches a search. @param name element name @param attributeName attribute name (ignored if empty)
  //! @param attributeValue attribute value @return true if it matches
  bool matches(const String & name, const String & attributeName, const String & attributeValue) const;

  String _name;                                      //!< name
  std::vector<Attribute> _attributes;                //!< attributes, in order
  String _text;                                      //!< character data (entities decoded)
  std::vector<std::unique_ptr<XmlElement>> _children;//!< child elements (owned)
};

} // namespace CAP

#endif

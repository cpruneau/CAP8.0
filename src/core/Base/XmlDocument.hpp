/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__XmlDocument
#define CAP__XmlDocument
#include "XmlElement.hpp"

namespace CAP
{

//!
//! @brief XML document: the declaration (version, encoding) and one root element.
//!
//! Documents are read from text or files with parse() and read() (see XmlReader) and written with toString() and
//! write() (see XmlWriter). They depend on the standard library and CAP's Base only (no ROOT).
//!
//! Example:
//! @code
//!   CAP::XmlDocument doc = CAP::XmlDocument::read("/data/hydro","model.xml");
//!   const CAP::XmlElement * t = doc.root().findFirst("PARAMETER","name","Temperature");
//!   double temperature = t->textDouble();
//!
//!   CAP::XmlDocument out("RESULTS");
//!   out.root().addChild("VALUE").setAttribute("name","yield").setText(12.5);
//!   out.write("/tmp","results.xml");
//! @endcode
//!
class XmlDocument
{
public:

  //! @brief Constructor. @param rootName name of the root element (an empty root if empty)
  explicit XmlDocument(const String & rootName="");
  //! @brief Copy constructor (deep copy).
  XmlDocument(const XmlDocument &) = default;
  //! @brief Move constructor.
  XmlDocument(XmlDocument &&) = default;
  //! @brief Copy assignment (deep copy).
  XmlDocument & operator=(const XmlDocument &) = default;
  //! @brief Move assignment.
  XmlDocument & operator=(XmlDocument &&) = default;
  //! @brief Destructor.
  ~XmlDocument() = default;

  //! @name Declaration
  //! @{
  //! @brief XML version of the declaration. @return version
  const String & version() const { return _version; }
  //! @brief Character encoding of the declaration. @return encoding
  const String & encoding() const { return _encoding; }
  //! @brief Set the declaration. @param version XML version @param encoding character encoding
  void setDeclaration(const String & version, const String & encoding) { _version = version; _encoding = encoding; }
  //! @}

  //! @name Root element
  //! @{
  //! @brief Root element. @return root
  XmlElement & root() { return _root; }
  //! @brief Root element. @return root
  const XmlElement & root() const { return _root; }
  //! @brief Replace the root element. @param root new root
  void setRoot(const XmlElement & root) { _root = root; }
  //! @brief Whether the document is empty (root without a name). @return true if so
  bool isEmpty() const { return _root.name().isNull(); }
  //! @brief First element of the document with a name and an optional attribute value (see XmlElement::findFirst()).
  //! @param name element name @param attributeName attribute name (ignored if empty) @param attributeValue attribute value
  //! @return element, or nullptr if none
  const XmlElement * findFirst(const String & name, const String & attributeName="", const String & attributeValue="") const
  { return _root.findFirst(name,attributeName,attributeValue); }
  //! @}

  //! @name Reading
  //! @{
  //! @brief Document parsed from XML text. @param text XML text @return document
  //! @throws XmlException if the text is not well-formed XML (with the line and column of the problem)
  static XmlDocument parse(const String & text);
  //! @brief Document read from a file. @param fileName file name @return document
  //! @throws FileException if the file cannot be read; XmlException if it is not well-formed XML
  static XmlDocument read(const String & fileName);
  //! @brief Document read from a file. @param path directory @param name file name (".xml" appended if missing)
  //! @return document @throws FileException, XmlException (see read(fileName))
  static XmlDocument read(const String & path, const String & name);
  //! @}

  //! @name Writing
  //! @{
  //! @brief XML text of the document, starting with the declaration. @param indent spaces per level (0: one line)
  //! @return text
  String toString(unsigned int indent=2) const;
  //! @brief Write the document to a file (replacing it). @param fileName file name @param indent spaces per level
  //! @throws FileException if the file cannot be written
  void write(const String & fileName, unsigned int indent=2) const;
  //! @brief Write the document to a file. @param path directory @param name file name (".xml" appended if missing)
  //! @param indent spaces per level @throws FileException if the file cannot be written
  void write(const String & path, const String & name, unsigned int indent=2) const;
  //! @}

  //! @brief Full name of a file: path/name, ".xml" appended if missing. @param path directory (may be empty)
  //! @param name file name @return file name
  static String fileName(const String & path, const String & name);

protected:

  String     _version;  //!< XML version of the declaration
  String     _encoding; //!< character encoding of the declaration
  XmlElement _root;     //!< root element
};

} // namespace CAP

#endif

/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__XmlFile
#define CAP__XmlFile
#include "TxtFile.hpp"
#include "XmlDocument.hpp"

namespace CAP
{

//!
//! @brief Text file holding XML code.
//!
//! The XML is held as lines of text (see TxtFile: read(), write(), append(), lines(), text(), setText()). XmlFile adds
//! helpers specific to XML: the declaration line (<?xml version="1.0" encoding="UTF-8"?>), escaping of special
//! characters, writing simple elements, and a check that the text is well formed (tags properly nested and closed,
//! one root element). document() parses the text into an XmlDocument and setDocument() replaces the text by a document.
//! The parsed document is kept until the lines change, so document() parses only once; load() reads the file and
//! parses it.
//!
//! Example:
//! @code
//!   CAP::XmlFile xml("run.xml", CAP::File::kWriteOnly);
//!   xml.setDeclaration();
//!   xml.addLine("<run>");
//!   xml.addElement("energy", "13.6 TeV", 1);
//!   xml.addElement("note", "p+p & Pb+Pb", 1);    // written as p+p &amp; Pb+Pb
//!   xml.addLine("</run>");
//!   if (xml.isWellFormed()) xml.write();
//! @endcode
//!
class XmlFile : public TxtFile
{
public:

  //! @brief Constructor. @param path path of the file @param access access allowed (read/write by default)
  XmlFile(const String & path="", Access access=kReadWrite);
  //! @brief Destructor.
  virtual ~XmlFile() {}

  //! @name Declaration
  //! @{

  //! @brief Whether the text starts (after blank lines) with an XML declaration. @return true if it does
  bool hasDeclaration() const;
  //!
  //! @brief Set the XML declaration: replaces an existing one, or inserts it as the first line.
  //! @param version XML version @param encoding character encoding
  //!
  void setDeclaration(const String & version="1.0", const String & encoding="UTF-8");

  //! @}

  //! @name Content
  //! @{

  //!
  //! @brief Append a line holding one element with text content: <name>content</name>, content escaped.
  //! @param name element name @param content text content (escaped with escape()) @param depth indentation (2 spaces per level)
  //!
  void addElement(const String & name, const String & content, unsigned int depth=0);

  //!
  //! @brief Check that the text is well-formed XML (parsed with XmlReader: tags properly nested and closed, one root
  //! element, valid names, attributes and entities).
  //! @param reason [out] if not null, set to the first problem found (empty if the text is well formed)
  //! @return true if the text is well formed
  //!
  bool isWellFormed(String * reason=nullptr) const;

  //! @brief The text parsed into a document (parsed once, kept until the lines change). @return document
  //! @throws XmlException if the text is not well-formed XML
  const XmlDocument & document() const;
  //! @brief Replace the text by a document. @param document document @param indent spaces per nesting level
  void setDocument(const XmlDocument & document, unsigned int indent=2);

  //! @}

  //! @name Loading
  //! @{

  //! @brief Read the file and parse it (text files have no lazy mode). @param loading ignored
  //! @throws FileException if the file cannot be read; XmlException if it is not well-formed XML (the lines are then
  //! loaded, and can be corrected)
  void load(Loading loading = Loading::kEager) override;
  //! @brief Remove the lines and the document held (the file is not changed).
  void unload() override;

  //! @}

  //! @name Escaping
  //! @{

  //! @brief Replace & < > " ' by &amp; &lt; &gt; &quot; &apos;. @param text text @return escaped text
  static String escape(const String & text);
  //! @brief Replace &amp; &lt; &gt; &quot; &apos; by & < > " '. @param text text @return unescaped text
  static String unescape(const String & text);

  //! @}

protected:

  //! @brief The lines changed: the document must be parsed again.
  void changed() override { _documentValid = false; }

  mutable XmlDocument _document;       //!< document parsed from the lines
  mutable bool        _documentValid;  //!< _document matches the lines
};

} // namespace CAP

#endif

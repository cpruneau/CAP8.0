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
#ifndef CAP__XmlWriter
#define CAP__XmlWriter
#include <ostream>
#include <vector>
#include "Aliases.hpp"

namespace CAP
{

class XmlElement;

//!
//! @brief Streaming XML writer: writes elements, attributes and text to a stream as they are given, with escaping and
//! indentation.
//!
//! Elements are opened with startElement() and closed with endElement() (or all at once with finish()). Attributes are
//! added with attribute() right after startElement(). An element without content is written as <name/>; an element with
//! text only is written on one line, <name>text</name>. With an indentation of 0, nothing but the content is written
//! (no line breaks). Names are checked and text and attribute values escaped (& < > " ').
//!
//! Large documents can be written without building them in memory first; write() writes a whole XmlElement.
//!
//! Example:
//! @code
//!   std::ofstream out("db.xml");
//!   CAP::XmlWriter w(out);
//!   w.declaration();
//!   w.startElement("PARTICLES");
//!   w.startElement("PARTICLE").attribute("name","pi+").attribute("mass",0.13957);
//!   w.endElement();
//!   w.finish();      // closes PARTICLES
//! @endcode
//!
class XmlWriter
{
public:

  //! @brief Constructor. @param out stream written @param indent spaces per nesting level (0: no line breaks)
  XmlWriter(std::ostream & out, unsigned int indent=2);
  //! @brief Destructor: does not close open elements (call finish()).
  ~XmlWriter() {}
  XmlWriter(const XmlWriter &) = delete;
  XmlWriter & operator=(const XmlWriter &) = delete;

  //! @brief Write the XML declaration (must come first). @param version XML version @param encoding character encoding
  //! @return this writer @throws XmlException if something was already written
  XmlWriter & declaration(const String & version="1.0", const String & encoding="UTF-8");
  //! @brief Open an element. @param name element name @return this writer @throws XmlException if the name is invalid
  //! or a root element was already closed
  XmlWriter & startElement(const String & name);
  //! @brief Add an attribute to the element just opened. @param name attribute name @param value value (escaped)
  //! @return this writer @throws XmlException if no start tag is open or the name is invalid
  XmlWriter & attribute(const String & name, const String & value);
  //! @brief Add an attribute. @param name attribute name @param value value @return this writer
  XmlWriter & attribute(const String & name, const char * value) { return attribute(name,String(value)); }
  //! @brief Add a numeric attribute (shortest exact text). @param name attribute name @param value value @return this writer
  XmlWriter & attribute(const String & name, double value);
  //! @brief Add an integer attribute. @param name attribute name @param value value @return this writer
  XmlWriter & attribute(const String & name, long value);
  //! @brief Add an integer attribute. @param name attribute name @param value value @return this writer
  XmlWriter & attribute(const String & name, int value) { return attribute(name,long(value)); }
  //! @brief Add a boolean attribute ("true" or "false"). @param name attribute name @param value value @return this writer
  XmlWriter & attribute(const String & name, bool value) { return attribute(name,String(value ? "true" : "false")); }
  //! @brief Write text in the current element (escaped). @param text text @return this writer
  //! @throws XmlException if no element is open
  XmlWriter & text(const String & text);
  //! @brief Write a CDATA section in the current element (split if it contains "]]>"). @param text text @return this writer
  XmlWriter & cdata(const String & text);
  //! @brief Write a comment. @param text comment (must not contain "--") @return this writer
  XmlWriter & comment(const String & text);
  //! @brief Write an element holding only text: <name>text</name>. @param name name @param text text @return this writer
  XmlWriter & element(const String & name, const String & text);
  //! @brief Close the current element. @return this writer @throws XmlException if no element is open
  XmlWriter & endElement();
  //! @brief Write an element and its sub-tree. @param element element @return this writer
  XmlWriter & write(const XmlElement & element);
  //! @brief Close all open elements and end the last line. @return this writer
  XmlWriter & finish();
  //! @brief Number of open elements. @return depth
  unsigned int depth() const { return (unsigned int)_open.size(); }

  //! @brief Escape & < > " ' as entities. @param text text @return escaped text
  static String escape(const String & text);

protected:

  //! @brief Close a pending start tag with '>'.
  void closeStartTag();
  //! @brief New line and indentation for the current depth (nothing if the indentation is 0).
  void newLine();

  //! @brief State of an open element.
  struct Open
  {
    String name;        //!< name of the element
    bool   hasChildren; //!< whether child elements were written
    bool   hasText;     //!< whether text was written
  };

  std::ostream &    _out;          //!< stream written
  unsigned int      _indent;       //!< spaces per level
  std::vector<Open> _open;         //!< open elements, outermost first
  bool              _startTagOpen; //!< whether the last start tag still awaits '>' (attributes may be added)
  bool              _wroteSomething; //!< whether anything was written
  bool              _rootClosed;   //!< whether the root element was closed
};

} // namespace CAP

#endif

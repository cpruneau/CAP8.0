/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__XmlReader
#define CAP__XmlReader
#include "XmlDocument.hpp"

namespace CAP
{

//!
//! @brief Reader of XML text into an XmlDocument (a non-validating, single-pass parser).
//!
//! Supported: the XML declaration, elements and attributes (single or double quotes), empty elements (<a/>), text,
//! CDATA sections, comments, processing instructions (skipped), a DOCTYPE declaration (skipped, internal subset
//! included), the predefined entities (&amp;amp; &amp;lt; &amp;gt; &amp;quot; &amp;apos;) and character references
//! (&amp;#65; &amp;#x41;, written as UTF-8). A leading UTF-8 byte order mark is ignored.
//!
//! Not supported: DTD validation, entities defined in a DTD (an unknown entity is an error, so external content is
//! never loaded), and namespace processing (prefixes are kept in the names).
//!
//! Errors (unclosed or mismatched tags, duplicate attributes, invalid names, text outside the root element, nesting
//! deeper than maximumDepth(), ...) throw XmlException with the line and column of the problem. The parser does not use
//! recursion, so deeply nested input cannot overflow the stack.
//!
class XmlReader
{
public:

  //! @brief Constructor. @param text XML text to read
  explicit XmlReader(const String & text);
  //! @brief Parse the text. @return document @throws XmlException if the text is not well-formed XML
  XmlDocument read();
  //! @brief Maximum nesting depth accepted (10000 by default). @return depth
  unsigned int maximumDepth() const { return _maximumDepth; }
  //! @brief Set the maximum nesting depth accepted. @param depth depth
  void setMaximumDepth(unsigned int depth) { _maximumDepth = depth; }

protected:

  //! @brief Throw XmlException at the current position. @param message description of the problem
  [[noreturn]] void error(const String & message) const;
  //! @brief Whether the text at the current position starts with s. @param s string @return true if so
  bool startsWith(const char * s) const;
  //! @brief Skip white space.
  void skipSpace();
  //! @brief Advance by n characters, keeping the line and column. @param n number of characters
  void advance(std::size_t n=1);
  //! @brief Read a name at the current position. @return name @throws XmlException if there is none
  String readName();
  //! @brief Skip text up to and including a terminator. @param terminator terminator @param what construct (for errors)
  //! @return the skipped text, terminator excluded
  std::string readUntil(const char * terminator, const char * what);
  //! @brief Decode an entity or character reference at the current position ('&') and append it. @param out text
  void readReference(std::string & out);
  //! @brief Read an attribute value (quoted, references decoded). @return value
  String readAttributeValue();
  //! @brief Read the XML declaration (current position after "<?xml"). @param document document
  void readDeclaration(XmlDocument & document);
  //! @brief Skip a DOCTYPE declaration (current position at "<!DOCTYPE").
  void skipDoctype();

  std::string  _text;         //!< text read
  std::size_t  _pos;          //!< current position
  int          _line;         //!< current line (from 1)
  int          _column;       //!< current column (from 1)
  unsigned int _maximumDepth; //!< maximum nesting depth
};

} // namespace CAP

#endif

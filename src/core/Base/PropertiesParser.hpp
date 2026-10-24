/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PropertiesParser
#define CAP__PropertiesParser
#include "Parser.hpp"
#include "Properties.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Reader and writer of property files ("name = value" lines).
//!
//! File format read by loadFrom():
//! - one property per line, "name = value"; white space is removed from the name, the value is trimmed but keeps its
//!   internal spaces;
//! - blank lines, comments (lines starting with '#' or ';') and section headers (lines starting with '[') are ignored;
//! - lines without '=' are ignored, with a warning.
//!
//! saveTo() writes the same format, so files written can be read back.
//!
//! Example (analysis.ini):
//! @code
//!   # PtPt analysis
//!   PtPt:nBins = 100
//!   PtPt:title = p_T correlations
//! @endcode
//!
class PropertiesParser
  :
  public Parser
{
public:

  //! @brief Default constructor.
  PropertiesParser();
  //! @brief Destructor.
  virtual ~PropertiesParser() {}

  //!
  //! @brief Read a property file and add its properties (replacing those of the same name).
  //! @param _properties properties to fill @param _inputPath directory @param _inputFileName file name
  //! @param _extension extension of the file
  //! @throws FileException if the file cannot be opened
  //!
  virtual void  loadFrom(Properties & _properties,
                             const String & _inputPath,
                             const String & _inputFileName,
                             const String & _extension = ".ini") ;
  //!
  //! @brief Write properties to a file as "name = value" lines (the format read by loadFrom()).
  //! @param _properties properties to write @param _path directory @param _fileName file name
  //! @param _extension extension of the file
  //! @throws FileException if the file cannot be opened
  //!
  virtual void  saveTo(Properties & _properties,
                             const String & _path,
                             const String & _fileName,
                             const String & _extension) ;

  ClassDef(PropertiesParser,0)

};

} // namespace CAP

#endif /* PropertiesParser_hpp */

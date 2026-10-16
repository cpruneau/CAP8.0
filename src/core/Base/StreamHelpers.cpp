/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "StreamHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Exceptions.hpp"
#include "TSystem.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

namespace CAP
{

namespace
{
  //!
  //! The open*File() functions return a reference to a stream that must outlive the call,
  //! and callers only ever close() it -- they never delete it. The streams used to be
  //! allocated with a bare `new` and were therefore leaked on every call. They are now owned
  //! here: each call first releases every previously returned stream that the caller has
  //! already closed, and all remaining streams are released at program exit.
  //! Do not use a returned stream after calling close() on it.
  //!
  template <class S>
  S & registerStream(std::unique_ptr<S> stream)
  {
  static std::mutex registryMutex;
  static std::vector<std::unique_ptr<S>> registry;
  std::lock_guard<std::mutex> lock(registryMutex);
  registry.erase(std::remove_if(registry.begin(), registry.end(),
                                [](const std::unique_ptr<S> & s) { return !s->is_open(); }),
                 registry.end());
  registry.push_back(std::move(stream));
  return *registry.back();
  }

  //!
  //! Build path/name.extension, inserting a '/' only when the path is non-empty and does not already end with one.
  //!
  String buildFileName(const String & path, const String & name, const String & extension)
  {
  String fileName = path;
  int len = fileName.length();
  if (len>0  &&  fileName.last('/')!=len-1) fileName += "/";
  fileName += name;
  if (!fileName.endsWith(extension)) fileName += extension;
  return fileName;
  }

  template <class S>
  S & openStream(const String & fileName, std::ios_base::openmode mode, const char * fctName, bool verbose)
  {
  if (verbose)
    {
    printCR();
    printValue((String(fctName)+"(...) Opening file named").data(),fileName);
    printValue((String(fctName)+"(...) with option").data(),static_cast<unsigned int>(mode));
    }
  std::unique_ptr<S> file(new S(fileName.data(),mode));
  if (!file->is_open()) throw FileException(fileName,"Unable to open file",fctName);
  if (verbose)
    {
    printCR();
    printValue((String(fctName)+"(...) Successfully opened file named").data(),fileName);
    }
  return registerStream(std::move(file));
  }
}

std::ifstream & openInputAsciiFile(const String & fileName,
                                   bool verbose)
{
  return openStream<std::ifstream>(fileName, std::ios_base::in, "openInputAsciiFile", verbose);
}

std::ofstream & openOutputAsciiFile(const String & fileName,
                                    bool verbose)
{
  return openStream<std::ofstream>(fileName, std::ios_base::out|std::ios_base::app, "openOutputAsciiFile", verbose);
}

std::ifstream & openInputAsciiFile(const String & path,
                                   const String & name,
                                   const String & extension,
                                   bool verbose)
{
  return openStream<std::ifstream>(buildFileName(path,name,extension), std::ios_base::in, "openInputAsciiFile", verbose);
}

std::ofstream & openOutputAsciiFile(const String & path,
                                    const String & name,
                                    const String & extension,
                                    bool verbose)
{
  return openStream<std::ofstream>(buildFileName(path,name,extension), std::ios_base::out|std::ios_base::trunc, "openOutputAsciiFile", verbose);
}

std::ifstream & openInputBinaryFile(const String & path,
                                    const String & name,
                                    const String & extension,
                                    bool verbose)
{
  return openStream<std::ifstream>(buildFileName(path,name,extension), std::ios_base::in|std::ios_base::binary, "openInputBinaryFile", verbose);
}

std::ofstream & openOutputBinaryFile(const String & path,
                                     const String & name,
                                     const String & extension,
                                     bool verbose)
{
  return openStream<std::ofstream>(buildFileName(path,name,extension), std::ios_base::out|std::ios_base::binary, "openOutputBinaryFile", verbose);
}

void copyFile(const String & inputPath,  const String & inputFileName, const String & inExtension,
              const String & outputPath, const String & outputFileName, const String & outExtension,
              bool verbose)
{
  std::ifstream & inputFile  = openInputBinaryFile(inputPath,inputFileName,inExtension,verbose);
  std::ofstream & outputFile = openOutputBinaryFile(outputPath,outputFileName,outExtension,verbose);
  if (verbose)
    {
    printCR();
    printValue("copyFile(...) Copying from file",inputFileName);
    printValue("copyFile(...) to file",outputFileName);
    }
  // do a binary buffer copy
  outputFile << inputFile.rdbuf();
  inputFile.close();
  outputFile.close();
  if (verbose)
    {
    printCR();
    printString("copyFile(...) Copy successfully completed");
    }
}

} // namespace CAP

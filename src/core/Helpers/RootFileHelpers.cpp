#include "Exceptions.hpp"
#include "PrintHelpers.hpp"
#include "RootFileHelpers.hpp"
#include "FileSystem.hpp"
#include "MathConstants.hpp"
#include "MathBasicFunctions.hpp"
#include "TROOT.h"
#include "TSystem.h"
#include "TParameter.h"
#include "TSystemDirectory.h"

namespace CAP
{

//!
//!Open Root file at the given path and with the given file name
//!ioOption : OLD, NEW, RECREATE
//
TFile * openRootFile(const String & path,
                     const String & name,
                     const String & ioOption,
                     bool verbose)
{
  String fileName = FileSystem::join(path,name,".root");
  if (verbose)
    {
    printCR();
    printValue("openRootFile(...) Opening file",fileName);
    printValue("ioOption",ioOption);
    }
  TFile * file = new TFile(fileName,ioOption);
  if (!file) throw  FileException(fileName,"File not found",__FUNCTION__);
  if (!file->IsOpen())  throw  FileException(fileName,"File not found/opened",__FUNCTION__);
  if (verbose) std::cout << "openRootFile(...) File opened successfully." << std::endl;
  return file;
}

TFile * openRootFile(const String & name,
                     const String & ioOption,
                     bool verbose)
{
  String fileName = name;
  if (!fileName.endsWith(".root")) fileName += ".root";
  if (verbose)
    {
    printCR();
    printValue("openRootFile(...) Opening file",fileName);
    printValue("ioOption",ioOption);
    }
  TFile * file = new TFile(fileName,ioOption);
  if (!file) throw  FileException(fileName,"File not found","openRootFile()");
  if (!file->IsOpen())  throw  FileException(fileName,"File not found/opened",__FUNCTION__);
  if (verbose)
    {
    printCR();
    printValue("openRootFile(...) File opened successfully",fileName);
    }
  return file;
}

TFile * openOldRootFile(const String & name,
                        bool verbose)
{
  return openRootFile(name,"OLD",verbose);
}

TFile * openOldRootFile(const String & path,
                        const String & name,
                        bool verbose)
{
  return openRootFile(path,name,"OLD",verbose);
}

TFile * openNewRootFile(const String & name,
                        bool verbose)
{
  return openRootFile(name,"NEW",verbose);
}

TFile * openNewRootFile(const String & path,
                        const String & name,
                        bool verbose)
{
  return openRootFile(path,name,"NEW",verbose);
}

TFile * openRecreateRootFile(const String & name,
                             bool verbose)
{
  return openRootFile(name,"RECREATE",verbose);
}

TFile * openRecreateRootFile(const String & path,
                             const String & name,
                             bool verbose)
{
  return openRootFile(path,name,"RECREATE",verbose);
}

std::vector<TFile*> openRootFiles(std::vector<String> & names,
                                  const String  & ioOption,
                                  bool verbose)
{
  int nFiles = names.size();
  if (nFiles<1)  throw FileException("Empty file list","File not found/opened",__FUNCTION__);
  if (verbose)
    {
    printCR();
    printValue("openRootFiles(...) Opening N files",names.size());
    }
  std::vector<TFile*> files;
  for (auto & name : names)
    {
    files.push_back( openRootFile(name,ioOption,verbose));
    }
  if (verbose)
    {
    printCR();
    printString("openRootFiles(...) Completed successfully");
    }
  return files;
}

std::vector<TFile*> openRootFiles(const String & path,
                                  std::vector<String> & names,
                                  const String  & ioOption,
                                  bool verbose)
{
  int nFiles = names.size();
  if (nFiles<1)  throw  FileException("Empty file list","File not found/opened",__FUNCTION__);
  if (verbose)
    {
    printCR();
    printValue("openRootFiles(...) Opening N files",names.size());
    printValue("openRootFiles(...) Path",path);
    }
  std::vector<TFile*> files;
  for (auto & name : names)
    {
    files.push_back( openRootFile(path,name,ioOption,verbose));
    }
  if (verbose)  std::cout << "openRootInputFiles(...) Completed successfully" << std::endl;
  return files;
}

String removeRootExtension(const String fileName)
{
  String name = fileName;
  int dot = name.last('.');
  int len = name.length();
  if (dot==len-5 ) name.remove(dot,len-dot);
  return name;
}

void exportParameter(TFile & outputFile,
                     const String & parameterName,
                     long value,
                     bool verbose)
{
  if (verbose)
    {
    printCR();
    printValue("exportParameter() output file",outputFile.GetName());
    printValue("exportParameter() parameterName",parameterName);
    printValue("exportParameter() value",value);
    }
  outputFile.cd();
  TParameter<Long64_t>(parameterName,value,'+').Write();
}

long importParameter(TFile & inputFile,
                     const String & parameterName,
                     bool verbose)
{
  TParameter<Long64_t> *par = (TParameter<Long64_t> *) inputFile.Get(parameterName);
  if (!par)
    {
    if (verbose)
      {
      printCR();
      printValue("importParameter() input file",inputFile.GetName());
      printValue("importParameter() Property not found:",parameterName);
      }
    throw FileException(parameterName,"Property not found",__FUNCTION__);
    }
  double value = par->GetVal();
  delete par;
  if (verbose)
    {
    printCR();
    printValue("importParameter() parameterName",parameterName);
    printValue("importParameter() value",value);
    }
  return value;
}

void exportAccountant(Accountant & accountant, TFile & outputFile, bool verbose)
{
  if (verbose) printString("Accountant::exportTo(...)");
  unsigned int size = accountant.size();
  String className  = "Accountant";
  String objectName = accountant.name();
  String outputName;

  // output the number of elements
  outputName  = className;
  outputName += "_";
  outputName += objectName;
  outputName += "_N";
  if (verbose) printValue(outputName,size);
  exportParameter(outputFile,outputName,size);

  unsigned int k = 0;
  for (auto value : accountant.values())
    {
    outputName  = className;
    outputName += "_";
    outputName += objectName;
    outputName += "_";
    outputName += k;
    if (verbose) printValue(outputName,value);
    exportParameter(outputFile,outputName,value);
    k++;
    }
}

void importAccountant(Accountant & accountant, TFile & inputFile, bool verbose)
{
  if (verbose) printString("Accountant::importFrom(...)");
  unsigned int size = accountant.size();;
  unsigned int n;
  String className  = "Accountant";
  String objectName = accountant.name();
  String inputName;

  if (size==0) // need to read the size first
    {
    inputName  = className;
    inputName += "_";
    inputName += objectName;
    inputName += "_N";
    n = importParameter(inputFile,inputName);
    if (verbose) printValue(inputName,n);
    }
  else
    {
    n = size;
    }
  for (unsigned int k = 0; k<n; k++)
    {
    inputName  = className;
    inputName += "_";
    inputName += objectName;
    inputName += "_";
    inputName += k;
    long value = importParameter(inputFile,inputName);
    if (verbose) printValue(inputName,value);
    if (size==0)
      accountant.values().push_back(value);
    else
      accountant.setValueAt(k,value);
    }
}

String makeFileName(const String & inputPath,
                         const String & fileName,
                         const String & extension)
{
  String inputFileName = FileSystem::join(inputPath,fileName);   // an absolute fileName is used as it is
  if (!inputFileName.endsWith(extension)) inputFileName += extension;
  return inputFileName;
}

void createDirectory(const String path,
                     bool verbose)
{
  if (verbose)
    {
    printCR();
    printValue("createDirectory(const String path) Creating directory (if non-existing)",path);
    }
  FileSystem::createDirectory(path,true);
}

String substitute(const String & inputString,
                  const String & subString,
                  const String & newSubString)
{
  String outputString(inputString);
  outputString.replaceAll(subString,newSubString);
  return outputString;
}





} // namespace CAP

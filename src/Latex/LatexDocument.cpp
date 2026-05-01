#include "LatexDocument.hpp"
#include "StreamHelpers.hpp"
#include <iostream>

ClassImp(CAP::LatexDocument);
using std::cout;
using std::endl;

namespace CAP
{
  LatexDocument::LatexDocument()
  :
  LatexElement()
  { }

  LatexDocument::LatexDocument(const LatexDocument & src)
  :
  LatexElement(src)
  { }

//  LatexDocument &  LatexDocument::operator=(const LatexDocument & rhs)
//  {
//  if (rhs!=*this)
//    {
//    LatexElement::operator=(rhs);
//    }
//  return *this;
//  }

  void LatexDocument::create()
  {
  std::ofstream & p = openOutputAsciiFile(_outputPath,_outFileName,".tex",true);
  write(p);
  }

  void LatexDocument::writeHeader(std::ofstream & out)
  {
  if (_documentClassName.Length()<1)
    out << "\\documentclass{beamer}" << endl;
  else
    out << "\\documentclass{" << _documentClassName << "}" << endl;
  if (_themeName.Length()<1)
    out << "\\usetheme{Warsaw}" << endl;
  else
    out << "\\usetheme{" << _themeName << "}" << endl;
  skipLines(out,2);
  out << "\\title{" << _title << "}" << endl;
  if (_subtitle.Length()>0)  out << "\\subtitle{" << _subtitle << "}" << endl;
  out << "\\author{" << _author << "}" << endl;
  out << "\\institute{" << _institution << "}" << endl;
  if (_useToday)
    out << "\\date{\\today}" << endl;
  else
    out << "\\date{" << _date << "}" << endl;
  skipLines(out,2);
  out << "\\begin{document}" << endl;
  skipLines(out,2);
  out << "\\begin{frame}" << endl;
  out << "\\titlepage" << endl;
  out << "\\end{frame}" << endl;
  skipLines(out,2);
  }

  void LatexDocument::writeTrailer(std::ofstream & out)
  {
  skipLines(out,2);
  out << "\\end{document}" << endl;
  }

  void LatexDocument::writePackages(std::ofstream & out)
  {
  for (auto package : _packages) package.write(out);
  }

  void LatexDocument::writeCommands(std::ofstream & out)
  {
  for(auto command : _commands) command.write(out);
  }

  void LatexDocument::writeAuthors(std::ofstream & out)
  {
  for(auto author : _authors) author.write(out);
  }

  void LatexDocument::writeAbstract(std::ofstream & out)
  {
  out << "\\begin{abstract}" << endl;
  out << _abstract << endl;
  out << "\\end{abstract}" << endl;
  }

  void LatexDocument::addPackage(const String & name, const String & option)
  {
  LatexPackage package;
  package.setName(name);
  package.setOption(option);
  _packages.push_back(package);
  }


  void LatexDocument::addCommand(const String & keyword, const String & command)
  {
  LatexCommand comm;
  comm.setKeyword(keyword);
  comm.setCommand(command);
  _commands.push_back(comm);
  }


  void LatexDocument::addAuthor(const String & name, const String & email, const String & affiliation)
  {
  LatexAuthor author;
  author.setName(name);
  author.setEmail(email);
  author.setAffiliation(affiliation);
  _authors.push_back(author);
  }

  void LatexDocument::addPackage(const LatexPackage & package)
  {
  _packages.push_back(package);
  }

  void LatexDocument::addCommand(const LatexCommand & command)
  {
  _commands.push_back(command);
  }

  void LatexDocument::addAuthor(const LatexAuthor & author)
  {
  _authors.push_back(author);
  }

  void LatexDocument::addPackages(const std::vector<LatexPackage> & packages)
  {
  for(auto package : packages) _packages.push_back(package);
  }

  void LatexDocument::addCommands(const std::vector<LatexCommand> & commands)
  {
  for(auto command : commands) _commands.push_back(command);
  }

  void LatexDocument::addAbstract(const String & text)
  {
  _abstract = text;
  }

  void LatexDocument::help()
  {
  cout << "======================================" << endl;
  cout << "LatexDocument::help()" << endl;
  cout << "======================================" << endl;
  cout << "Known DocumentClasses/Styles" << endl;
  cout << "-- beamer    : creation of presentations (frame)"<< endl;
  cout << "  -- Warsaw  : theme Warsaw"<< endl;
  cout << endl;
  cout << "-- aps    : aps style papers"<< endl;
  cout << "-- prc    : prc style papers"<< endl;
  cout << "-- prl    : prlc style papers"<< endl;

  }

} // namespace CAP


#ifndef CAP__LatexDocument
#define CAP__LatexDocument
#include "LatexElement.hpp"
#include "LatexPackage.hpp"
#include "LatexCommand.hpp"
#include "LatexAuthor.hpp"

namespace CAP
{
  class LatexDocument : public LatexElement
  {
  public:

  LatexDocument();
  LatexDocument(const LatexDocument & src);
  LatexDocument  & operator=(const LatexDocument & rhs)
  {
//  if (rhs!=*this)
//    {
    LatexElement::operator=(rhs);
//    }
  return *this;
  }

  virtual ~LatexDocument() { }
  virtual void create();
  virtual void writeHeader(std::ofstream & out);
  virtual void writeTrailer(std::ofstream & out);
  virtual void writePackages(std::ofstream & out);
  virtual void writeCommands(std::ofstream & out);
  virtual void writeAuthors(std::ofstream & out);
  virtual void writeAbstract(std::ofstream & out);
  virtual void help();

  const String & documentClassName() const { return  _documentClassName;}
  const String & themeName()         const { return  _themeName;}
  const String & author()            const { return  _author;}
  const String & institution()       const { return  _institution;}
  const String & title()             const { return  _title;}
  const String & subtitle()          const { return  _subtitle;}
  const String & date()              const { return  _date;}
  bool  useToday()                   const { return  _useToday;}
  const String & outputPath()        const { return  _outputPath;}
  const String & outFileName()       const { return  _outFileName;}
  std::ofstream & out()              const { return  *_out;}
  const String & abstract()          const { return  _abstract;}

  const std::vector<LatexPackage> & packages() const { return _packages;}
  const std::vector<LatexCommand> & commands() const { return _commands;}
  const std::vector<LatexAuthor> &  authors()  const { return _authors;}

  void setDocumentClassName(const String & v)  { _documentClassName = v;}
  void setThemeName(const String & v)          { _themeName = v;}
  void setAuthor(const String & v)             { _author = v;}
  void setInstitution(const String & v)        { _institution = v;}
  void setTitle(const String & v)              { _title = v;}
  void setSubtitle(const String & v)           { _subtitle = v;}
  void setDate(const String & v)               { _date = v;}
  void setUseToday(bool v)                     { _useToday = v;}
  void setOutputPath(const String & v)         { _outputPath = v;}
  void setOutFileName(const String & v)        { _outFileName = v;}

  void addPackage(const String & name, const String & option);
  void addCommand(const String & keyword, const String & command);
  void addAuthor(const String & name, const String & email, const String & affiliation);

  void addPackage(const LatexPackage & package);
  void addCommand(const LatexCommand & command);
  void addAuthor(const LatexAuthor & author);
  void addPackages(const std::vector<LatexPackage> & package);
  void addCommands(const std::vector<LatexCommand> & commands);
  void addAuthors(const std::vector<LatexAuthor> & authors);
  void addAbstract(const String & text);

  protected:

  String _documentClassName;
  String _themeName;
  String _author;
  String _institution;
  String _title;
  String _subtitle;
  String _date;
  bool   _useToday;
  String _outputPath;
  String _outFileName;
  std::ofstream * _out;
  std::vector<LatexPackage> _packages;
  std::vector<LatexCommand> _commands;
  std::vector<LatexAuthor>  _authors;
  String _abstract;


  ClassDef(LatexDocument,0)

  };

} // namespace CAP

#endif // !CAP__LatexDocument

#ifndef CAP__LatexCommand
#define CAP__LatexCommand
#include "LatexElement.hpp"

namespace CAP
{
  class LatexCommand : public LatexElement
  {
  public:

  LatexCommand();
  LatexCommand(const LatexCommand & src);
  LatexCommand  & operator=(const LatexCommand & rhs);
  virtual ~LatexCommand() { reset(); }
  virtual void write(std::ofstream & out);

  void setKeyword(const String & text) { _name = text; }
  void setCommand(const String & text) { _comm = text; }
  const String & keyword() const { return _name; }
  const String & command() const { return _comm; }

  protected:

  String _comm;

  ClassDef(LatexCommand,0)
  };

} // namespace CAP

#endif // !CAP__LatexText

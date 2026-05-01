#ifndef CAP__LatexAuthor
#define CAP__LatexAuthor
#include "LatexElement.hpp"

namespace CAP
{
  class LatexAuthor : public LatexElement
  {
  public:

  LatexAuthor();
  LatexAuthor(const LatexAuthor & src);
  LatexAuthor  & operator=(const LatexAuthor & rhs);
  virtual ~LatexAuthor() { }
  virtual void write(std::ofstream & out);

  void setAuthor(const String & text) { _name = text; }
  void setEmail(const String & text) { _email = text; }
  void setAffiliation(const String & text) { _affiliation = text; }
  const String & author() const { return _name; }
  const String & email() const { return _email; }
  const String & affiliation() const { return _affiliation; }

  protected:

  String _email;
  String _affiliation;

  ClassDef(LatexAuthor,0)
  };

} // namespace CAP

#endif // !CAP__LatexAuthor

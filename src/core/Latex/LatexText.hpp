#ifndef DATUM__LatexText
#define DATUM__LatexText
#include "LatexElement.hpp"

namespace Datum
{
  class LatexText : public LatexElement
  {
  public:

  LatexText();
  LatexText(const LatexText & src);
  LatexText  & operator=(const LatexText & rhs);
  virtual ~LatexText() { reset(); }
  virtual void writeContent(std::ofstream & out);

  void setText(const String & text) { _name = text; }
  const String & text() const { return _name; }

  };

} // namespace Datum

#endif // !DATUM__LatexText

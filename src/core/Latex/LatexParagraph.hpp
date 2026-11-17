#ifndef DATUM__LatexParagraph
#define DATUM__LatexParagraph
#include "LatexElement.hpp"

namespace Datum
{
  class LatexParagraph : public LatexElement
  {
  public:

  LatexParagraph();
  LatexParagraph(const LatexParagraph & src);
  LatexParagraph  & operator=(const LatexParagraph & rhs);
  ~LatexParagraph(){ }
  virtual void writeHeader(std::ofstream & out);
  void setTitle(const String &title) { setName(title); }
  const String & title() const { return name(); }

  };

} // namespace Datum

#endif // !DATUM__LatexParagraph

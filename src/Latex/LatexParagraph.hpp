#ifndef CAP__LatexParagraph
#define CAP__LatexParagraph
#include "LatexElement.hpp"

namespace CAP
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

  ClassDef(LatexParagraph,0)
  };

} // namespace CAP

#endif // !CAP__LatexParagraph

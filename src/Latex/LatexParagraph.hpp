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

  ClassDef(LatexParagraph,0)
  };

} // namespace CAP

#endif // !CAP__LatexParagraph

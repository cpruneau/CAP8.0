#ifndef DATUM__LatexFigureGroup
#define DATUM__LatexFigureGroup
#include "LatexElement.hpp"

namespace Datum
{
  class LatexFigureGroup : public LatexElement
  {
  public:

  LatexFigureGroup();
  LatexFigureGroup(const LatexFigureGroup & src);
  LatexFigureGroup  & operator=(const LatexFigureGroup & rhs);
  virtual ~LatexFigureGroup() { }
  virtual void writeContent(std::ofstream & out);

  };

} // namespace Datum

#endif // !DATUM__LatexFigureGroup

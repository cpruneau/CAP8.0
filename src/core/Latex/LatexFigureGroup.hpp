#ifndef CAP__LatexFigureGroup
#define CAP__LatexFigureGroup
#include "LatexElement.hpp"

namespace CAP
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

} // namespace CAP

#endif // !CAP__LatexFigureGroup

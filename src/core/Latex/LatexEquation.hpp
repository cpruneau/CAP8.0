#ifndef DATUM__LatexEquation
#define DATUM__LatexEquation
#include "LatexElement.hpp"

namespace Datum
{
  class LatexEquation : public LatexElement
  {
  public:

  LatexEquation();
  LatexEquation(const LatexEquation & src);
  LatexEquation  & operator=(const LatexEquation & rhs);
  virtual ~LatexEquation() { }
  void setText(const String & text) { _name = text; }
  const String & text() const { return _name; }
  virtual void writeHeader(std::ofstream & out);
  virtual void writeContent(std::ofstream & out);
  virtual void writeTrailer(std::ofstream & out);

  };

} // namespace Datum

#endif // !DATUM__LatexEquation

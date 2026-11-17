#ifndef DATUM__LatexPackage
#define DATUM__LatexPackage
#include "LatexElement.hpp"

namespace Datum
{
  class LatexPackage : public LatexElement
  {
  public:

  LatexPackage();
  LatexPackage(const LatexPackage & src);
  LatexPackage  & operator=(const LatexPackage & rhs);
  virtual ~LatexPackage() { reset(); }
  virtual void write(std::ofstream & out);

  void setOption(const String & text) { _option = text; }
  const String & option() const { return _option; }

  protected:

  String _option;

  };

} // namespace Datum

#endif // !DATUM__LatexPackage

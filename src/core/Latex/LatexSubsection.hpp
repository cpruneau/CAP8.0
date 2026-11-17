#ifndef DATUM__LatexSubsection
#define DATUM__LatexSubsection
#include "LatexSection.hpp"

namespace Datum
{
  class LatexSubsection  : public LatexSection
  {
  public:

  LatexSubsection();
  LatexSubsection(const LatexSubsection & src);
  LatexSubsection  & operator=(const LatexSubsection & rhs);
  virtual ~LatexSubsection() { }
  virtual void writeHeader(std::ofstream & out);

  };

} // namespace Datum

#endif // !DATUM__LatexSubsection

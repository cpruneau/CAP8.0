#ifndef DATUM__LatexSubsubsection
#define DATUM__LatexSubsubsection
#include "LatexSection.hpp"

namespace Datum
{

  class LatexSubsubsection : public LatexSection
  {
  public:

  LatexSubsubsection();
  LatexSubsubsection(const LatexSubsubsection & src);
  LatexSubsubsection  & operator=(const LatexSubsubsection & rhs);
  ~LatexSubsubsection(){ }
  virtual void writeHeader(std::ofstream & out);
  void setTitle(const String &title) { setName(title); }
  const String & title() const { return name(); }

  };


} // namespace Datum

#endif // !DATUM__LatexSubsubsection

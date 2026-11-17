#ifndef DATUM__LatexSection
#define DATUM__LatexSection
#include "LatexElement.hpp"

namespace Datum
{
  class LatexSection : public LatexElement
  {
  public:

  LatexSection();
  LatexSection(const LatexSection & src);
  LatexSection  & operator=(const LatexSection & rhs);
  virtual ~LatexSection() { }
  virtual void writeHeader(std::ofstream & out);
  void setTitle(const String &title) { setName(title); }
  const String & title() const { return name(); }

  };

} // namespace Datum

#endif // !DATUM__LatexSection

#ifndef CAP__FillAttributes
#define CAP__FillAttributes
#include "Attributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

 
  class FillAttributes : public Attributes
  {
  public:

  FillAttributes();
  FillAttributes(const FillAttributes & src);
  FillAttributes  & operator=(const FillAttributes & rhs);
  virtual ~FillAttributes() { }
  virtual void reset();

  static FillAttributes * defaultFillAttributes();
  static void setDefaultFillAttributes(int color, float alpha, int style);

  protected:

  private:
  static FillAttributes * _defaultFillAttributes;


  ClassDef(FillAttributes,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__FillAttributes

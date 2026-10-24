#ifndef CAP__AxisRange
#define CAP__AxisRange

namespace CAP
{
  namespace GRAPHICS
  {

  class AxisRange
  {
  public:

  AxisRange();
  AxisRange(float minimum, float maximum);
  AxisRange(const AxisRange & src);
  AxisRange  & operator=(const AxisRange & rhs);
  virtual ~AxisRange() { }

  void  set(float minimum, float maximum);
  float minimum() const { return _minimum; }
  float maximum() const { return _maximum; }
  float range() const { return _maximum - _minimum; }

  protected:

  float _minimum;
  float _maximum;

 
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__AxisRange

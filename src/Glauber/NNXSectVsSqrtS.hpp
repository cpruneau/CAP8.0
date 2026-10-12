#ifndef CAP__NNXSectVsSqrtS
#define CAP__NNXSectVsSqrtS

namespace CAP
{
  //!
  //! @brief Parameterization of the inelastic nucleon-nucleon cross section vs collision energy sqrt(s), with several
  //! functional forms (option) and parameters a, b, c, d, n, eta.
    //!
  class NNXSectVsSqrtS
  {
  public:

  //! @brief Default constructor.
  NNXSectVsSqrtS();
  //! @brief Copy constructor. @param src object to copy
  NNXSectVsSqrtS(const NNXSectVsSqrtS & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  NNXSectVsSqrtS & operator=(const NNXSectVsSqrtS & rhs);
  virtual~NNXSectVsSqrtS(){}
  //! @brief Cross section at an energy. @param x sqrt(s) @return cross section
  virtual double evaluate(double x);
  //! @brief Select the functional form. @param option form
  virtual void setOption(unsigned int option);
  //! @brief Set the parameters of the form (a, b, c, d, n, eta). @param value value
  virtual void setA(double value) { _a = value; }
  virtual void setB(double value) { _b = value; }
  virtual void setC(double value) { _c = value; }
  virtual void setD(double value) { _d = value; }
  virtual void setN(double value) { _n = value; }
  virtual void setEta(double value) { _eta = value; }

  //! @brief Functional form. @return option
  unsigned int option() { return _option; }
  double a() const { return _a; }   // was unsigned int (truncated the parameter)
  double b() const { return _b; }   // was unsigned int (truncated the parameter)
  double c() const { return _c; }   // was unsigned int (truncated the parameter)
  double d() const { return _d; }   // was unsigned int (truncated the parameter)
  double n() const { return _n; }   // was unsigned int (truncated the parameter)
  double eta() const { return _eta; }   // was unsigned int (truncated the parameter)
  //! @brief Reset the parameters.
  virtual void reset();

  protected:

  unsigned int _option;
  double _a;
  double _b;
  double _c;
  double _d;
  double _n;
  double _eta;

  };

} // namespace CAP

#endif // !CAP__NNXSectVsSqrtS




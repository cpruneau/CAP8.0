#ifndef CAP__NNProfile
#define CAP__NNProfile

namespace CAP
{
  //!
    //! @brief Nucleon-nucleon collision probability vs transverse distance, with cross section fluctuations (gamma
    //! distribution), ranging from a hard sphere (omega=0) to a Gaussian (omega=1).
    //!
  //!Nucleon-Nucleon Profile function and cross section fluctuations
  //!
  //!Based on the paper by Maciej Rybczynski, Zbigniew Wlodarczyk
  //!from https://arxiv.org/abs/1307.0636
  //!code modifieed from Lozides et al. (Glauber.C)
  //!
  //!Summary: The nucleon-nucleon collision profile , is usually adopted in the form of hard sphere
  //!or a Gaussian shape. Rybczynski et al introduce a formulation in terms of cross section fluctuations
  //!given by the gamma distribution. This formulation  leads to profile functions that smoothly ranges
  //!between hard sphere and Gaussian profile.
  //!Examples demonstrating the sensitivity of profile function on cross section fluctuations are discussed
  //!in https://arxiv.org/abs/1307.0636
  //!
  //!Parameters of the function:
  //!_xSection : nucleon nucleon (inelastic) interaction cross section in units of milli-barn. The default value is 67.6.
  //!
  //!The default value is set to
  //!
  //!_omega: normalized variance of the fluctuations
  //!
  //!_g: Impact parameter elastic amplitude
  //!
  //!The variable _omega determines the shape of the function and is defined according to
  //! _omega = Var[sigma]/E[sigma]^2
  //!
  //!Use:
  //!_omega=0 : Hard sphere
  //!_omega=1 : Gaussian shall be  w/ exp(-b^2/R_0^2)
  //!
  //!The variable _g controls the overall normalization of the function. Use _g=1.
  //!
  //!The variable _r2 is the square of the radius corresponding to the scattering cross section _sect
  //!σinel = π· R2.
  //!Units for R2 are fm^2 
  //!

  class NNProfile
  {
  public:

  //! @brief Default constructor.
  NNProfile();
  //! @brief Copy constructor. @param src object to copy
  NNProfile(const NNProfile & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  NNProfile & operator=(const NNProfile & rhs);
  //! @brief Destructor.
  virtual ~NNProfile(){}
  //! @brief Collision probability at a transverse distance. @param x distance (fm) @return probability
  virtual double evaluate(double x);
  //! @brief Set the parameters. @param xSection inelastic cross section (mb) @param omega normalized variance of the
  //! cross section fluctuations @param g normalization
  virtual void setParameters(double xSection,
                             double omega=0.4,
                             double g=1);
  //! @brief Inelastic cross section (mb). @return cross section
  double xSection() { return _xSection; }
  //! @brief Normalized variance of the cross section fluctuations. @return omega
  double omega() { return _omega; }
  //! @brief Normalization. @return g
  double g() { return _g; }
  //! @brief Squared radius corresponding to the cross section (fm^2). @return r^2
  double rSq() { return _r2; }
  //! @brief 1/omega. @return value
  double omegaInv() { return _omegaInv; }
  //! @brief g/(omega r^2). @return value
  double gOverOmegaR2() { return _gOverOmegaR2; }

  //! @brief Reset the parameters.
  virtual void reset();

  protected:

  double _xSection;
  double _omega;
  double _g;
  double _r;
  double _r2;
  double _omegaInv;
  double _gOverOmegaR2;
  };

} // namespace CAP

#endif // !CAP__NNProfile


# Fourier module (core/Math/Fourier) — catalog of analytical transforms (branch `Fourier`, November 2026)

The module will hold the Fourier transforms of Datum: analytical transforms of functions (`Function`), numerical
transforms of functions on a domain, and fast Fourier transforms (FFT) of regularly spaced values and histograms. This
first document lists the functions whose transform is known in closed form: those of Datum, and others worth adding.

## Conventions (to confirm)

| Setting | Definition | Used for |
|---|---|---|
| Transform on the real line | F(k) = ∫ f(x) e^{-ikx} dx,  f(x) = (1/2π) ∫ F(k) e^{ikx} dk | integrable functions (Gaussian, Cauchy, ...) |
| Transform on a domain [a, b] | F_ab(k) = ∫_a^b f(x) e^{-ikx} dx | any function, with a domain given by the user |
| Fourier series, period L | c_n = (1/L) ∫_{x0}^{x0+L} f(x) e^{-i k_n x} dx, k_n = 2πn/L,  f(x) = Σ c_n e^{i k_n x} | periodic functions |
| Discrete-time transform | F(k) = Σ_m f(m) e^{-ikm}, periodic in k (period 2π) | distributions of counts, sampled values |
| DFT / FFT | X_m = Σ_{j=0}^{N-1} x_j e^{-2πi jm/N} | arrays and histograms |

k is an angular frequency (radians per unit of x). This is the convention of physics and of characteristic functions
(φ(t) = F(-t) for a probability density), so that the transforms of the distributions are their characteristic
functions. Other conventions (ordinary frequency ξ = k/2π, unitary 1/√(2π) factors, e^{+ikx}) are a change of variable
and a factor, which the module can offer as options.

### One formula for the three continuous settings

- The **series coefficients are the domain transform at the harmonics**: c_n = F_{x0, x0+L}(k_n) / L. A periodic
  function and a non-periodic function given on a domain [a, b] (understood as one period of its periodic extension,
  L = b - a) are therefore handled the same way: what the function needs is F_ab(k).
- **Periodized functions** (Poisson summation): if f_L(x) = Σ_m f(x - mL) is the periodic sum of a function whose
  line transform F is known, then c_n = F(k_n) / L exactly. This gives the wrapped Gaussian, the wrapped Cauchy, ...
  For a function well inside one period (σ ≪ L), F(k_n)/L is also an excellent approximation of the coefficients of
  the truncated function.
- **Sampling and bins** (link to the FFT): N values x_j = f(a + jΔ) give X_m ≈ F(k_m) e^{ik_m a} / Δ, k_m = 2πm/(NΔ),
  with aliasing of the frequencies beyond π/Δ. For a histogram (bin averages over width Δ), each term is also
  multiplied by sinc(k_m Δ/2), with sinc(u) = sin(u)/u. This is how an analytical transform is compared to the FFT of
  binned data, and how the bin averaging is corrected.

## Rules that extend the catalog

Every entry below gives the transform of a basic form; the rules give those of its scaled, shifted and combined forms.
They are exact on the line; on a domain, the domain is transformed with the function.

| f(x) | F(k) |
|---|---|
| A f(x) | A F(k) |
| f(x - x0) | e^{-ik x0} F(k) |
| f(x / s), s > 0 | s F(s k) |
| e^{i k0 x} f(x) | F(k - k0) |
| cos(k0 x) f(x) | [F(k - k0) + F(k + k0)] / 2 |
| f'(x) | ik F(k) |
| x f(x) | i F'(k) |
| (f * g)(x) (convolution) | F(k) G(k) |
| f(x) g(x) | (1/2π) (F * G)(k) |
| f(x) g(y) (separable, 2D) | F(k_x) G(k_y) |
| Σ f_i(x) | Σ F_i(k) |

With them: a sum of functions (`CompositeFunction` kSum, `ParameterizedComposite`), a separable product (kProduct)
and the amplitude, shift and scale parameters of the Datum functions (`BesselFunction` A Z_n(k(x - x0)), the location
and scale of the distributions, the mapping of the orthogonal series) are analytical when their parts are. A pointwise
product (kPointwiseProduct) is a convolution of the transforms: analytical in special cases only (Gaussian × Gaussian,
cos or e^{ik0x} × anything, polynomial × Gaussian, polynomial × exponential), numerical otherwise.

## Catalog: functions of Datum

"Line": transform on the real line. "Domain": transform on [a, b], hence the Fourier series of the periodic extension.
"Faddeeva": closed form with the complex error function w(z) = e^{-z²} erfc(-iz), to be added to SpecialFunctions.

| Datum class | f(x) | Line: F(k) | Domain [a, b] | Notes |
|---|---|---|---|---|
| `Constant` | c | 2π c δ(k) | c (e^{-ika} - e^{-ikb}) / (ik); c (b - a) at k = 0 | series: c_0 = c, others 0 |
| `Polynomial`, `Linear`, `Parabolic`, `Cubic`, `Quartic` | Σ c_m x^m | derivatives of δ (2π i^m δ^{(m)}(k)) | exact: ∫ x^m e^{cx} dx = e^{cx} Σ_{j=0}^{m} (-1)^j m!/(m-j)! x^{m-j}/c^{j+1}, c = -ik | for small k(b - a), a series in k avoids cancellations |
| `Exponential` | A e^{bx} | not integrable; with b < 0 on [0, ∞): A / (-b + ik) | A [e^{(b-ik)x} / (b - ik)]_a^b | |
| `Normal` | A e^{-(x-μ)²/2σ²} | A σ√(2π) e^{-ikμ} e^{-σ²k²/2} | Faddeeva (erf of a complex argument) | wrapped Gaussian: c_n = F(k_n)/L exactly |
| `Normal2D` | A e^{-[u² - 2ρuv + v²]/2(1-ρ²)} | 2π A σx σy √(1-ρ²) e^{-i(kx μx + ky μy)} e^{-(σx² kx² + 2ρ σx σy kx ky + σy² ky²)/2} | Faddeeva for ρ = 0 (product), numerical otherwise | rank 2 |
| `CauchyDistribution` (Breit-Wigner, Lorentzian) | A γ / (π((x-x0)² + γ²)) | A e^{-ik x0} e^{-γ\|k\|} | numerical (or exponential integrals) | wrapped Cauchy: c_n = F(k_n)/L |
| `StudentDistribution` | A p(x; ν, μ, σ) | A e^{-ikμ} K_{ν/2}(√ν σ\|k\|) (√ν σ\|k\|)^{ν/2} / (Γ(ν/2) 2^{ν/2-1}) | numerical | K_ν of real order: `Bessel` has integer orders |
| `ChiSquareDistribution` | A p(x; ν) | A (1 + 2ik)^{-ν/2} | incomplete gamma of a complex argument (numerical) | |
| `WeibullDistribution` | A p(x; k, λ) | k = 1 (exponential): A / (1 + ikλ); k = 2 (Rayleigh): Faddeeva; other k: no closed form | numerical | |
| `MaxwellBoltzmannDistribution` | A p(v; a) | Faddeeva (Gaussian moments of e^{-ikv} on v ≥ 0) | numerical | |
| `FDistribution` | A p(x; d1, d2) | confluent hypergeometric U: not worth it | numerical | |
| `LogNormal` | A p(x; μ, σ) | no closed form (the moment series diverges) | numerical | |
| `PoissonDistribution` | A μ^m e^{-μ}/m! on m = 0, 1, ... | A exp(μ(e^{-ik} - 1)) | (finite support: direct sum) | discrete-time transform, period 2π in k |
| `BinomialDistribution` | A C(n,m) p^m (1-p)^{n-m} | A (1 - p + p e^{-ik})^n | | discrete |
| `NegativeBinomialDistribution` | A P(m; μ, k) (mean μ, shape k) | A (1 + (μ/k)(1 - e^{-ik}))^{-k} | | discrete |
| `HypergeometricDistribution` | A P(m) | finite sum over the support (exact) | | discrete |
| `FourierHarmonic`, `FourierDecomposition`, `FourierFlow` | c cos(nx) + s sin(nx), v cos(n(x - ψ)) | π[(c - is) δ(k - n) + (c + is) δ(k + n)] | period 2π: c_{±n} = (c ∓ is)/2 = (v/2) e^{∓inψ}, c_0 = the constant | the coefficients are the parameters (exact) |
| `Legendre`, `LegendreSeries` | P_n(x) on [-1, 1] (0 outside) | 2 (-i)^n j_n(k) (spherical Bessel) | the series maps [low, high] onto [-1, 1]: scale and shift rules | j_n to add (simple recurrences) |
| `ChebyshevSeries` | T_n(x) on [-1, 1] | weighted by 1/√(1-x²): π (-i)^n J_n(k); unweighted: polynomial rule | | |
| `HermiteSeries` | H_n(t), t = (x - x0)/s (a polynomial) | polynomial: line transform diverges | polynomial rule | the Hermite *functions* H_n(x) e^{-x²/2} are eigenfunctions: √(2π) (-i)^n H_n(k) e^{-k²/2} |
| `Laguerre`, `LaguerreSeries` | L_n^α(t) (a polynomial) | polynomial: line transform diverges | polynomial rule | with the weight x^α e^{-x} on [0, ∞): Γ(n+α+1)/n! (ik)^n / (1 + ik)^{n+α+1} |
| `BesselJFunction`, `Bessel` J | A J_n(q(x - x0)) | A e^{-ik x0} 2 (-i)^n T_n(k/q) / √(q² - k²) for \|k\| < q, 0 for \|k\| > q | numerical | integrable singularities at \|k\| = q |
| `BesselKFunction`, `Bessel` K | A K_0(q\|x - x0\|) | A e^{-ik x0} π / √(q² + k²) | numerical | K_n, n ≥ 1: singular at 0 (not integrable for n ≥ 1) |
| `BesselYFunction`, `BesselIFunction`, Struve | | Y: distributions only; I grows exponentially; Struve: no | numerical on a domain | |
| `BezierCurve` | piecewise cubic through knots | ends held constant: δ terms | exact: polynomial rule on each segment | |
| `SphericalHarmonics` | Y_l^m(θ, φ) | | in φ (period 2π): one coefficient, at n = m | the transform on the sphere is a separate subject |
| `CompositeFunction`, `ParameterizedComposite` | sums, separable products | from the rules | from the rules | pointwise products: special cases only |
| `GammaFunction`, `BetaFunction`, `IncompleteGammaFunction`, `ScalarField`, `XmlScalarField`, `Pdf` | | | numerical (quadrature on a domain, or FFT of samples) | |

## Catalog: functions worth adding to Datum (closed-form transforms, common in physics)

| Function | f(x) | F(k) |
|---|---|---|
| Rectangle (boxcar), width w | 1 for \|x\| < w/2 | w sinc(kw/2) |
| Triangle, half-width w | 1 - \|x\|/w for \|x\| < w | w sinc²(kw/2) |
| sinc | sin(qx)/(qx) | (π/q) for \|k\| < q, 0 otherwise |
| Two-sided exponential (Laplace) | e^{-\|x\|/τ} | 2τ / (1 + k²τ²) |
| One-sided exponential (decay) | θ(x) e^{-x/τ} | τ / (1 + ikτ) |
| Damped oscillation | θ(x) e^{-x/τ} sin(ω0 x) | ω0 / ((1/τ + ik)² + ω0²) |
| Gaussian wave packet | cos(k0 x) e^{-x²/2σ²} | σ√(π/2) [e^{-σ²(k-k0)²/2} + e^{-σ²(k+k0)²/2}] |
| Hyperbolic secant | sech(x/s) | π s sech(π s k/2) |
| Gamma distribution | x^{α-1} e^{-x/θ} / (Γ(α) θ^α), x ≥ 0 | (1 + ikθ)^{-α} |
| Hermite functions | H_n(x) e^{-x²/2} | √(2π) (-i)^n H_n(k) e^{-k²/2} |
| Voigt (Gaussian ⊗ Cauchy, both of area 1) | convolution | e^{-ikx0} e^{-σ²k²/2 - γ\|k\|} (computing f itself needs Faddeeva) |
| Dirac comb, period L | Σ δ(x - mL) | (2π/L) Σ δ(k - 2πn/L) |
| Heaviside step | θ(x) | π δ(k) - i/k (principal value) |

Three-dimensional radial transforms (form factors, F(q) = 4π ∫ r² f(r) sin(qr)/(qr) dr), for the nuclear densities of
Glauber and for source functions of femtoscopy:

| Density | F(q) |
|---|---|
| Uniform sphere, radius R | (4π R³/3) · 3 j_1(qR)/(qR) |
| Gaussian e^{-r²/2σ²} | (2π)^{3/2} σ³ e^{-q²σ²/2} |
| Exponential e^{-r/a} | 8π a³ / (1 + q²a²)² (dipole form factor) |
| Yukawa e^{-r/a}/r | 4π a² / (1 + q²a²) |
| Symmetrized Fermi (Woods-Saxon with radius R, diffuseness d) | 4π² d / (q sinh(πqd)) [π d coth(πqd) sin(qR) - R cos(qR)] (central density 1; F(0) = (4π/3) R³ (1 + π²d²/R²)) |

## What is not in closed form, and how it will be done

- **Function on a domain, no closed form**: quadrature of f(x) cos(kx) and f(x) sin(kx) (oscillatory: Filon-type or
  Gauss-Legendre with enough points per period), or the FFT of fine samples with the bin correction above.
- **Sampled values and histograms**: FFT (radix-2, and a mixed-radix or Bluestein algorithm for any N), real-input
  transforms, zero padding, windows (Hann, Hamming, Blackman, ...), power spectrum, inverse transform, convolution and
  correlation by FFT, 2D FFT for histograms of rank 2.
- `CalculateFourierDecomposition` (Fitting) already gives the harmonics of angular distributions (flow) by averages or
  fits; the new module will give it an FFT path and the analytical coefficients of the functions above.

## Implementation (November 2026)

Folder `src/core/Math/Fourier` (in libMath), and the transforms declared with each function class.

**Interface of `Function`** (convention F(k) = ∫ f(x) e^{-ikx} dx):

| Member | What it gives |
|---|---|
| `hasAnalyticalFourierTransform()` | the transform on the whole line is in closed form (special functions included) |
| `hasAnalyticalFourierTransformOnDomain()` | the transform on a domain is exact (true for every polynomial) |
| `fourierTransformAt(k)`, `fourierTransformAt({kx, ky})` | transform on the line (`std::complex<double>`) |
| `fourierTransformAt(k, low, high)` | transform of f on [low, high]: exact, or approximated |
| `fourierCoefficient(n, x0, period)` | Fourier series coefficient: F_{[x0, x0+period]}(2πn/period)/period |
| `fourierTransform()` | the transform on the line as a `FourierTransform` (a `Function` of k) |
| `fourierTransform(low, high)` | the transform on a domain as a `FourierTransform` (with `coefficient(n)`) |
| `polynomialDegree()`, `copyFunction()` | what the defaults need (degree of a polynomial, copy of the function) |

Derived classes override the protected `lineTransform(k)` (rank 1), `lineTransformND(k)` (any rank), `domainTransform(k,
low, high)`, and, if needed, `newLineTransform()` / `newDomainTransform()`.

**`FourierTransform`** (a `Function` of rank 1 or 2): `valueAt(k)` gives F(k); `evaluateAt(k)` the part chosen by
`setPart()` (real part by default, imaginary part, modulus, squared modulus, phase), so that a transform can be drawn,
fitted or integrated like any function. `method()` says how it was obtained: `kClosedForm`, `kExactOnDomain` or
`kApproximation`, and `description()` gives the details.

**Functions without a closed form: a polynomial-based approximation.** On a domain, f is replaced by its Legendre series
(the polynomial through the Gauss-Legendre nodes, of a degree raised from 16 to 2048 until the last eighth of the
coefficients is below 1e-13 of the largest), whose transform is exact: (b - a)/2 e^{-ikm} Σ c_n 2(-i)^n j_n(k(b - a)/2).
The error of the transform is at most ∫|f - series| over the domain, for every k. For the probability densities without a
closed form (log-normal, F, Weibull of shapes other than 1 and 2), `fourierTransform()` needs no domain: the density is
approximated by a Legendre series on each of 38 segments cut at its quantiles (from 1e-12 to 1 - 1e-12), and the
approximation is made once for given parameters.

**Status of the catalog**

| Class | On the line | On a domain |
|---|---|---|
| `Normal`, `GaussianWavePacket` | closed form | exact (Faddeeva function) |
| `Normal2D` | closed form (rank 2) | — |
| `CauchyDistribution`, `StudentDistribution` (K_ν of real order), `ChiSquareDistribution`, `GammaDistribution` (new), `MaxwellBoltzmannDistribution` (Faddeeva), `WeibullDistribution` (shapes 1 and 2) | closed form | approximation |
| `LogNormal`, `FDistribution`, `WeibullDistribution` (other shapes) | piecewise Legendre approximation | approximation |
| `Poisson`, `Binomial`, `NegativeBinomial` | closed form (discrete-time transform) | exact (sum) |
| `Hypergeometric` | exact (finite sum) | exact (sum) |
| `Polynomial` (and Constant ... Quartic), `Legendre`, `Laguerre`, the orthogonal series | — | exact |
| `Exponential` | — | exact |
| `FourierHarmonic`, `FourierDecomposition`, `FourierFlow` | — | exact (the coefficients are the parameters) |
| `BesselJFunction`, `Bessel` J0 J1 | closed form | approximation |
| `BesselKFunction` order 0, `Bessel` K0 (defined on one side of x0) | closed form | approximation |
| `BezierCurve` | — | exact (piecewise cubic) |
| `Boxcar`, `Triangle`, `TwoSidedExponential`, `OneSidedExponential`, `DampedOscillation` (new) | closed form | exact |
| `Sinc`, `HyperbolicSecant`, `HermiteFunction`, `Voigt` (new) | closed form | approximation |
| `HeavisideStep`, `DiracComb` (new) | — (distributions) | exact; the comb gives `transformPeriod()`, `transformWeight(n)` |
| `CompositeFunction`, `ParameterizedComposite` | sums and separable products: from the components | sums: from the components |
| any other function of rank 1 | — | approximation |

The distributions of counts are transformed as distributions of counts (Σ P(m) δ(x - m)), not as f continued between the
integers; on a domain, an integer on a bound counts half (the same for the teeth of a Dirac comb).

`FourierTools` holds the building blocks: `faddeeva()` (Weideman's approximation, 1e-13), `scaledErf()`,
`exponentialIntegral()`, `gaussianLine()`, `gaussianDomain()`, `halfGaussianMoment()`, `sphericalBesselJ()`,
`besselKNormalized()`, `legendreCoefficients()`, `adaptiveLegendreCoefficients()`, `legendreTransform()`,
`polynomialTransform()`, `PiecewiseLegendre`.

Tests: `TestFourierTransforms` (ctest `FourierTransforms`, 192 checks): every closed form and exact domain transform
against the quadrature of f(x) e^{-ikx}, the approximations to 1e-9, the special functions, the series coefficients, the
distributions of counts, the composites.

## Next step

FFT of regularly spaced values and of histograms (rank 1 and 2), with the bin correction sinc(kΔ/2), windows, power
spectra, inverse transforms, convolutions; then the inverse transforms of the functions.

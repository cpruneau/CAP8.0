################################################################################################
# Project CAP/Math
#
# One library (libMath) and one ROOT dictionary, the sources in sub-folders by subject:
#   Basic          basic numerical functions, constants, exceptions, complex numbers
#   LinearAlgebra  matrices and vectors (2D, 3D, n-D, Lorentz)
#   Functions      Function, ParameterizedFunction and the functions of CAP, special functions, fields
#   Data           data points, histograms, profiles, tables
#   Search         extremum searches (SearchEngine and its engines, ExtremumSearch)
#   Fitting        fits (Fitter, likelihoods, chi square, Bayesian fits, fit records)
#   StatisticalTests  statistical tests (binomial, Poisson, z, t, chi square, histograms, Kolmogorov-Smirnov)
################################################################################################

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Math SHARED
FolderTypes.cpp
MathReflection.cpp
# Basic
Basic/Complex.cpp
Basic/MathBasicFunctions.cpp
Basic/MathBasicGeometry.cpp
Basic/MathBasicIntegration.cpp
Basic/MathBasicInterpolation.cpp
Basic/MathBasicSearch.cpp
Basic/MathBasicSortingFunctions.cpp
Basic/MathBasicStatistics.cpp
Basic/MathConstants.cpp
Basic/MathExceptions.cpp
# LinearAlgebra
LinearAlgebra/Matrix.cpp
LinearAlgebra/Vector2D.cpp
LinearAlgebra/Vector3D.cpp
LinearAlgebra/VectorLorentz.cpp
LinearAlgebra/VectorND.cpp
# Functions
Functions/Bessel.cpp
Functions/BesselFunction.cpp
Functions/BesselFunctions.cpp
Functions/CompositeFunction.cpp
Functions/Constant.cpp
Functions/Cubic.cpp
Functions/Exponential.cpp
Functions/Function.cpp
Functions/FunctionRegistry.cpp
Functions/Laguerre.cpp
Functions/Legendre.cpp
Functions/Linear.cpp
Functions/LogNormal.cpp
Functions/Normal.cpp
Functions/Normal2D.cpp
Functions/OrthogonalSeries.cpp
Functions/ParameterizedComposite.cpp
Functions/Parabolic.cpp
Functions/Parameter.cpp
Functions/ParameterizedFunction.cpp
Functions/Polynomial.cpp
Functions/Polynomial2D.cpp
Functions/Quartic.cpp
Functions/ScalarField.cpp
Functions/SphericalHarmonics.cpp
Functions/XmlScalarField.cpp
# Data
Data/DataPoint.cpp
Data/DataPoint2D.cpp
Data/DataPoint3D.cpp
Data/DataPointColumns.cpp
Data/DataPointND.cpp
Data/DataPoints.cpp
Data/DataPointsVector.cpp
Data/Histogram.cpp
Data/Partition.cpp
Data/Profile.cpp
Data/Table2D.cpp
# Search
Search/AnnealingSearch.cpp
Search/DivideAndConquerSearch.cpp
Search/EvolutionarySearch.cpp
Search/ExploreExploitSearch.cpp
Search/ExtremumSearch.cpp
Search/GradientSearch.cpp
Search/GreedySearch.cpp
Search/HillClimbingSearch.cpp
Search/ScanSearch.cpp
Search/SearchDomain.cpp
Search/SearchEngine.cpp
Search/SearchProblem.cpp
# Fitting
Fitting/BayesianFitter.cpp
Fitting/ChiSquare.cpp
Fitting/FitRecord.cpp
Fitting/FitViews.cpp
Fitting/Fitter.cpp
Fitting/Likelihood.cpp
Fitting/MetropolisHastings.cpp
Fitting/ModelValues.cpp
Fitting/Prior.cpp
# StatisticalTests
StatisticalTests/Distributions.cpp
StatisticalTests/StatisticalTests.cpp
Random/MersenneTwister.cpp
Random/RandomDistribution.cpp
Random/RandomGenerator.cpp)

target_link_libraries(Math Base ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Math  PUBLIC Base ${EXTRA_INCLUDES} )
# the headers are included by their names (e.g. #include "Histogram.hpp"): each sub-folder is on the include path,
# here for the library and its dictionary, and in src/CMakeLists.txt for the other modules

# Tests (run with ctest in the build directory; not installed)
add_executable(TestMathReflection tests/TestMathReflection.cpp)
target_link_libraries(TestMathReflection Math Base ${ROOT_LIBRARIES})
add_test(NAME MathReflection COMMAND TestMathReflection)
add_executable(TestRandom Random/tests/TestRandom.cpp)
target_link_libraries(TestRandom Math Base ${ROOT_LIBRARIES})
add_test(NAME Random COMMAND TestRandom)
add_executable(TestExtremumSearch Search/tests/TestExtremumSearch.cpp)
target_link_libraries(TestExtremumSearch Math Base ${ROOT_LIBRARIES})
add_test(NAME ExtremumSearch COMMAND TestExtremumSearch)
add_executable(TestFitter Fitting/tests/TestFitter.cpp)
target_link_libraries(TestFitter Math Base ${ROOT_LIBRARIES})
add_test(NAME Fitter COMMAND TestFitter)
add_executable(TestLikelihood Fitting/tests/TestLikelihood.cpp)
target_link_libraries(TestLikelihood Math Base ${ROOT_LIBRARIES})
add_test(NAME Likelihood COMMAND TestLikelihood)
add_executable(TestBayesian Fitting/tests/TestBayesian.cpp)
target_link_libraries(TestBayesian Math Base ${ROOT_LIBRARIES})
add_test(NAME Bayesian COMMAND TestBayesian)
add_executable(TestFits Fitting/tests/TestFits.cpp)
target_link_libraries(TestFits Math Base ${ROOT_LIBRARIES})
add_test(NAME Fits COMMAND TestFits)
add_executable(TestFitStorage Fitting/tests/TestFitStorage.cpp)
target_link_libraries(TestFitStorage Math Base ${ROOT_LIBRARIES})
add_test(NAME FitStorage COMMAND TestFitStorage)
add_executable(TestFitFunctions Functions/tests/TestFitFunctions.cpp)
target_link_libraries(TestFitFunctions Math Base ${ROOT_LIBRARIES})
add_test(NAME FitFunctions COMMAND TestFitFunctions)
add_executable(TestStatisticalTests StatisticalTests/tests/TestStatisticalTests.cpp)
target_link_libraries(TestStatisticalTests Math Base ${ROOT_LIBRARIES})
add_test(NAME StatisticalTests COMMAND TestStatisticalTests)
add_executable(TestFits2D Fitting/tests/TestFits2D.cpp)
target_link_libraries(TestFits2D Math Base ${ROOT_LIBRARIES})
add_test(NAME Fits2D COMMAND TestFits2D)

install(TARGETS Math  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


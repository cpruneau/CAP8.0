################################################################################################
# Project CAP/Math
################################################################################################
ROOT_GENERATE_DICTIONARY(G__Math
ParameterizedFunction.hpp
Bessel.hpp
ChiSquare.hpp
Complex.hpp
CompositeFunction.hpp
Constant.hpp
Cubic.hpp
DataPoint.hpp
DataPointColumns.hpp
DataPoint2D.hpp
DataPoint3D.hpp
DataPointND.hpp
DataPoints.hpp
DataPointsVector.hpp
Exponential.hpp
Function.hpp
Histogram.hpp
Profile.hpp
Partition.hpp
Vector2D.hpp
Vector3D.hpp
VectorLorentz.hpp
VectorND.hpp
Laguerre.hpp
Legendre.hpp
Linear.hpp
Matrix.hpp
LogNormal.hpp
Normal.hpp
Parabolic.hpp
Parameter.hpp
Polynomial.hpp
Table2D.hpp
Quartic.hpp
ScalarField.hpp
SphericalHarmonics.hpp
LINKDEF MathLinkDef.h)

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Math SHARED
ParameterizedFunction.cpp
Bessel.cpp
BesselFunctions.cpp
ChiSquare.cpp
Complex.cpp
CompositeFunction.cpp
Constant.cpp
Cubic.cpp
DataPoint.cpp
DataPointColumns.cpp
DataPoint2D.cpp
DataPoint3D.cpp
DataPointND.cpp
DataPoints.cpp
DataPointsVector.cpp
Exponential.cpp
FolderTypes.cpp
BayesianFitter.cpp
Fitter.cpp
Likelihood.cpp
MetropolisHastings.cpp
Function.cpp
Histogram.cpp
Profile.cpp
MathBasicFunctions.cpp
MathBasicGeometry.cpp
MathBasicInterpolation.cpp
MathBasicIntegration.cpp
MathBasicSearch.cpp
MathBasicSortingFunctions.cpp
MathBasicStatistics.cpp
MathConstants.cpp
AnnealingSearch.cpp
DivideAndConquerSearch.cpp
EvolutionarySearch.cpp
ExploreExploitSearch.cpp
ExtremumSearch.cpp
GradientSearch.cpp
GreedySearch.cpp
HillClimbingSearch.cpp
ScanSearch.cpp
SearchDomain.cpp
SearchEngine.cpp
SearchProblem.cpp
MathExceptions.cpp
Partition.cpp
Vector2D.cpp
Vector3D.cpp
VectorLorentz.cpp
VectorND.cpp
Laguerre.cpp
Legendre.cpp
Linear.cpp
Matrix.cpp
LogNormal.cpp
Normal.cpp
Parabolic.cpp
Parameter.cpp
Prior.cpp
Polynomial.cpp
Table2D.cpp
Quartic.cpp
ScalarField.cpp
SphericalHarmonics.cpp
XmlScalarField.cpp
G__Math.cxx)

target_link_libraries(Math Base ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Math  PUBLIC Base ${EXTRA_INCLUDES} )

# Tests (run with ctest in the build directory; not installed)
add_executable(TestExtremumSearch tests/TestExtremumSearch.cpp)
target_link_libraries(TestExtremumSearch Math Base ${ROOT_LIBRARIES})
add_test(NAME ExtremumSearch COMMAND TestExtremumSearch)
add_executable(TestFitter tests/TestFitter.cpp)
target_link_libraries(TestFitter Math Base ${ROOT_LIBRARIES})
add_test(NAME Fitter COMMAND TestFitter)
add_executable(TestLikelihood tests/TestLikelihood.cpp)
target_link_libraries(TestLikelihood Math Base ${ROOT_LIBRARIES})
add_test(NAME Likelihood COMMAND TestLikelihood)
add_executable(TestBayesian tests/TestBayesian.cpp)
target_link_libraries(TestBayesian Math Base ${ROOT_LIBRARIES})
add_test(NAME Bayesian COMMAND TestBayesian)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/libMath.rootmap" "${CMAKE_CURRENT_BINARY_DIR}/libMath_rdict.pcm" DESTINATION "$ENV{CAP_LIB_PATH}")
install(TARGETS Math  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


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
DataPoint2D.hpp
DataPoints.hpp
DataPointsVector.hpp
Exponential.hpp
Folder.hpp
Function.hpp
Histogram.hpp
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
DataPoint2D.cpp
DataPoints.cpp
DataPointsVector.cpp
Exponential.cpp
Folder.cpp
Function.cpp
Histogram.cpp
MathBasicFunctions.cpp
MathBasicGeometry.cpp
MathBasicInterpolation.cpp
MathBasicIntegration.cpp
MathBasicSearch.cpp
MathBasicSortingFunctions.cpp
MathBasicStatistics.cpp
MathConstants.cpp
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
Polynomial.cpp
Quartic.cpp
ScalarField.cpp
SphericalHarmonics.cpp
G__Math.cxx)

target_link_libraries(Math Base ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Math  PUBLIC Base ${EXTRA_INCLUDES} )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/libMath.rootmap" "${CMAKE_CURRENT_BINARY_DIR}/libMath_rdict.pcm" DESTINATION "$ENV{CAP_LIB_PATH}")
install(TARGETS Math  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


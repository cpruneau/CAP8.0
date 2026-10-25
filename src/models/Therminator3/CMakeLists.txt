###############################################################################################
# Project CAP/Therminator3
###############################################################################################

add_compile_options(-Wall -Wextra -pedantic)
add_library(Therminator3 SHARED
Therminator3Tasks.cpp
Chemistry.cpp
Hypersurface.cpp
Model.cpp
ModelBlastWave.cpp
ModelBWA.cpp
ModelHydro2D.cpp
ModelHydro3D.cpp
ModelKrakowSFO.cpp
ModelHadronGas.cpp
ModelSettings.cpp
ThermalDerivedHistos.cpp
ThermalHadron.cpp
ThermalHadronGas.cpp
ThermalHadronGasFD.cpp
ThermalHistos.cpp
ThermalModel.cpp
Therminator3.cpp
Thermodynamics.cpp
Viscosity.cpp)

target_link_libraries(Therminator3 Base Math Helpers Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Therminator3  PUBLIC  Base Math Helpers Particles  ${EXTRA_INCLUDES} )
install(TARGETS Therminator3  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

# Tests (run with ctest in the build directory; not installed)
add_executable(TestTherminator3Models tests/TestTherminator3Models.cpp)
target_link_libraries(TestTherminator3Models Therminator3 Particles Math Base ${ROOT_LIBRARIES})
target_compile_definitions(TestTherminator3Models PRIVATE CAP_PARTICLE_DB="${CMAKE_SOURCE_DIR}/../DB/ParticleData/ParticleDb.xml")
add_test(NAME Therminator3Models COMMAND TestTherminator3Models)
# the comparison on Therminator 2's RHIC hypersurface runs when the directory of Therminator 2 is given:
# cmake -S src -B build -DTHERMINATOR2_DIR=/path/to/therminator2
if (THERMINATOR2_DIR)
  set_tests_properties(Therminator3Models PROPERTIES ENVIRONMENT "THERMINATOR2_DIR=${THERMINATOR2_DIR}")
endif()

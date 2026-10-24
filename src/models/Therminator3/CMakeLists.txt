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

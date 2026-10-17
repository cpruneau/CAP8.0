################################################################################################
# Project CAP/Base
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})

ROOT_GENERATE_DICTIONARY(G__Base
Accountant.hpp
Configuration.hpp
ConfigurationManager.hpp
EnvironmentVariables.hpp
Factory.hpp
Filter.hpp
BinaryFile.hpp
CapFile.hpp
Directory.hpp
File.hpp
FileSystem.hpp
FileSystemEntry.hpp
Folder.hpp
LongParameter.hpp
Object.hpp
Parser.hpp
PrintHelpers.hpp
Properties.hpp
PropertiesParser.hpp
Property.hpp
String.hpp
SelectionGenerator.hpp
StreamHelpers.hpp
Task.hpp
Timer.hpp
TxtFile.hpp
LINKDEF BaseLinkDef.h)

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
# The XML classes (XmlElement, XmlReader, XmlWriter, XmlDocument, XmlFile) are plain C++ (no ROOT dictionary); they are
# part of Base so that Task and ConfigurationManager can read and write their configuration as XML.
add_library(Base SHARED
Accountant.cpp
Condition.cpp
Configuration.cpp
ConfigurationManager.cpp
EnvironmentVariables.cpp
Exceptions.cpp
Factory.cpp
Filter.cpp
BinaryFile.cpp
CapFile.cpp
Directory.cpp
File.cpp
FileSystem.cpp
FileSystemEntry.cpp
Folder.cpp
LongParameter.cpp
NameHelpers.cpp
Object.cpp
Parser.cpp
PrintHelpers.cpp
Properties.cpp
PropertiesParser.cpp
Property.cpp
String.cpp
SelectionGenerator.cpp
StreamHelpers.cpp
Task.cpp
Timer.cpp
TxtFile.cpp
XmlDocument.cpp
XmlElement.cpp
XmlFile.cpp
XmlReader.cpp
XmlWriter.cpp
G__Base.cxx)

target_link_libraries(Base ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Base PUBLIC Base ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


# Tests (run with ctest in the build directory; not installed)
add_executable(TestFileSystem tests/TestFileSystem.cpp)
target_link_libraries(TestFileSystem Base ${ROOT_LIBRARIES})
add_test(NAME FileSystem COMMAND TestFileSystem)
add_executable(TestDirectory tests/TestDirectory.cpp)
target_link_libraries(TestDirectory Base ${ROOT_LIBRARIES})
add_test(NAME Directory COMMAND TestDirectory)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/libBase.rootmap" "${CMAKE_CURRENT_BINARY_DIR}/libBase_rdict.pcm" DESTINATION "$ENV{CAP_LIB_PATH}")
install(TARGETS Base  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


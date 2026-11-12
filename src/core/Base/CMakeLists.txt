################################################################################################
# Project CAP/Base
################################################################################################



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
FileListing.cpp
FileSystemEntry.cpp
Folder.cpp
Reflection.cpp
BaseReflection.cpp
LongParameter.cpp
StringParameter.cpp
NameHelpers.cpp
Object.cpp
Parser.cpp
PrintHelpers.cpp
Properties.cpp
PropertiesParser.cpp
Property.cpp
String.cpp
StreamHelpers.cpp
Task.cpp
Timer.cpp
TxtFile.cpp
XmlDocument.cpp
XmlElement.cpp
XmlFile.cpp
XmlReader.cpp
XmlWriter.cpp)

target_link_libraries(Base ${EXTRA_LIBS} )
target_include_directories(Base PUBLIC Base ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


# Tests (run with ctest in the build directory; not installed)
add_executable(TestFileSystem tests/TestFileSystem.cpp)
target_link_libraries(TestFileSystem Base)
add_test(NAME FileSystem COMMAND TestFileSystem)
add_executable(TestDirectory tests/TestDirectory.cpp)
target_link_libraries(TestDirectory Base)
add_test(NAME Directory COMMAND TestDirectory)
add_executable(TestFileLoading tests/TestFileLoading.cpp)
target_link_libraries(TestFileLoading Base)
add_test(NAME FileLoading COMMAND TestFileLoading)
add_executable(TestLazyLoading tests/TestLazyLoading.cpp)
target_link_libraries(TestLazyLoading Base)
add_test(NAME LazyLoading COMMAND TestLazyLoading)
add_executable(TestReflection tests/TestReflection.cpp)
target_link_libraries(TestReflection Base)
add_test(NAME Reflection COMMAND TestReflection)
add_executable(TestFolderEditing tests/TestFolderEditing.cpp)
target_link_libraries(TestFolderEditing Base)
add_test(NAME FolderEditing COMMAND TestFolderEditing)

cap_install(Base LIBRARY "$ENV{CAP_LIB_PATH}")


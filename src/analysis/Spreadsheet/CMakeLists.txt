################################################################################################
# Project Datum/Spreadsheet
#
# Analysis of tables of data (Table2D): reading and writing text, CSV, binary and Excel files,
# statistics and correlations of the columns, plots (y vs x, bars, distributions, correlation
# maps), and the task SpreadsheetAnalyzer. The table pane of DatumGui (gui/SpreadsheetPane) uses it.
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Spreadsheet SHARED
TableFormat.cpp
TableReader.cpp
TableWriter.cpp
TableStatistics.cpp
TablePlots.cpp
SpreadsheetAnalyzer.cpp
SpreadsheetTasks.cpp)

target_link_libraries(Spreadsheet Base Math ${EXTRA_LIBS})
target_include_directories(Spreadsheet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${EXTRA_INCLUDES})

datum_install(Spreadsheet LIBRARY "$ENV{DATUM_LIB_PATH}")

add_executable(TestSpreadsheet tests/TestSpreadsheet.cpp)
target_link_libraries(TestSpreadsheet Spreadsheet Base Math)
add_test(NAME Spreadsheet COMMAND TestSpreadsheet WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

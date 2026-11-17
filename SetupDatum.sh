#!/bin/bash

####################################################################################################
#
# Datum (formerly CAP) -- source this script, from the Datum folder, to define the environment variables used to
# build and run Datum:
#
#    cd Datum                 : the folder of Datum
#    source SetupDatum.sh     : the environment variables
#    python3 BuildDatum.py    : a window to configure and run the build (or: cmake -S src -B build; make -C build install)
#    python3 RunDatum.py      : a window to compose and run the workflows (or: bin/Datum <task> <dir>/ <file.xml> <out>/)
#    bin/DatumGui             : the histograms, plots and figures
#
# Tools: cmake 3.14 or later, make, a C++17 compiler (clang or gcc). Optional packages (see BuildDatum.py): Pythia 8
# (https://pythia.org), FastJet (https://fastjet.fr), Herwig 7, ROOT (https://root.cern, for .root files only).
#
# Folders of your data, defined in your shell profile (e.g. ~/.zshrc) rather than here:
#    DATUM_USER_HISTO_IMPORT_PATH, DATUM_USER_HISTO_EXPORT_PATH : histograms read and written
#    DATUM_USER_DATA_IMPORT_PATH,  DATUM_USER_DATA_EXPORT_PATH  : data and calibrations read and written
# (the former names CAP_USER_... are still read).
#
# Every variable is defined under its name, DATUM_..., and under its former name, CAP_...: the programs read DATUM_...
# (CAP_... when DATUM_... is not set), and scripts written for CAP (e.g. grid jobs) keep working.
#
####################################################################################################

# former name of a variable used when the new one is not set: datum_from <DATUM_name> <CAP_name>
datum_from() { eval "[ -z \"\${$1}\" ] && [ -n \"\${$2}\" ] && export $1=\"\${$2}\""; }
datum_from DATUM_USER_HISTO_IMPORT_PATH CAP_USER_HISTO_IMPORT_PATH
datum_from DATUM_USER_HISTO_EXPORT_PATH CAP_USER_HISTO_EXPORT_PATH
datum_from DATUM_USER_DATA_IMPORT_PATH  CAP_USER_DATA_IMPORT_PATH
datum_from DATUM_USER_DATA_EXPORT_PATH  CAP_USER_DATA_EXPORT_PATH

export DATUM_ROOT_PATH=`pwd`
export DATUM_SRC_PATH="$DATUM_ROOT_PATH/src/"
export DATUM_BIN_PATH="$DATUM_ROOT_PATH/bin/"
export DATUM_LIB_PATH="$DATUM_ROOT_PATH/lib/"
export DATUM_GRID_PATH="$DATUM_ROOT_PATH/Grid/"
export DATUM_GRID_PATH_WSU="$DATUM_ROOT_PATH/Grid/WSU/"
export DATUM_GRID_PATH_ROM="$DATUM_ROOT_PATH/Grid/ROM/"
export DATUM_MACROS_PATH="$DATUM_SRC_PATH/Macros/"
export DATUM_PROJECTS_PATH="$DATUM_ROOT_PATH/projects/"
export DATUM_DATABASE_PATH="$DATUM_ROOT_PATH/DB/"
export DATUM_DATA_IMPORT_PATH="$DATUM_USER_DATA_IMPORT_PATH"
export DATUM_DATA_EXPORT_PATH="$DATUM_USER_DATA_EXPORT_PATH"
export DATUM_CALIB_IMPORT_PATH="$DATUM_USER_DATA_IMPORT_PATH"
export DATUM_CALIB_EXPORT_PATH="$DATUM_USER_DATA_EXPORT_PATH"
export DATUM_HISTOS_IMPORT_PATH="$DATUM_USER_HISTO_IMPORT_PATH"
export DATUM_HISTOS_EXPORT_PATH="$DATUM_USER_HISTO_EXPORT_PATH"
export DATUM_PYTHIA8_PATH="$PYTHIA8"
export DATUM_PYTHIA_ROOT_DIR="$DATUM_PYTHIA8_PATH"
export DATUM_PYTHIA8_INCLUDE_PATH="$DATUM_PYTHIA8_PATH/include/"
export DATUM_PYTHIA8_LIB_PATH="$DATUM_PYTHIA8_PATH/lib/"
export DATUM_FASTJET_PATH="$FASTJET"
export DATUM_FASTJET_INCLUDE_PATH="$DATUM_FASTJET_PATH/include/"
export DATUM_FASTJET_LIB_PATH="$DATUM_FASTJET_PATH/lib/"

export PATH="$DATUM_BIN_PATH:$PATH"
export DYLD_LIBRARY_PATH="$DATUM_LIB_PATH:$DYLD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$DATUM_LIB_PATH:$LD_LIBRARY_PATH"

# the former names, for the scripts and workflows written for CAP
for name in USER_HISTO_IMPORT_PATH USER_HISTO_EXPORT_PATH USER_DATA_IMPORT_PATH USER_DATA_EXPORT_PATH \
            ROOT_PATH SRC_PATH BIN_PATH LIB_PATH GRID_PATH GRID_PATH_WSU GRID_PATH_ROM MACROS_PATH PROJECTS_PATH \
            DATABASE_PATH DATA_IMPORT_PATH DATA_EXPORT_PATH CALIB_IMPORT_PATH CALIB_EXPORT_PATH HISTOS_IMPORT_PATH \
            HISTOS_EXPORT_PATH PYTHIA8_PATH PYTHIA_ROOT_DIR PYTHIA8_INCLUDE_PATH PYTHIA8_LIB_PATH FASTJET_PATH \
            FASTJET_INCLUDE_PATH FASTJET_LIB_PATH; do
  eval "export CAP_$name=\"\${DATUM_$name}\""
done
unset -f datum_from

echo "========================================================================================"
echo " Datum 1.0 (formerly CAP): environment variables (also defined as CAP_...)"
echo "========================================================================================"
for name in ROOT_PATH SRC_PATH BIN_PATH LIB_PATH DATABASE_PATH PROJECTS_PATH GRID_PATH MACROS_PATH \
            HISTOS_IMPORT_PATH HISTOS_EXPORT_PATH DATA_IMPORT_PATH DATA_EXPORT_PATH CALIB_IMPORT_PATH CALIB_EXPORT_PATH \
            PYTHIA8_PATH FASTJET_PATH; do
  eval "printf ' %-28s : %s\n' DATUM_$name \"\${DATUM_$name}\""
done
echo "========================================================================================"

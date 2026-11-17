# Datum
Datum 1.0, formerly CAP (the Correlation Analysis Package).

The programs were renamed with Datum 1.0; their former names still work:

| Datum 1.0          | Former name (CAP) |
|--------------------|-------------------|
| `bin/Datum`        | `bin/CAP`         |
| `bin/DatumGui`     | `bin/CapGui`      |
| `bin/DatumShell`   | `bin/CapShell`    |
| `bin/DatumRootConvert` | `bin/CapRootConvert` |
| `RunDatum.py`      | `RunCAP.py`       |
| `BuildDatum.py`    | `BuildCAP.py`     |
| `SetupDatum.sh`    | `SetupCAP.sh`     |
| `DATUM_...` environment variables | `CAP_...` (both are defined by `SetupDatum.sh`) |
| `-DDATUM_WITH_...` CMake options | `-DCAP_WITH_...` |

The C++ classes (namespace `CAP`), the workflow files and the data files (`.cap`) are unchanged.

# Prerequisites
- `cmake` (3.14 or later), `make` and a C++17 compiler. Datum does not need ROOT.
- Optional: `PYTHIA8` (Pythia event generator, `-DDATUM_WITH_PYTHIA8=ON`), with its location in
 ```
 export PYTHIA8=<your_pythia8_directory>
 ```
- Optional: `FastJet` (jet analysis, `-DDATUM_WITH_JETS=ON`), with its location in
 ```
 export FASTJET=<your_fastjet_directory>
 ```
- Optional: `root` (`-DDATUM_WITH_ROOT=ON`: `.root` histogram files in DatumGui and `bin/DatumRootConvert`, the Epos
  and Pythia readers of ROOT event trees), activated with
 ```
 source <your_root_install_path>/bin/thisroot.sh
 ```
- Folders of your histograms and data, in your shell profile:
 ```
 export DATUM_USER_HISTO_IMPORT_PATH=<folder>   DATUM_USER_HISTO_EXPORT_PATH=<folder>
 export DATUM_USER_DATA_IMPORT_PATH=<folder>    DATUM_USER_DATA_EXPORT_PATH=<folder>
 ```

# Build Datum
With the build GUI (Python 3 with tkinter):
```
cd <your_datum_directory>
python3 BuildDatum.py
```
Choose the optional modules (Therminator3, Latex, Jets, Epos, PHSD) and the event generators (Pythia8, Herwig), enter
a build directory name (it is created in `<your_datum_directory>`; several build directories can coexist) and click
Build. The output of cmake and make is shown in the lower half of the window and can be saved to a file.

From a shell:
```
cd <your_datum_directory>
source SetupDatum.sh
cmake -S src -B build
make -C build -j8 install
```
The core modules (Base, Math, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, PtPt, NuDyn,
Spherocity, SubSample, Performance, BasicEventGen, Glauber) are always built. Pythia8, Herwig and FastJet are found
from `DATUM_PYTHIA8_DIR`, `DATUM_HERWIG_DIR`, `DATUM_FASTJET_DIR` (or `CAP_...`) if set, else from a copy in
`<your_datum_directory>` (`pythia8*`, `Herwig*`, `fastjet*`, or under `external/`), the environment, or
`pythia8-config`, `Herwig-config`, `fastjet-config`.

# Run
```
python3 RunDatum.py [workflow.xml]     # compose, check and run workflows
bin/DatumGui [file.cap ...]            # histograms, plots and figures
```

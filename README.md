# CAP
 Correlation Analysis Package

# Prerequisites
- Valid `PYTHIA8` installation
- Valid `FastJet` installation
- Valid `root` installation built with `PYTHIA8` support
- Environment variable for `PYTHIA8` location
 ```
 export PYTHIA8=<your_pythia8_directory>
 ```
- Environment variable for `FastJet` location
 ```
 export FASTJET=<your_fastjet_directory>
 ```
- `root` environment activated with
 ```
 source <your_root_install_path>/bin/thisroot.sh
 ```
- massive storage path (for massive storage of `root` files)
 ```
 export CAP_LARGE_DATA=<your_massive_storage_path>
 ```

# Build CAP
With the build GUI (Python 3 with tkinter):
```
cd <your_cap_directory>
python3 BuildCAP.py
```
Choose the optional modules (Therminator3, Latex, Jets, Epos, PHSD) and the event generators (Pythia8, Herwig), enter
a build directory name (it is created in `<your_cap_directory>`; several build directories can coexist) and click
Build. The output of cmake and make is shown in the lower half of the window and can be saved to a file.

From a shell:
```
cd <your_cap_directory>
source SetupCAP.sh
mkdir build
cd build
cmake --install-prefix <your_cap_install_directory> ../src
make install
```
The core modules (Base, Math, Helpers, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, PtPt, NuDyn,
Spherocity, SubSample, Performance, BasicEventGen, Glauber) are always built. Pythia8, Herwig and FastJet are found
from `CAP_PYTHIA8_DIR`, `CAP_HERWIG_DIR`, `CAP_FASTJET_DIR` if set, else from a copy in `<your_cap_directory>`
(`pythia8*`, `Herwig*`, `fastjet*`, or under `external/`), the environment, or `pythia8-config`, `Herwig-config`,
`fastjet-config`.

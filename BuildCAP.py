#!/usr/bin/env python3
"""BuildCAP.py -- former name of BuildDatum.py (CAP is now Datum): runs BuildDatum.py, and, imported, is the module
BuildDatum."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import BuildDatum                                # noqa: E402

if __name__ == "__main__":
    sys.exit(BuildDatum.main())
sys.modules[__name__] = BuildDatum

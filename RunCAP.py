#!/usr/bin/env python3
"""RunCAP.py -- former name of RunDatum.py (CAP is now Datum): runs RunDatum.py, and, imported, is the module RunDatum."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import RunDatum                                  # noqa: E402

if __name__ == "__main__":
    sys.exit(RunDatum.main())
sys.modules[__name__] = RunDatum

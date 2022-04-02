import unittest
import subprocess
import os

from pyskat_cpp import run_all_tests

class MainTest(unittest.TestCase):
    ret = run_all_tests()
    if not ret == 0:
        raise RuntimeError("Some Tests failed (see above).")


if __name__ == '__main__':
    unittest.main()
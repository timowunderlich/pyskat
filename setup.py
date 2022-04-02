from setuptools import setup
from glob import glob
from pybind11.setup_helpers import Pybind11Extension
import platform

if platform.system() == "Windows":
    boost_flag = []
else:
    boost_flag = ["-DBOOST_ALL_DYN_LINK"]

ext = Pybind11Extension("pyskat_cpp", 
    sorted(glob("src/*.cpp")),
    libraries=["gtest", "boost_thread", "boost_log", "boost_system"],
    extra_compile_args=boost_flag,
    test_suite='tests',
    cxx_std=14)

setup(ext_modules=[ext])
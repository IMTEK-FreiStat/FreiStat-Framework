"""
Setup file used for the pip installment of the FreiStat library package.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
from setuptools import setup, find_packages
import Python.FreiStat as FS

# Open readme file
with open("README.md", "r") as readme_file:
    readme = readme_file.read()

# Intialize variables
requirements : list = ['matplotlib==3.3.4',
                       'numpy==1.20.1',
                       'pyserial==3.5']

setup(
    name= FS.__name__,
    version= FS.__version__,
    author= FS.__author__,
    description= FS.__description__,
    long_description=readme,
    long_description_content_type="text/markdown",
    url="https://github.com/IMTEK-FreiStat/FreiStat-Framework",
    package_dir={"": "Python"},
    packages=find_packages(where="Python"),
    install_requires=requirements,
    classifiers=[
        "Programming Language :: Python :: 3.9.1",
    ],
)
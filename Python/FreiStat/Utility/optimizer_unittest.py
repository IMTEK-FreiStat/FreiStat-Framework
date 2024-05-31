"""
Module implementing unittests for the optimizer module.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging
import unittest

# Import internal dependencies
from .optimizer import Optimizer
from ..Data_storage.constants import *


class Optimizer_UnitTest(unittest.TestCase):
    """
    Description
    -----------
    Class which handles all unittests in regard to the class Optimizer.

    """

    def test_check_Optimizer(self) -> None:
        """
        Description
        -----------
        Method for testing the optimizer in total

        """
        # Test string
        strMethod: str = CV
        listExperimentParameters: list = []

        # Create a test instance of the optimzier
        _Optimizer = Optimizer()
        results = _Optimizer.start(strMethod, listExperimentParameters)
        print("X")
        print(results)

        # Print results
        """
        self.assertTrue(results[1] == False, "Error in _check_Bool occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Bool " 
                         + "occured, string not completly tested")
        """


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()

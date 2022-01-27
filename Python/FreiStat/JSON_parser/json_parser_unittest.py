"""
Module implementing different unittests for the JSON_parser module.

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
from ..Data_storage.data_software_storage import DataSoftwareStorage
from .json_parser import JSON_Parser

class JSON_Parser_UnitTest(unittest.TestCase):
    """
    Description
    -----------
    Class which handles all unittests in regard to the class JSON_Parser.

    """
    def test_check_Bool(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Bool method of the JSON parser.

        """
        # Test string
        strTest : str = "null"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []

        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Bool(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Bool occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Bool " 
                         + "occured, string not completly tested")

    def test_check_Digit(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Digit method of the JSON parser.

        """
        # Test string
        strTest : str = "18"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []

        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Digit(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Digit occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Bool " 
                         + "occured, string not completly tested")

    def test_check_Number(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Number method of the JSON parser.

        """
        # Test string
        strTest : str = "123"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []
        
        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Number(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Number occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Bool "
                         + "occured, string not completly tested")

    def test_check_String(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_String method of the JSON parser.

        """
        # Test string
        strTest : str = "\"CV\""
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []
        
        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_String(listTest,strTest,iTestLen,0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_String occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_String "
                         + "occured, string not completly tested")

    def test_check_Object(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Object method of the JSON parser.

        """
        # Test string
        strTest : str = \
            "{\"R\":1,\"M\":{\"D\":136,\"V\":-59.902320,\"C\":-0.5461352}}"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []
        
        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Object(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Object occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Object "
                         + "occured, string not completly tested")
    
    def test_check_Value(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Value method of the JSON parser.

        """
        # Test string
        strTest : str = "5"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest : list = []
        
        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Value(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Value occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Value "
                         + "occured, string not completly tested")

    def test_check_Array(self) -> None:
        """
        Description
        -----------
        Method for testing the _check_Array method of the JSON parser.

        """
        # Test string
        strTest : str = \
            "[\"1254\",123,{\"R\":50,\"M\":{\"D\":2,\"V\":1.1,\"C\":-3.57}}]"
        iTestLen : int = strTest.__len__()

        # Test list
        listTest = []
        
        # Create a test instance of the JSON_Parser
        _JSON_Parser = JSON_Parser(DataSoftwareStorage())

        # Call method which should be tested
        results = _JSON_Parser._check_Array(listTest, strTest, iTestLen, 0)

        # Print results
        self.assertTrue(results[1] == False, "Error in _check_Array occured"
                        + " - Error position: " + str(results[0]) 
                        + " - Character: " + strTest[results[0]])
        self.assertEqual(iTestLen, results[0] + 1, "Error in _check_Array "
                         + "occured, string not completly tested")

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    unittest.main()
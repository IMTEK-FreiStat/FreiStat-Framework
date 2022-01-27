"""
Interface implementing the parent class for the setup behavior of the
electrochemical methods.

From this class all other setup classes inherit. This is done to enable the 
`electrochemical_method` class to create dynamic behavior depending on the
chosen methods.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies

# Import internal dependencies
from ...Data_storage.constants import *
from ...Data_storage.data_software_storage import DataSoftwareStorage
from ...JSON_parser.json_telegram_generator import JSON_Telegram_Generator

class SetupBehavior():
    """
    Description
    -----------
    Interface parent class implementing the setup behavior of the
    electrochemical methods.
    
    """

    def __init__(self, dataSoftwareStorage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of the class SetupBehavior.

        Parameters
        ----------
        `dataSoftwareStorage`: DataSoftwareStorage
            Reference to DataSoftwareStorage object

        """
        # Set reference to object storing all software related parameters
        self._dataSoftwareStorage = dataSoftwareStorage
    
        # Set reference to data handling object
        self._dataHandling = self._dataSoftwareStorage.getDataHandling()

        # Set reference to JSON parser object
        self._jsonParser = self._dataSoftwareStorage.getJSON_Parser()        

        # Check if JSON telegram generator object is already created
        if self._dataSoftwareStorage.getJSON_TelegramGenerator() is None: 
            # Object is not created yet
            self._jsonTelegramGenerator = JSON_Telegram_Generator(
                self._dataSoftwareStorage)
        else:
            # Object is already created, get reference
            self._jsonTelegramGenerator = self._dataSoftwareStorage.\
                getJSON_TelegramGenerator() 

        # Create an data object for this electrochemical method
        self._dataHandling.create_DataObject()

    def setup(self) -> int:
        """
        Description
        -----------
        Abstract method for running setup for the different methods.

        Return
        ------
        `Errorcode` : int
            Abstract method returns 11999 (Execute inheritance error)

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        11999       :   Execute inheritance error

        """
        # Return general default error, since this method should never be used
        return EC_SETUP + EC_SE_INHERIT_ERROR
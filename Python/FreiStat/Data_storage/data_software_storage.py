"""
Module implementing a class for handling the data which is required for the 
FreiStat software to operate.

This class holds reference to all other main objects, which are required by the 
FreiStat Python, in order to operate.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies

# Import internal dependencies
from .constants import *


# Forward declaration
def Communication():
    pass


def JSON_Parser():
    pass


def JSON_Telegram_Generator():
    pass


def DataHandling():
    pass


class DataSoftwareStorage:
    """
    Description
    -----------
    Class which stores all references and general variables for the Python
    library of the FreiStat software.

    """

    def __init__(self) -> None:
        """
        Description
        -----------
        Constructor of class DataSoftwareStorage

        """
        # Initalize class variables
        self._dataHandling = None
        self._jsonParser = None
        self._jsonTelegramGenerator = None

        self._bLowPerformanceMode: bool = False

        self._systemStatus: int = FREISTAT_BOOTUP

    # Setter methods
    def set_SystemStatus(self, iSystemStatus: int) -> None:
        """
        Description
        -----------
        Save system status. The following status modes are defined*:

        FREISTAT_BOOTUP         = 0             # FreiStat is in boot up process
        FREISTAT_WAITING        = 1             # FreiStat in waiting state
        FREISTAT_EXP_STARTED    = 2             # Start experiment
        FREISTAT_EXP_RUNNING    = 3             # Experiment running
        FREISTAT_EXP_COMPLETED  = 4             # Experiment completed
        FREISTAT_EXP_CANCELED   = 5             # Experiment canceled

        *Taken from constants.py

        Parameters
        ----------
        `iSystemStatus` : int
            Status of the system encoded in an integer value.

        """
        self._systemStatus = iSystemStatus

    def set_LowPerformanceMode(self, bLowPerformanceMode: bool) -> None:
        """
        Description
        -----------
        Save low performanance mode flag

        Parameters
        ----------
        `bLowPerformanceMode ` bool
            Flag indicating if the low performance mode is enabled or not.

        """
        self._bLowPerformanceMode = bLowPerformanceMode

    def setJSON_Parser(self, jsonParser: JSON_Parser) -> None:
        """
        Description
        -----------
        Set reference of the JSON parser object.

        Parameters
        ----------
        `jsonParser` : JSON_Parser
            Reference to the JSON parser object

        """
        self._jsonParser = jsonParser

    def setCommunication(self, communication: Communication) -> None:
        """
        Description
        -----------
        Set reference of the serial communication object.

        Parameters
        ----------
        `communication` : Communication
            Reference to the serial communication object

        """
        self._communication = communication

    def setDataHandling(self, dataHandling: DataHandling) -> None:
        """
        Description
        -----------
        Set reference of the data handling object.

        Parameters
        ----------
        `dataHandling` : DataHandling
            Reference to the data handling object

        """
        self._dataHandling = dataHandling

    def setJSON_TelegramGenerator(
        self, jsonTelegramGenerator: JSON_Telegram_Generator
    ) -> None:
        """
        Description
        -----------
        Set reference of the JSON telegram generator object.

        Parameters
        ----------
        `jsonTelegramGenerator` : JSON_Telegram_Generator
            Reference to the JSON telegram generator object

        """
        self._jsonTelegramGenerator = jsonTelegramGenerator

    # Getter methods
    def get_SystemStatus(self) -> int:
        """
        Description
        -----------
        Get system status. See setter method or constant.py for possible values.

        Return
        ------
        `iSystemStatus` : int
            Status of the system encoded in an integer value

        """
        return self._systemStatus

    def get_LowPerformanceMode(self) -> bool:
        """
        Description
        -----------
        Get low performanance mode flag

        Return
        ------
        `bLowPerformanceMode ` bool
            Flag indicating if the low performance mode is enabled or not.

        """
        return self._bLowPerformanceMode

    def getJSON_Parser(self) -> JSON_Parser:
        """
        Description
        -----------
        Get reference of the JSON parser object.

        Return
        ------
        `jsonParser` : JSON_Parser
            Reference to the JSON parser object

        """
        return self._jsonParser

    def getCommunication(self) -> Communication:
        """
        Description
        -----------
        Get reference of the serial Communication object.

        Return
        ------
        `communication` : Communication
            Reference to the serial Communication object

        """
        return self._communication

    def getDataHandling(self) -> DataHandling:
        """
        Description
        -----------
        Get reference of the data handling object.

        Return
        ----------
        `dataHandling` : DataHandling
            Reference to the data handling object

        """
        return self._dataHandling

    def getJSON_TelegramGenerator(self) -> JSON_Telegram_Generator:
        """
        Description
        -----------
        Get reference of the JSON telegram generator object.

        Return
        ------
        `jsonTelegramGenerator`: JSON_Telegram_Generator
            Reference to the JSON telegram generator object

        """
        return self._jsonTelegramGenerator

"""
Interface implementing the parent class for the executing behavior of the
electrochemical methods.

From this class all other execute classes inherit. This is done to enable the 
`electrochemical_method` class to create dynamic behavior depending on the
chosen methods

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
from multiprocessing.queues import Queue

# Import internal dependencies
from ...Data_storage.constants import *
from ...Data_storage.data_software_storage import DataSoftwareStorage


class ExecuteBehavior:
    """
    Description
    -----------
    Interface parent class implementing the execute behavior of the
    electrochemical methods.

    """

    def __init__(self, dataSoftwareStorage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of the class ExecuteBehavior

        Parameter
        ---------
        `dataSoftwareStorage`: DataSoftwareStorage
            Reference to DataSoftwareStorage object

        """
        # Set reference to object storing all software related parameters
        self._dataSoftwareStorage = dataSoftwareStorage

        # Get reference to JSON telegram generator object
        self._jsonTelegramGenerator = (
            self._dataSoftwareStorage.getJSON_TelegramGenerator()
        )

        # Get reference to JSON parser object
        self._jsonParser = self._dataSoftwareStorage.getJSON_Parser()

        # Get referene to data handling object
        self._dataHandling = self._dataSoftwareStorage.getDataHandling()

        # Get flag of the low performance mode
        self._lowPerformaneMode = self._dataSoftwareStorage.get_LowPerformanceMode()

    def execute(
        self,
        dataQueue: Queue,
        iTelegrams: int = 3,
        bEnableReading: bool = True,
        bPorgressiveMesurement: bool = False,
    ) -> int:
        """
        Description
        -----------
        Abstract method for running execute for the different methods.

        Parameters
        ----------
        `dataQueue` : Queue
            Data queue which is used as a pipe between processes

        `iTelegrams` : int
            Number of telegrams which should be send by the ec-method

        `bEnableReading` : bool
            Flag indicating if incoming data should be read or not

        `bPorgressiveMesurement` : bool
            Flag indicating if measurement should be displayed continously or
            if each cycle should start at time = 0

        Return
        ------
        `Errorcode` : int
            Abstract method returns 12999 (Execute inheritance error)

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        12999       :   Execute inheritance error

        """
        # Execute inheritance error, since this method should never be used
        return EC_EXECUTE + EC_EX_INHERIT_ERROR

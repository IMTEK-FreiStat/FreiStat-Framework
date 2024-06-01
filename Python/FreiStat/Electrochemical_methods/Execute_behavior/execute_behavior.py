"""
Interface implementing the parent class for the executing behavior of the
electrochemical methods.

From this class all other execute classes inherit. This is done to enable the 
`electrochemical_method` class to create dynamic behavior depending on the
chosen methods

"""

from __future__ import annotations

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ...serial_communication.serial_communication import Communication

from multiprocessing.queues import Queue
from multiprocessing.synchronize import Event

from ...data_storage.constants import EC_EX_INHERIT_ERROR, EC_EXECUTE
from ...data_storage.data_software_storage import DataSoftwareStorage


class ExecuteBehavior:
    """
    Description
    -----------
    Interface parent class implementing the execute behavior of the
    electrochemical methods.

    """

    def __init__(self, data_software_storage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of the class ExecuteBehavior

        Parameter
        ---------
        `dataSoftwareStorage`: DataSoftwareStorage
            Reference to DataSoftwareStorage object

        """
        self._progressive_measurement: bool
        self._reference_time: float
        self._event: Event
        self._serial_connection: Communication
        self._communication_mode: int

        # Set reference to object storing all software related parameters
        self._data_software_storage = data_software_storage

        # Get reference to JSON telegram generator object
        self._json_telegram_generator = (
            self._data_software_storage.get_json_telegram_generator()
        )

        # Get reference to JSON parser object
        self._json_parser = self._data_software_storage.get_json_parser()

        # Get referene to data handling object
        self._data_handling = self._data_software_storage.get_data_handling()

        # Get flag of the low performance mode
        self._low_performane_mode = (
            self._data_software_storage.get_low_performance_mode()
        )

    def execute(
        self,
        data_queue: Queue,
        telegrams: int = 3,
        enable_reading: bool = True,
        progressive_measurement: bool = False,
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
        # pylint: disable=unused-argument
        # Execute inheritance error, since this method should never be used
        return EC_EXECUTE + EC_EX_INHERIT_ERROR

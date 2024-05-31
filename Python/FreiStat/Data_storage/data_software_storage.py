"""
Module implementing a class for handling the data which is required for the 
FreiStat software to operate.

This class holds reference to all other main objects, which are required by the 
FreiStat Python, in order to operate.

"""

from __future__ import annotations

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"


from typing import TYPE_CHECKING, Optional

from .constants import FREISTAT_BOOTUP

if TYPE_CHECKING:
    from FreiStat.data_storage.data_handling import DataHandling
    from FreiStat.json_parser.json_parser import JsonParser
    from FreiStat.json_parser.json_telegram_generator import JSON_Telegram_Generator
    from FreiStat.serial_communication.serial_communication import Communication


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
        self._data_handling: Optional[DataHandling] = None
        self._json_parser: Optional[JsonParser] = None
        self._json_telegram_generator: Optional[JSON_Telegram_Generator] = None
        self._communication: Optional[Communication] = None

        self._low_performance_mode = False

        self._system_status: int = FREISTAT_BOOTUP

    # Setter methods
    def set_system_status(self, system_status: int) -> None:
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
        self._system_status = system_status

    def set_low_performance_mode(self, low_performance_mode: bool) -> None:
        """
        Description
        -----------
        Save low performanance mode flag

        Parameters
        ----------
        `bLowPerformanceMode ` bool
            Flag indicating if the low performance mode is enabled or not.

        """
        self._low_performance_mode = low_performance_mode

    def set_json_parser(self, json_parser: JsonParser) -> None:
        """
        Description
        -----------
        Set reference of the JSON parser object.

        Parameters
        ----------
        `jsonParser` : JSON_Parser
            Reference to the JSON parser object

        """
        self._json_parser = json_parser

    def set_communication(self, communication: Communication) -> None:
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

    def set_data_handling(self, data_handling: DataHandling) -> None:
        """
        Description
        -----------
        Set reference of the data handling object.

        Parameters
        ----------
        `dataHandling` : DataHandling
            Reference to the data handling object

        """
        self._data_handling = data_handling

    def set_json_telegram_generator(
        self, json_telegram_generator: JSON_Telegram_Generator
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
        self._json_telegram_generator = json_telegram_generator

    # Getter methods
    def get_system_status(self) -> int:
        """
        Description
        -----------
        Get system status. See setter method or constant.py for possible values.

        Return
        ------
        `iSystemStatus` : int
            Status of the system encoded in an integer value

        """
        return self._system_status

    def get_low_performance_mode(self) -> bool:
        """
        Description
        -----------
        Get low performanance mode flag

        Return
        ------
        `bLowPerformanceMode ` bool
            Flag indicating if the low performance mode is enabled or not.

        """
        return self._low_performance_mode

    def get_json_parser(self) -> Optional[JsonParser]:
        """
        Description
        -----------
        Get reference of the JSON parser object.

        Return
        ------
        `jsonParser` : JSON_Parser
            Reference to the JSON parser object

        """
        return self._json_parser

    def get_communication(self) -> Optional[Communication]:
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

    def get_data_handling(self) -> Optional[DataHandling]:
        """
        Description
        -----------
        Get reference of the data handling object.

        Return
        ----------
        `dataHandling` : DataHandling
            Reference to the data handling object

        """
        return self._data_handling

    def get_json_telegram_generator(self) -> Optional[JSON_Telegram_Generator]:
        """
        Description
        -----------
        Get reference of the JSON telegram generator object.

        Return
        ------
        `jsonTelegramGenerator`: JSON_Telegram_Generator
            Reference to the JSON telegram generator object

        """
        return self._json_telegram_generator

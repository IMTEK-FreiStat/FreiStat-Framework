"""
Module for generating telegrams in JSON format, which are transmitted from the
Pyhton library to the microcontroller.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
from typing import Union

# Import internal dependencies
from ..Data_storage.constants import *
from ..Data_storage.data_software_storage import DataSoftwareStorage

class JSON_Telegram_Generator():
    """
    Description
    -----------
    Class for creating JSON telegrams which are used as communication with the
    FreiStat software on the microcontroller.

    """

    def __init__(self, dataSoftwareStorage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of class JSON_Telegram_Generator

        Parameter
        ---------
        `dataSoftwareStorage`: DataSoftwareStorage
            Reference to DataSoftwareStorage object
        """

        # Save data software storage object and set own reference
        self._dataSoftwareStorage = dataSoftwareStorage
        self._dataSoftwareStorage.setJSON_TelegramGenerator(self)  

        # Get reference to data handling object
        self._dataHandling = self._dataSoftwareStorage.getDataHandling()

    def generateCommandTelegram(self, iCodeID : int, iCodeSubID: int) \
                                -> Union[str, int]:
        """
        Description
        -----------
        Translates command ID into JSON telegram in string format.

        Parameters
        ----------
        `iCodeID` : int
            Integer with the ID of the command which should be transmitted

        `iCodeSubID` : int
            Integer with the ID of the subcommand which should be transmitted

        Return
        ------
        `iErrorCode` : int
            Error code indicating describing conversion result

        `strJSON` : string
            String containing JSON telegram

        Description
        -----------
        The following table shows all defined Command code IDs

        Command ID : Command SubID : Description 
        1 : 1 : Experiment type                             \n
        2 : 1 : Experiment parameters                       \n
        3 : X : Experiment control                          \n
          : 1 : Start                                       \n
          : 2 : Stop                                        \n
        4 : X : Sequence control                            \n
          : 1 : Enable sequence                             \n
          : 2 : Disable sequence 

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   Conversion successful
        13001       :   iCodeID is not defined

        """
        # Initalize variables
        strJSON : str = "\"" + COMMAND_TELEGRAM + "\""
        iErrorCode : int = EC_NO_ERROR

        # Check ID
        if (iCodeID in (COMMAND_EXT, COMMAND_EXP, COMMAND_EXC, COMMAND_EXS)):
            pass
        else:
            iErrorCode = EC_JSON_PARSER + EC_JP_CODEID_UNDEF
            strJSON = "iCodeID is not defined"
            return iErrorCode, strJSON

        # ' {"C": iCodeID ,'
        strJSON = "{" + strJSON + ":" + str(iCodeID) + ","

        # Generate command telegram for transmitting experiment type
        if (iCodeID == COMMAND_EXT and iCodeSubID == 1):
            # Get electrochemical method
            strTemp : str = self._dataHandling.get_ExperimentType()

            # ' {"C":1,"ExT": "EC Method" } '
            strJSON =  strJSON + "\"" + COMMAND_EXT_STR + "\":\"" + strTemp + "\"}"

        # Generate command telegram for transmitting experiment parameters
        elif(iCodeID == COMMAND_EXP and iCodeSubID == 1):
            # Get electrochemical experiment parameters
            listTemp : list = self._dataHandling.get_ExperimentParameters()
            
            # Initalize variables
            strTemp : str = ""

            # Convert experiment parameters from list format into JSON
            for iEntry in range (len(listTemp)):
                # '"X"'
                strTemp = strTemp +"\"" + str(listTemp[iEntry][0]) + "\":"
                strTemp = strTemp + str(listTemp[iEntry][1]) 
                if (iEntry == len(listTemp) - 1):
                    break
                strTemp = strTemp + ","

            # ' {"C":2,"ExP": {           } '
            strJSON =  strJSON + "\"" + COMMAND_EXP_STR + "\":{" + strTemp + "}}"

        # Generate command telegram for experiment control
        elif (iCodeID == COMMAND_EXC):
            if (iCodeSubID == FREISTAT_START_I):
                # ' {"C":3,"ExC": "START" } '
                strJSON =  strJSON + "\"" + COMMAND_EXC_STR + "\":\"" + \
                           FREISTAT_START_STR + "\"}"

            if (iCodeSubID == FREISTAT_STOP_I):
                # ' {"C":3,"ExC": "STOP" } '
                strJSON =  strJSON + "\"" + COMMAND_EXC_STR + "\":\"" + \
                           FREISTAT_STOP_STR + "\"}"   

        # Generate command telegram for sequence control
        elif (iCodeID == COMMAND_EXS):
            if (iCodeSubID == SEQUENCE_ENABLE_I):
                # ' {"C":4,"ExS": "SE"} '
                strJSON =  strJSON + "\"" + COMMAND_EXS_STR + "\": \"" + \
                           SEQUENCE_ENABLE_STR + "\"}"
            if (iCodeSubID == SEQUENCE_DISABLE_I):
                # ' {"C":4,"ExS": "SD"} '
                strJSON =  strJSON + "\"" + COMMAND_EXS_STR + "\": \"" + \
                           SEQUENCE_DISABLE_STR + "\"}"
        
        # Print send telegram
        print(strJSON)

        # Check if string is 128 byte long
        if (len(strJSON) == 128):
            # Add whitespace to prevent transmission bug
            strJSON += " "

        # Check if string is longer than 256 byte

        # Return union
        return iErrorCode, strJSON

    def generateAcknowledgeTelegram(self, iCodeID : int, iCodeSubID: int) \
                                -> Union[str, int]:
        """
        Description
        -----------
        Method for sending acknowledge telegrams from the Python library to the 
        microcontroller.

        Parameters
        ----------
        `iCodeID` : int
            Integer with the ID of the command which should be transmitted

        `iCodeSubID` : int
            Integer with the ID of the subcommand which should be transmitted
            
        Return
        ------
        `iErrorCode` : int
            Error code indicating describing conversion result

        `strJSON` : string
            String containing JSON telegram

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode :Description             \n
        0 : Conversion successful           \n
        1 : iCodeID is not defined          \n

        """
        # TODO not required until now (25.08.2021)
        pass
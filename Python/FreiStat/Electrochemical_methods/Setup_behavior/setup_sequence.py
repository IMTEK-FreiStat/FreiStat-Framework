"""
Module implementing the behavior of the setup function for running a sequence
of electrochemical methods.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies

# Import internal dependencies
from ...data_storage.constants import *
from .setup_behavior import SetupBehavior


class SetupSequence(SetupBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the setup function when
    running a sequence of electrochemical methods.

    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for conducting a sequence of electrochemical methods.

        List of required parameters
        ----------
        Cycle : int
            Amount of cycles the CV should run

        Parameters
        ----------
        `listExperimentParameters`: list
            List which needs to include all required parameters for the
            sequence in the above stated order.

        Return
        ------
        `iErrorCode` : int
            Error code describing conversion result

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   General no error
        12001       :   Mismatch between send command ID and received acknowledge ID

        """
        # Initialize class variables
        self._referenceTime: float = -1

        # Initialize variables
        bErrorflag: bool = False

        iCurrenPosition: int = 0

        strReadTelegram: str = ""

        listReadData: list = []
        listCommandIDs: list = [COMMAND_EXS, COMMAND_EXP]
        listCommandSubIDs: list = [1, 1]

        # Save reference to serial connection object
        self._serialConnection = self._dataSoftwareStorage.getCommunication()
        self._communicationMode = self._serialConnection.get_CommunicationMode()

        # Safe experiment parameters
        self._dataHandling.save_ExperimentType(SEQUENCE)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Execute the following commands after another
        # 1: Transmit experiment type
        # 2: Transmit experiment parameters
        for iIndex in range(2):
            # Write command telegram with command type
            self._serialConnection.write_Data(
                self._jsonTelegramGenerator.generateCommandTelegram(
                    listCommandIDs[iIndex], listCommandSubIDs[iIndex]
                )[1]
            )

            # Wait as long as the input buffer is empty
            while self._communicationMode == FREISTAT_SERIAL:
                if self._serialConnection.get_SerialConnection().in_waiting > 0:
                    break
                pass

            # Read acknowledge telegram
            strReadTelegram = self._serialConnection.read_Data("JSON").decode("utf-8")

            # Print received acknowledge telegram
            print(strReadTelegram)

            # Parse read telegram
            iCurrenPosition, bErrorflag, listReadData = (
                self._jsonParser.parse_JSON_string(listReadData, strReadTelegram)
            )

            # Compare code to previously send code
            if int(listReadData[0][1]) != listCommandIDs[iIndex]:
                return EC_EXECUTE + EC_EX_C_A_MISMATCH

        # No error occured
        return EC_NO_ERROR

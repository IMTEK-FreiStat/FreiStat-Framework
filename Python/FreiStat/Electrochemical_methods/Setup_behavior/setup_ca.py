"""
Module implementing the behavior of the setup function for running
chronoamperometry.

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


class SetupCA(SetupBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the setup function when
    running chronoamperometry.

    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for chronoamperometry.

        List of required parameters
        ----------
        Potential_Steps : list
            List containing all potential steps in mV, which get executed after
            another.

        Pulse_Lengths : list
            List containing the corresponding pulse lengths of every potential
            step in ms.

        Sampling_Rate : float
            Sampling rate defining the distance between measurement results
            in ms.

        Cycle : int
            Amount of cycles the CV should run.

        LPTIA_Resistor : int
            Size of the low power transimpendance amplifier resistor which is
            used to measure the current. Range 200 - 512000 ohm.

        MainsFilter : bool
            Enable/ Disable 50 Hz/ 60 Hz mains filter.
            If enabled `Sinc2_Oversampling` must be defiend (Default: 667)

        Sinc2_Oversampling : int
            Oversampling rate of the Sinc 2 filter
            Defiend OSR rates: [22, 44, 89, 178, 267, 533,
            640, 667, 800, 889, 1067, 1333]

        Sinc3_Oversampling : int
            Oversampling rate of the Sinc 3 filter
            Defiend OSR rates: [0 (Disabled), 2, 4, 5]
            Oversampling rate of 5 is not recommanded

        Parameters
        ----------
        `listExperimentParameters`: list
            List which needs to include all required parameters for CA in the
            above stated order

        Return
        ------
        `iErrorCode` : int
            Error code encoded as integer

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   No error occured, setup successful
        11001       :   List contains wrong amount of parameters
        11002       :   Scan range exceeded by given parameters
        11003       :   Amount of potential steps don't match amount of pulse lengths
        11004       :   To many parameters in list
        11100       :   Potential steps named wrong or not found as 1 entry
        11101       :   Pulse lengths named wrong or not found as 2 entry
        11102       :   Sampling rate named wrong or not found as 3 entry
        11103       :   Cycle named wrong or not found as 4 enty
        11104       :   LPTIA Rtia size named wrong or not found as 5 entry
        11105       :   Mains filter named wrong or not found as 6 entry
        11106       :   Sinc2 oversampling rate named wrong or not found as 7 entry
        11107       :   Sinc3 oversampling rate named wrong or not found as 8 entry
        11200       :   Potential steps out of bounds
        11201       :   Pulse lengths out of bounds
        11202       :   Sampling rate out of bounds
        11203       :   Cycle out of bounds
        11204       :   LPTIA Rtia size out of bounds
        11205       :   Mains filter out of bounds
        11206       :   Sinc2 oversampling rate out of bounds
        11207       :   Sinc3 oversampling rate out of bounds

        """
        # Initialize variables
        iErrorCode: int = EC_NO_ERROR

        fMaxPotential: float = 0
        fMinPotential: float = 0

        # Check if list of Parameters is in correct format
        if len(listExperimentParameters) != CA_NUM_PARAMETER:
            # Not enough Parameters
            iErrorCode = EC_SETUP + EC_SE_AMOUNT_PARAMETER
            return iErrorCode

        # Search for the highest defined potential step
        listPotentialSteps: list = listExperimentParameters[0][1]

        for iSteps in range(len(listPotentialSteps)):
            if listPotentialSteps[iSteps] > fMaxPotential:
                fMaxPotential = listPotentialSteps[iSteps]

            if listPotentialSteps[iSteps] < fMinPotential:
                fMinPotential = listPotentialSteps[iSteps]

        # Calculate required voltage range
        fRange = fMaxPotential - fMinPotential

        # Check if voltage range is exeeds possible range
        if fRange >= VOLTAGE_RANGE:
            iErrorCode = EC_SETUP + EC_SE_SCAN_RANGE_ERROR
            return iErrorCode

        # Check if list contains to many entries
        if (
            len(listExperimentParameters[0][1]) > EXPERIMENT_BUFFER
            or len(listExperimentParameters[1][1]) > EXPERIMENT_BUFFER
        ):
            iErrorCode = EC_SETUP + EC_SE_LIST_OVERFLOW
            return iErrorCode

        # Check if lists contain different amount of entries
        if len(listExperimentParameters[0][1]) != len(listExperimentParameters[1][1]):
            iErrorCode = EC_SETUP + EC_SE_LIST_MISMATCH
            return iErrorCode

        # Initialize reference list
        listReferenceList = [
            [POTENTIAL_STEPS, fMinPotential, fMaxPotential],
            [PULSE_LENGTH, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH],
            [SAMPLING_RATE, MIN_SAMPLING_RATE, MAX_SAMPLING_RATE],
            [CYCLE, MIN_CYCLE, MAX_CYCLE],
            [LPTIA_RTIA_SIZE, LPTIARTIA_OPEN, LPTIARTIA_512K],
            [MAINS_FILTER, 0, 1],
            [SINC2_OVERSAMPLING, ADCSINC2OSR_DISABLED, ADCSINC2OSR_1333],
            [SINC3_OVERSAMPLING, ADCSINC3OSR_DISABLED, ADCSINC3OSR_2],
        ]

        # Check individual entries
        for iEntry in range(len(listExperimentParameters)):
            # Check if parameter is named correct and at the right position
            if listExperimentParameters[iEntry][0] != listReferenceList[iEntry][0]:
                iErrorCode = EC_SETUP + EC_SE_PARAM_NOT_FOUND + iEntry
                return iErrorCode

            if iEntry == 0 or iEntry == 1:
                # Check if the parameters in the lists are in the boundaries
                for iStep in range(len(listExperimentParameters[iEntry][1])):
                    if (
                        listExperimentParameters[iEntry][1][iStep]
                        < listReferenceList[iEntry][1]
                        or listExperimentParameters[iEntry][1][iStep]
                        > listReferenceList[iEntry][2]
                    ):
                        iErrorCode = EC_SETUP + EC_SE_PARAM_OUT_OF_BOUND + iEntry
                        return iErrorCode
            else:
                # Check if the parameters are in the boundaries
                if (
                    listExperimentParameters[iEntry][1] < listReferenceList[iEntry][1]
                    or listExperimentParameters[iEntry][1]
                    > listReferenceList[iEntry][2]
                ):
                    iErrorCode = EC_SETUP + EC_SE_PARAM_OUT_OF_BOUND + iEntry
                    return iErrorCode

        # Safe experiment parameters
        self._dataHandling.save_ExperimentType(CA)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Setup CSV export for the chosen experiment
        self._dataSoftwareStorage._dataHandling.setup_ExportFiles()

        # Return error code
        return iErrorCode

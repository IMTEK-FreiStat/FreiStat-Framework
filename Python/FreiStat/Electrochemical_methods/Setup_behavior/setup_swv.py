"""
Module implementing the behavior of the setup function for running square wave
voltammetery.

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
from .setup_behavior import SetupBehavior

class SetupSWV(SetupBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the setup function when
    running square wave voltammetry.
    
    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for square wave voltammetry
        
        List of required parameters
        ----------
        StartVoltage : float
            Potential at which FreiStat should start to ramp up

        StopVoltage : float
            Potential at which FreiStat should stop in V

        DeltaV_Staircase : float
            Increase of the base potential of the underlying staircase function
            with every cycle in V

        DeltaV_Peak : float
            Delta V in V, which defines the voltage jump applied to the cell.

        Pulse_Lengths : list
            List [Tau', Tau] containing the corresponding pulse lengths of every 
            potential step in s.

        Sampling_Duration : float
            Time in s, which defines how long values should be sampled at the 
            end of each potential step

        Cycle : int                  
            Amount of cycles the CV should run

        FixedWEPotential : int
            Defines if the we electrode potential should be fixed to the middle
            of the dynamic range (1.3 V) or not.

        LPTIA_Resistor : int
            Size of the low power transimpendance amplifier resistor which is
            used to measure the current. Range 200 - 512000 ohm.

        MainsFilter: bool
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
            List which needs to include all required parameters for CV in the
            above stated order.

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
        11100       :   StartVoltage named wrong or not found as 1 entry 
        11101       :   StopVoltage named wrong or not found as 2 entry  
        11102       :   DeltaV_Staircase named wrong or not found as 3 entry  
        11103       :   DeltaV_Peak named wrong or not found as 4 enty 
        11104       :   PulseLength named wrong or not found as 5 entry
        11105       :   Sampling_Duration named wrong or not found as 6 entry  
        11106       :   Cycle named wrong or not found as 7 entry   
        11107       :   LPTIA Rtia size named wrong or not found as 8 entry
        11108       :   FixedWEPotential named wrong or not found as 9 entry
        11109       :   Mains filter named wrong or not found as 10 entry   
        11110       :   Sinc2 oversampling rate named wrong or not found as 11 entry
        11111       :   Sinc3 oversampling rate named wrong or not found as 12 entry
        11200       :   StartVoltage out of bounds
        11201       :   StopVoltage out of bounds    
        11202       :   DeltaV_Staircase out of bounds     
        11203       :   DeltaV_Peak out of bounds     
        11204       :   PulseLength out of bounds
        11205       :   Sampling_Duration out of bounds
        11206       :   Cycle out of bounds
        11207       :   LPTIA Rtia size out of bounds
        11208       :   FixedWEPotential out of bounds
        11209       :   Mains filter out of bounds
        11210       :   Sinc2 oversampling rate out of bounds
        11211       :   Sinc3 oversampling rate out of bounds  

        """
        # Initialize variables
        iErrorCode : int = EC_NO_ERROR

        fMinPulseLength : float = 1e5

        # Determine Min and Max occuring voltage
        fMinVoltage = listExperimentParameters[0][1] - \
                      listExperimentParameters[3][1]
        fMaxVoltage = listExperimentParameters[1][1] + \
                      listExperimentParameters[3][1]

        # Scan range is limited to 1.9 V / 4.4 V (Depending on the mode)
        fRange = fMaxVoltage - fMinVoltage
        
        # Check if potential of the working electrode is fixed or not
        if (FIXED_WE_POTENTIAL == 1):
            fVoltageRange = VOLTAGE_RANGE_FWP
        else:
            fVoltageRange = VOLTAGE_RANGE * 2

        # Search for the shortest pulse lenght
        for iIndex in range(len(listExperimentParameters[4][1])):
            if (listExperimentParameters[4][1][iIndex]  < fMinPulseLength):
                fMinPulseLength = listExperimentParameters[4][1][iIndex]

        # Initialize reference list
        listReferenceList = [
            [START_POTENTIAL, fMaxVoltage - fVoltageRange, fMaxVoltage],
            [STOP_POTENTIAL, fMinVoltage, fMinVoltage + fVoltageRange],
            [DELTA_V_STAIRCASE, 0, fMaxVoltage - fMinVoltage],
            [DELTA_V_PEAK, 0, fVoltageRange - fMaxVoltage],
            [PULSE_LENGTH, MIN_PULSE_LENGTH , MAX_PULSE_LENGTH],
            [SAMPLING_DURATION, MIN_SAMPLING_DURATION, fMinPulseLength],
            [CYCLE, MIN_CYCLE, MAX_CYCLE],
            [LPTIA_RTIA_SIZE, LPTIARTIA_OPEN, LPTIARTIA_512K],
            [FIXED_WE_POTENTIAL, 0, 1],
            [MAINS_FILTER, 0, 1],
            [SINC2_OVERSAMPLING, ADCSINC2OSR_DISABLED, ADCSINC2OSR_1333],
            [SINC3_OVERSAMPLING, ADCSINC3OSR_DISABLED, ADCSINC3OSR_2]
        ]

        # Check if list of Parameters is in correct format
        if (len(listExperimentParameters) != SWV_NUM_PARAMETER):
            # Not enough Parameters
            iErrorCode = EC_SETUP + EC_SE_AMOUNT_PARAMETER
            return iErrorCode

        # Check individual entries
        for iEntry in range (len(listExperimentParameters)):
            # Check if parameter is named correct and at the right position
            if (listExperimentParameters[iEntry][0] != 
                listReferenceList[iEntry][0]):
                iErrorCode = EC_SETUP + EC_SE_PARAM_NOT_FOUND + iEntry
                return iErrorCode

            if (iEntry == 4):
                # Check if the parameters in the lists are in the boundaries
                for iStep in range(len(listExperimentParameters[iEntry][1])):
                    if (listExperimentParameters[iEntry][1][iStep] < 
                        listReferenceList[iEntry][1] or 
                        listExperimentParameters[iEntry][1][iStep] > 
                        listReferenceList[iEntry][2]):
                        iErrorCode = EC_SETUP + EC_SE_PARAM_OUT_OF_BOUND + iEntry
                        return iErrorCode
            else: 
                # Check if the parameters are in the boundaries
                if (listExperimentParameters[iEntry][1] < 
                    listReferenceList[iEntry][1] or 
                    listExperimentParameters[iEntry][1] > 
                    listReferenceList[iEntry][2]):
                    iErrorCode = EC_SETUP + EC_SE_PARAM_OUT_OF_BOUND + iEntry
                    return iErrorCode

        # Check if scan range is not exceeded
        if (fRange >= fVoltageRange):
            iErrorCode = EC_SETUP + EC_SE_SCAN_RANGE_ERROR
            return iErrorCode

        # Safe experiment parameters
        self._dataHandling.save_ExperimentType(SWV)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Setup CSV export for the chosen experiment
        self._dataSoftwareStorage._dataHandling.setup_ExportFiles()

        # Return error code
        return iErrorCode
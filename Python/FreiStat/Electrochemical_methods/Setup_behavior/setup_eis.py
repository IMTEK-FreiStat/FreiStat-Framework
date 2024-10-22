"""
Module implementing the behavior of the setup function for running differential
pulse voltammetery.

"""

__author__ = "Cedic Neumann"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Cedirc Neumann"

__version__ = "1.0.0"
__maintainer__ = "Cedirc Neumann"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies

# Import internal dependencies
from ...Data_storage.constants import *
from .setup_behavior import SetupBehavior

class SetupEIS(SetupBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the setup function when
    running differential pulse voltammetry.
    
    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for differential pulse voltammetry
        
        List of required parameters
        ----------
        startFrequency : float
            Frequency at which FreiStat should start in Hz.

        stopFrequency : float
            Frequency at which FreiStat should stop in Hz.

        acAmplitude : float
            Amplitude of the Sinussignal in mVp.

        dcOffset : float
            Offset of the Sinussignal in mV.

        sweep_Typ : bool
            true for a logarithmic sweep and false for a linear sweep.


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

        startFrequency : float = 0
        stopFrequency : float = 0

        maxFrequency = 250e3
        minFrequncy = 0.0149
        
        acAmplitude : float = 0

        minAcAmplitude = 0
        maxAcAmplitude = 800

        dcOffset : float = 0

        minDcOffset = -2700
        maxDcOffset = 2700
        
        # Check if potential of the working electrode is fixed or not
        if (FIXED_WE_POTENTIAL == 1):
            fVoltageRange = VOLTAGE_RANGE_FWP
        else:
            fVoltageRange = VOLTAGE_RANGE * 2

       

        # Initialize reference list
        listReferenceList = [
            [START_FREQUENCY, minFrequncy, maxFrequency],
            [STOP_FREQUENCY, minFrequncy, maxFrequency],
            [AC_AMPLITUDE, minAcAmplitude, maxAcAmplitude],
            [DC_OFFSET, minDcOffset, maxDcOffset],
            [SWEEP_TYPE, 0, 1 ],
            [LPTIA_RTIA_SIZE, LPTIARTIA_OPEN, LPTIARTIA_512K],
            [FIXED_WE_POTENTIAL, 0, 1],
            [MAINS_FILTER, 0, 1],
            [SINC2_OVERSAMPLING, ADCSINC2OSR_DISABLED, ADCSINC2OSR_1333],
            [SINC3_OVERSAMPLING, ADCSINC3OSR_DISABLED, ADCSINC3OSR_2]
        ]

        # Check if list of Parameters is in correct format
        if (len(listExperimentParameters) != DPV_NUM_PARAMETER):
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
        self._dataHandling.save_ExperimentType(DPV)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Setup CSV export for the chosen experiment
        self._dataSoftwareStorage._dataHandling.setup_ExportFiles()

        # Return error code
        return iErrorCode
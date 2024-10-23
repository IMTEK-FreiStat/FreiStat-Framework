"""
Module implementing the behavior of the setup function for running
electrochemical impedance spectroscopy.

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
    running electrochemical impedance spectroscopy.
    
    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for electrochemical impedance spectroscopy
        
        List of required parameters
        ----------
        startFrequency : float
            Frequency at which FreiStat should start in Hz.

        stopFrequency : float
            Frequency at which FreiStat should stop in Hz.

        acAmplitude : float
            Amplitude of the sinussignal in Vp.

        dcOffset : float
            Offset of the sinussignal in V.

        sweep_Typ : bool
            true for a logarithmic sweep and false for a linear sweep.

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
        11200       :   start_frequemcy out of bounds
        11201       :   stop_frequemcy out of bounds    
        11202       :   DeltaV_Staircase out of bounds     
        11203       :   DeltaV_Peak out of bounds     
        11204       :   Number of sample points out of bounds
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

        #start_frequency : float = 0
        #stop_frequency : float = 0

        max_frequency = 250e3
        min_frequncy = 0.0149
        
        #ac_amplitude : float = 0

        min_ac_amplitude = 0
        max_ac_amplitude = 800

        #dc_offset : float = 0

        min_dc_offset = -2700
        max_dc_offset = 2700
        

        # Initialize reference list
        listReferenceList = [
            [START_FREQUENCY, min_frequncy, max_frequency],
            [STOP_FREQUENCY, min_frequncy, max_frequency],
            [AC_AMPLITUDE, min_ac_amplitude, max_ac_amplitude],
            [DC_OFFSET, min_dc_offset, max_dc_offset],
            [NUM_POINTS,1],
            [SWEEP_TYPE, 0, 1 ],
            [MAINS_FILTER, 0, 1],
            [SINC2_OVERSAMPLING, ADCSINC2OSR_DISABLED, ADCSINC2OSR_1333],
            [SINC3_OVERSAMPLING, ADCSINC3OSR_DISABLED, ADCSINC3OSR_2]
        ]
    
        # Check if list of Parameters is in correct format
        if (len(listExperimentParameters) != EIS_NUM_PARAMETER):
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
                if (listExperimentParameters[iEntry][1]< listReferenceList[iEntry][1]):
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


        # Safe experiment parameters
        self._dataHandling.save_ExperimentType(EIS)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Setup CSV export for the chosen experiment
        self._dataSoftwareStorage._dataHandling.setup_ExportFiles()

        # Return error code
        return iErrorCode
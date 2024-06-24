"""
Module implementing the behavior of the setup function for running
amperometry.

"""

__author__ = 'Cedic Neumann'
__copyright__ = 'pending'
__credits__ = 'Cedric Neumann'

__license__ = 'pending'
__version__ = '1.0.0'
__maintainer__ = 'Cedric Neumnn'
__email__ = 'jasper@informatik.uni-freiburg.de'
__status__ = 'Development'

# Import dependencies

# Import internal dependencies
from ...Data_storage.constants import *
from .setup_behavior import SetupBehavior
import logging

logger = logging.getLogger(__name__)


class SetupA(SetupBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the setup function when
    running amperometry.
    
    """

    def setup(self, listExperimentParameters: list) -> int:
        """
        Description
        -----------
        Setup method for amperometry.
        
        List of required parameters
        ----------
        Potential : float
            Potential in mV, which get executed.

        Sampling_Rate : float
            Sampling rate defining the distance between measurement results 
            in ms.


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

        #logging.basicConfig(filename= "setup_a.log", encoding= "utf-8",
        #                     level=logging.DEBUG, 
        ##                    format='%(asctime)s %(levelname)s %(message)s')
        logger.info("SetupA")
        # Initialize variables
        iErrorCode : int = EC_NO_ERROR

        potential : float = 0     


        # Check if list of Parameters is in correct format
        if (len(listExperimentParameters) != A_NUM_PARAMETER):
            # Not enough Parameters
            iErrorCode = EC_SETUP + EC_SE_AMOUNT_PARAMETER
            return iErrorCode
        logger.warning("Experiment list: %s", listExperimentParameters)
        potential =listExperimentParameters[0][1][0]
        logger.info(potential)


       


        # Initialize reference list
        listReferenceList = [
            [POTENTIAL_A, potential],
            [SAMPLING_RATE, MIN_SAMPLING_RATE, MAX_SAMPLING_RATE],
            [LPTIA_RTIA_SIZE, LPTIARTIA_OPEN, LPTIARTIA_512K],
            [MAINS_FILTER, 0, 1],
            [SINC2_OVERSAMPLING, ADCSINC2OSR_DISABLED, ADCSINC2OSR_1333],
            [SINC3_OVERSAMPLING, ADCSINC3OSR_DISABLED, ADCSINC3OSR_2]
        ]

        logger.warning("Reference liste: %s", listReferenceList)
        # Check individual entries
        for iEntry in range (len(listExperimentParameters)):
            # Check if parameter is named correct and at the right position
            if (listExperimentParameters[iEntry][0] != 
                listReferenceList[iEntry][0]):
                iErrorCode = EC_SETUP + EC_SE_PARAM_NOT_FOUND + iEntry
                return iErrorCode
            if (iEntry == 0):
                 if (listExperimentParameters[iEntry][1][0] < 
                        listReferenceList[iEntry][1] or 
                        listExperimentParameters[iEntry][1][0] > 
                        listReferenceList[iEntry][1]):
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
        self._dataHandling.save_ExperimentType(A)
        self._dataHandling.save_ExperimentParmeters(listExperimentParameters)

        # Setup CSV export for the chosen experiment
        self._dataSoftwareStorage._dataHandling.setup_ExportFiles()

        # Return error code
        return iErrorCode
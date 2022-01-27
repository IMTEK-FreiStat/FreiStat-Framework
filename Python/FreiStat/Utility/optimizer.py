"""
Module implementing a class for optimizing the given experiment parameters. 

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging

# Import internal dependencies
from ..Data_storage.constants import *
from .decoder import _decode_LPTIA_Resistor_Size
from .decoder import _decode_SincXOSR
from .encoder import _encode_Bool_Flag
from .encoder import _encode_LPTIA_Resistor_Size
from .encoder import _encode_Sinc_Oversampling_Rate

class Optimizer:
    """
    Description
    -----------
    Class which implements an optimizer to tune the experiment parameters given 
    by the user to get the optimal results.

    """
    def __init__(self, logger : logging.Logger, iCommunicationMode : int) -> None:
        """
        Description
        -----------
        Constructor of the optimizer class.

        Parameter
        ---------
        `logger` : logging.Logger
            Reference to the logger module, which should be used

        `iCommunicationMode` : int
            Integer flag encoding if Python library communicates via serial (1)
            or WiFi (2)

        """
        # Initialize class variables
        self._logger = logger
        self._iCommunicationMode = iCommunicationMode

        self._strMethod : str = ""

        self._iSamplingRateADC : int = AD5940_SAMPLING_RATE
        self._iOsrSinc2 : int = -1
        self._iOsrSinc3 : int = -1

        self._fSamplingDuration : float = 0.0
        self._fSamplingRate : float = 0.0
        self._fScanRate : float = 0.0
        self._fStepSize : float = 0.0

        # Check for mode of operation
        if (self._iCommunicationMode == FREISTAT_SERIAL):
            self._fSampleTimeCA : float = FREISTAT_CA_ST_SERIAL
            self._fSampleTimeCV : float = FREISTAT_CV_ST_SERIAL
            self._fSampleTimeDPV : float = FREISTAT_DPV_ST_SERIAL
        elif (self._iCommunicationMode == FREISTAT_WLAN): 
            self._fSampleTimeCA : float = FREISTAT_CA_ST_WLAN
            self._fSampleTimeCV : float = FREISTAT_CV_ST_WLAN
            self._fSampleTimeDPV : float = FREISTAT_DPV_ST_WLAN

    def start(self, strMethod : str, listExperimentParameters : list) -> int:
        """
        Description
        -----------
        Constructor of the optimizer class.

        Parameters
        ----------
        `strMethod` : string
            String containing the electrochemical method, which should be
            optimized

        `listExperimentParameters` : list
            List containing the corresponding experiment parameters.

        Return
        ------
        `iErrorcode` : int
            Errorcode encoded as integer

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   No error occured, setup successful
        17001       :   Electrochemical method not known
        17002       :   Sinc 2 Oversampling rate missing
        17003       :   Sinc 3 Oversampling rate missing
        17004       :   Sampling rate missing
        17005       :   Sampling duration missing
        17006       :   Stepsize missing
        17007       :   Scanrate missing

        """
        # Initialize variables
        iErrorcode : int = EC_NO_ERROR

        # Save list of experiment parameters
        self._listExperimentParameters = listExperimentParameters

        # Check if experiment type is known and safe it
        iErrorcode = self._check_Method(strMethod)
        if (iErrorcode == EC_NO_ERROR):
            self._strMethod = strMethod
        else :
            return iErrorcode

        # Extract the experiment parameters
        iErrorcode = self._extract_Parameters(listExperimentParameters)
        if (iErrorcode != EC_NO_ERROR):
            return iErrorcode

        # Call different optimization depending on the electrochemical method
        # Open circuit potenital

        # Chronoamperometry
        if (self._strMethod == CA):
            iErrorcode = self._optimizeCA()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode

        # Linear sweep voltammetry
        if (self._strMethod == LSV):
            iErrorcode = self._optimizeCV()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode

        # Cyclic voltammetry
        if (self._strMethod == CV):
            iErrorcode = self._optimizeCV()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode

        # Normal pulse voltammetry
        if (self._strMethod == NPV):
            iErrorcode = self._optimizeDPV()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode

        # Differential pulse voltammetry
        if (self._strMethod == DPV):
            iErrorcode = self._optimizeDPV()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode

        # Square wave voltammetry
        if (self._strMethod == SWV):
            iErrorcode = self._optimizeDPV()
            if (iErrorcode != EC_NO_ERROR):
                return iErrorcode


        return EC_NO_ERROR

    def return_Parameters(self) -> list:
        """
        Description
        -----------
        Method for returning the optimized parameters

        Return
        ------
        `listOptimizedParameters` : list
            List containing the optimized parameters

        """
        # Loop over all entries of the experiment parameter list
        for iIndex in range(len(self._listExperimentParameters)):
            # Save Sinc2 oversampling rate
            if (self._listExperimentParameters[iIndex][0] == SINC2_OVERSAMPLING):
                self._listExperimentParameters[iIndex][1] = self._iOsrSinc2
            # Save Sinc3 oversampling rate
            elif (self._listExperimentParameters[iIndex][0] == SINC3_OVERSAMPLING):
                self._listExperimentParameters[iIndex][1] = self._iOsrSinc3
            # Save sampling rate in ms
            elif (self._listExperimentParameters[iIndex][0] == SAMPLING_RATE):
                self._listExperimentParameters[iIndex][1] = self._fSamplingRate
            # Save sampling duration in ms
            elif (self._listExperimentParameters[iIndex][0] == SAMPLING_DURATION):
                self._listExperimentParameters[iIndex][1] = self._fSamplingDuration
            # Save stepsize in mV
            elif (self._listExperimentParameters[iIndex][0] == STEP_SIZE):
                self._listExperimentParameters[iIndex][1] = self._fStepSize
            # Save scanrate in mV/s
            elif (self._listExperimentParameters[iIndex][0] == SCAN_RATE):
                self._listExperimentParameters[iIndex][1] = self._fScanRate

        return self._listExperimentParameters

    def _extract_Parameters(self, listExperimentParameters) -> int:
        """
        Description
        -----------
        Method which extracts the required experiment parameters based on their
        name abbreviation from the list and stores it.

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the corresponding experiment parameters.

        Return
        ------
        `iErrorcode` : int
            Errorcode encoded as integer

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   No error occured, setup successful
        17002       :   Sinc 2 Oversampling rate missing
        17003       :   Sinc 3 Oversampling rate missing
        17004       :   Sampling rate missing
        17005       :   Sampling duration missing
        17006       :   Stepsize missing
        17007       :   Scanrate missing

        """
        # Loop over all entries of the experiment parameter list
        for iIndex in range(len(listExperimentParameters)):
            # Save Sinc2 oversampling rate
            if (listExperimentParameters[iIndex][0] == SINC2_OVERSAMPLING):
                self._iOsrSinc2 = listExperimentParameters[iIndex][1]
            # Save Sinc3 oversampling rate
            elif (listExperimentParameters[iIndex][0] == SINC3_OVERSAMPLING):
                self._iOsrSinc3 = listExperimentParameters[iIndex][1]
            # Save sampling rate in ms
            elif (listExperimentParameters[iIndex][0] == SAMPLING_RATE):
                self._fSamplingRate = listExperimentParameters[iIndex][1]
            # Save sampling duration in ms
            elif (listExperimentParameters[iIndex][0] == SAMPLING_DURATION):
                self._fSamplingDuration = listExperimentParameters[iIndex][1]
            # Save stepsize in mV
            elif (listExperimentParameters[iIndex][0] == STEP_SIZE):
                self._fStepSize= listExperimentParameters[iIndex][1]
            # Save scanrate in mV/s
            elif (listExperimentParameters[iIndex][0] == SCAN_RATE):
                self._fScanRate = listExperimentParameters[iIndex][1]


        # Check depending on the electrochemical method which parameters should
        # have been found and saved
        if (self._strMethod == OCP):
            # TODO : Method doesn't work so not optimization yet
            pass
        elif (self._strMethod == CA):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fSamplingRate == 0.0):
                return EC_UTILITY + EC_UT_SAMPLING_RATE_MIS

        elif (self._strMethod == LSV):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fStepSize == 0.0):
                return EC_UTILITY + EC_UT_STEP_SIZE_MIS
            if (self._fScanRate == 0.0):
                return EC_UTILITY + EC_UT_SCAN_RATE_MIS

        elif (self._strMethod == CV):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fStepSize == 0.0):
                return EC_UTILITY + EC_UT_STEP_SIZE_MIS
            if (self._fScanRate == 0.0):
                return EC_UTILITY + EC_UT_SCAN_RATE_MIS

        elif (self._strMethod == NPV):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fSamplingDuration == 0.0):
                return EC_UTILITY + EC_UT_SAMPLING_DUR_MIS

        elif (self._strMethod == DPV):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fSamplingDuration == 0.0):
                return EC_UTILITY + EC_UT_SAMPLING_DUR_MIS

        elif (self._strMethod == SWV):
            if (self._iOsrSinc2 == -1):
                return EC_UTILITY + EC_UT_SINC2_MIS
            if (self._iOsrSinc3 == -1):
                return EC_UTILITY + EC_UT_SINC3_MIS
            if (self._fSamplingDuration == 0.0):
                return EC_UTILITY + EC_UT_SAMPLING_DUR_MIS

        return EC_NO_ERROR

    def _check_Method(self, strMethod : str) -> int:
        """
        Description
        -----------
        Method for checking if a electrochemical method is supported by the
        system.

        Parameters
        ----------
        `strMethod` : string
            String containing the electrochemical method, which should be 
            checked 

        Return
        ------
        `iErrorCode` : int
            Error code encoded in an integer
        
        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   No error occured, setup successful
        17001       :   Electrochemical method not known

        """
        if (strMethod == OCP or strMethod == CA or strMethod == LSV or 
            strMethod == CV or strMethod == NPV or strMethod == DPV or 
            strMethod == SWV):
            return EC_NO_ERROR
        else :
            return EC_UTILITY + EC_UT_METHOD_UNKNOWN

    def _check_StepSize(self, fStepsize : float) -> float:
        """
        Description
        -----------
        Method for calculating the real stepsize which is used by the FreiStat

        Parameters
        ----------
        `fStepsize` : float
            Stepsize of the electrochemical cell in mV

        Return
        ------
        `fTrueStepsize` : float
            Real stepsize calculated through the resolution of the 12-Bit DAC

        """
        fTrueStepsize : float = int(fStepsize / AD5940_12BIT_DAC_1LSB + 0.5) * \
                                    AD5940_12BIT_DAC_1LSB

        if (fTrueStepsize != fStepsize):
            # Truncate float
            fTrueStepsize = float(int(fTrueStepsize * 10000) / 10000)
 
            return fTrueStepsize
        else:
            # Truncate float
            fStepsize = float(int(fStepsize * 10000) / 10000)

            return fStepsize

    def _optimizeCA(self) -> int:
        """
        Description
        -----------
        Method for optimizing the parameters of the chronoamerometry

        In chronoamperometry the three tunable parameters are:

        - Sampling rate in ms
        - Sinc2 oversampling rate
        - Sinc3 oversampling rate

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

        """
        # Initialize variables
        fDataSample: float = 0.0
        fSampleBufferSize : float = SAMPLE_BUFFER

        # Decode Sinc2 and Sinc3
        self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, SINC2_OVERSAMPLING)
        self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, SINC3_OVERSAMPLING)

        # Temporarsy save the sampling time
        fTempSamplingRate : float = self._fSamplingRate

        # Check if the sample time is smaller than the operation window and if,
        # increase the sampling to the minimum
        if (fTempSamplingRate * 1e-3 < self._fSampleTimeCA ):
            self._fSamplingRate = self._fSampleTimeCA * 1e3

            self._logger.warning("Optimizer:" +
                " Chosen sampling rate of " + str(fTempSamplingRate) + 
                " ms is not possible, optimizer changed the value to " +
                str(self._fSamplingRate) + " ms.") 

        # Temporary save oversampling rates
        iTempOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                SINC2_OVERSAMPLING)

        iTempOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                SINC3_OVERSAMPLING)


        while(True):
            # Calculate the amount of samples during the sampling time
            fDataSample = self._fSamplingRate * 1e-3 / (self._iOsrSinc2Value * 
                self._iOsrSinc3Value/ self._iSamplingRateADC)

            # Check if amount of data samples exceeds sample buffer size
            if (fDataSample <= fSampleBufferSize):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 < 2):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 += 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 > 0):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 -= 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

            # Check if filters are at minimum setting
            if (self._iOsrSinc2 == ADCSINC2OSR_22 and 
                self._iOsrSinc3 == ADCSINC3OSR_2):
                break


        while(True):
            # Calculate the amount of samples during the sampling time
            fDataSample = self._fSamplingRate * 1e-3 / (self._iOsrSinc2Value * 
                self._iOsrSinc3Value/ self._iSamplingRateADC)

            # Check sample time
            if (self._fSamplingRate < 3.5):
                # Limit size of data samples to 25 in the range from 3.0 to 3.5
                # ms sampling time, to account for the non-linear relation 
                # between execution time of the program (sampling time) and
                # time need to sum up the data samples.
                fSampleBufferSize = 25

            # Check if amount of data samples exceeds sample buffer size
            if (fDataSample >= fSampleBufferSize):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 > 1):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 -= 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 < 11):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 += 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

        # Check if Sinc2 osr has changed
        if (iTempOsrSinc2Value != self._iOsrSinc2Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc2 oversampling rate of " + 
                str(iTempOsrSinc2Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc2Value) + ".") 

        # Check if Sinc3 osr has changed
        if (iTempOsrSinc3Value != self._iOsrSinc3Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc3 oversampling rate of " + 
                str(iTempOsrSinc3Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc3Value) + ".") 

        return EC_NO_ERROR

    def _optimizeCV(self) -> int:
        """
        Description
        -----------
        Method for optimizing the parameters of the cyclic voltammetry

        In cyclic voltammetry the four tunable parameters are:

        - Stepsize in mV
        - Scanrate in mV/s
        - Sinc2 oversampling rate
        - Sinc3 oversampling rate

        The scanrate is not tuned, because the chemistry can change depending
        on the chosen value. Therefore only the other 3 values are tuned.

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

        """
        # Initialize variables
        fCurrentSampleTime : float = 0.0
        fRequiredSampleTime : float = 0.0

        # Decode Sinc2 and Sinc3
        self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, SINC2_OVERSAMPLING)
        self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, SINC3_OVERSAMPLING)

        # Temporary save stepsize
        fTempStepSize = self._fStepSize

        # Check if the sample time is larger thanthe operation window and if,
        # decrease the stepsize to a minimum
        while(True):
            # Calculate sampling time in ms
            fCurrentSampleTime = self._fStepSize / self._fScanRate   

            # Check for limit of operation
            if (fCurrentSampleTime > self._fSampleTimeCV):
                self._fStepSize -= AD5940_12BIT_DAC_1LSB
                self._fStepSize = self._check_StepSize(self._fStepSize)
            else :
                break

        # Check if sample time is in the operation window of the FreiStat
        while(True):
            # Calculate sampling time in ms
            fCurrentSampleTime = self._fStepSize / self._fScanRate            

            # Check for limit of operation
            if (fCurrentSampleTime < self._fSampleTimeCV):
                self._fStepSize += AD5940_12BIT_DAC_1LSB
                self._fStepSize = self._check_StepSize(self._fStepSize)
            else :        
                break
        
        # Check if stepsize has changed
        if (fTempStepSize != self._fStepSize):
            self._logger.warning("Optimizer:" +
                " Chosen stepsize of " + str(fTempStepSize) + 
                " mV is not optimal, optimizer changed the value to " +
                str(self._fStepSize) + " mV.") 

        # Calculate sampling time in ms
        fCurrentSampleTime = self._fStepSize / self._fScanRate

        # Temporary save oversampling rates
        iTempOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                SINC2_OVERSAMPLING)

        iTempOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                SINC3_OVERSAMPLING)

        while(True):
            # Calculate required sample time for one value in ms
            fRequiredSampleTime = self._iOsrSinc2Value * 2 * \
                self._iOsrSinc3Value / self._iSamplingRateADC

            # Check for limit of operation
            if (fCurrentSampleTime > fRequiredSampleTime):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 > 0):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 -= 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 < 11):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 += 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

        # Check if sample time is in the operation window of the FreiStat
        while(True):
            # Calculate required sample time for one value in ms
            fRequiredSampleTime = self._iOsrSinc2Value * 2 * \
                self._iOsrSinc3Value / self._iSamplingRateADC

            # Check for limit of operation
            if (fCurrentSampleTime < fRequiredSampleTime):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 < 2):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 += 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 > 0):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 -= 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

        # Check if Sinc2 osr has changed
        if (iTempOsrSinc2Value != self._iOsrSinc2Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc2 oversampling rate of " + 
                str(iTempOsrSinc2Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc2Value) + ".") 

        # Check if Sinc3 osr has changed
        if (iTempOsrSinc3Value != self._iOsrSinc3Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc3 oversampling rate of " + 
                str(iTempOsrSinc3Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc3Value) + ".") 

        return EC_NO_ERROR

    def _optimizeDPV(self) -> int:
        """
        Description
        -----------
        Method for optimizing the parameters of the differential pulse 
        voltammetry.

        In differential pulse voltammetry the three tunable parameters are:

        - Sample duration in ms
        - Sinc2 oversampling rate
        - Sinc3 oversampling rate

        The sample duration is not tuned and set fix to a certain percentage of
        the total pulse durations [Tau', Tau]. Therefore the Sinc2/3 osr is
        tuned to allow enough values in this defined timeframe.

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

        """
        # Initialize variables
        bOptimizationDone : bool = False

        fCurrentSampleTime : float = self._fSamplingDuration
        fRequiredSampleTime : float = 0.0
        fDataSample: float = 0.0
        fSampleBufferSize : float = SAMPLE_BUFFER


        # Decode Sinc2 and Sinc3
        self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, SINC2_OVERSAMPLING)
        self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, SINC3_OVERSAMPLING)

        # Temporary save oversampling rates
        iTempOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                SINC2_OVERSAMPLING)

        iTempOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                SINC3_OVERSAMPLING)


        while(True):
            # Calculate the amount of samples during the sampling time
            fDataSample = self._fSamplingDuration * 1e-3 / (self._iOsrSinc2Value * 
                self._iOsrSinc3Value/ self._iSamplingRateADC)

            # Check if amount of data samples exceeds sample buffer size
            if (fDataSample <= fSampleBufferSize):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 < 2):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 += 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 > 0):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 -= 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

            # Check if filters are at minimum setting
            if (self._iOsrSinc2 == ADCSINC2OSR_22 and 
                self._iOsrSinc3 == ADCSINC3OSR_2):
                break


        while(True):
            # Calculate the amount of samples during the sampling time
            fDataSample = self._fSamplingDuration * 1e-3 / (self._iOsrSinc2Value * 
                self._iOsrSinc3Value/ self._iSamplingRateADC)

            # Check if amount of data samples exceeds sample buffer size
            if (fDataSample >= fSampleBufferSize):
                # Check if Sinc3 can be increased
                if (self._iOsrSinc3 > 1):
                    # Increase Sinc3 osr
                    self._iOsrSinc3 -= 1

                    # Update Sinc3 osr value
                    self._iOsrSinc3Value = _decode_SincXOSR(self._iOsrSinc3, 
                                                SINC3_OVERSAMPLING)
                                                
                # Check if Sin2 can be increased
                elif (self._iOsrSinc2 < 11):
                    # Recudce Sinc2 osr
                    self._iOsrSinc2 += 1

                    # Update Sinc2 osr value
                    self._iOsrSinc2Value = _decode_SincXOSR(self._iOsrSinc2, 
                                                SINC2_OVERSAMPLING)
            else :
                break

        # Check if Sinc2 osr has changed
        if (iTempOsrSinc2Value != self._iOsrSinc2Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc2 oversampling rate of " + 
                str(iTempOsrSinc2Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc2Value) + ".") 

        # Check if Sinc3 osr has changed
        if (iTempOsrSinc3Value != self._iOsrSinc3Value):
            self._logger.warning("Optimizer:" +
                " Chosen Sinc3 oversampling rate of " + 
                str(iTempOsrSinc3Value) + 
                " is not optimal, optimizer changed the value to " +
                str(self._iOsrSinc3Value) + ".") 

        return EC_NO_ERROR

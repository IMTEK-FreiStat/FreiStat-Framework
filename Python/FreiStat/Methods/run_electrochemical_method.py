"""
Module implementing parent class from which all facades inherit.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging
import multiprocessing as mp
from multiprocessing import shared_memory
import numpy as np
import platform

# Import internal dependencies
from ..Data_storage.constants import *
from ..Serial_communication.serial_communication import Communication
from ..Data_storage.data_handling import DataHandling
from ..Data_storage.data_software_storage import DataSoftwareStorage
from ..Electrochemical_methods.electrochemical_method import ElectrochemicalMethod
from ..JSON_parser.json_parser import JSON_Parser
from ..Plotter.plotter import Plotter


class Run_Electrochemical_Method:
    """
    Description
    -----------
    Parent class from which all facade classes inherit.

    """

    def __init__(
        self,
        logger=logging.Logger("FreiStat_Library"),
        commnicationMode=FREISTAT_SERIAL,
        wlanSetting=[
            FREISTAT_UDP_SERVER_IP,
            FREISTAT_UDP_SERVER_PORT,
            FREISTAT_UDP_CLIENT_IP,
            FREISTAT_UDP_CLIENT_PORT,
        ],
        mode: str = FREISTAT_STANDALONE,
    ) -> None:
        """
        Description
        -----------
        Constructor of the super class Run_Electrochemical_Method.

        Parameters
        ----------
        `logger` : logging.Logger
            Logger which should be used in the library

        `commnicationMode` : int
            Integer flag encoding if Python library communicates via serial (1)
            or WiFi (2)

        `wlanSetting` : list
            [Server IP (str), Server Port (int), Client IP (str), Client Port(int)]

        `mode` : string
            String defining in which mode the FreiStat library should be used
            Defined: "standalone", "backend"

        """
        # Save class variables
        self._logger = logger
        self._iCommunicationMode = commnicationMode
        self._listWLANSetting = wlanSetting

        # Check if mode is defined
        if mode == FREISTAT_STANDALONE or mode == FREISTAT_BACKEND:
            self._FreiStatMode = mode
        else:
            raise RuntimeError("Operation mode doesn't exist.")

    def P_DataCollection(
        self,
        strMethod: str,
        dataQueue: mp.Queue,
        event: mp.Event(),
        listTempExperimentParameters: list,
        bLowPerformanceMode: bool,
        sharedMemoryLocation_name: str,
    ) -> None:
        """
        Description
        -----------
        Method running in a seperate process, which is used to setup and execute
        the electrochemical method.
        Thereby, the data from the serial connection is read and stored in the
        corresponding data handling, as well as in the DataQueue, which is
        accessed by the plotter class.

        Parameters
        ----------
        `strMethod` : str
            String containing the name of the electrochemical method

        `dataQueue` : Queue
            Data queue connecting the different processes with each other

        `listTempExperimentParameters` : list
            List containing all defined experiment parameters for cyclic
            voltammetry

        `bLowPerformanceMode` : bool
            Enables low performance mode of the FreiStat, which disables
            plotting of the data and most data output

        `sharedMemoryLocation_name` : str
            Unique name of the shared memory location in the ram. This is used
            to return later the name of the file-path back to the user.

        """
        self._logger = logging.Logger("FreiStat_Library")

        # Save event reference
        self._event = event

        # Creating an object which stores all references to other objects
        self._dataSoftwareStorage = DataSoftwareStorage()

        # Save the low performance mode flag
        self._dataSoftwareStorage.set_LowPerformanceMode(bLowPerformanceMode)

        # Create an object which handles all data
        self._dataHandling = DataHandling(self._dataSoftwareStorage)

        # Create an object for handling communication
        self._serialConnection = Communication(
            self._dataSoftwareStorage, self._iCommunicationMode, self._listWLANSetting
        )

        # Create an object for parsing JSON strings
        self._jsonParser = JSON_Parser(self._dataSoftwareStorage)

        # Creating an object for general electrochemical methods
        self._ecMethod = ElectrochemicalMethod(strMethod, self._dataSoftwareStorage)

        # Execute setup for electrochemical methods
        iErrorCode = self._ecMethod.setup(listTempExperimentParameters)

        # Check if setup was successfull
        if iErrorCode != 0:
            self._logger.warning(
                strMethod
                + "setup failed: Error code: "
                + str(iErrorCode)
                + " Check error list for further informations."
            )
            return iErrorCode

        # Start thread to run execute behavior
        self._ecMethod.execute(dataQueue, self._event)

        # Set system status to starting experiment
        self._dataSoftwareStorage.set_SystemStatus(FREISTAT_EXP_STARTED)

        # Post processing of experiment data
        # Export data to previously setup csv export after experiment is done
        strExportPath = self._dataHandling.export_Data_csv(
            self._dataHandling.get_StoredData()
        )

        # Export experiment type and parameters
        self._dataHandling.export_ExperimentParameters_csv(
            self._dataHandling.get_ExperimentType(),
            self._dataHandling.get_ExperimentParameters(),
        )

        # Save data object persistent
        self._dataHandling.export_DataStorage()

        # Get address of the shared memory in the RAM through the unique name
        existing_shm = shared_memory.SharedMemory(
            name=sharedMemoryLocation_name, create=False
        )

        # Define array which is located in the shared memory and has the right
        # data format
        np_array = np.ndarray((1, 200), dtype="|S1", buffer=existing_shm.buf)

        # Loop over the length of the file-path and write it into the array in
        # the shared memory
        for i in range(len(strExportPath)):
            np_array[0, i] = strExportPath[i].encode("UTF-8")

        # Close exisitng serial connection
        self._serialConnection._closeConnection()

    def _check_OsProcess(self) -> str:
        """
        Description
        -----------
        Method returning the correct process type encoded as string, depending
        on the operating system.

        spawn : Both Windows and Unix
        fork  : Unix only

        Return
        ------
        `strProcess` : string
            Process used on the operating system encoded as string

        """
        # Variable initaliztaion
        strProcess = ""

        # Check operating system
        if platform.system() == LINUX:
            strProcess = "fork"
        elif platform.system() == MACOS:
            strProcess = "spawn"
        elif platform.system() == WINDOWS:
            strProcess = "spawn"

        return strProcess

    def _check_StartingPotential(
        self, fStartingPotential: float, iFixedWEPotential: int
    ) -> float:
        """
        Description
        -----------
        Method for calculating the real starting potential which is used by the
        FreiStat.

        Parameters
        ----------
        `fStartingPotential` : float
            Starting potential of the electrochemical cell in mV

        `iFixedWEPotential` : int
            Integer encoding if the potential of the working electrode is
            static (1 - True) or dynamic (0 - False)

        Return
        ------
        `fTrueStartingPotential` : float
            Real starting potential calculated through the resolution of the
            6-Bit and 12-Bit DAC

        """

        # Check if working electrode potential is fixed and calculate potential
        if iFixedWEPotential == 1:
            fWePotential = (
                AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT
            ) / 2 + AD5940_MIN_DAC_OUTPUT
        else:
            fWePotential = AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT

        # Calculate 6-Bit DAC code
        f6BitDacCode: int = int(
            (fWePotential - AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB
        )

        # Calculate 6-Bit DAC potential
        f6BitDacPotential = f6BitDacCode * AD5940_6BIT_DAC_1LSB

        # Calculate 12-Bit DAC code
        f12BitDacCode: int = int(
            f6BitDacCode * 64 - fStartingPotential / AD5940_12BIT_DAC_1LSB + 0.5
        )

        if f12BitDacCode < f6BitDacCode * 64:
            f12BitDacCode -= 1

        # Calculate 12-Bit DAC potential
        f12BitDacPotential = f12BitDacCode * AD5940_12BIT_DAC_1LSB

        # Calculate 12-Bit DAC potential
        fTrueStartingPotential: float = f6BitDacPotential - f12BitDacPotential

        if fTrueStartingPotential != fStartingPotential:
            # Truncate float
            fTrueStartingPotential = float(int(fTrueStartingPotential * 10000) / 10000)

            self._logger.warning(
                "Chosen potential of "
                + str(fStartingPotential)
                + " mV is not possible, "
                + "closest value of "
                + str(fTrueStartingPotential)
                + " mV will be used instead."
            )

            return fTrueStartingPotential
        else:
            return fStartingPotential

    def _check_StepSize(self, fStepsize: float) -> float:
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
        fTrueStepsize: float = (
            int(fStepsize / AD5940_12BIT_DAC_1LSB + 0.5) * AD5940_12BIT_DAC_1LSB
        )

        if fTrueStepsize != fStepsize:
            # Truncate float
            fTrueStepsize = float(int(fTrueStepsize * 10000) / 10000)

            self._logger.warning(
                "Chosen stepsize of "
                + str(fStepsize)
                + " mV is not possible, closest value of "
                + str(fTrueStepsize)
                + " mV will be used instead."
            )

            return fTrueStepsize
        else:
            return fStepsize

    def _check_FixedWEPotential(
        self,
        iFixedWEPotential: int,
        fLowerTurningVoltage: float,
        fUpperTurningVoltage: float,
    ) -> int:
        """
        Description
        -----------
        Method for checking if size of sweaping range is in conflict with
        flag for fixing the working electrode potential.

        Parameters
        ----------
        `iFixedWEPotential` : int
            Boolean flag encoded as integer (0 : True | 1 : False)
        `fLowerTurningVoltage` : float
            Lower turning potential in mV
        `fUpperTurningVoltage` : float
            Upper turning potential in mV

        Return
        ------
        `iNewFixedWEPotential` : int
            Boolean flag encoded as integer (0 : True | 1 : False)

        """
        # Calculate sweap range
        fRange = abs(fUpperTurningVoltage - fLowerTurningVoltage)

        # Check if flag needs to be adjusted
        if fRange > VOLTAGE_RANGE:
            iNewFixedWEPotential = 0
        else:
            iNewFixedWEPotential = 1

        # Check if flag has changed
        if iNewFixedWEPotential != iFixedWEPotential:
            if iFixedWEPotential == 0:
                pass
            elif iFixedWEPotential == 1:
                self._logger.warning(
                    "Sweap range exceeds operating range for fixed"
                    + " working electrode potential. Fixed potential"
                    + " disabled."
                )
                pass
        return iNewFixedWEPotential

    def _pressEvent(self, event) -> None:
        """
        Description
        -----------
        Method for implementing a press event in the plot window, in order to be
        able to terimante experiements early by pressing the 'q'-button.

        Parameters
        ----------
        event : event
            Event which should be executed

        """
        if event.key == "q":
            self._terminateExperiment()

    def _terminateExperiment(self) -> None:
        """
        Description
        -----------
        Method used to terminate the current experiment

        """
        # Set event flag to true to end child process
        self._event.set()

    def get_plotter(self) -> Plotter:
        """
        Description
        -----------
        Getter method returning reference to plotter object.

        Return
        ------
        `Plotter` : Plotter
            Reference to the plotter object used by the facade

        """
        return self._plotter

    def get_dataQueue(self):
        """
        Description
        -----------
        Getter method returning reference to data queue

        Return
        ------
        `dataQueue` : Queue
            Reference to the data queue object used by FreiStat

        """
        return self._dataQueue

    def get_process(self):
        """
        Description
        -----------
        Getter method returning reference to the process

        Return
        ------
        `process` : process
            Reference to the process used by FreiStat

        """
        return self._process

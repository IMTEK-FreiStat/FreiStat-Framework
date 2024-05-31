"""
Module which implements a facade interface for executing square wave voltammetry.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import multiprocessing as mp
from multiprocessing import shared_memory
import numpy as np

# Import internal dependencies
from ..Data_storage.constants import *
from ..Plotter.plotter import Plotter
from .run_electrochemical_method import Run_Electrochemical_Method
from ..Utility.optimizer import Optimizer
from ..Utility.encoder import _encode_Bool_Flag
from ..Utility.encoder import _encode_LPTIA_Resistor_Size
from ..Utility.encoder import _encode_Sinc_Oversampling_Rate


class Run_SWV(Run_Electrochemical_Method):
    """
    Description
    -----------
    Facade which implements square wave voltammetry on the FreiStat.

    """

    def start(
        self,
        StartVoltage: float = START_POTENTIAL_F,
        StopVoltage: float = STOP_POTENTIAL_F,
        DeltaV_Staircase: float = DELTA_V_STAIRCASE_F,
        DeltaV_Peak: float = DELTA_V_PEAK_F,
        DutyCycle: float = PULSE_LENGTH_F,
        Sampling_Duration: float = SAMPLING_DURATION_F,
        Cycle: int = CYCLE_I,
        CurrentRange: float = CURRENT_RANGE_F,
        FixedWEPotential: bool = True,
        MainsFilter: bool = False,
        Sinc2_Oversampling: int = SINC2_OVERSAMPLING_I,
        Sinc3_Oversampling: int = SINC3_OVERSAMPLING_I,
        EnableOptimizer: bool = True,
        LowPerformanceMode: bool = False,
    ) -> str:
        """
        Description
        -----------
        Start square wave voltammetry with defined parameters.

        Parameters
        ----------
        `StartVoltage` : float
            Potential at which FreiStat should start in V

        `StopVoltage` : float
            Potential at which FreiStat should stop in V

        `DeltaV_Staircase` : float
            Increase of the base potential of the underlying staircase function
            with every cycle in V

        `DeltaV_Peak` : float
            Delta V in V, which defines the voltage jump applied to the cell.

        `Sampling_Duration` : float
            Time in s, which defines how long values should be sampled at the
            end of each potential step

        `DutyCycle` : float
            Duty cycle (period length) in s.

        `Cycle` : int
            Amount of cycles the CV should run

        `CurrentRange`: float
            Current range of the electrochemical cell in A

        `FixedWEPotential`: bool
            Defines if the we electrode potential should be fixed to the middle
            of the dynamic range (1.3 V) or not.

        `MainsFilter`: bool
            Enable/ Disable 50 Hz/ 60 Hz mains filter.
            If enabled `Sinc2_Oversampling` must be defiend (Default: 667)

        `Sinc2_Oversampling` : int
            Oversampling rate of the Sinc 2 filter
            Defiend OSR rates: [22, 44, 89, 178, 267, 533,
            640, 667, 800, 889, 1067, 1333]

        `Sinc3_Oversampling` : int
            Oversampling rate of the Sinc 3 filter
            Defiend OSR rates: [0 (Disabled), 2, 4, 5]
            Oversampling rate of 5 is not recommanded

        `EnableOptimizer` : bool
            Enables the optimizer, which tunes automatically the experiment
            parameters to fit best the performance of the FreiStat

        `LowPerformanceMode` : bool
            Enables low performance mode of the FreiStat, which disables
            plotting of the data

        Return
        ------
        `ExportedFilePath` : string
            Returns system path to the location where the experiment data and
            experiment parameters where stored after the experiment

        """
        # Intialize variables
        bSerialBuffer = b""

        iPosition: int = 0

        # Convert parameters from SI-units to internal units
        StartVoltage = StartVoltage * 1000.0
        StopVoltage = StopVoltage * 1000.0
        DeltaV_Staircase = DeltaV_Staircase * 1000.0
        DeltaV_Peak = DeltaV_Peak * 1000.0
        Sampling_Duration = Sampling_Duration * 1000.0
        DutyCycle = DutyCycle * 1000.0
        CurrentRange = CurrentRange * 1e6

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate(
            "Sinc2", Sinc2_Oversampling, self._logger
        )

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate(
            "Sinc3", Sinc3_Oversampling, self._logger
        )

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, StartVoltage, StartVoltage
        )

        # Safe experiment parameters in correct list format for differential
        # pulse voltammetry
        self._listExperimentParameters = [
            [START_POTENTIAL, fStartVoltage],
            [STOP_POTENTIAL, StopVoltage],
            [DELTA_V_STAIRCASE, DeltaV_Staircase],
            [DELTA_V_PEAK, DeltaV_Peak],
            [PULSE_LENGTH, [DutyCycle / 2, DutyCycle / 2]],
            [SAMPLING_DURATION, Sampling_Duration],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING, Sinc3_Oversampling],
        ]

        # Check if optimizer is enabled
        if EnableOptimizer == True:
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(SWV, self._listExperimentParameters)

            # Check for error
            if iErrorcode != EC_NO_ERROR:
                self._logger.warning(
                    "Optimizer: Failed - Errorcode: "
                    + str(iErrorcode)
                    + " - FreiStat tries to run experiment with user "
                    + "defined parameters"
                )
            else:
                # Overwrite parameters with optimized ones
                self._listExperimentParameters = _Optimizer.return_Parameters()

        # Define electrochemical method
        strMethod: str = SWV

        # Unbound the logger
        self._logger = None

        # Define start method for multiprocessing using helper method
        mp.get_context(self._check_OsProcess())

        # Define data queue
        manager = mp.Manager()
        dataQueue = manager.Queue()

        # Define event
        self._event = mp.Event()

        # Define location in the RAM as shared memory between processes
        sharedMemoryLocation = shared_memory.SharedMemory(create=True, size=200)

        # Define byte array which accesses the prviously defined memory spacee
        np_arrbSharedMemory = np.ndarray(
            (1, 200), dtype="|S1", buffer=sharedMemoryLocation.buf
        )

        # Define a process which deals with the reading of the data from the
        # Serial connection and the storage of the data
        self._process = mp.Process(
            target=self.P_DataCollection,
            args=(
                strMethod,
                dataQueue,
                self._event,
                self._listExperimentParameters,
                LowPerformanceMode,
                sharedMemoryLocation.name,
            ),
        )

        # Start the process
        self._process.start()

        # Check if Library is used as interface backend or not
        if self._FreiStatMode == FREISTAT_STANDALONE:

            if LowPerformanceMode == False:
                # Create an object for plotting the data
                self._plotter = Plotter(
                    strMethod,
                    self._listExperimentParameters,
                    FREISTAT_STANDALONE,
                    self._process,
                )

                # Initialize plot
                self._plotter.initPlot()

                #  Attack press event to plot window
                self._plotter.attachEvent(self._pressEvent)

                # Start display data in main process and hand reference to dataQueue
                self._plotter.T_Animate(dataQueue)

                # Join process (Blocks until process is done)
                self._process.join()
            else:
                # Join process (Blocks until process is done)
                self._process.join()

            # Loop until the end of the shared memory array
            while True:
                # Check if the array ended
                if np_arrbSharedMemory[0, iPosition] == b"":
                    break
                else:
                    # Append data on serial buffer
                    bSerialBuffer += np_arrbSharedMemory[0, iPosition]

                    # Increase position counter
                    iPosition += 1

            # Close shared memory
            sharedMemoryLocation.close()

            # Release allocated memory
            sharedMemoryLocation.unlink()

            # End process
            self._process.close()

            # Decode the string containing the file path and return it
            return bSerialBuffer.decode("UTF-8")

        elif self._FreiStatMode == FREISTAT_BACKEND:
            # Create an object for plotting the data
            self._plotter = Plotter(
                strMethod,
                self._listExperimentParameters,
                FREISTAT_BACKEND,
                self._process,
            )

            # Initialize plot
            self._plotter.initPlot()

            # Save data queue to prevent dumping it, because scope is left
            self._dataQueue = dataQueue

            # Save shared memory to prevent dumpint it, because scope is left
            self._sharedMemoryLocation = sharedMemoryLocation

"""
Module which implements a facade interface for measuring the open circuit
potential.

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
from ..data_storage.constants import *
from ..plotter.plotter import Plotter
from .run_electrochemical_method import Run_Electrochemical_Method
from ..utility.encoder import _encode_Bool_Flag
from ..utility.encoder import _encode_LPTIA_Resistor_Size
from ..utility.encoder import _encode_Sinc_Oversampling_Rate


class Run_OCP(Run_Electrochemical_Method):
    """
    Description
    -----------
    Facade which implements measurement of the open circuit potential on the
    FreiStat.

    """

    def start(
        self,
        Measurement_Length: float = PULSE_LENGTH_F,
        Sampling_Rate: float = SAMPLING_RATE_F,
        Cycle: int = CYCLE_I,
        MainsFilter: bool = False,
        Sinc2_Oversampling: int = SINC2_OVERSAMPLING_I,
        Sinc3_Oversampling: int = SINC3_OVERSAMPLING_I,
        LowPerformanceMode: bool = False,
    ) -> str:
        """
        Description
        -----------
        Start measuring the open circuit potential with the defined parameters.

        Parameters
        ----------
        `Measurement_Length` : float
            Measurement length of the ocp in ms.

        `Sampling_Rate` : float
            Sampling rate defining the distance between measurement results
            in ms.

        `Cycle` : int
            Amount of cycles the CV should run

        `MainsFilter` : bool
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

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate(
            "Sinc2", Sinc2_Oversampling, self._logger
        )

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate(
            "Sinc3", Sinc3_Oversampling, self._logger
        )

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Safe experiment parameters in correct list format for measuring OCP
        self._listExperimentParameters = [
            [PULSE_LENGTH, Measurement_Length],
            [SAMPLING_RATE, Sampling_Rate],
            [CYCLE, Cycle],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING, Sinc3_Oversampling],
        ]

        # Define electrochemical method
        strMethod: str = OCP

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

        if LowPerformanceMode == False:
            # Create an object for plotting the data
            self._plotter = Plotter(strMethod, self._listExperimentParameters)

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

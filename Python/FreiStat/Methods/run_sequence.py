"""
Module which implements a facade interface for executing sequences of 
electrochemical methods.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging
import multiprocessing  as mp
from multiprocessing import shared_memory
import numpy as np

# Import internal dependencies
from ..Data_storage.constants import *
from ..Serial_communication.serial_communication import Communication
from ..Data_storage.data_handling import DataHandling
from ..Data_storage.data_software_storage import DataSoftwareStorage
from ..Electrochemical_methods.electrochemical_method import ElectrochemicalMethod
from ..JSON_parser.json_parser import JSON_Parser
from ..Plotter.plotter import Plotter
from .run_electrochemical_method import Run_Electrochemical_Method
from ..Utility.optimizer import Optimizer
from ..Utility.encoder import _encode_Bool_Flag
from ..Utility.encoder import _encode_LPTIA_Resistor_Size
from ..Utility.encoder import _encode_Sinc_Oversampling_Rate

class Run_Sequence(Run_Electrochemical_Method):
    """
    Description
    -----------
    Facade which implements a sequence of electrochemical methods.

    """

    def __init__(self, 
                 logger = logging.Logger("FreiStat_Library"),
                 commnicationMode = FREISTAT_SERIAL,
                 wlanSetting = [FREISTAT_UDP_SERVER_IP, 
                                FREISTAT_UDP_SERVER_PORT,
                                FREISTAT_UDP_CLIENT_IP,
                                FREISTAT_UDP_CLIENT_PORT],
                 EnableOptimizer : bool = True, 
                 mode: str = FREISTAT_STANDALONE ) -> None:
        """
        Description
        -----------
        Constructor of the class Run_Sequence

        Parameter
        ---------
        `logger` : logging.Logger
            Logger which should be used in the library

        `commnicationMode` : int
            Integer flag encoding if Python library communicates via serial (1)
            or WiFi (2)

        `wlanSetting` : list
            [Server IP (str), Server Port (int), Client IP (str), Client Port(int)]

        `EnableOptimizer` : bool
            Enables the optimizer, which tunes automatically the experiment
            parameters to fit best the performance of the FreiStat
        
        `mode` : string
            String defining in which mode the FreiStat library should be used
            Defined: "standalone", "backend"

        """
        # Initialize class variable
        self._logger= logger
        self._iCommunicationMode = commnicationMode
        self._listWLANSetting = wlanSetting

        self._iSetupFailed : int = 0

        self._listEcMethod : list = []

        # Creating an object which stores all references to other objects
        self._dataSoftwareStorage = DataSoftwareStorage()

        # Create an object which handles all data
        self._dataHandling = DataHandling(self._dataSoftwareStorage)

        # Create an object for parsing JSON strings
        self._jsonParser = JSON_Parser(self._dataSoftwareStorage)

        # Save if optimizer should be used for methods
        self._bEnableOptimizer = EnableOptimizer

        # Check if mode is defined
        if (mode == FREISTAT_STANDALONE or mode == FREISTAT_BACKEND):
            self._FreiStatMode = mode
        else:
            raise RuntimeError("Operation mode doesn't exist.")

    def start(self, 
              SequenceCycles : float = CYCLE_I,
              LowPerformanceMode : bool = False) -> str :
        """
        Description
        -----------
        Start sequence of electrochemical methods
        
        Parameters
        ----------
        `SequenceCycles` : int                  
            Amount of cycles the sequence of ec-methods should run

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

        iPosition : int = 0

        # Check sequence length
        if (len(self._listEcMethod) <= 1):
            self._logger.error("Sequence to short, add more methods via the \"Run" +
                          "Sequence.add___()\" methods. If you want to use " +
                          "only one method, use the other Run___ methods.")
            return ""

        # Check sequence length
        if (len(self._listEcMethod) > FREISTAT_SEQUENCE_LENGTH):
            self._logger.error("Sequence to long, remove methods. Sequence length " +
                          "of " + str(len(self._listEcMethod)) + 
                          " is larger then maximum length of " +
                          str(FREISTAT_SEQUENCE_LENGTH))
            return ""

        # Check if setups were successful
        if (self._iSetupFailed != 0):
            self._logger.error("Setup error: sequence.add___ method number " + 
                          str(self._iSetupFailed) + " failed. Check parameters" + 
                          " and try again.")
            return ""  

        # Safe experiment parameters in correct list format for cyclic voltammetry
        listTempExperimentParameters = [
            [SEQUENCE_LENGTH, len(self._listEcMethod)],
            [CYCLE, SequenceCycles]
        ]
        
        # Define electrochemical method
        strMethod = SEQUENCE

        # Unbound the logger
        self._logger = None

        # Define start method for multiprocessing using helper method
        mp.get_context(self._check_OsProcess())

        # Define data queue
        manager = mp.Manager()
        dataQueue = manager.Queue()

        # Define location in the RAM as shared memory between processes
        sharedMemoryLocation = shared_memory.SharedMemory(create= True, size= 200)

        # Define byte array which accesses the prviously defined memory spacee
        np_arrbSharedMemory = np.ndarray((1,200), dtype='|S1', 
            buffer=sharedMemoryLocation.buf)

        # Define a process which deals with the reading of the data from the 
        # Serial connection and the storage of the data
        self._process = mp.Process(target= self.P_DataCollection, 
                                   args=(dataQueue, 
                                         SequenceCycles,
                                         listTempExperimentParameters,
                                         LowPerformanceMode,  
                                         sharedMemoryLocation.name))

        # Start the process                                                 
        self._process.start() 

        # Add cycles and names of the different sequence methods to the 
        # experiment parameters for the plotter
        listTempExperimentParameters.append([])

        # Move to first object
        self._dataHandling.move_first_DataObject()

        # Loop over all objects
        for iIndex in range(len(self._listEcMethod)):
            # Find experiment parameter for cycle
            listTempParameters = self._dataHandling.get_ExperimentParameters()

            for iPosition in range(len(listTempParameters)):
                if (listTempParameters[iPosition][0] == CYCLE):
                    iCycle = listTempParameters[iPosition][1]

            # Append parameters
            listTempExperimentParameters[len(listTempExperimentParameters) - 1]. \
                append([self._dataHandling.get_ExperimentType(), iCycle, 
                        listTempParameters])

            # Move to next object
            self._dataHandling.move_next_DataObject()

        # Check if Library is used as interface backend or not
        if(self._FreiStatMode == FREISTAT_STANDALONE):

            if (LowPerformanceMode == False):
                # Create an object for plotting the data
                self._plotter = Plotter(strMethod, listTempExperimentParameters,
                                        FREISTAT_STANDALONE, self._process)

                # Initialize plot
                self._plotter.initPlot()

                #  Attack press event to plot window
                self._plotter.attachEvent(self._pressEvent)

                # Start display data in main process and hand reference to dataQueue
                self._plotter.T_Animate(dataQueue)

                # Join process (Blocks until process is done)
                self._process.join()
            else :
                # Join process (Blocks until process is done)
                self._process.join()

            # Loop until the end of the shared memory array
            while (True):
                # Check if the array ended
                if (np_arrbSharedMemory[0, iPosition] == b''):
                    break
                else :
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

        elif (self._FreiStatMode == FREISTAT_BACKEND):
            # Create an object for plotting the data
            self._plotter = Plotter(strMethod, listTempExperimentParameters,
                                    FREISTAT_BACKEND, self._process)

            # Initialize plot
            self._plotter.initPlot()

            # Save data queue to prevent dumping it, because scope is left
            self._dataQueue = dataQueue

            # Save shared memory to prevent dumpint it, because scope is left
            self._sharedMemoryLocation = sharedMemoryLocation

    def P_DataCollection(self, 
                         dataQueue : mp.Queue, 
                         SequenceCycles : int,
                         listTempExperimentParameters : list, 
                         bLowPerformanceMode : bool,
                         sharedMemoryLocation_name : str) -> None:
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
        # Create an object for handling communication
        self._serialConnection = Communication(self._dataSoftwareStorage,
                                               self._iCommunicationMode,
                                               self._listWLANSetting)

        # Save the low performance mode flag
        self._dataSoftwareStorage.set_LowPerformanceMode(bLowPerformanceMode)

        # Creating an object for general electrochemical methods
        self._ecMethod = ElectrochemicalMethod(SEQUENCE, self._dataSoftwareStorage)
        
        # Safe experiment parameters in correct list format for cyclic voltammetry
        listTempExperimentParameters = [
            [SEQUENCE_LENGTH, len(self._listEcMethod)],
            [CYCLE, SequenceCycles]
        ]

        # Execute setup of the sequence
        # Enable sequence mode in the microcontroller
        # Transmit sequence related parameters to the controller
        self._ecMethod.setup(listTempExperimentParameters)

        # Move to the first stored data object in the list
        self._dataHandling.move_first_DataObject()

        # Iterate over every method in the sequence
        for iPosition in range(len(self._listEcMethod)):         
            # Run execute behavior (only send the first two telegrams)
            # 1. Experiment type
            # 2. Experiment parameters
            self._listEcMethod[iPosition].execute(dataQueue, iTelegrams= 2, 
                bEnableReading= False, bPorgressiveMesurement= False)

            # Move to the next stored data object
            self._dataHandling.move_next_DataObject()

        # Move to the first stored data object in the list
        self._dataHandling.move_first_DataObject()

        # Execute behavior for sequence needed -> thread reading data 
        # Persistant data export , etc. 
        self._ecMethod.execute(dataQueue, iTelegrams= 2, bEnableReading= True,
            bPorgressiveMesurement= False)

        # Set system status to starting experiment
        self._dataSoftwareStorage.set_SystemStatus(FREISTAT_EXP_STARTED)

        # Post processing of experiment data
        # Move to the first stored data object in the list
        self._dataHandling.move_first_DataObject()

        # Loop over all files
        for iPosition in range(len(self._listEcMethod)):
            # Export data to previously setup csv export after experiment is done
            strExportPath = self._dataHandling. \
                export_Data_csv(self._dataHandling.get_StoredData())

            # Save data object persistent
            self._dataHandling.export_DataStorage()

            # Export experiment parameters 
            self._dataHandling.export_ExperimentParameters_csv(
                self._dataHandling.get_ExperimentParameters())

            # Move to the next stored data object
            self._dataHandling.move_next_DataObject()

        # Get address of the shared memory in the RAM through the unique name
        existing_shm = shared_memory.SharedMemory(
            name=sharedMemoryLocation_name, create= False)

        # Define array which is located in the shared memory and has the right
        # data format
        np_array = np.ndarray((1,200), dtype='|S1', buffer=existing_shm.buf)
        
        # Loop over the length of the file-path and write it into the array in
        # the shared memory
        for i in range(len(strExportPath)):
            np_array[0,i] = strExportPath[i].encode("UTF-8")

        # Close exisitng serial connection
        self._serialConnection._closeConnection()

    def add_CV(self,
               StartVoltage : float = START_POTENTIAL_F , 
               FirstVertex : float = LOWER_POTENTIAL_F, 
               SecondVertex : float = UPPER_POTENTIAL_F, 
               Stepsize : float = STEP_SIZE_F,
               Scanrate : float = SCAN_RATE_F,
               Cycle : int = CYCLE_I,
               CurrentRange : float = CURRENT_RANGE_F,
               FixedWEPotential: bool = True,
               MainsFilter: bool = False,
               Sinc2_Oversampling: int = SINC2_OVERSAMPLING_I,
               Sinc3_Oversampling: int = SINC3_OVERSAMPLING_I) -> None :
        """
        Description
        -----------
        Method for adding cyclic voltammetry to the sequence of electrochemical
        methods.
        
        Parameters
        ----------
        `StartVoltage` : float
            Potential at which FreiStat should start in V

        `FirstVertex` : float
            Potential of the first vertex in V

        `SecondVertex` : float
            Potential of the second vertex in V

        `Stepsize` : float               
            In V

        `Scanrate` : float               
            In V/s

        `Cycle` : int                  
            Amount of cycles the CV should run

        `CurrentRange` : float
            Current range of the electrochemical cell in A

        `FixedWEPotential` : bool
            Defines if the we electrode potential should be fixed to the middle
            of the dynamic range (1.3 V) or not.

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

        """
        # Convert parameters from SI-units to internal units
        StartVoltage = StartVoltage * 1000.0
        FirstVertex = FirstVertex * 1000.0
        SecondVertex = SecondVertex * 1000.0
        Stepsize = Stepsize * 1000.0
        Scanrate = Scanrate * 1000.0
        CurrentRange = CurrentRange * 1e6

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Check stepsize and return real stepsize used by FreiStat
        fStepsize = self._check_StepSize(Stepsize)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check stepsize and return real stepsize used by FreiStat
        fStepsize = self._check_StepSize(fStepsize)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(
            StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to 
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, FirstVertex, SecondVertex)

        # Safe experiment parameters in correct list format for cyclic voltammetry
        listTempExperimentParameters = [
            [START_POTENTIAL, fStartVoltage],
            [LOWER_POTENTIAL, SecondVertex],
            [UPPER_POTENTIAL, FirstVertex],
            [STEP_SIZE, fStepsize],
            [SCAN_RATE, Scanrate],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]

        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(CV, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            CV, self._dataSoftwareStorage))
        
        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1]. \
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("CV setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)

    def add_LSV(self, 
                StartVoltage : float = START_POTENTIAL_F , 
                StopVoltage : float = LOWER_POTENTIAL_F, 
                Stepsize : float = STEP_SIZE_F,
                Scanrate : float = SCAN_RATE_F,
                Cycle : int = CYCLE_I,
                CurrentRange : float = CURRENT_RANGE_F,
                FixedWEPotential : bool = True,
                MainsFilter : bool = False,
                Sinc2_Oversampling : int = SINC2_OVERSAMPLING_I,
                Sinc3_Oversampling : int = SINC3_OVERSAMPLING_I) -> None :
        """
        Description
        -----------
        Start linear sweep voltammetry with defined parameters.
        
        Parameters
        ----------
        `StartVoltage` : float
            Potential at which FreiStat should start in V

        `StopVoltage` : float
            Potential at which FreiStat should stop in V

        `Stepsize` : float               
            In V

        `Scanrate` : float               
            In V/s

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

        """
        # Convert parameters from SI-units to internal units
        StartVoltage = StartVoltage * 1000.0
        StopVoltage = StopVoltage * 1000.0
        Stepsize = Stepsize * 1000.0
        Scanrate = Scanrate * 1000.0
        CurrentRange = CurrentRange * 1e6

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Check stepsize and return real stepsize used by FreiStat
        fStepsize = self._check_StepSize(Stepsize)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check stepsize and return real stepsize used by FreiStat
        fStepsize = self._check_StepSize(fStepsize)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(
            StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to 
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, StartVoltage, StopVoltage)

        # Safe experiment parameters in correct list format for cyclic voltammetry
        listTempExperimentParameters = [
            [START_POTENTIAL, fStartVoltage],
            [STOP_POTENTIAL, StopVoltage],
            [STEP_SIZE, fStepsize],
            [SCAN_RATE, Scanrate],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]

        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(LSV, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            LSV, self._dataSoftwareStorage))
        
        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1]. \
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("LSV setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)

    def add_CA(self, 
               Potential_Steps : list = [START_POTENTIAL_F, 
                                         LOWER_POTENTIAL_F, 
                                         UPPER_POTENTIAL_F],
               Pulse_Lengths : list = [PULSE_LENGTH_F, 
                                       PULSE_LENGTH_F, 
                                       PULSE_LENGTH_F],
               Sampling_Rate : float = SAMPLING_RATE_F,    
               Cycle : int = CYCLE_I,
               CurrentRange : float = CURRENT_RANGE_F,
               MainsFilter : bool = False,
               Sinc2_Oversampling: int = SINC2_OVERSAMPLING_I,
               Sinc3_Oversampling: int = SINC3_OVERSAMPLING_I,
               Progressive_Measurement: bool = False) -> str :
        """
        Description
        -----------
        Start chronpampormetry with defined parameters.
        
        Parameters
        ----------
        `Potential_Steps` : list
            List containing all potential steps in V, which get executed after
            another.

        `Pulse_Lengths` : list
            List containing the corresponding pulse lengths of every potential
            step in s.

        `Sampling_Rate` : float
            Sampling rate defining the distance between measurement results 
            in s.

        `Cycle` : int                  
            Amount of cycles the CV should run

        `CurrentRange` : float
            Current range of the electrochemical cell in A

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

        `Progressive_Measurement` : bool
            Defines if different Cycles should be plotted above each other or
            after each other

        """
        # Initialize variables
        listPotential_Steps : list = []
        listPulse_Lengths : list = []

        # Convert parameters from SI-units to internal units
        for iIndex in range(len(Pulse_Lengths)):
            listPulse_Lengths.append(Pulse_Lengths[iIndex] * 1000.0)

        Sampling_Rate = Sampling_Rate * 1000.0
        CurrentRange = CurrentRange * 1e6

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check potentail steps and return real potential steps used by
        # FreiStat
        for iSteps in range (len(Potential_Steps)):
            listPotential_Steps.append(self._check_StartingPotential(
                                        Potential_Steps[iSteps] * 1000.0, 1))

        # Safe experiment parameters in correct list format for chronoamperometry
        listTempExperimentParameters = [
            [POTENTIAL_STEPS, listPotential_Steps],
            [PULSE_LENGTH,  listPulse_Lengths],
            [SAMPLING_RATE, Sampling_Rate],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]
        print(listTempExperimentParameters)
        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(CA, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            CA, self._dataSoftwareStorage))
        
        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1].\
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("CA setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)

    def add_NPV(self, 
                BaseVoltage : float = BASE_POTENTIAL_F,
                StartVoltage : float = START_POTENTIAL_F ,
                StopVoltage : float = STOP_POTENTIAL_F,
                DeltaV_Staircase : float = DELTA_V_STAIRCASE_F,
                Pulse_Lengths : list = [PULSE_LENGTH_F, 
                                        PULSE_LENGTH_F],
                Sampling_Duration : float = SAMPLING_DURATION_F,
                Cycle : int = CYCLE_I,
                CurrentRange : float = CURRENT_RANGE_F,
                FixedWEPotential : bool = True,
                MainsFilter : bool = False,
                Sinc2_Oversampling : int = SINC2_OVERSAMPLING_I,
                Sinc3_Oversampling : int = SINC3_OVERSAMPLING_I) -> str :
        """
        Description
        -----------
        Method for adding normal pulse voltammetry to the sequence of 
        electrochemical methods.
        
        Parameters
        ----------
        `BaseVoltage` : float
            Base Potential which the FreiStat should apply in V

        `StartVoltage` : float
            Potential at which FreiStat should start in V

        `StopVoltage` : float
            Potential at which FreiStat should stop in V

        `DeltaV_Staircase` : float
            Increase of the base potential of the underlying staircase function
            with every cycle in V

        `Pulse_Lengths` : list
            List [Tau', Tau] containing the corresponding pulse lengths of every 
            potential step in s.

        `Sampling_Duration` : float
            Time in s, which defines how long values should be sampled at the 
            end of each potential step

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

        Return
        ------
        `ExportedFilePath` : string
            Returns system path to the location where the experiment data and
            experiment parameters where stored after the experiment

        """
        # Initialize variables
        listPulse_Lengths : list = []

        # Convert parameters from SI-units to internal units
        BaseVoltage = BaseVoltage * 1000.0
        StartVoltage = StartVoltage * 1000.0
        StopVoltage = StopVoltage * 1000.0
        DeltaV_Staircase = DeltaV_Staircase * 1000.0
        Sampling_Duration = Sampling_Duration * 1000.0
        CurrentRange = CurrentRange * 1e6

        for iIndex in range(len(Pulse_Lengths)):
            listPulse_Lengths.append(Pulse_Lengths[iIndex] * 1000.0)

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check base potential and return real base potential used by FreiStat
        fBaseVoltage = self._check_StartingPotential(
            BaseVoltage, iFixedWEPotential)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(
            StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to 
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, StartVoltage, StartVoltage)

        # Safe experiment parameters in correct list format for differential 
        # pulse voltammetry
        listTempExperimentParameters = [
            [BASE_POTENTIAL, fBaseVoltage],
            [START_POTENTIAL, fStartVoltage],
            [STOP_POTENTIAL, StopVoltage],
            [DELTA_V_STAIRCASE, DeltaV_Staircase],
            [PULSE_LENGTH,  listPulse_Lengths],
            [SAMPLING_DURATION, Sampling_Duration],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]

        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(NPV, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            NPV, self._dataSoftwareStorage))

        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1]. \
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("NPV setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)           

    def add_DPV(self, 
                StartVoltage : float = START_POTENTIAL_F ,
                StopVoltage : float = STOP_POTENTIAL_F,
                DeltaV_Staircase : float = DELTA_V_STAIRCASE_F,
                DeltaV_Peak : float = DELTA_V_PEAK_F,
                Pulse_Lengths : list = [PULSE_LENGTH_F, 
                                        PULSE_LENGTH_F],
                Sampling_Duration : float = SAMPLING_DURATION_F,
                Cycle : int = CYCLE_I,
                CurrentRange : float = CURRENT_RANGE_F,
                FixedWEPotential : bool = True,
                MainsFilter : bool = False,
                Sinc2_Oversampling : int = SINC2_OVERSAMPLING_I,
                Sinc3_Oversampling : int = SINC3_OVERSAMPLING_I) -> str :
        """
        Description
        -----------
        Method for adding differential pulse voltammetry to the sequence of 
        electrochemical methods.
        
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

        `Pulse_Lengths` : list
            List [Tau', Tau] containing the corresponding pulse lengths of every 
            potential step in s.

        `Sampling_Duration` : float
            Time in s, which defines how long values should be sampled at the 
            end of each potential step

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

        Return
        ------
        `ExportedFilePath` : string
            Returns system path to the location where the experiment data and
            experiment parameters where stored after the experiment

        """    
        # Initialize variables
        listPulse_Lengths : list = []

        # Convert parameters from SI-units to internal units
        StartVoltage = StartVoltage * 1000.0
        StopVoltage = StopVoltage * 1000.0
        DeltaV_Staircase = DeltaV_Staircase * 1000.0
        DeltaV_Peak = DeltaV_Peak * 1000.0
        Sampling_Duration = Sampling_Duration * 1000.0
        CurrentRange = CurrentRange * 1e6

        for iIndex in range(len(Pulse_Lengths)):
            listPulse_Lengths.append(Pulse_Lengths[iIndex] * 1000.0)

        # Calculate the LPTIA Rtia size based on the current range
        fLptiaRtia = 0.9 / CurrentRange * 1e6

        # Translate LPTIA Rtia into integer value
        fLptiaRtia = _encode_LPTIA_Resistor_Size(fLptiaRtia, self._logger)

        # Translate Sinc2 oversampling rate into integer value
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(
            StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to 
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, StartVoltage, StartVoltage)

        # Safe experiment parameters in correct list format for differential 
        # pulse voltammetry
        listTempExperimentParameters = [
            [START_POTENTIAL, fStartVoltage],
            [STOP_POTENTIAL, StopVoltage],
            [DELTA_V_STAIRCASE, DeltaV_Staircase],
            [DELTA_V_PEAK, DeltaV_Peak],
            [PULSE_LENGTH,  listPulse_Lengths],
            [SAMPLING_DURATION, Sampling_Duration],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]

        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(DPV, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            DPV, self._dataSoftwareStorage))
        
        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1]. \
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("DPV setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)

    def add_SWV(self, 
                StartVoltage : float = START_POTENTIAL_F ,
                StopVoltage : float = STOP_POTENTIAL_F,
                DeltaV_Staircase : float = DELTA_V_STAIRCASE_F,
                DeltaV_Peak : float = DELTA_V_PEAK_F,
                DutyCycle : float = PULSE_LENGTH_F,
                Sampling_Duration : float = SAMPLING_DURATION_F,
                Cycle : int = CYCLE_I,
                CurrentRange : float = CURRENT_RANGE_F,
                FixedWEPotential : bool = True,
                MainsFilter : bool = False,
                Sinc2_Oversampling : int = SINC2_OVERSAMPLING_I,
                Sinc3_Oversampling : int = SINC3_OVERSAMPLING_I) -> str :
        """
        Description
        -----------
        Method for adding square wave voltammetry to the sequence of 
        electrochemical methods.
        
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

        Return
        ------
        `ExportedFilePath` : string
            Returns system path to the location where the experiment data and
            experiment parameters where stored after the experiment

        """
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
        Sinc2_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc2", 
                                Sinc2_Oversampling, self._logger)

        # Translate Sinc3 oversampling rate into integer value
        Sinc3_Oversampling = _encode_Sinc_Oversampling_Rate("Sinc3",
                                Sinc3_Oversampling, self._logger)

        # Translate bool of FixedWEPotential into integer
        iFixedWEPotential = _encode_Bool_Flag(FixedWEPotential)

        # Translate bool of MainsFilter into integer
        iMainsFilter = _encode_Bool_Flag(MainsFilter)

        # Check starting potential and return real starting potential used by
        # FreiStat
        fStartVoltage = self._check_StartingPotential(
            StartVoltage, iFixedWEPotential)

        # Check if bool flag for FixedWEPotential must be changed due to 
        # required sweap range
        iFixedWEPotential = self._check_FixedWEPotential(
            iFixedWEPotential, StartVoltage, StartVoltage)

        # Safe experiment parameters in correct list format for differential 
        # pulse voltammetry
        listTempExperimentParameters = [
            [START_POTENTIAL, fStartVoltage],
            [STOP_POTENTIAL, StopVoltage],
            [DELTA_V_STAIRCASE, DeltaV_Staircase],
            [DELTA_V_PEAK, DeltaV_Peak],
            [PULSE_LENGTH,  [DutyCycle / 2, DutyCycle / 2]],
            [SAMPLING_DURATION, Sampling_Duration],
            [CYCLE, Cycle],
            [LPTIA_RTIA_SIZE, fLptiaRtia],
            [FIXED_WE_POTENTIAL, iFixedWEPotential],
            [MAINS_FILTER, iMainsFilter],
            [SINC2_OVERSAMPLING, Sinc2_Oversampling],
            [SINC3_OVERSAMPLING ,Sinc3_Oversampling]
        ]

        # Check if optimizer is enabled
        if (self._bEnableOptimizer == True):
            # Optimize experiment parameters
            # Create instance of the optimizer class
            _Optimizer = Optimizer(self._logger, self._iCommunicationMode)

            # Start optimization
            iErrorcode = _Optimizer.start(SWV, listTempExperimentParameters)

            # Check for error
            if (iErrorcode != EC_NO_ERROR):
                self._logger.warning("Optimizer: Failed - Errorcode: "  + 
                                str(iErrorcode) +
                                " - FreiStat tries to run experiment with user " + 
                                "defined parameters")
            else:
                # Overwrite parameters with optimized ones
                listTempExperimentParameters = _Optimizer.return_Parameters()

        # Creating an object for general electrochemical methods
        self._listEcMethod.append(ElectrochemicalMethod(
            SWV, self._dataSoftwareStorage))
        
        # Execute setup for electrochemical methods
        iErrorCode = self._listEcMethod[len(self._listEcMethod) - 1]. \
            setup(listTempExperimentParameters)

        # Check if setup was successfull
        if(iErrorCode != 0):
            self._logger.warning("SWV setup failed: Error code: " + str(iErrorCode) +
                            " Check error list for further informations.")
            self._iSetupFailed = len(self._listEcMethod)
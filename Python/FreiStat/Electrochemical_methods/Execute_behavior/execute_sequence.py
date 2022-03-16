"""
Module implementing the behavior of the execute function for running a sequence
of different electrochemical methods.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import multiprocessing as mp
from multiprocessing.queues import Queue

# Import internal dependencies
from ...Data_storage.constants import *
from .execute_behavior import ExecuteBehavior

class ExecuteSequence(ExecuteBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the execute function when
    running a sequence of electrochemical methods.
    
    """

    def execute(self, 
                dataQueue : Queue,
                event : mp.Event(),
                iTelegrams : int = 2,
                bEnableReading : bool = True,
                bPorgressiveMesurement : bool = False) -> int:
        """
        Description
        -----------
        Execute method for conducting a sequence of electrochemical methods.

        Parameters
        ----------
        `dataQueue` : Queue
            Data queue which is used as a pipe between processes

        `event` : Event
            Multiprocessing event to indicate termination event

        `iTelegrams` : int 
            Number of telegrams which should be send by the ec-method

        `bEnableReading` : bool
            Flag indicating if incoming data should be read or not

        `bPorgressiveMesurement` : bool
            Flag indicating if measurement should be displayed continously or
            if each cycle should start at time = 0

        Return
        ------
        `Errorcode` : int
           Returns error code

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description                                                                                  
        0           :   No error                                              
        12001       :   Mismatch between send command ID and received acknowledge ID

        """
        # Initialize class variables
        self._referenceTime : float = -1
        self._referenceTimeCycle : float = -1
        self._referenceTimeSequenceCycle : float = -1

        self._event = event

        # Intialize variables
        bErrorflag : bool = False

        iCurrenPosition : int = 0

        strReadTelegram : str = ""

        listReadData : list = []
        listCommandIDs: list = [COMMAND_EXS, COMMAND_EXC]
        listCommandSubIDs : list = [2, FREISTAT_START_I]

        # Save reference to serial connection object
        self._serialConnection = self._dataSoftwareStorage.getCommunication()
        self._communicationMode = self._serialConnection.get_CommunicationMode()

        # Execute the following commands after another
        # 1: Disable sequence mode
        # 2: Send start command
        for iIndex in range (iTelegrams):
            # Write command telegram with command type
            self._serialConnection.write_Data(self._jsonTelegramGenerator
            .generateCommandTelegram(listCommandIDs[iIndex], 
                                     listCommandSubIDs[iIndex])[1])

            # Wait as long as the input buffer is empty
            while(self._communicationMode == FREISTAT_SERIAL):
                if (self._serialConnection.get_SerialConnection().in_waiting > 0):
                    break
                pass

            # Read acknowledge telegram
            strReadTelegram = self._serialConnection.read_Data("JSON"). \
                decode("utf-8")

            # Print received acknowledge telegram
            print(strReadTelegram)

            # Parse read telegram
            iCurrenPosition, bErrorflag, listReadData = \
            self._jsonParser.parse_JSON_string(listReadData, strReadTelegram)

            # Compare code to previously send code
            if (int(listReadData[0][1]) != listCommandIDs[iIndex]):
                return EC_EXECUTE + EC_EX_C_A_MISMATCH

            # Reset list containing read data
            listReadData = []

        # Check if reading is enabled
        if (bEnableReading == True):
            # Start thread to handle data exchange
            self._handleSequenceData(dataQueue)

        # No error occured
        return EC_NO_ERROR

    def _handleSequenceData(self, dataQueue) -> None:
        """
        Description
        -----------
        Function which runs periodically to read data from serialConnection, 
        parse them and store the data.

        Parameters
        ----------
        `dataQueue` : Queue
            Data queue which is used as a pipe between processes

        """
        # Initialize variables
        iDataPoint : int = 0
        iMethodCount : int = 1
        iSequenceCycle : int = 1

        fCurrent : float = 0
        fVoltage : float = 0
        fTimeStamp : float = 0

        strRun : str = ""
        strReadData : str = ""

        while(True):
            # Initialize variables
            listReadData : list = []

            while(self._communicationMode == FREISTAT_SERIAL):
                if(self._serialConnection.get_SerialConnection().in_waiting > 0):
                    break
                pass

            # Read byte stream from serial connection and convert into string
            strReadData = self._serialConnection.read_Data("JSON").decode("utf-8")

            iCurrenPosition, bErrorflag, listReadData = \
            self._jsonParser.parse_JSON_string(listReadData, strReadData)

            # Set FreiStat status into running if not done yet
            if (self._dataSoftwareStorage.get_SystemStatus() == FREISTAT_EXP_STARTED):
                # Set system status to starting experiment
                self._dataSoftwareStorage.set_SystemStatus(FREISTAT_EXP_RUNNING)

            # Check if termination event occured
            if (self._event.is_set()):
                # Send stop command
                self._serialConnection.write_Data("{\"C\":3,\"ExC\":\"Stop\"}")

                while(self._serialConnection.get_SerialConnection().in_waiting > 0 and
                    self._iCommunicationMode == FREISTAT_SERIAL):
                    # Read JSON-telegram
                    self._serialConnection.read_Data("JSON").decode("utf-8")
                    
                # Export data storage object
                self._dataHandling.export_DataStorage()      
                break    

            # Check if send telegram is a data telegram
            if (listReadData[0][0] == ("\"" + RUN + "\"")):
                # Set reference time for the whole experiment
                if (self._referenceTime == -1):
                    self._referenceTime = float(listReadData[1][1][3][1])

                # Set reference time for the cycle of the ec-method
                if (self._referenceTimeCycle == -1):
                    self._referenceTimeCycle = float(listReadData[1][1][3][1])

                # Set reference time for the cycle of the sequence
                if (self._referenceTimeSequenceCycle == -1):
                    self._referenceTimeSequenceCycle = \
                        float(listReadData[1][1][3][1])

                # Check if a new run started
                if (strRun != listReadData[0][1]):
                    # Export data storage object
                    self._dataHandling.export_DataStorage()

                    # Reset reference time for a cycle
                    self._referenceTimeCycle = float(listReadData[1][1][3][1]) 

                    # Check if low performance mode is enabled
                    if (self._lowPerformaneMode == True):
                        print("Cycle: " + strRun)

                # Check if new method has started
                if (iDataPoint > int(listReadData[1][1][0][1])):
                    # Catching case a method only has one cycle
                    # Export data storage object
                    self._dataHandling.export_DataStorage()

                    # Reset reference time for a cycle
                    self._referenceTimeCycle = float(listReadData[1][1][3][1]) 

                    # Check if new sequence cycle has started
                    if (iMethodCount % (self._dataHandling.\
                        get_SequenceLength() - 1) == 0):
                        # Increase sequence cycle
                        iSequenceCycle += 1

                        # Reset reference time of sequence cycle
                        self._referenceTimeSequenceCycle = \
                            float(listReadData[1][1][3][1])

                    # Increase method counter
                    iMethodCount += 1

                    # Move to next data storage element
                    self._dataHandling.move_next_DataObject()

                # Get current run
                strRun = listReadData[0][1]

                # Get datapoint
                iDataPoint = int(listReadData[1][1][0][1])

                # Convert data
                fCurrent = float(listReadData[1][1][2][1])          
                fVoltage = float(listReadData[1][1][1][1])        
                fTimeStamp : float = float(listReadData[1][1][3][1]) - \
                    self._referenceTimeCycle

                # Add data to data storage
                self._dataHandling.append_StoredData(
                                [iSequenceCycle,
                                int(strRun,10),
                                iDataPoint,
                                fVoltage,
                                fCurrent,
                                fTimeStamp,
                                float(listReadData[1][1][3][1]) - 
                                self._referenceTimeSequenceCycle,
                                float(listReadData[1][1][3][1]) - 
                                self._referenceTime])
                
                # Add data to dataQueue
                dataQueue.put([iSequenceCycle,
                                int(strRun,10),
                                iDataPoint,
                                fVoltage,
                                fCurrent,
                                fTimeStamp,
                                float(listReadData[1][1][3][1]) - 
                                self._referenceTimeSequenceCycle,
                                float(listReadData[1][1][3][1]) - 
                                self._referenceTime,
                                self._dataHandling.get_ExperimentType()])

            # Check if send telegram is a command telegram
            elif (listReadData[0][0] == ("\"" + COMMAND_TELEGRAM + "\"")):
                self._dataSoftwareStorage.set_SystemStatus(FREISTAT_EXP_COMPLETED)

                # Fill in Blank command to stop plotter
                dataQueue.put([iSequenceCycle, 0, 0, 0, 0, 0, 0, 0,
                                UNDEFIEND])
                dataQueue.put([iSequenceCycle, 0, 0, 0, 0, 0, 0, 0,
                                UNDEFIEND])
                break
"""
Module implementing the behavior of the execute function for running 
chronoamperometry.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

from multiprocessing.queues import Queue
from multiprocessing.synchronize import Event

from ...data_storage.constants import *
from .execute_behavior import ExecuteBehavior


class ExecuteCA(ExecuteBehavior):
    """
    Description
    -----------
    Behavior class implementing the functionality of the execute function when
    running chronoamperometry.

    """

    def execute(
        self,
        data_queue: Queue,
        event: Event,
        telegrams: int = 3,
        enable_reading: bool = True,
        progressive_measurement: bool = False,
    ) -> int:
        """
        Description
        -----------
        Exectute method for conducting a chronoamperometry sequence.

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
            Error code encoded as integer

        -----------------------------------------------------------------------

        Error Codes
        -----------
        The following table shows all error codes

        iErrorCode  :   Description
        0           :   No error
        12001       :   Mismatch between send command ID and received acknowledge ID

        """
        self._progressive_measurement = progressive_measurement

        self._reference_time = -1

        self._event = event

        read_telegram = ""

        read_data: list = []
        command_ids: list = [COMMAND_EXT, COMMAND_EXP, COMMAND_EXC]
        command_sub_ids: list = [1, 1, FREISTAT_START_I]

        # Save reference to serial connection object
        self._serial_connection = (
            self._data_software_storage.get_communication()
        )  # type: ignore
        self._communication_mode = self._serial_connection.get_CommunicationMode()

        # Execute the following commands after another
        # 1: Transmit experiment type
        # 2: Transmit experiment parameters
        # 3: Send start command
        for idx in range(telegrams):
            # Write command telegram with command type
            self._serial_connection.write_Data(
                self._json_telegram_generator.generateCommandTelegram(
                    command_ids[idx], command_sub_ids[idx]
                )[1]
            )

            # Wait as long as the input buffer is empty
            while self._communication_mode == FREISTAT_SERIAL:
                if self._serial_connection.get_SerialConnection().in_waiting > 0:
                    break

            # Read acknowledge telegram
            read_telegram = self._serial_connection.read_Data("JSON").decode("utf-8")

            # Print received acknowledge telegram
            print(read_telegram)

            # Parse read telegram
            _, _, read_data = self._json_parser.parse_json_string(
                read_data, read_telegram
            )

            # Compare code to previously send code
            if int(read_data[0][1]) != command_ids[idx]:
                return EC_EXECUTE + EC_EX_C_A_MISMATCH

            # Reset list containing read data
            read_data = []

        # Check if reading is enabled
        if enable_reading:
            # Start thread to handle data exchange
            self._handle_data_ca(data_queue)

        # No error occured
        return EC_NO_ERROR

    def _handle_data_ca(self, data_queue) -> None:
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
        datapoint = 0

        current: float = 0
        voltage: float = 0
        timestamp: float = 0

        run = ""
        read_data = ""

        while True:
            # Initialize variables
            read_data_list: list = []

            while self._communication_mode == FREISTAT_SERIAL:
                if self._serial_connection.get_SerialConnection().in_waiting > 0:
                    break

            # Read byte stream from serial connection and convert into string
            read_data = self._serial_connection.read_Data("JSON").decode("utf-8")

            # Parse read data string
            _, _, read_data_list = self._json_parser.parse_json_string(
                read_data_list, read_data
            )

            # Check if system status is set to experiment startet
            if self._data_software_storage.get_system_status() == FREISTAT_EXP_STARTED:
                # Set system status to running experiment
                self._data_software_storage.set_system_status(FREISTAT_EXP_RUNNING)

            # Check if termination event occured
            if self._event.is_set():
                # Send stop command
                self._serial_connection.write_Data('{"C":3,"ExC":"Stop"}')

                while (
                    self._serial_connection.get_SerialConnection().in_waiting > 0
                    and self._communication_mode == FREISTAT_SERIAL
                ):
                    # Read JSON-telegram
                    self._serial_connection.read_Data("JSON").decode("utf-8")

                # Export data storage object
                self._data_handling.export_data_storage()
                break

            # Check if send telegram is a data telegram
            if read_data_list[0][0] == ('"' + RUN + '"'):
                # Set reference time
                if self._reference_time == -1:
                    self._reference_time = float(read_data_list[1][1][3][1])

                # Check if a new run started
                if run != read_data_list[0][1]:
                    # Export data storage object
                    self._data_handling.export_data_storage()

                    # Check if progressive measurement is disabled
                    if not self._progressive_measurement:
                        self._reference_time = float(read_data_list[1][1][3][1])

                    # Check if low performance mode is enabled
                    if self._low_performane_mode:
                        print("Cycle: " + run)

                # Get current run
                run = read_data_list[0][1]

                # Get datapoint
                datapoint = int(read_data_list[1][1][0][1], 10)

                # Convert data
                voltage = float(read_data_list[1][1][1][1])
                current = float(read_data_list[1][1][2][1])
                timestamp: float = (
                    float(read_data_list[1][1][3][1]) - self._reference_time
                )

                # Add data to data storage
                self._data_handling.append_stored_data(
                    [int(run, 10), datapoint, voltage, current, timestamp]
                )

                # Add data to dataQueue
                data_queue.put([int(run, 10), datapoint, voltage, current, timestamp])

            # Check if send telegram is a command telegram
            elif read_data_list[0][0] == ('"' + COMMAND_TELEGRAM + '"'):
                # Experiment completed, update system status
                self._data_software_storage.set_system_status(FREISTAT_EXP_COMPLETED)
                break

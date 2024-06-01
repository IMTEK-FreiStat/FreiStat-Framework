"""
Module for reading and writing on the serial port. Establishes communication 
with FreiStat via JSON strings and exchanges data.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging
import platform
import serial
import serial.tools.list_ports
import socket

# Import internal dependencies
from ..data_storage.constants import *
from ..data_storage.data_software_storage import DataSoftwareStorage


class Communication:
    """
    Descirption
    -----------
    Class to establish communication with the serial port or WiFi to receive/
    transmit data.

    """

    def __init__(
        self,
        dataSoftwareStorage: DataSoftwareStorage,
        iOperationMode: int,
        wlanSetting=[
            FREISTAT_UDP_SERVER_IP,
            FREISTAT_UDP_SERVER_PORT,
            FREISTAT_UDP_CLIENT_IP,
            FREISTAT_UDP_CLIENT_PORT,
        ],
        logger=logging.Logger("Communication"),
    ) -> None:
        """
        Description
        -----------
        Constructor of class Communication for serial communication.

        iOperationMode : 1 = Serial | 2 = WiFi | TBD

        Parameters
        ----------
        `dataSoftwareStorage` : DataSoftwareStorage
            Reference to the data software storage object

        `iOperationMode` : int
            Flag indicating in which mode the communication class is operated.

        `wlanSetting` : list
            [Server IP (str), Server Port (int), Client IP (str), Client Port(int)]

        `logger` : logging.Logger
            Logger which should be used in the library

        """
        # Save variables
        self._logger = logger

        # Initialize class variables
        self._iOperationMode: int = iOperationMode
        self._iClientPort: int = wlanSetting[3]
        self._iServerPort: int = wlanSetting[1]
        self._iSerialBaud: int = FREISTAT_SERIAL_BAUDRATE

        self._fSerialTimeout: float = FREISTAT_SERIAL_TIMEOUT

        self._strClientIP: str = wlanSetting[2]
        self._strServerIP: str = wlanSetting[0]
        self._strSerialPort: str = FREISTAT_SERIAL_PORT

        # Safe data software storage reference and save own reference
        self._dataSoftwareStorage = dataSoftwareStorage
        self._dataSoftwareStorage.setCommunication(self)

        # Check operation mode
        if self._iOperationMode == FREISTAT_SERIAL:
            # Check for available ports
            self._checkSerialPorts()

            # Establish connection with the serial port
            self._establish_SerialConnection()

        elif self._iOperationMode == FREISTAT_WLAN:
            # Establish connection wia WLAN
            self._establish_WiFiConnection()

    def _establish_SerialConnection(self) -> None:
        """
        Description
        -----------
        Establish connection to serial port.

        """
        # Create a serial object with the interal parameters and try to open a
        # connection
        try:
            self._serialConnection = serial.Serial(
                self._strSerialPort, self._iSerialBaud, timeout=self._fSerialTimeout
            )
        except:
            self._logger.error(
                "Port already in use, connection could not be established"
            )
            return

        # Try to open port until it's open
        while self._serialConnection.is_open == False:
            self._serialConnection.open()

        # Print connection setting
        self._logger.info("Connection established")
        self._logger.info(
            "Connect to port "
            + self._strSerialPort
            + " with "
            + str(self._iSerialBaud)
            + " Baud"
        )

    def _establish_WiFiConnection(self) -> None:
        """
        Description
        -----------
        Establish connection via the WiFi module

        """
        self._UdpServerSocket = socket.socket(
            family=socket.AF_INET, type=socket.SOCK_DGRAM
        )
        self._UdpServerSocket.bind((self._strServerIP, self._iServerPort))

        # Print connection setting
        self._logger.info("UDP server up and listening")

    def _closeConnection(self) -> None:
        """
        Description
        -----------
        Close open serial or WiFi connection.

        """
        # Check operation mode
        if self._iOperationMode == FREISTAT_SERIAL:
            self._serialConnection.close()

        elif self._iOperationMode == FREISTAT_WLAN:
            self._UdpServerSocket.close()

    def _checkSerialPorts(self):
        """
        Description
        -----------
        Method for checking all available ports on the operating system for the
        Adafruit microcontroller.

        """
        # Get list of available serial ports
        listSerialObjects = serial.tools.list_ports.comports()

        # Check all found serial ports
        for SerialObjects in listSerialObjects:
            # Check for vendor IDs (see constants.py)
            # Adafruit
            if SerialObjects.vid == VID_ADAFRUIT:
                # Overwrite serial port
                self._strSerialPort = SerialObjects.name

            # Arduino
            elif SerialObjects.vid == VID_ARDUINO:
                # Overwrite serial port
                self._strSerialPort = SerialObjects.name

        # Check operating system
        if platform.system() == LINUX:
            self._strSerialPort = "/dev/" + self._strSerialPort
        elif platform.system() == MACOS:
            self._strSerialPort = "/dev/" + self._strSerialPort
        elif platform.system() == WINDOWS:
            pass

    def read_Data(self, strFileFormat: str = "JSON") -> bytes:
        """
        Description
        -----------
        Method selecting correct method of reading

        Parameters
        ----------
        `strFileFormat` : string
            Defines in which format the data is been read

        Return
        ------
        `bSerialBuffer` : bytes
            Byte stream containing one JSON telegram

        """
        # Check operation mode
        if self._iOperationMode == FREISTAT_SERIAL:
            return self._read_Serial(strFileFormat)

        elif self._iOperationMode == FREISTAT_WLAN:
            return self._read_WiFi(strFileFormat)

    def _read_WiFi(self, strFileFormat: str = "JSON") -> bytes:
        """
        Description
        -----------
        Read data from the WiFi connection in the expected format

        Parameters
        ----------
        `strFileFormat` : string
            Defines in which format the data is been read

        Return
        ------
        `bSerialBuffer` : bytes
            Byte stream containing one JSON telegram

        """
        # Intialize variables
        bufferSize = 1024

        # For JSON Format
        if strFileFormat == "JSON":
            bytesAddressPair = self._UdpServerSocket.recvfrom(bufferSize)
        return bytesAddressPair[0]

    def _read_Serial(self, strFileFormat: str = "JSON") -> bytes:
        """
        Description
        -----------
        Read data from the serial port in the expected format

        Parameters
        ----------
        `strFileFormat` : string
            Defines in which format the data is been read

        Return
        ------
        `bSerialBuffer` : bytes
            Byte stream containing one JSON telegram

        """
        # For JSON Format
        if strFileFormat == "JSON":
            # Initialize variables
            iTimeoutCounter = 0
            iObjectCounter = 0
            bSerialBuffer = b""

            # Loop until iObjectCounter = 0
            while True:
                bInputByte = self._serialConnection.read(1)
                # Check for brackets
                if bInputByte == b"}":
                    iObjectCounter -= 1
                elif bInputByte == b"{":
                    iObjectCounter += 1

                # Append byte to serial buffer
                bSerialBuffer += bInputByte

                # Check if while loop can be left
                if iObjectCounter == 0:
                    break

                # Increase timout counter
                iTimeoutCounter += 1

                # Break out if faulty message is read.
                # Try to read at max 256 bytes.
                if iTimeoutCounter >= 256:
                    break

        return bSerialBuffer

    def write_Data(self, strJSONtelegram: str) -> None:
        """
        Description
        -----------
        Read data from the serial port in the expected format.

        Parameters
        ----------
        `strJSONtelegram` : string
            JSON telegram encoded as a string

        """
        # Check operation mode
        if self._iOperationMode == FREISTAT_SERIAL:
            self._serialConnection.write(strJSONtelegram.encode("utf-8"))

        elif self._iOperationMode == FREISTAT_WLAN:
            self._UdpServerSocket.sendto(
                strJSONtelegram.encode("utf-8"), (self._strClientIP, self._iClientPort)
            )

    def data_available(self) -> int:
        if self._iOperationMode == FREISTAT_SERIAL:
            return self._serialConnection.in_waiting

        elif self._iOperationMode == FREISTAT_WLAN:
            self._UdpServerSocket.inWaiting()

    # Getter methods
    def get_SerialConnection(self) -> serial.Serial:
        """
        Description
        -----------
        Get reference to serial.Serial object.

        Return
        ------
        `_serialConnection` : serial.Serial
            reference to ther serial object

        """
        return self._serialConnection

    def get_CommunicationMode(self) -> int:
        """
        Description
        -----------
        Get reference operation mode of the communication encoded as integer

        Return
        ------
        `_iOperationMode` : int
            Integer encoding the operation mode: Serial (1) | WiFi (2)

        """
        return self._iOperationMode

    def get_SerialPort(self) -> str:
        """
        Description
        -----------
        Get serial port used by the serial connection.

        Return
        ------
        `_strSerialPort` : string
            Returns port in string format (e.g. 'COM3')

        """
        return self._strSerialPort

    def get_SerialBaud(self) -> int:
        """
        Description
        -----------
        Get baudrate used by the serial connection.

        Return
        ------
        `_iSerialBaud` : int
            Returns Buadrate as integer used by the serial communication

        """
        return self._iSerialBaud

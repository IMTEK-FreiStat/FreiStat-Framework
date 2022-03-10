"""
Module containing constant naming which is used for communication between 
FreiStat and Python and general operation of the Software.

These names should not be changed!
Changed names must also be changed in the Program Code of FreiStat or the
program will recognize the data as sended by the python program as wrong.

src/data_storage/constants.c

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

"""-----------------------------------------------------------------------------
| FreiStat library mode
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_STANDALONE     = "standalone"  # The FreiStat library is used as an standalone library
FREISTAT_BACKEND        = "backend"     # The FreiStat library is used as an backend for a GUI

"""-----------------------------------------------------------------------------
| FreiStat communication mode
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_SERIAL         = 1             # FreiStat is communicating over the serial port
FREISTAT_WLAN           = 2             # FreiStat is communicating over the WiFi module

"""-----------------------------------------------------------------------------
| Operating system
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
LINUX                   = "Linux"       # Name of the linux platform
MACOS                   = "Darwin"      # Name of the macOS platform
WINDOWS                 = "Windows"     # Name of the windows platform

"""-----------------------------------------------------------------------------
| System status
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_BOOTUP         = 0             # FreiStat is in boot up process
FREISTAT_WAITING        = 1             # FreiStat in waiting state
FREISTAT_EXP_STARTED    = 2             # Start experiment
FREISTAT_EXP_RUNNING    = 3             # Experiment running
FREISTAT_EXP_COMPLETED  = 4             # Experiment completed
FREISTAT_EXP_CANCELED   = 5             # Experiment canceled

"""-----------------------------------------------------------------------------
| Electrochemical methods : Abbreviations
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
UNDEFIEND               = "UDF"         # Undefiend ec-method
SEQUENCE                = "SEQ"         # Sequence
OCP                     = "OCP"         # Open circuit potential
CA                      = "CA"          # Chronoamperometry
LSV                     = "LSV"         # Linear sweap voltammetry
CV                      = "CV"          # Cyclic voltammetry
NPV                     = "NPV"         # Normal pulse voltammetry
DPV                     = "DPV"         # Differential pulse voltammetry
SWV                     = "SWV"         # Square wave voltammetry
EIS                     = "EIS"         # Electrochemical impedance spectroscopy

"""-----------------------------------------------------------------------------
| Electrochemical methods : Number of parameters for ec-Method
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
CA_NUM_PARAMETER        = 8             # CA setup parameters
OCP_NUM_PARAMETER       = 6             # OCP setup parameters
LSV_NUM_PARAMETER       = 10            # LSV setup parameters
CV_NUM_PARAMETER        = 11            # CV setup parameters
NPV_NUM_PARAMETER       = 12            # NPV setup parameters
DPV_NUM_PARAMETER       = 12            # DPV setup parameters
SWV_NUM_PARAMETER       = 12            # SWV setup parameters

"""-----------------------------------------------------------------------------
| Configuration parameters: Abbreviations
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
SEQUENCE_LENGTH         = "pSL"         # Parameter sequence length
BASE_POTENTIAL          = "pBP"         # Parameter base potential
START_POTENTIAL         = "pSP"         # Parameter start potential
STOP_POTENTIAL          = "pSTP"        # Parameter stop potential
LOWER_POTENTIAL         = "pLP"         # Parameter lower potential
UPPER_POTENTIAL         = "pUP"         # Parameter upper potential
POTENTIAL_STEPS         = "pPS"         # Parameter potential steps
PULSE_LENGTH            = "pPL"         # Parameter pulse length
SAMPLING_RATE           = "pSAR"        # Parameter sampling rate
SAMPLING_DURATION       = "pSAD"        # Parameter sampling duration
STEP_SIZE               = "pSZ"         # Parameter step size
SCAN_RATE               = "pSR"         # Parameter scan rate
DELTA_V_STAIRCASE       = "pDVS"        # Parameter delta V staircase
DELTA_V_PEAK            = "pDVP"        # Parameter delta V peak
CYCLE                   = "pC"          # Parameter cycle
LPTIA_RTIA_SIZE         = "pLRS"        # Parameter low power TIA resistor size encoded as int
FIXED_WE_POTENTIAL      = "pFWP"        # Parameter fixed working electrorde potential
MAINS_FILTER            = "pMF"         # Parameter 50 Hz/ 60 Hz mains filter enabled
SINC2_OVERSAMPLING      = "pOS2"        # Parameter oversampling rate sinc2 filter
SINC3_OVERSAMPLING      = "pOS3"        # Parameter oversampling rate sinc3 filter

"""-----------------------------------------------------------------------------
| Configuration parameters: Default values
|   
|   Constant                 Value                     Meaning
-----------------------------------------------------------------------------"""
BASE_POTENTIAL_F        = 0.0       # Default value for base potential in V
START_POTENTIAL_F       = 0.5       # Default value for start potential in V
STOP_POTENTIAL_F        = 0.9       # Default value for stop potential in V
LOWER_POTENTIAL_F       = -0.62     # Default value for lower turning potential in V
UPPER_POTENTIAL_F       = 1.05      # Default value for upper turning potential in V
STEP_SIZE_F             = 0.002     # Default value for step size in V
SCAN_RATE_F             = 0.2       # Default value for scan rate in V/s
CURRENT_RANGE_F         = 45e-6     # Default value for the current range in A
CYCLE_I                 = 1         # Default value for amount of cycles
DELTA_V_STAIRCASE_F     = 0.05      # Default value for delta V staircase in V
DELTA_V_PEAK_F          = 0.01      # Default value for delta V peak in V
PULSE_LENGTH_F          = 5.0       # Default value for the length of a pulse in s
SAMPLING_RATE_F         = 0.01      # Default value for the sampling rate in s
SAMPLING_DURATION_F     = 0.005     # Default value for the sampling duration in s
SINC2_OVERSAMPLING_I    = 667       # Default value for the oversampling rate of sinc2 filter
SINC3_OVERSAMPLING_I    = 4         # Default value for the oversampling rate of sinc3 filter

"""-----------------------------------------------------------------------------
| Configuration parameters: Boundaries
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
VOLTAGE_RANGE           = 2050.0        # Voltage range FreiStat can supply
VOLTAGE_RANGE_FWP       = 2150.0        # Voltage range FreiStat supplies while using fixed WE potential
MIN_STEP_SIZE           = 0             # Min. stepsize in mV
MAX_STEP_SIZE           = 100           # Max. stepsize in mV
MIN_SCAN_RATE           = 0             # Min. scanrate in mV/s
MAX_SCAN_RATE           = 10000         # Max. scanrate in mV/s
MIN_CYCLE               = 0             # Min. amount of cycles per experiment
MAX_CYCLE               = 2100000000    # Max. amount of cycles per experiment
MIN_PULSE_LENGTH        = 0             # Min. pulse length in ms
MAX_PULSE_LENGTH        = 2100000000.0  # Max. pulse length in ms
MIN_SAMPLING_RATE       = 0.001         # Min. sampling rate in ms
MAX_SAMPLING_RATE       = 2100000000.0  # Max. sampling rate in ms 
MIN_SAMPLING_DURATION   = 0.001         # Min. sampling duration in ms
EXPERIMENT_BUFFER       = 50            # Size of the buffer for misc. experiment parameters
SAMPLE_BUFFER           = 250           # Site of the sample buffer which is used to store the read data

"""-----------------------------------------------------------------------------
| Electrochemical parameters: AD5940/AD5941 specifications
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
AD5940_MAX_DAC_OUTPUT   = 2400.0        # Max. voltage the 6-Bit and 12-Bit DAC can provide
AD5940_MIN_DAC_OUTPUT   = 200.0         # Min. voltage the 6-Bit and 12-Bit DAC can provide
AD5940_6BIT_DAC_1LSB    = ((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 64)      
                                        # Size of one LSB of the 6-Bit DAC
AD5940_12BIT_DAC_1LSB   = ((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 4095)   
                                        # Size of one LSB of the 12-Bit DAC
AD5940_SAMPLING_RATE    = 800000        # Clock = 16 MHz -> 800 kHz sampling
                                        # Clock = 32 MHz -> 1.6 MHz sampling

"""-----------------------------------------------------------------------------
| Electrochemical parameters: Limits of Operation
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_SEQUENCE_LENGTH= 35            # Maximum sequence length which is supoorted by FreiStat
FREISTAT_CA_ST_SERIAL   = 3.000e-3      # Minimal sampling time in s at which the CA still operates
FREISTAT_CV_ST_SERIAL   = 2.875e-3      # Minimal sampling time in s at which the CV still operates
FREISTAT_DPV_ST_SERIAL  = 3e-3          # Minimal sampling time in s at which the DPV still operates
FREISTAT_CA_ST_WLAN     = 3.000e-3      
FREISTAT_CV_ST_WLAN     = 2.875e-3      
FREISTAT_DPV_ST_WLAN    = 3e-3          

"""-----------------------------------------------------------------------------
| Communication: Connection parameters
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_SERIAL_PORT    = "COM5"        # Name of the serial port used for communication
FREISTAT_SERIAL_BAUDRATE= 230400        # Used baudrate in symbols per second
FREISTAT_SERIAL_TIMEOUT = 400           # Timeout of serial connection in seconds

FREISTAT_UDP_CLIENT_PORT= 20000         # Port of the client (Microcontroller)
FREISTAT_UDP_CLIENT_IP  = "192.168.178.40" # IP address of the client
FREISTAT_UDP_SERVER_PORT= 20001         # Port of the server (Python library)
FREISTAT_UDP_SERVER_IP  = "192.168.178.21" # IP address of the server

"""-----------------------------------------------------------------------------
| Communication: Vendor IDs
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
VID_ADAFRUIT            = 0x239A        # Vendor ID of Adafruit
VID_ARDUINO             = 0x2341        # Vendor ID of Ardunio

"""-----------------------------------------------------------------------------
| Telegrams: Telegram type abbreviations
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
COMMAND_TELEGRAM        = "C"           # Command telegram
ACKNOWLEDGE_TELEGRAM    = "A"           # Acknowledge telegram

"""-----------------------------------------------------------------------------
| Telegrams: Command telegram types
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
COMMAND_EXT             = 1             # Send experiment type telegram
COMMAND_EXP             = 2             # Send experiment parameter telegram
COMMAND_EXC             = 3             # Send control command
COMMAND_EXS             = 4             # Send sequence control command

COMMAND_EXT_STR         = "ExT"         # String for JSON telegram
COMMAND_EXP_STR         = "ExP"         # String for JSON telegram
COMMAND_EXC_STR         = "ExC"         # String for JSON telegram
COMMAND_EXS_STR         = "ExS"         # String for JSON telegram

"""-----------------------------------------------------------------------------
| Telegrams: Command telegram type 3 (Experiment control)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_WAIT_STR       = "Waiting"     # Set FreiStat in waiting mode
FREISTAT_START_STR      = "Start"       # Set FreiStat in running mode
FREISTAT_STOP_STR       = "Stop"        # Set FreiStat in stop mode

FREISTAT_WAIT_I         = 0             # Set FreiStat in waiting mode
FREISTAT_START_I        = 1             # Set FreiStat in running mode
FREISTAT_STOP_I         = 2             # Set FreiStat in stop mode

"""-----------------------------------------------------------------------------
| Telegrams: Command telegram type 4 (Sequence control)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
SEQUENCE_ENABLE_STR     = "SE"          # Enable sequence
SEQUENCE_DISABLE_STR    = "SD"          # Disable sequence

SEQUENCE_ENABLE_I       = 1             # Enable sequence
SEQUENCE_DISABLE_I      = 2             # Disable sequence

"""-----------------------------------------------------------------------------
| Telegrams: Experiment parameter abbreviations
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
RUN                     = "R"           # Run / Cycle
MEASUREMENTS            = "M"           # Measurement object containing data
DATA_PAIR_NUMBER        = "D"           # Number of the value pair inside a cycle
VOLTAGE_VALUE           = "V"           # Voltage value
CURRENT_VALUE           = "C"           # Current value   
TIME_STAMP              = "T"           # Time 

"""-----------------------------------------------------------------------------
| Data export
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
FREISTAT_CSV_EXPORT             = 0                         # Directory select for exporting 
FREISTAT_OBJECT_EXPORT          = 1                         # into correct directory

FREISTAT_CORE_DATA_FOLDER       = "Measurements"            # Core folder for storing all experiment parameters
FREISTAT_DATA                   = "Experiment_Data"         # File name for exporting data
FREISTAT_EXPERIMENT_PARAMETERS  = "Experiment_Parameters"   # File name for exporting experiment parameters
FREISTAT_SEQUENCE_POSITION      = "SP"                      # Addition for the file name indicating the position in the sequence

FREISTAT_CORE_OBJECT_FOLDER     = "Persistent_Data_Objects" # Core folder for storing data objects in a persistent way
FREISTAT_DATA_STORAGE           = "Data_Storage_Object"     # File name for the data storage object

"""-----------------------------------------------------------------------------
| Data export: Tags
|   
|   Constant              Value
-----------------------------------------------------------------------------"""
DE_TAG_CURRENT          = "Current in uA"
DE_TAG_CYCLE            = "Cycle"
DE_TAG_CYCLE_TIME       = "Cycle time in ms"
DE_TAG_DATAPOINT        = "Data point"
DE_TAG_TIME             = "Time in ms"
DE_TAG_TOTAL_TIME       = "Total time in ms"
DE_TAG_SEQ_CYCLE        = "Sequence Cycle"
DE_TAG_SEQ_TIME         = "Sequence time in ms"
DE_TAG_VOLTAGE          = "Voltage in mV"

"""-----------------------------------------------------------------------------
| Data export: Labels
|   
| Disclaimer: Labels for all Voltammetry methods identical (CV, LSV)
|
|   Constant              Value
-----------------------------------------------------------------------------"""
FREISTAT_OCP_LABEL      = [DE_TAG_CYCLE, DE_TAG_DATAPOINT, DE_TAG_VOLTAGE, 
                           DE_TAG_TIME]
FREISTAT_OCP_LABEL_SEQ  = [DE_TAG_SEQ_CYCLE, DE_TAG_CYCLE, DE_TAG_DATAPOINT, 
                           DE_TAG_VOLTAGE, DE_TAG_CYCLE_TIME, 
                           DE_TAG_SEQ_TIME, DE_TAG_TOTAL_TIME]
FREISTAT_CV_LABEL       = [DE_TAG_CYCLE, DE_TAG_DATAPOINT, DE_TAG_VOLTAGE, 
                           DE_TAG_CURRENT, DE_TAG_TIME]
FREISTAT_CA_LABEL       = [DE_TAG_CYCLE, DE_TAG_DATAPOINT, DE_TAG_VOLTAGE, 
                           DE_TAG_CURRENT, DE_TAG_TIME]
FREISTAT_CV_LABEL_SEQ   = [DE_TAG_SEQ_CYCLE, DE_TAG_CYCLE, DE_TAG_DATAPOINT, 
                           DE_TAG_VOLTAGE, DE_TAG_CURRENT, DE_TAG_CYCLE_TIME, 
                           DE_TAG_SEQ_TIME, DE_TAG_TOTAL_TIME]
FREISTAT_CA_LABEL_SEQ   = [DE_TAG_SEQ_CYCLE, DE_TAG_CYCLE, DE_TAG_DATAPOINT, 
                           DE_TAG_VOLTAGE, DE_TAG_CURRENT, DE_TAG_CYCLE_TIME, 
                           DE_TAG_SEQ_TIME, DE_TAG_TOTAL_TIME]

"""-----------------------------------------------------------------------------
| Plotter
|
| Disclaimer: Labels for all Voltammetry methods identical (CV, LSV)  
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
PADDING_VOLTAGE_MV      = 50                    # Additional spacing to the boundaries in mV
PADDING_CURRENT_UA      = 5                     # Additional spacing to the boundaries in uA
PADDING_TIME_S          = 25                    # Additional spacing to the boundaries in ms

PLOT_WINDOW_TITLE_MAIN  = "Live feed"           # Title of the live feed window
PLOT_WINDOW_SEQUENCE    = "Sequence position"   # Part of the title of the submethod windows

PLOT_LEGEND_NAME        = "Legend"              # Name of the legend 
PLOT_SEQUENCE_CYCLE_NAME= "SC"                  # Name of the sequence cycle
PLOT_CYCLE_NAME         = "C"                   # Name of the cycle

PLOT_DISPLAY_CYCLES     = 10                    # Amount of cycles which should be displayed in the plot

PLOT_CA_X_LABEL         = "Time in ms"          # Label for the x-axis of the CA-plot
PLOT_CA_Y_LABEL         = "Current in \u03BCA"  # Label for the y-axis of the CA-plot

PLOT_OCP_X_LABEL        = "Time in ms"          # Label for the x-axis of the OCP-plot
PLOT_OCP_Y_LABEL        = "Voltage in mV"       # Label for the y-axis of the OCP-plot

PLOT_CV_X_LABEL         = "Voltage in mV"       # Label for the x-axis of the CV-plot
PLOT_CV_Y_LABEL         = "Current in \u03BCA"  # Label for the y-axis of the CV-plot

"""-----------------------------------------------------------------------------
| Error Codes : General
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_NO_ERROR             = 0             # No error occured
EC_FACADE               = 10000         # Errorcode for facade
EC_SETUP                = 11000         # Errorcode for setup behavior
EC_EXECUTE              = 12000         # Errorcode for execute behavior
EC_JSON_PARSER          = 13000         # Errorcode for the JSON parser
EC_SERIAL_COMMUNICATION = 14000         # Errorcode for the serial communication
EC_DATASTORAGE          = 15000         # Errorcode for the data storage
EC_PLOTTER              = 16000         # Errorcode for the plotter
EC_UTILITY              = 17000         # Errorcode for misc. functions

"""-----------------------------------------------------------------------------
| Error Codes : Facade (FA)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""

"""-----------------------------------------------------------------------------
| Error Codes : Setup (SE)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_SE_AMOUNT_PARAMETER  = 1             # Amount of parameters given to the function doesn't match required amount.
EC_SE_SCAN_RANGE_ERROR  = 2             # Scan range exceeded
EC_SE_LIST_MISMATCH     = 3             # The two compared lists contain a different amount of entries
EC_SE_LIST_OVERFLOW     = 4             # The list contains to many entries
EC_SE_PARAM_NOT_FOUND   = 100           # Parameter nr. 100 + X not found (100-199)
EC_SE_PARAM_OUT_OF_BOUND= 200           # Parameter nr. 200 + X not fount (200-299)
EC_SE_INHERIT_ERROR     = 999           # Class inheritance not properly, tired to execute parent class

"""-----------------------------------------------------------------------------
| Error Codes : Execute (EX)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_EX_C_A_MISMATCH      = 1             # Mismatch between send and received telegram
EC_EX_INHERIT_ERROR     = 999           # Class inheritance not properly, tired to execute parent class

"""-----------------------------------------------------------------------------
| Error Codes : JSON-Parser (JP)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_JP_CODEID_UNDEF      = 1             # Passed CodeID is undefiend

"""-----------------------------------------------------------------------------
| Error Codes : Serial communication (SC)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""

"""-----------------------------------------------------------------------------
| Error Codes : Data storage (DS)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_DS_METHOD_UNKOWN     = 1             # Electrochemical method not known
EC_DS_EXPORT_SETUP_ERR  = 2             # Export setup not called before calling export method

"""-----------------------------------------------------------------------------
| Error Codes : Plotter (PT)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""

"""-----------------------------------------------------------------------------
| Error Codes : Utility (UT)
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
EC_UT_METHOD_UNKNOWN    = 1             # Electrochemical method not known
EC_UT_SINC2_MIS         = 2             # Sinc 2 Oversampling rate missing
EC_UT_SINC3_MIS         = 3             # Sinc 3 Oversampling rate missing
EC_UT_SAMPLING_RATE_MIS = 4             # Sampling rate missing
EC_UT_SAMPLING_DUR_MIS  = 5             # Sampling duration missing
EC_UT_STEP_SIZE_MIS     = 6             # Step size missing
EC_UT_SCAN_RATE_MIS     = 7             # Scan rate missing
EC_UT_OPTIMIZER_FAILED  = 8             # Optimizer has failed

"""-----------------------------------------------------------------------------
| Sinc2 & Sinc3 filter: Oversampling rates
|
| Note: This definition must be matched to the defition in the ad5940.h
| Last checked: 09.08.2021 by Mark Jasper
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
ADCSINC2OSR_DISABLED    = -1            # Sinc2 disabled
ADCSINC2OSR_22          = 0             # Sinc2 OSR 22
ADCSINC2OSR_44          = 1             # Sinc2 OSR 44
ADCSINC2OSR_89          = 2             # Sinc2 OSR 89
ADCSINC2OSR_178         = 3             # Sinc2 OSR 178
ADCSINC2OSR_267         = 4             # Sinc2 OSR 267
ADCSINC2OSR_533         = 5             # Sinc2 OSR 533
ADCSINC2OSR_640         = 6             # Sinc2 OSR 640
ADCSINC2OSR_667         = 7             # Sinc2 OSR 667
ADCSINC2OSR_800         = 8             # Sinc2 OSR 800
ADCSINC2OSR_889         = 9             # Sinc2 OSR 889
ADCSINC2OSR_1067        = 10            # Sinc2 OSR 1067
ADCSINC2OSR_1333        = 11            # Sinc2 OSR 1333

ADCSINC3OSR_DISABLED    = -1            # Sinc3 disabled
ADCSINC3OSR_5           = 0             # Sinc3 OSR 5
ADCSINC3OSR_4           = 1             # Sinc3 OSR 4
ADCSINC3OSR_2           = 2             # Sinc3 OSR 2

ADCSINC2OSR_DISABLED_VALUE = 0          # Sinc2 disabled
ADCSINC2OSR_22_VALUE    = 22            # Sinc2 OSR 22
ADCSINC2OSR_44_VALUE    = 44            # Sinc2 OSR 44
ADCSINC2OSR_89_VALUE    = 89            # Sinc2 OSR 89
ADCSINC2OSR_178_VALUE   = 178           # Sinc2 OSR 178
ADCSINC2OSR_267_VALUE   = 267           # Sinc2 OSR 267
ADCSINC2OSR_533_VALUE   = 533           # Sinc2 OSR 533
ADCSINC2OSR_640_VALUE   = 640           # Sinc2 OSR 640
ADCSINC2OSR_667_VALUE   = 667           # Sinc2 OSR 667
ADCSINC2OSR_800_VALUE   = 800           # Sinc2 OSR 800
ADCSINC2OSR_889_VALUE   = 889           # Sinc2 OSR 889
ADCSINC2OSR_1067_VALUE  = 1067          # Sinc2 OSR 1067
ADCSINC2OSR_1333_VALUE  = 1333          # Sinc2 OSR 1333

ADCSINC3OSR_DISABLED_VALUE = 0          # Sinc3 disabled
ADCSINC3OSR_5_VALUE     = 5             # Sinc3 OSR 5
ADCSINC3OSR_4_VALUE     = 4             # Sinc3 OSR 4
ADCSINC3OSR_2_VALUE     = 2             # Sinc3 OSR 2

"""-----------------------------------------------------------------------------
| Low power transimpendence amplifier: Resisotr sizes
|
| Note: This definition must be matched to the defition in the ad5940.h
| Last checked: 09.08.2021 by Mark Jasper
|   
|   Constant              Value                     Meaning
-----------------------------------------------------------------------------"""
LPTIARTIA_OPEN          = 0             # Disconnect LPTIA internal Rtia 
LPTIARTIA_200R          = 1             # 200 Ohm internal Rtia 
LPTIARTIA_1K            = 2             # 1 kOhm internal Rtia  
LPTIARTIA_2K            = 3             # 2 kOhm 
LPTIARTIA_3K            = 4             # 3 kOhm 
LPTIARTIA_4K            = 5             # 4 kOhm 
LPTIARTIA_6K            = 6             # 6 kOhm 
LPTIARTIA_8K            = 7             # 8 kOhm 
LPTIARTIA_10K           = 8             # 10 kOhm 
LPTIARTIA_12K           = 9             # 12 kOhm 
LPTIARTIA_16K           = 10            # 16 kOhm 
LPTIARTIA_20K           = 11            # 20 kOhm 
LPTIARTIA_24K           = 12            # 24 kOhm 
LPTIARTIA_30K           = 13            # 30 kOhm 
LPTIARTIA_32K           = 14            # 32 kOhm 
LPTIARTIA_40K           = 15            # 40 kOhm 
LPTIARTIA_48K           = 16            # 48 kOhm 
LPTIARTIA_64K           = 17            # 64 kOhm 
LPTIARTIA_85K           = 18            # 85 kOhm 
LPTIARTIA_96K           = 19            # 96 kOhm 
LPTIARTIA_100K          = 20            # 100 kOhm 
LPTIARTIA_120K          = 21            # 120 kOhm 
LPTIARTIA_128K          = 22            # 128 kOhm 
LPTIARTIA_160K          = 23            # 160 kOhm 
LPTIARTIA_196K          = 24            # 196 kOhm 
LPTIARTIA_256K          = 25            # 256 kOhm 
LPTIARTIA_512K          = 26            # 512 kOhm 

LPTIARTIA_OPEN_VALUE    = 0             # Disconnect LPTIA internal Rtia 
LPTIARTIA_200R_VALUE    = 200           # 200 Ohm internal Rtia 
LPTIARTIA_1K_VALUE      = 1000          # 1 kOhm internal Rtia  
LPTIARTIA_2K_VALUE      = 2000          # 2 kOhm 
LPTIARTIA_3K_VALUE      = 3000          # 3 kOhm 
LPTIARTIA_4K_VALUE      = 4000          # 4 kOhm 
LPTIARTIA_6K_VALUE      = 6000          # 6 kOhm 
LPTIARTIA_8K_VALUE      = 8000          # 8 kOhm 
LPTIARTIA_10K_VALUE     = 10000         # 10 kOhm 
LPTIARTIA_12K_VALUE     = 12000         # 12 kOhm 
LPTIARTIA_16K_VALUE     = 16000         # 16 kOhm 
LPTIARTIA_20K_VALUE     = 20000         # 20 kOhm 
LPTIARTIA_24K_VALUE     = 24000         # 24 kOhm 
LPTIARTIA_30K_VALUE     = 30000         # 30 kOhm 
LPTIARTIA_32K_VALUE     = 32000         # 32 kOhm 
LPTIARTIA_40K_VALUE     = 40000         # 40 kOhm 
LPTIARTIA_48K_VALUE     = 48000         # 48 kOhm 
LPTIARTIA_64K_VALUE     = 64000         # 64 kOhm 
LPTIARTIA_85K_VALUE     = 85000         # 85 kOhm 
LPTIARTIA_96K_VALUE     = 96000         # 96 kOhm 
LPTIARTIA_100K_VALUE    = 100000        # 100 kOhm 
LPTIARTIA_120K_VALUE    = 120000        # 120 kOhm 
LPTIARTIA_128K_VALUE    = 128000        # 128 kOhm 
LPTIARTIA_160K_VALUE    = 160000        # 160 kOhm 
LPTIARTIA_196K_VALUE    = 196000        # 196 kOhm 
LPTIARTIA_256K_VALUE    = 256000        # 256 kOhm 
LPTIARTIA_512K_VALUE    = 512000        # 512 kOhm 
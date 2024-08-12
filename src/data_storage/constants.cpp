/******************************************************************************
 * @brief: File containing all constant definitions for the FreiStat software.
 * The parameters which are defined in this file must coincide with the
 * constants in:
 * 
 * Python/FreiStat/Data_Storage/constants.py
 * 
 * If you want to assign different values to the constants, make sure to do the 
 * same on python side or the communication between both software parts won't
 * work.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef FreiStat_Constants_C
#define FreiStat_Constants_C

/******************************************************************************
 * General definitions : Enabled methods
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define ENABLE_OCP              false       // flag indicating if method is enabled
#define ENABLE_CA               true        
#define ENABLE_LSV              true
#define ENABLE_CV               true
#define ENABLE_NPV              true
#define ENABLE_DPV              true
#define ENABLE_SWV              true

/******************************************************************************
 * General definitions : Modes of operation
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
//#define FREISTAT_STANDALONE     true        // Flag indicating if the FreiStat is used in standalone mode
#define FREISTAT_STANDALONE     false
//#define WiFiEnabled             true        // Flag indicating if WiFi communication is enabled
#define WiFiEnabled             false

/******************************************************************************
 * General definitions : WiFi config
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define WiFiSSID                "SSID"      // Name of the SSID which should be connected to
#define WiFiPASSWORD            "PASSWORD"  // Password of the SSID
#define WiFiIPOCTET_1           192         // 1 octet of the IP address
#define WiFiIPOCTET_2           168         // 2 octet of the IP address
#define WiFiIPOCTET_3           178         // 3 octet of the IP address
#define WiFiIPOCTET_4           21          // 4 octet of the IP address
#define WiFiLOCALPORT           20000       // Own port used for sending data
#define WiFiSERVERPORT          20001       // Server port data is send to

/******************************************************************************
 * General definitions
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define MAX_TELEGRAM_LENGTH     512         // Max amount of bytes which are read at one time by the serial communication
#define BAUDRATE                230400      // Baudrate in symbols per second
#define SAMPLE_BUFFER           250         // Size of the sample buffer to store data from AD5940
#define TRANSMIT_BUFFER         10          // Size of the transmit buffer who temporary stores the result before sending them
#define SAMPLE_DELAY            4.0         // Define sample delay > 1.0 ms is suitable
#define EXPERIMENT_BUFFER       50          // Size of the buffer for misc. experiment parameters
#define PREFIX_BUFFER           6           // Size of the prefix buffer which is used to store JSON-telegram components

#define CURRENT_BLOCK_0         0           // Block definition for ping pong buffer slot 0
#define CURRENT_BLOCK_1         1           // Block definition for ping pong buffer slot 1

/******************************************************************************
 * System status
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FREISTAT_BOOTUP         0           // FreiStat is in boot up process
#define FREISTAT_WAITING        1           // FreiStat in waiting state
#define FREISTAT_ExT            2           // Experiment type received
#define FREISTAT_ExP            3           // Experiment parameters received
#define FREISTAT_EXP_STARTED    4           // Start experiment
#define FREISTAT_EXP_RUNNING    5           // Experiment running
#define FREISTAT_EXP_COMPLETED  6           // Experiment completed
#define FREISTAT_EXP_CANCELED   7           // Experiment canceled

/******************************************************************************
 * Sequence status
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FREISTAT_EXS_ENABLED    1           // Sequence mode enabled
#define FREISTAT_EXS_DISABLED   2           // Sequence mode disabled

/******************************************************************************
 * Wakeup timer status
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FREISTAT_START_TIMER    0           // Start wakeup timer
#define FREISTAT_STOP_TIMER     1           // Stop wakeup timer

/******************************************************************************
 * Electrochemical methods: Experiment status
 * 
 * Depending on the chosen ec-method the states are defined in a different way.
 * See below for the different methods:
 * 
 * Cyclic voltammetry (CV):
 * 
 * State 0 :    Initalize sweaping direction
 * State 1 :    Sweaping from starting potential to upper turning potential
 * State 2 :    Sweaping from upper turning potential to starting potential
 * State 3 :    Sweaping from starting potential to lower turning potential
 * State 4 :    Sweaping from lower turning potential to starting potential
 * State 5 :    Stop sweaping
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define EC_METHOD_STATE_0       0           // State 0
#define EC_METHOD_STATE_1       1           // State 1
#define EC_METHOD_STATE_2       2           // State 2
#define EC_METHOD_STATE_3       3           // State 3
#define EC_METHOD_STATE_4       4           // State 4
#define EC_METHOD_STATE_5       5           // State 5

/******************************************************************************
 * Electrochemical methods: General abbreviations
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define UNDEFINED_EC_METHOD     "Udf"       // No electrochemical method saved
#define SEQUENCE                "Seq"       // Sequence mode (required for transmitting related parameters)
#define OCP                     "OCP"       // Open circuit potential
#define CA                      "CA"        // Chronoamperometry
#define LSV                     "LSV"       // Linear sweap voltammetry
#define CV                      "CV"        // Cyclic voltammetry
#define NPV                     "NPV"       // Normal pulse voltammetry
#define DPV                     "DPV"       // Differential pulse voltammetry
#define SWV                     "SWV"       // Square wave voltammetry
#define EIS                     "EIS"       // Electrochemical impedance spectroscopy

/******************************************************************************
 * Electrochemical methods: General abbreviations as integer
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define UNDEFINED_EC_METHOD_I   0           // No electrochemical method saved
#define SEQUENCE_I              1           // Sequence mode (required for transmitting related parameters)
#define OCP_I                   2           // Open circuit potential
#define CA_I                    3           // Chronoamperometry
#define LSV_I                   4           // Linear sweap voltammetry
#define CV_I                    5           // Cyclic voltammetry
#define NPV_I                   6           // Normal pulse voltammetry
#define DPV_I                   7           // Differential pulse voltammetry
#define SWV_I                   8           // Square wave voltammetry
#define EIS_I                   9           // Electrochemical impedance spectroscopy

/******************************************************************************
 * Electrochemical parameters: Default values
 *
 *      Constant                    Value                 Meaning
 *****************************************************************************/
#define BASE_POTENTIAL_F            0.0     // Default value for base potential in mV
#define START_POTENTIAL_F           500.0   // Default value for starting potential in mV
#define STOP_POTENTIAL_F            1050.0  // Default value for stop potential in mV
#define LOWER_POTENTIAL_F           -620.0  // Default value for lower potential in mV
#define UPPER_POTENTIAL_F           1050.0  // Default value for upper potential in mV
#define STEP_SIZE_F                 2.0     // Default value for step size in mV
#define SCAN_RATE_F                 200.0   // Default value for scan rate in mV/s
#define DELTA_V_STAIRCASE_F         50.0    // Default value for delta V staircase in mV
#define DELTA_V_PEAK_F              50.0    // Default value for delta V peak in mV
#define CYCLE_I                     1       // Default value for amount of cycles
#define LPTIA_RTIA_SIZE_I           11      // Parameter low power TIA resistor size encoded as int
#define FIXED_WE_POTENTIAL_B        1       // Parameter Fixed Working electrode Potential
#define MAINS_FILTER_B              1       // Parameter mains filter

#define BUFFER_ENTRIES              3       // Amount of entries in the following arrays
// Parameter for CA Standalone potential steps
const float POTENTIAL_STEPS_F[] =   {800.0, -400.0, -300.0}; 
// Parameter for CA Standalone pulse lengths in ms
const float PULSE_LENGTHS_F[] =     {4000, 3000, 3000};         

/******************************************************************************
 * Electrochemical parameters: Abbreviations for telegram exchange
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define SEQUENCE_LENGTH         "pSL"       // Parameter sequnece length
#define BASE_POTENTIAL          "pBP"       // Parameter base potential
#define START_POTENTIAL         "pSP"       // Parameter Start Potential
#define STOP_POTENTIAL          "pSTP"      // Parameter Stop Potential
#define LOWER_POTENTIAL         "pLP"       // Parameter Lower Potential
#define UPPER_POTENTIAL         "pUP"       // Parameter Upper Potential
#define POTENTIAL_STEPS         "pPS"       // Parameter potential steps
#define PULSE_LENGTH            "pPL"       // Parameter pulse length
#define SAMPLING_RATE           "pSAR"      // Parameter sampling rate
#define SAMPLING_DURATION       "pSAD"      // Parameter sampling duration
#define STEP_SIZE               "pSZ"       // Parameter StepSize
#define SCAN_RATE               "pSR"       // Parameter ScanRate
#define DELTA_V_STAIRCASE       "pDVS"      // Parameter delta V staircase
#define DELTA_V_PEAK            "pDVP"      // Parameter delta V peak
#define CYCLE                   "pC"        // Parameter Cycle
#define LPTIA_RTIA_SIZE         "pLRS"      // Parameter low power TIA resistor size encoded as int
#define FIXED_WE_POTENTIAL      "pFWP"      // Parameter Fixed Working electrode Potential
#define MAINS_FILTER            "pMF"       // Parameter 50 Hz/ 60 Hz mains filter enabled
#define SINC2_OVERSAMPLING      "pOS2"      // Parameter oversampling rate sinc2 filter
#define SINC3_OVERSAMPLING      "pOS3"      // Parameter oversampling rate sinc3 filter

/******************************************************************************
 * Telegram: Telegram type abbreviations
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define COMMAND_TELEGRAM        "C"         // Command telegram
#define ACKNOWLEDGE_TELEGRAM    "A"         // Acknowledge telegram
#define ERROR_TELEGRAM          "E"         // Error telegram

#define SEQUENCE_ENABLE         "SE"        // Enable sequence
#define SEQUENCE_DISABLE        "SD"        // Disable sequence

/******************************************************************************
 * Telegram: Experiment parameter abbreviations
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define RUN                     "R"         // Run / Cycle
#define MEASUREMENTS            "M"         // Measurement object containing data
#define DATA_PAIR_NUMBER        "D"         // Number of the value pair inside a cycle
#define VOLTAGE_VALUE           "V"         // Voltage value
#define CURRENT_VALUE           "C"         // Current value           
#define TIME_VALUE              "T"         // Time stamp  

/******************************************************************************
 * Telegram: Command telegram types
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define COMMAND_EXT             1           // Send experiment type telegram
#define COMMAND_EXP             2           // Send experiment parameter telegram
#define COMMAND_EXC             3           // Send control command
#define COMMAND_EXS             4           // Send sequence control command

#define COMMAND_EXT_STR         "ExT"       // String for JSON telegram
#define COMMAND_EXP_STR         "ExP"       // String for JSON telegram
#define COMMAND_EXC_STR         "ExC"       // String for JSON telegram
#define COMMAND_EXS_STR         "ExS"       // String for JSON telegram

/******************************************************************************
 * Control status
 *
 * Commands send via command telegram type 3 (Experiment control) telegrams
 * 
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FREISTAT_UNDEF_STR      "Undefined" // No command stored
#define FREISTAT_WAIT_STR       "Waiting"   // FreiStat in waiting mode
#define FREISTAT_START_STR      "Start"     // FreiStat is running
#define FREISTAT_STOP_STR       "Stop"      // FreiStat stopped

#define FREISTAT_UNDEF_I        -1          // No command stored
#define FREISTAT_WAIT_I         0           // FreiStat in waiting mode
#define FREISTAT_START_I        1           // FreiStat is running
#define FREISTAT_STOP_I         2           // FreiStat stopped

/******************************************************************************
 * Board configuration - Defined board / Chip
 * 
 *****************************************************************************/
#define ADAFRUIT_FEATHER_M0_WIFI
#define CHIP_AD5940
#define ADAFRUIT_FEATHER_M0_ADALOGGER

#ifdef ADAFRUIT_FEATHER_M0_WIFI
/******************************************************************************
 * Board configuration ADAFRUIT_FEATHER_M0_WIFI
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FEATHER_M0_IntPin           A1          // Analog Output A1 of MCU is connected to GPIO0 of AD594x
#define FEATHER_M0_AFE_SLEEP_LED    A2          // Analog Input  A2 of MCU enables AD5940 to control the LED (Status)
#define FEATHER_M0_ResetPin         A4          // Analog Output A4 of MCU is connected to Reset pin of AD594x
#define FEATHER_M0_SPI_CS_Pin       A5          // Analog Output A5 of MCU is connected to CS pin of AD594x 

#define FEATHER_M0_SYS_CLOCK_FREQ   48000000.0  // Defined system frequency of the MCU

#endif /* ADAFRUIT_FEATHER_M0_WIFI */

#ifdef CHIP_AD5940
/******************************************************************************
 * Chip configuration parameters for the AD5940/ AD5941
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define ADCSINC2OSR_DISABLED    -1          // Extension of the ad5940.h 
#define ADCSINC3OSR_DISABLED    -1          // ADCSINC2OSR_CONST define

#define AD5940_FIFO_THRESHOLD   240         // Amound of data blocks at which the FIFO threshold interrupt is triggered 
#define AD5940_LFOSC_CAL_TIME   1000.0      // Time for calibrating Low frequency oscillator
#define AD5940_SEQ_MAX_ADDR     1024        // Max. address of the sequencer
#define AD5940_SEQ_START_ADDR   0x10        // Start address of the sequencer                                        
#define AD5940_SEQ_COMMAND_SIZE 4           // Size of single sequencer commands in bytes
#define AD5940_SYS_CLOCK_FREQ   16000000.0  // Defined system frequency of the AD5940

#define AD5940_MAX_DAC_OUTPUT   2400.0      // Max. voltage the 6-Bit and 12-Bit DAC can provide
#define AD5940_MIN_DAC_OUTPUT   200.0       // Min. voltage the 6-Bit and 12-Bit DAC can provide

#define AD5940_ADC_REF_VOLTAGE  1820.0      // Voltage measured over C3 (Feather extension board) 
#define AD5940_CAL_RESISTOR     10000.0     // Size of the calibration resistor in Ohm


#define AD5940_6BIT_DAC_1LSB    ((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 64)      // Size of one LSB of the 6-Bit DAC
#define AD5940_12BIT_DAC_1LSB   ((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 4095)    // Size of one LSB of the 12-Bit DAC

#define AD5940_NUM_SEQ_COMMANDS 4           // Number of sequencer commands that are supported by the hardware
#define AD5940_SIZE_SEQ_COMMAND 4           // Size of a sequencer command in byte    
#define AD5940_BUFFER_CV        5           // Size of the sequencer command buffer for CV
#define AD5940_BUFFER_CA        6           // Size of the sequencer command buffer for CA
#define AD5940_BUFFER_DPV       7           // Size of the sequencer command buffer for DPV

#endif /* CHIP_AD5940 */

#ifdef ADAFRUIT_FEATHER_M0_ADALOGGER
/******************************************************************************
 * Board configuration ADAFRUIT_FEATHER_M0_WIFI
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define FEATHER_M0_SD_CS_PIN    10          // Pin number of the chip select of the SD card reader for SPI

#endif /* ADAFRUIT_FEATHER_M0_ADALOGGER */

/******************************************************************************
 * Error codes
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define EC_NO_ERROR             0           // No error occured
#define EC_FREISTAT             20000       // Errorcode for FreiStat.ino
#define EC_SETUP                21000       // Errorcode for setup behavior
#define EC_EXECUTE              22000       // Errorcode for execute behavior
#define EC_JSON_PARSER          23000       // Errorcode for the JSON parser
#define EC_SERIAL_COMMUNICATION 24000       // Errorcode for the serial communication
#define EC_DATASTORAGE          25000       // Errorcode for the data storage
#define EC_AD5941_SETUP         26000       // Errorcode for the AD5941/5940 setup

/******************************************************************************
 * Error codes : Freistat (FS)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/

/******************************************************************************
 * Error codes : Setup (SE)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define EC_SE_WAKEUP_AFE_ERR    1           // Error while trying to wakeup AFE
#define EC_SE_SAMPLE_BUFF_SIZE  2           // Sample buffer too small
#define EC_SE_SEQ_BUFF_SIZE     3           // Sequence buffer too small
#define EC_SE_TP_OOR            10          // Turning potentials out of range
#define EC_SE_METHOD_DISABLED   100         // Chosen method not enabled
#define EC_SE_EC_METHOD_UKNOWN  999         // Electrochemical method unknown

/******************************************************************************
 * Error codes : Execute (EX)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define EC_EX_WAKEUP_AFE_ERR    1           // Error while trying to wakeup AFE

/******************************************************************************
 * Error codes : JSON-Parser (JP)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/
#define EC_JP_PARSING_SUC       -1          // Parsing successful
#define EC_JP_NO_JSON_FORMAT    1           // Telegram has no JSON format
#define EC_JP_NO_STRING_FOUND   2           // No string found in segment
#define EC_JP_PARA_NOT_KNOWN    3           // Parameter not known

/******************************************************************************
 * Error codes : Serial communication (SC)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/

/******************************************************************************
 * Error codes : Data Storage (DS)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/

/******************************************************************************
 * Error codes : AD5941/5940 Setup (AS)
 *
 *      Constant                Value                     Meaning
 *****************************************************************************/

#endif /* FreiStat_Constants_C */
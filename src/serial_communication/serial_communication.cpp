/******************************************************************************
 * @brief: Source file for establishing communication and sending packages
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef serial_communication_CPP
#define serial_communication_CPP

// Include dependencies
#include "avr/dtostrf.h"

// Include headers
#include "serial_communication.h"

/******************************************************************************
 * @brief Constructor of the class C_Communication
 * 
 *****************************************************************************/ 
C_Communication::C_Communication(){}

/******************************************************************************
 * @brief Starting method for the class C_Communication
 * @param iBaudrate: is used to define the baudrate
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Communication::Begin(int iBaudrate, 
                            C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save baudrate
    iBaudrate_ = iBaudrate;

    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Get reference of JSON parser object
    c_JSONParser_ = c_DataSoftwareStorage_->get_JSONParser();

    // Establish Communication with serial port or WLAN
    this->funEstablishCommunication();
}

/******************************************************************************
 * @brief Function to establish connection with the serial port or the WLAN
 * @return Error code encoded as integer
 *****************************************************************************/ 
int C_Communication::funEstablishCommunication(){
    // Check if WiFi is enabled
    if (WiFiEnabled){
        // Enable Serial port -> TODO change to SD-card
        Serial.begin(iBaudrate_);

        // Configure WiFi pins
        WiFi.setPins(8,7,4,2);

        // Intialize variables
        int iStatus = WL_IDLE_STATUS;

        IPaddress  = IPAddress(WiFiIPOCTET_1, WiFiIPOCTET_2, WiFiIPOCTET_3, WiFiIPOCTET_4);

        // Try to connect to WiFi network
        while (iStatus != WL_CONNECTED) {
            // Connect to WPA/WPA2 network
            iStatus = WiFi.begin(WiFiSSID, WiFiPASSWORD);
            delay(1000);
        }
        Udp.begin(WiFiLOCALPORT); 
    }
    else {
        // Open serial port with defined baudrate
        Serial.begin(iBaudrate_);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to terminate connection with the serial port or the WLAN
 *****************************************************************************/ 
void C_Communication::funEndCommunication(){
    if (WiFiEnabled){
        WiFi.end();
    }    
}

/******************************************************************************
 * @brief Function to setup SD card for standalone mode or WLAN mode
 * @return Error code encoded as integer
 *****************************************************************************/ 
int C_Communication::funSetupSDcard(){
    // Check if FreiStat runs in Standalone or WLAN mode 
    #if WiFiEnabled || FREISTAT_STANDALONE

    // Intialize variables
    char chrIntBuff[5];

    int iCounter = 0;
    iFlushCounter = 0;

    // Initialize SD card reader
    SD.begin(FEATHER_M0_SD_CS_PIN);

    // Open directory
    File directory = SD.open("/");

    // e.g. CV
    strncpy(chrFilename, c_DataSoftwareStorage_->get_DataStorageLocal()->
        get_ExperimentType(), sizeof(chrFilename));

    // Get amount of files on the SD card
    while(true){
        // Open next file
        File entry = directory.openNextFile((O_READ));
        
        iCounter += 1;

        // Check if there is a next file
        if(!entry){
            break;
        }
        entry.close();
    }
    
    // e.g. CV2
    strcat(chrFilename, itoa(iCounter, chrIntBuff, 10));
    if (WiFiEnabled)
        // CV2.dat
        strcat (chrFilename, ".dat");    
    else if (FREISTAT_STANDALONE){
        // CV2.csv
        strcat (chrFilename, ".csv");
    }

    // Open file writer
    dataFile = SD.open(chrFilename ,(O_CREAT | O_WRITE | O_APPEND));

    #endif
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to stop SD card for standalone mode or WLAN mode
 * @return Error code encoded as integer
 *****************************************************************************/ 
int C_Communication::funStopSDcard(){
    // Check if FreiStat runs in Standalone or WLAN mode 
    #if WiFiEnabled || FREISTAT_STANDALONE

    // Initalize variables
    char chrIntBuff[16];
    char chrFloatBuff[33];
    char chrBuff[128];

    // Close dataFile
    dataFile.close();

    // Check if WLAN mode is enabled
    if (WiFiEnabled){
        // Intialize variables
        char chrBuff[128];

        // Open the file for reading
        dataFile = SD.open(chrFilename, FILE_READ);

        // Send all stored data
        while(dataFile.available()){
            // Reset char array
            struct S_DataContainer S_ExperimentData;
            dataFile.read((uint8_t *)&S_ExperimentData, sizeof(S_ExperimentData));

            strncpy(chrBuff, chrPrefix1_, sizeof(chrPrefix1_));
            strncat(chrBuff, itoa(S_ExperimentData.iCycle, chrIntBuff, 10), 
                sizeof(chrIntBuff));
            strncat(chrBuff, chrPrefix2_, sizeof(chrPrefix2_));
            strncat(chrBuff, chrPrefix3_, sizeof(chrPrefix3_));
            strncat(chrBuff, itoa(S_ExperimentData.iMeasurmentPair, chrIntBuff, 10), 
                sizeof(chrIntBuff));
            strncat(chrBuff, chrPrefix4_, sizeof(chrPrefix1_));

            dtostrf(S_ExperimentData.fVoltage, 7, 5, chrFloatBuff);

            strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
            strncat(chrBuff, chrPrefix5_, sizeof(chrPrefix1_));

            dtostrf(S_ExperimentData.fCurrent, 7, 5, chrFloatBuff);

            strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
            strncat(chrBuff, chrPrefix6_, sizeof(chrPrefix6_));
            strncat(chrBuff, itoa(S_ExperimentData.fTimeStamp, chrIntBuff, 10), 
                sizeof(chrIntBuff));
            strncat(chrBuff, "}}", sizeof("}}"));

            Udp.beginPacket(IPaddress, WiFiSERVERPORT);
            Udp.write(chrBuff);
            Udp.endPacket();
        }
        // Delete file
        dataFile.close();
    }
    #endif
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to create telegram prefixes depending on the chosen
 * electrochemical method
 * @param chrEcMethod: Char array containing the abbreviation for the electro-
 * chemical method
 * @return Error code
 *****************************************************************************/
int C_Communication::funConstructPrefixes(char * chrEcMethod){
    if (strcmp(chrEcMethod, CA)  == 0 ||
        strcmp(chrEcMethod, LSV) == 0 ||
        strcmp(chrEcMethod, CV)  == 0 ||
        strcmp(chrEcMethod, NPV) == 0 ||
        strcmp(chrEcMethod, DPV) == 0 ||
        strcmp(chrEcMethod, SWV) == 0 ){
        c_JSONParser_->funWrapObjectChar(chrPrefix1_, RUN, true);
        c_JSONParser_->funWrapObjectChar(chrPrefix2_, MEASUREMENTS, false);
        c_JSONParser_->funWrapObjectChar(chrPrefix3_, DATA_PAIR_NUMBER, true);
        c_JSONParser_->funWrapObjectChar(chrPrefix4_, VOLTAGE_VALUE, false);
        c_JSONParser_->funWrapObjectChar(chrPrefix5_, CURRENT_VALUE, false);
        c_JSONParser_->funWrapObjectChar(chrPrefix6_, TIME_VALUE, false);
    }
    else if (strcmp(chrEcMethod, OCP) == 0){
        c_JSONParser_->funWrapObjectChar(chrPrefix1_, RUN, true);
        c_JSONParser_->funWrapObjectChar(chrPrefix2_, MEASUREMENTS, false);
        c_JSONParser_->funWrapObjectChar(chrPrefix3_, DATA_PAIR_NUMBER, true);
        c_JSONParser_->funWrapObjectChar(chrPrefix4_, VOLTAGE_VALUE, false);
        c_JSONParser_->funWrapObjectChar(chrPrefix5_, TIME_VALUE, false);   
    }   
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function which checks if data is available for the chosen method
 * of communication (Serial | WiFi)
 * @return Flag if data is available or not
 *****************************************************************************/
bool C_Communication::funDataAvailable(){
    // Check if WiFi is enabled
    if (WiFiEnabled){
        return Udp.available() > 0;
    }
    else {
        return Serial.available() > 0;
    }
}

/******************************************************************************
 * @brief Function to send experiment data in json format to the serial port
 * @param s_ExperimentData: struct with the experiment data
 * @param iEcMethod: Integer containing the abbreviation for the electro-
 * chemical method
 * @return Error code encoded as integer
 *****************************************************************************/ 
int C_Communication::funSendExperimentData(S_DataContainer S_ExperimentData,
                                           int iEcMethod){
    // Initalize variables
    char chrIntBuff[16];
    char chrFloatBuff[33];
    char chrBuff[128];

    #if !FREISTAT_STANDALONE && !WiFiEnabled
    // Send different data depending on the electrochemical method
    switch (iEcMethod){
    case OCP_I:
        strncpy(chrBuff, chrPrefix1_, sizeof(chrPrefix1_));
        strncat(chrBuff, itoa(S_ExperimentData.iCycle, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, chrPrefix2_, sizeof(chrPrefix2_));
        strncat(chrBuff, chrPrefix3_, sizeof(chrPrefix3_));
        strncat(chrBuff, itoa(S_ExperimentData.iMeasurmentPair, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, chrPrefix4_, sizeof(chrPrefix1_));

        dtostrf(S_ExperimentData.fVoltage, 7, 5, chrFloatBuff);

        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, chrPrefix5_, sizeof(chrPrefix1_));
        strncat(chrBuff, itoa(S_ExperimentData.fTimeStamp, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, "}}", sizeof("}}"));

        break;
    case SWV_I:
    case DPV_I:
    case NPV_I:
    case CA_I:
    case LSV_I:
    case CV_I:
        strncpy(chrBuff, chrPrefix1_, sizeof(chrPrefix1_));
        strncat(chrBuff, itoa(S_ExperimentData.iCycle, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, chrPrefix2_, sizeof(chrPrefix2_));
        strncat(chrBuff, chrPrefix3_, sizeof(chrPrefix3_));
        strncat(chrBuff, itoa(S_ExperimentData.iMeasurmentPair, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, chrPrefix4_, sizeof(chrPrefix1_));

        dtostrf(S_ExperimentData.fVoltage, 7, 5, chrFloatBuff);

        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, chrPrefix5_, sizeof(chrPrefix1_));

        dtostrf(S_ExperimentData.fCurrent, 7, 5, chrFloatBuff);

        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, chrPrefix6_, sizeof(chrPrefix6_));
        strncat(chrBuff, itoa(S_ExperimentData.fTimeStamp, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, "}}", sizeof("}}"));

        break;
    default:
        break;
    }
    #endif
    #if FREISTAT_STANDALONE
    // Send different data depending on the electrochemical method
    switch (iEcMethod){
    case OCP_I:
        strncat(chrBuff, itoa(S_ExperimentData.iCycle, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, ",", sizeof(","));

        strncat(chrBuff, itoa(S_ExperimentData.iMeasurmentPair, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, ",", sizeof(","));

        dtostrf(S_ExperimentData.fVoltage, 7, 5, chrFloatBuff);
        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, ",", sizeof(","));

        strncat(chrBuff, itoa(S_ExperimentData.fTimeStamp, chrIntBuff, 10), 
            sizeof(chrIntBuff));

        break;
    case SWV_I:
    case DPV_I:
    case NPV_I:
    case CA_I:
    case LSV_I:
    case CV_I:
        strncpy(chrBuff, itoa(S_ExperimentData.iCycle, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, ",", sizeof(","));

        strncat(chrBuff, itoa(S_ExperimentData.iMeasurmentPair, chrIntBuff, 10), 
            sizeof(chrIntBuff));
        strncat(chrBuff, ",", sizeof(","));

        dtostrf(S_ExperimentData.fVoltage, 7, 5, chrFloatBuff);

        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, ",", sizeof(","));

        dtostrf(S_ExperimentData.fCurrent, 7, 5, chrFloatBuff);

        strncat(chrBuff, chrFloatBuff, sizeof(chrFloatBuff));
        strncat(chrBuff, ",", sizeof(","));

        strncat(chrBuff, itoa(S_ExperimentData.fTimeStamp, chrIntBuff, 10), 
            sizeof(chrIntBuff));

        break;
    default:
        break;
    }
    #endif

    if (WiFiEnabled || FREISTAT_STANDALONE){
        // Check if FreiStat runs in WLAN mode or Standalone mode
        #if FREISTAT_STANDALONE
        // Append string terminator
        strncat(chrBuff, "\n\0", sizeof("\n\0"));
        
        // Store data on SD-card
        dataFile.write(chrBuff);
        #endif
        #if WiFiEnabled
        dataFile.write((const uint8_t *)&S_ExperimentData, sizeof(S_ExperimentData));
        
        iFlushCounter += 1;
        
        if (iFlushCounter >= 20){
            iFlushCounter = 0;
            dataFile.flush();
        }
        #endif
    }
    else {
        // Send char arry via serial port
        Serial.write(chrBuff);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to send acknowledge telegram to the serial port
 * @return Error code
 *****************************************************************************/ 
int C_Communication::funSendAcknowledgeTelegram(){
    // Intialize variables
    char chrBuffer[8];
    char chrStr[8];
    
    int iCommandNumber = 0;

    // Write {"A": COMMAND_TELEGRAM_NUMBER
    c_JSONParser_->funWrapObjectChar(chrBuffer, ACKNOWLEDGE_TELEGRAM, true);

    iCommandNumber = c_DataSoftwareStorage_->get_DataStorageGeneral()->
        get_CommandNumber();

    strcat(chrBuffer, itoa(iCommandNumber, chrStr, 10));
    strcat(chrBuffer, "}");

    this->funWriteSerial(chrBuffer);

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to send error telegram to the serial port
 * @return Error code
 *****************************************************************************/ 
int C_Communication::funSendErrorTelegram(int iErrorCode){
    // Intialize variables
    char chrBuffer[128];
    char chrStr[16];

    // Write {"E": COMMAND_TELEGRAM_NUMBER
    c_JSONParser_->funWrapObjectChar(chrBuffer, ERROR_TELEGRAM, true);

    strcat(chrBuffer, itoa(iErrorCode, chrStr, 10));
    strcat(chrBuffer, "}");

    this->funWriteSerial(chrBuffer);

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to send command telegram to the serial port
 * @param chrCommandType: Command type which should be send 
 * @return Error code
 *****************************************************************************/ 
int C_Communication::funSendCommandTelegram(char * chrCommandType){
    // Initalize variables
    char chrBuffer1[7];
    char chrBuffer2[2];
    char chrBuffer[32];

    c_JSONParser_->funWrapObjectChar(chrBuffer, COMMAND_TELEGRAM, true);
    c_JSONParser_->funWrapObjectChar(chrBuffer1, COMMAND_EXC_STR, false);

    strncat(chrBuffer, itoa(COMMAND_EXC, chrBuffer2, 10), 
        sizeof(itoa(COMMAND_EXC, chrBuffer2, 10)));
    strncat(chrBuffer, chrBuffer1, sizeof(chrBuffer1));
    strncat(chrBuffer, "\"", sizeof("\""));
    strncat(chrBuffer, chrCommandType, sizeof(chrCommandType));
    strncat(chrBuffer, "\"}", sizeof("\"}"));

    if (WiFiEnabled){
        // Send char array via WiFi
        Udp.beginPacket(IPaddress, WiFiSERVERPORT);
        Udp.write(chrBuffer);
        Udp.endPacket();
    }
    else {
        // Send char arry via serial port
        Serial.write(chrBuffer);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to send data to the serial port
 * @param chrJSONString: Part of the JSON string
 * @return Error code encoded as integer
 *****************************************************************************/ 
int C_Communication::funWriteSerial(char* chrJSONString){
    if (WiFiEnabled){
        // Send char array via WiFi
        Udp.beginPacket(IPaddress, WiFiSERVERPORT);
        Udp.write(chrJSONString);
        Udp.endPacket();
    }
    else {
        // Send char arry via serial port
        Serial.write(chrJSONString);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Function to send int to the serial port
 * @param arrcName: Name of the JSON object
 * @param iValue: Integer value (e.g. cycle number)
 * @return Error code encoded as integer
 ****************************************************************************
int C_Communication::funWriteSerial(char* arrcName, int iValue){
    // Initialize variables
    char chrStr[8];

    // Convert int to char array 
    itoa(iValue, chrStr, 10);

    if (WiFiEnabled){
        // Send char array via WiFi
        Udp.beginPacket(IPaddress, WiFiSERVERPORT);
        Udp.write(arrcName);
        Udp.write(chrStr);
        Udp.endPacket();
    }
    else {
        // Write object name
        Serial.write(arrcName);

        // Send char arry via serial port
        Serial.write(chrStr);
    }
    return EC_NO_ERROR;    
}
*/ 
/******************************************************************************
 * @brief Function to read JSON telegram from serial connection
 * @return string containing JSON telegram
 *****************************************************************************/ 
String C_Communication::funReadSerial(){
    // Initialize variables
    int iObjectCounter = 0;
    int iTimeoutCounter = 0;

    char cInputByte[1];
    char cPacketBuffer[255];
    // Check if WiFi is enabled
    if (WiFiEnabled){
        // Try to parse data
        iObjectCounter = Udp.parsePacket();

        // Check if data was received
        if (iObjectCounter > 0){
            // Read received data
            iTimeoutCounter = Udp.read(cPacketBuffer, 255);
            if (iTimeoutCounter > 0){
                cPacketBuffer[iTimeoutCounter] = 0;
            }
            return cPacketBuffer;
        }
    }
    else {
        String strSerialBuffer = "";    

        while (true){
            // Read one byte from serial connection
            Serial.readBytes(cInputByte, 1);

            // Increase/ decrease object counter to read complete JSON telegrams
            if (cInputByte[0] == '}'){
                iObjectCounter -= 1;
            }
            else if(cInputByte[0] == '{'){
                iObjectCounter += 1;
            }

            // Append byte to serial buffer
            strSerialBuffer += cInputByte[0];

            // Break out of while loop if no JSON file was read or telegram is
            // completed.
            if(iObjectCounter == 0){
                break;
            }

            // Increase timout counter
            iTimeoutCounter += 1;

            // Break out if faulty message is read. Try to read at max 512 bytes.
            if (iTimeoutCounter >= MAX_TELEGRAM_LENGTH){
                break;
            }
        }
        return strSerialBuffer;
    }
    return "";
}

#endif /* serial_communication_CPP */
/******************************************************************************
 * @brief: Source file for the JSON parser, which translates the data, which 
 * should be sended into JSON format.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *
 *****************************************************************************/

// Include guard
#ifndef json_parser_CPP
#define json_parser_CPP

// Include headers
#include "json_parser.h"

/******************************************************************************
 * @brief Constructor of the class C_JSONParser
 * 
 *****************************************************************************/ 
C_JSONParser::C_JSONParser(){}

/******************************************************************************
 * @brief Starting method for the class C_JSONParser
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_JSONParser::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Initialize variables
    iCommandNumber_ = 0;
    strCommandType_ = "";
    chrExperimentType_[4];

    // Save object references
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;
    c_DataStorageGeneral_ = c_DataSoftwareStorage->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage->get_DataStorageLocal();
}

/******************************************************************************
 * @brief Wrapping the object name with additional required data
 * @param chrWrappedObject: Char array of minimum size 6
 * @param arrcObjectName: Object name as charr array
 * @param bPrimaryObject: Flag indicating if the object is at the start of a 
 *                        JSON-telgeram 
 * @return encoded error code
 *****************************************************************************/
int C_JSONParser::funWrapObjectChar(char * chrWrappedObject, 
    char * arrcObjectName, bool bPrimaryObject){
    if (bPrimaryObject == true){
        // Wrap Objectname from: ObjName -> "{"ObjName:"
        strncpy(chrWrappedObject, "{\"", sizeof("{\""));
        strncat(chrWrappedObject, arrcObjectName, sizeof(arrcObjectName));
        strncat(chrWrappedObject, "\":", sizeof("\":"));
    }
    else{
        // Wrap Objectname from: ObjName -> ,"ObjName:"
        strncpy(chrWrappedObject, ",\"", sizeof(",\""));
        strncat(chrWrappedObject, arrcObjectName, sizeof(arrcObjectName));
        strncat(chrWrappedObject, "\":", sizeof("\":"));
    }

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Wrapping the object name with additional required data
 * @param arrcObjectName: Object name as a string
 * @param bPrimaryObject: Flag indicating if the object is at the start of a 
 *                        JSON-telgeram 
 * @return Return wrapped object name
 *****************************************************************************/
String C_JSONParser::funWrapObjectString(String strObjectName, 
                                      bool bPrimaryObject){
    // Initialize variables
    String strConcat;

    if (bPrimaryObject == true){
        // Wrap Objectname from: ObjName -> "{"ObjName:"
        strConcat = "{\"" + strObjectName + "\":";
    }
    else{
        // Wrap Objectname from: ObjName -> ,"ObjName:"
        strConcat = ",\"" + strObjectName + "\":";
    }
    return strConcat; 
}

/******************************************************************************
 * @brief Analyzing incoming string from serial port and extracting relevant 
 * data
 * @param strJSON: string containing JSON telegram
 * @return Status encoded in int value
 *****************************************************************************/
int C_JSONParser::funParseSerialData(String strJSON){
    // Initialzie variables
    int iJSONLength = strJSON.length();
    int iCurrentPosition = 0;
    int iTempPosition = 0;

    String strTemp = "";    
       
    // Update reference
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Check for "{"
    if (strJSON[iCurrentPosition] != '{'){
        // Received data is not in JSON format
        return EC_JSON_PARSER + EC_JP_NO_JSON_FORMAT;
    }
    iCurrentPosition += 1;

    // Check for string
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckString(strJSON,
                                            iJSONLength,
                                            iCurrentPosition);

    // Check if string was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition+1, iCurrentPosition);
    }         
    else {
        // Error occured, no string found
        return EC_JSON_PARSER + EC_JP_NO_STRING_FOUND;
    }
    iCurrentPosition += 1;

    // Check for type of telegram
    // Command telegram
    if (strTemp == COMMAND_TELEGRAM){
        iCurrentPosition = this->funParseCommandTelegram(
            strJSON, iJSONLength, iCurrentPosition);
    }
    // Acknowledge telegram
    else if (strTemp == ACKNOWLEDGE_TELEGRAM){
        /* TODO 28.06.2021 if acknowledge telegrams to the FreiStat 
        get implemented*/
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Analyzing command telegram
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funParseCommandTelegram(String strJSON, int iJSONLength, 
                                          int iCurrentPosition){
    // Initialzie variables
    int iTempPosition = 0;

    String strTemp = "";

    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);

    // Check for ':'
    if (strJSON[iCurrentPosition] != ':'){
        return iCurrentPosition;
    }
        
    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);

    iCurrentPosition += 1; 
    
    // Check for number
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckNumber(
        strJSON, iJSONLength, iCurrentPosition);

    // Check if number was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition, iCurrentPosition);
                
        // Save command number in temp variable
        iCommandNumber_ = strTemp.toInt();
    }
    else {
        return iCurrentPosition;
    }

    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);

    // Check for ','
    if (strJSON[iCurrentPosition] != ','){
        return iCurrentPosition;
    }

    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);
    iCurrentPosition += 1;
        
    // Check for string
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckString(
        strJSON, iJSONLength, iCurrentPosition);

    // Check if string was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition+1, iCurrentPosition);
        iCurrentPosition += 1;
        // Save command type in temp variable
        strCommandType_ = strTemp;
    }
    else {
        return iCurrentPosition;
    }
    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);    

    // Check for ':'
    if (strJSON[iCurrentPosition] != ':'){
        return iCurrentPosition;
    }
    iCurrentPosition += 1; 

    // Check for whitespaces
    iCurrentPosition = this->funCheckWhiteSpaces(
        strJSON, iJSONLength, iCurrentPosition);

    // Change behavior depending on Command number send
    switch (iCommandNumber_){
    // Telegram with experiment type was send
    case COMMAND_EXT:
        iCurrentPosition = this->funParseExperimentType(
            strJSON, iJSONLength, iCurrentPosition);

        // Check if error has occured
        if (iCurrentPosition == iJSONLength - 1){
            // Save parameters
            c_DataStorageGeneral_->set_CommandNumber(iCommandNumber_);
            c_DataStorageLocal_->set_ExperimentType(chrExperimentType_);

            // Experiment type received. Change system status 1 -> 2
            c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_ExT);
        }
        break;
    // Telegram with experiment parameters was send
    case COMMAND_EXP:
        if(c_DataStorageLocal_->get_ExperimentType() != UNDEFINED_EC_METHOD){
            iCurrentPosition = this->funParseExperimentParameters(
                strJSON, iJSONLength, iCurrentPosition);
        }
        // Check if error has occured
        if (iCurrentPosition == iJSONLength - 1){
            // Save parameters
            c_DataStorageGeneral_->set_CommandNumber(iCommandNumber_);

            // Experiment parameters received. Change system status 2 -> 3
            c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_ExP);     
        }
        break;
    // Telegram with control command was send
    case COMMAND_EXC:
        iCurrentPosition = this->funParseControl(
            strJSON, iJSONLength, iCurrentPosition);

        // Check if error has occured
        if (iCurrentPosition == iJSONLength - 1){
            // Save parameters
            c_DataStorageGeneral_->set_CommandNumber(iCommandNumber_);

            // Save control command
            c_DataSoftwareStorage_->set_ControlStatus(iControlStatus_);

            if (iControlStatus_ == FREISTAT_WAIT_I){
                // TODO implement case for waiting
            }                                                                                    
            else if (iControlStatus_ == FREISTAT_START_I){
                // Control command was received. Change system status 3 -> 4
                c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_STARTED);
            }
            else if (iControlStatus_ == FREISTAT_STOP_I){
                c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_WAITING);     
            }
        }
        break;
    // Telegram with sequence control command was send
    case COMMAND_EXS:
        iCurrentPosition = this->funParseSequenceControl(
            strJSON, iJSONLength, iCurrentPosition);

        // Check if error has occured
        if (iCurrentPosition == iJSONLength - 1){
            // Save parameters
            c_DataStorageGeneral_->set_CommandNumber(iCommandNumber_);

            // Save sequence control command
            c_DataSoftwareStorage_->set_SequenceStatus(iControlStatus_);

            if (iControlStatus_ == FREISTAT_EXS_ENABLED){
                c_DataStorageLocal_->set_ExperimentType(SEQUENCE);
            }
        }        
        break;
    default:
        break;
    }
    // Parsing successful
    return EC_JP_PARSING_SUC;
}

/******************************************************************************
 * @brief Extract sequence control command from JSON telegram
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funParseSequenceControl(String strJSON, int iJSONLength, 
                                          int iCurrentPosition){
    // Initialzie variables
    int iTempPosition = 0;

    String strTemp = "";

    // Check for string
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckString(
        strJSON, iJSONLength, iCurrentPosition);

    // Check if string was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition+1, iCurrentPosition);
        iCurrentPosition += 1;
                                        
        // Save sequence control command
        if (strTemp == SEQUENCE_ENABLE){
            iControlStatus_ = FREISTAT_EXS_ENABLED;
        }
        else if (strTemp == SEQUENCE_DISABLE){
            iControlStatus_ = FREISTAT_EXS_DISABLED;
        }
    }
    else {
        return iCurrentPosition;
    }
    // Check for '}'
    if (strJSON[iCurrentPosition] != '}'){
        return iCurrentPosition;
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Extract control command from JSON telegram
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funParseControl(String strJSON, int iJSONLength, 
                                  int iCurrentPosition){
    // Initialzie variables
    int iTempPosition = 0;

    String strTemp = "";

    // Check for string
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckString(
        strJSON, iJSONLength, iCurrentPosition);

    // Check if string was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition+1, iCurrentPosition);
        iCurrentPosition += 1;
                                        
        // Save control command
        if (strTemp == FREISTAT_WAIT_STR){
            iControlStatus_ = FREISTAT_WAIT_I;
        }
        else if (strTemp == FREISTAT_START_STR){
            iControlStatus_ = FREISTAT_START_I;
        }
        else if (strTemp == FREISTAT_STOP_STR){
            iControlStatus_ = FREISTAT_STOP_I;
        }
    }
    else {
        return iCurrentPosition;
    }
    // Check for '}'
    if (strJSON[iCurrentPosition] != '}'){
        return iCurrentPosition;
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Extract experiment type from string
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funParseExperimentType(String strJSON, int iJSONLength, 
        int iCurrentPosition){
    // Initialzie variables
    int iTempPosition = 0;

    String strTemp = "";

    // Check for string
    iTempPosition = iCurrentPosition;
    iCurrentPosition = this->funCheckString(strJSON, iJSONLength, 
        iCurrentPosition);

    // Check if string was found
    if (iTempPosition < iCurrentPosition){
        strTemp = strJSON.substring(iTempPosition+1, iCurrentPosition);
        iCurrentPosition += 1;
                                        
        // Save experiment type in temp variable
        strTemp.toCharArray(chrExperimentType_, sizeof(chrExperimentType_));
    }
    else {
        return iCurrentPosition;
    }
    // Check for '}'
    if (strJSON[iCurrentPosition] != '}'){
        return iCurrentPosition;
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Extract experiment parameters from string
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funParseExperimentParameters(String strJSON, int iJSONLength, 
                                               int iCurrentPosition){
    // Initialzie variables
    int iTempPosition = 0;

    String strTempParameter = "";
    String strTempNumber = "";

    // Get current parameters
    String strTempExperimentType = c_DataStorageLocal_->get_ExperimentType();      
    
    // Check for '{'
    if (strJSON[iCurrentPosition] != '{'){
        return iCurrentPosition;
    }
    iCurrentPosition += 1;

    // Loop over all experiment parameters
    for (int iPosition = iCurrentPosition; iPosition < iJSONLength; iPosition ++){
        // Check for string
        iTempPosition = iCurrentPosition;
        iCurrentPosition = this->funCheckString(strJSON, iJSONLength,
                                                iCurrentPosition);

        // Check if string was found
        if (iTempPosition < iCurrentPosition){
            strTempParameter = strJSON.substring(iTempPosition + 1,
                                                 iCurrentPosition);
            iCurrentPosition += 1; 
        }
        else {
            break;
        }
        // Check for ':'
        if (strJSON[iCurrentPosition] != ':'){
            break;
        }
        iCurrentPosition += 1;

        // Check for whitespaces
        iCurrentPosition = this->funCheckWhiteSpaces(
            strJSON, iJSONLength, iCurrentPosition);

        // Check for '['
        if (strJSON[iCurrentPosition] == '['){
            // Save reference
            int iReferencePosition = iCurrentPosition;

            iCurrentPosition += 1;

            // Save current position
            iTempPosition = iCurrentPosition;

            while(iReferencePosition != iCurrentPosition){
                // Check for whitespaces
                iCurrentPosition = this->funCheckWhiteSpaces(
                    strJSON, iJSONLength, iCurrentPosition);

                // Update reference position
                iReferencePosition = iCurrentPosition;

                // Check for number
                iCurrentPosition = this->funCheckNumber(
                    strJSON, iJSONLength, iCurrentPosition);
                
                // Check if numbers were found
                if (iTempPosition < iCurrentPosition){
                    strTempNumber = strJSON.substring(iTempPosition, 
                        iCurrentPosition);
                }

                // Check for whitespaces
                iCurrentPosition = this->funCheckWhiteSpaces(
                    strJSON, iJSONLength, iCurrentPosition);

                // Check for ','
                if (strJSON[iCurrentPosition] == ','){
                    iCurrentPosition += 1;
                }
 
                // Check for whitespaces
                iCurrentPosition = this->funCheckWhiteSpaces(
                    strJSON, iJSONLength, iCurrentPosition);
            }
            // Check for ']'
            if (strJSON[iCurrentPosition] == ']'){
                iCurrentPosition += 1;
            }
            else {
                break;
            }
        }
        else {
            // Check for number
            iTempPosition = iCurrentPosition;
            iCurrentPosition = this->funCheckNumber(
                strJSON, iJSONLength, iCurrentPosition);
            // Check if number was found
            if (iTempPosition < iCurrentPosition){
                strTempNumber = strJSON.substring(iTempPosition, 
                    iCurrentPosition);
            }
            else {
                break;
            }
        }

        // Check for electrochemical method
        if (strTempExperimentType == OCP){
            int iErrorCode = this->funHandleOCPParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == CA){
            int iErrorCode = this->funHandleCAParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == LSV){
            int iErrorCode = this->funHandleLSVParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == CV){
            int iErrorCode = this->funHandleCVParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == NPV){
            int iErrorCode = this->funHandleNPVParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == DPV or
                 strTempExperimentType == SWV){
            int iErrorCode = this->funHandleDPVParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }
        }
        else if (strTempExperimentType == SEQUENCE){
            int iErrorCode = this->funHandleSequenceParameter(
                strTempParameter, strTempNumber);
            if (iErrorCode != 0){
                break;
            }        
        }
        else {
            // TODO other electrochemical methods 30.06.2021
        }          

        // Check for '}' or ','
        if (strJSON[iCurrentPosition] == '}'){
            iCurrentPosition += 1;
            break;
        }
        else if (strJSON[iCurrentPosition] == ','){
            iCurrentPosition += 1;
                
            // Check for whitespaces
            iCurrentPosition = this->funCheckWhiteSpaces(
                strJSON, iJSONLength, iCurrentPosition);
        }
        else {
            break;
        }
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleCAParameter(String strParameter, String strNumber){
    // Initialize variables
    int iEntry = 0;
    int iTempPosition = -1 ;

    // Check if parameter is known as CA parameter
    if (strParameter == POTENTIAL_STEPS){
        for (int iPosition = 0; iPosition < strNumber.length(); iPosition ++){
            // Check for ','
            if (strNumber[iPosition] == ',' || strNumber[iPosition] == ' '){
                if (iPosition > iTempPosition + 1){
                    c_DataStorageLocal_->set_PotentialSteps(strNumber.substring(
                        iTempPosition + 1, iPosition).toFloat(), iEntry);

                    iEntry += 1;
                }
                iTempPosition = iPosition;
            }
        }

        if (strNumber.length() > iTempPosition + 1){
            c_DataStorageLocal_->set_PotentialSteps(strNumber.substring(
                iTempPosition + 1, strNumber.length()).toFloat(), iEntry);
        }
        // Save amount of entries
        c_DataStorageLocal_->set_BufferEntries(iEntry + 1);
    }
    else if (strParameter == PULSE_LENGTH){
        for (int iPosition = 0; iPosition < strNumber.length(); iPosition ++){
            // Check for ','
            if (strNumber[iPosition] == ',' || strNumber[iPosition] == ' '){
                if (iPosition > iTempPosition + 1){
                    c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
                        iTempPosition + 1, iPosition).toFloat(), iEntry);

                    iEntry += 1;
                }
                iTempPosition = iPosition;
            }
        }
        if (strNumber.length() > iTempPosition + 1){
            c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
                iTempPosition + 1, strNumber.length()).toFloat(), iEntry);
        }
    }
    else if (strParameter == SAMPLING_RATE){
        float fTempSamplingRate = strNumber.toFloat();

        // Check if sampling rate is positive
        if (fTempSamplingRate < 0.0){
            fTempSamplingRate = - fTempSamplingRate;
        }

        // Save sampling rate
        c_DataStorageLocal_->set_Scanrate(fTempSamplingRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == FIXED_WE_POTENTIAL){
        if (strNumber.toInt() == 1){
            // Save that fixed working electrode potential is used
            c_DataStorageLocal_->set_FixedWEPotential(true);
        }
        else if (strNumber.toInt() == 0){
            // Save that fixed working electrode potential is not used
            c_DataStorageLocal_->set_FixedWEPotential(false);
        }
    }
    else if (strParameter == LPTIA_RTIA_SIZE){
        int iTempRtiaSize = strNumber.toInt();

        // Save encoded LPTIA Rtia size
        c_DataStorageLocal_->set_LPTIARtiaSize(iTempRtiaSize);
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleOCPParameter(String strParameter, String strNumber){

    // Check if parameter is known as OCP parameter
    if (strParameter == PULSE_LENGTH){
        c_DataStorageLocal_->set_StartVoltage(strNumber.toFloat()); // TODO
    }
    else if (strParameter == SAMPLING_RATE){
        float fTempSamplingRate = strNumber.toFloat();

        // Check if sampling rate is positive
        if (fTempSamplingRate < 0.0){
            fTempSamplingRate = - fTempSamplingRate;
        }
    }
    else if (strParameter == SCAN_RATE){
        float fTempScanRate = strNumber.toFloat();

        // Check if scan rate is positive
        if (fTempScanRate < 0){
            fTempScanRate = - fTempScanRate;
        }
        // Save scan rate
        c_DataStorageLocal_->set_Scanrate(fTempScanRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleLSVParameter(String strParameter, String strNumber){

    // Check if parameter is known as CV parameter
    if (strParameter == START_POTENTIAL){
        c_DataStorageLocal_->set_StartVoltage(strNumber.toFloat());
    }

    else if (strParameter == STOP_POTENTIAL){
        c_DataStorageLocal_->set_LowerVoltage(strNumber.toFloat());
        c_DataStorageLocal_->set_UpperVoltage(strNumber.toFloat());
    }
    else if (strParameter == STEP_SIZE){
        float fTempStepSize = strNumber.toFloat();

        // Check if step size is positive
        if (fTempStepSize < 0){
            fTempStepSize = - fTempStepSize;
        }
        // Save step size
        c_DataStorageLocal_->set_Stepsize(fTempStepSize);
    }
    else if (strParameter == SCAN_RATE){
        float fTempScanRate = strNumber.toFloat();

        // Check if scan rate is positive
        if (fTempScanRate < 0){
            fTempScanRate = - fTempScanRate;
        }
        // Save scan rate
        c_DataStorageLocal_->set_Scanrate(fTempScanRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == LPTIA_RTIA_SIZE){
        int iTempRtiaSize = strNumber.toInt();

        // Save encoded LPTIA Rtia size
        c_DataStorageLocal_->set_LPTIARtiaSize(iTempRtiaSize);
    }
    else if (strParameter == FIXED_WE_POTENTIAL){
        if (strNumber.toInt() == 1){
            // Save that fixed working electrode potential is used
            c_DataStorageLocal_->set_FixedWEPotential(true);
        }
        else if (strNumber.toInt() == 0){
            // Save that fixed working electrode potential is not used
            c_DataStorageLocal_->set_FixedWEPotential(false);
        }
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleCVParameter(String strParameter, String strNumber){

    // Check if parameter is known as CV parameter
    if (strParameter == START_POTENTIAL){
        c_DataStorageLocal_->set_StartVoltage(strNumber.toFloat());
    }
    else if (strParameter == LOWER_POTENTIAL){
        c_DataStorageLocal_->set_LowerVoltage(strNumber.toFloat());
    }
    else if (strParameter == UPPER_POTENTIAL){
        c_DataStorageLocal_->set_UpperVoltage(strNumber.toFloat());
    }
    else if (strParameter == STEP_SIZE){
        float fTempStepSize = strNumber.toFloat();

        // Check if step size is positive
        if (fTempStepSize < 0){
            fTempStepSize = - fTempStepSize;
        }
        // Save step size
        c_DataStorageLocal_->set_Stepsize(fTempStepSize);
    }
    else if (strParameter == SCAN_RATE){
        float fTempScanRate = strNumber.toFloat();

        // Check if scan rate is positive
        if (fTempScanRate < 0){
            fTempScanRate = - fTempScanRate;
        }
        // Save scan rate
        c_DataStorageLocal_->set_Scanrate(fTempScanRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == LPTIA_RTIA_SIZE){
        int iTempRtiaSize = strNumber.toInt();

        // Save encoded LPTIA Rtia size
        c_DataStorageLocal_->set_LPTIARtiaSize(iTempRtiaSize);
    }
    else if (strParameter == FIXED_WE_POTENTIAL){
        if (strNumber.toInt() == 1){
            // Save that fixed working electrode potential is used
            c_DataStorageLocal_->set_FixedWEPotential(true);
        }
        else if (strNumber.toInt() == 0){
            // Save that fixed working electrode potential is not used
            c_DataStorageLocal_->set_FixedWEPotential(false);
        }
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleNPVParameter(String strParameter, String strNumber){
    // Initialize variables
    int iEntry = 0;
    int iTempPosition = -1 ;

    // Check if parameter is known as DPV parameter
    if (strParameter == BASE_POTENTIAL){
        c_DataStorageLocal_->set_PotentialSteps(strNumber.toFloat(), 1);
    }
    else if (strParameter == START_POTENTIAL){
        c_DataStorageLocal_->set_StartVoltage(strNumber.toFloat());
    }
    else if (strParameter == STOP_POTENTIAL){
        c_DataStorageLocal_->set_UpperVoltage(strNumber.toFloat());
    }
    else if (strParameter == DELTA_V_STAIRCASE){
        c_DataStorageLocal_->set_PotentialSteps(strNumber.toFloat(), 0);
    }
    else if (strParameter == PULSE_LENGTH){
        for (int iPosition = 0; iPosition < strNumber.length(); iPosition ++){
            // Check for ','
            if (strNumber[iPosition] == ','){
                c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
                    iTempPosition + 1, iPosition).toFloat(), iEntry);
                
                // Increment counters
                iEntry += 1;
                iTempPosition = iPosition + 1;
            }
        }
        c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
            iTempPosition, strNumber.length()).toFloat(), iEntry);
    }
    else if (strParameter == SAMPLING_DURATION){
        float fTempSamplingRate = strNumber.toFloat();

        // Check if sampling rate is positive
        if (fTempSamplingRate < 0.0){
            fTempSamplingRate = - fTempSamplingRate;
        }

        // Save sampling rate
        c_DataStorageLocal_->set_Scanrate(fTempSamplingRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == LPTIA_RTIA_SIZE){
        int iTempRtiaSize = strNumber.toInt();

        // Save encoded LPTIA Rtia size
        c_DataStorageLocal_->set_LPTIARtiaSize(iTempRtiaSize);
    }
    else if (strParameter == FIXED_WE_POTENTIAL){
        if (strNumber.toInt() == 1){
            // Save that fixed working electrode potential is used
            c_DataStorageLocal_->set_FixedWEPotential(true);
        }
        else if (strNumber.toInt() == 0){
            // Save that fixed working electrode potential is not used
            c_DataStorageLocal_->set_FixedWEPotential(false);
        }
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the defined
 * electrochemical method
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleDPVParameter(String strParameter, String strNumber){
    // Initialize variables
    int iEntry = 0;
    int iTempPosition = -1 ;

    // Check if parameter is known as DPV parameter
    if (strParameter == START_POTENTIAL){
        c_DataStorageLocal_->set_StartVoltage(strNumber.toFloat());
    }
    else if (strParameter == STOP_POTENTIAL){
        c_DataStorageLocal_->set_UpperVoltage(strNumber.toFloat());
    }
    else if (strParameter == DELTA_V_STAIRCASE){
        c_DataStorageLocal_->set_PotentialSteps(strNumber.toFloat(), 0);
    }
    else if (strParameter == DELTA_V_PEAK){
        c_DataStorageLocal_->set_PotentialSteps(strNumber.toFloat(), 1);
    }
    else if (strParameter == PULSE_LENGTH){
        for (int iPosition = 0; iPosition < strNumber.length(); iPosition ++){
            // Check for ','
            if (strNumber[iPosition] == ','){
                c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
                    iTempPosition + 1, iPosition).toFloat(), iEntry);
                
                // Increment counters
                iEntry += 1;
                iTempPosition = iPosition + 1;
            }
        }
        c_DataStorageLocal_->set_PulseDurations(strNumber.substring(
            iTempPosition, strNumber.length()).toFloat(), iEntry);
    }
    else if (strParameter == SAMPLING_DURATION){
        float fTempSamplingRate = strNumber.toFloat();

        // Check if sampling rate is positive
        if (fTempSamplingRate < 0.0){
            fTempSamplingRate = - fTempSamplingRate;
        }

        // Save sampling rate
        c_DataStorageLocal_->set_Scanrate(fTempSamplingRate);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageLocal_->set_Cycle(iTempCycle);
    }
    else if (strParameter == LPTIA_RTIA_SIZE){
        int iTempRtiaSize = strNumber.toInt();

        // Save encoded LPTIA Rtia size
        c_DataStorageLocal_->set_LPTIARtiaSize(iTempRtiaSize);
    }
    else if (strParameter == FIXED_WE_POTENTIAL){
        if (strNumber.toInt() == 1){
            // Save that fixed working electrode potential is used
            c_DataStorageLocal_->set_FixedWEPotential(true);
        }
        else if (strNumber.toInt() == 0){
            // Save that fixed working electrode potential is not used
            c_DataStorageLocal_->set_FixedWEPotential(false);
        }
    }
    else if (strParameter == MAINS_FILTER){
        if (strNumber.toInt() == 1){
            // Save that the notch filter for 50 Hz/ 60 Hz is not bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(false);
        }
        else if (strNumber.toInt() == 0){
            // Save that the notch filter for 50 Hz/ 60 Hz is bypassed used
            c_DataStorageLocal_->set_AdcNotchFilter(true);
        }        
    }
    else if (strParameter == SINC2_OVERSAMPLING){
        int iTempOSRSinc2 = strNumber.toInt();

        // Save encoded Sinc2 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc2(iTempOSRSinc2);
    }
    else if (strParameter == SINC3_OVERSAMPLING){
        int iTempOSRSinc3 = strNumber.toInt();

        // Save encoded Sinc3 oversampling rate
        c_DataStorageLocal_->set_AdcOsrSinc3(iTempOSRSinc3);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Check if the transmitted parameter is known and part of the sequence
 * parameters
 * @param strParameter: String containing name of the parameter     
 * @param strNumber: String containing value of the parameter
 * @return Retruns error code encoded as integer
 * 0 : Value successfully saved
 * 1 : Parameter not known, check constants.py and constants.c for mismatch or 
 *     check for correct experiment type
 * 2 :  
 *****************************************************************************/
int C_JSONParser::funHandleSequenceParameter(String strParameter, 
                                             String strNumber){
    // Initialize variables
    int iEntry = 0;
    int iTempPosition = -1 ;

    // Check if parameter is known as sequence parameter
    if (strParameter == SEQUENCE_LENGTH){
        int iTempSequenceLength = strNumber.toInt();

        // Check if sequence length is positive
        if (iTempSequenceLength < 0){
            iTempSequenceLength = - iTempSequenceLength;
        }
        // Save cycle number
        c_DataStorageGeneral_->set_SequenceLength(iTempSequenceLength);
    }
    else if (strParameter == CYCLE){
        int iTempCycle = strNumber.toInt();

        // Check if cycle is positive
        if (iTempCycle < 0){
            iTempCycle = - iTempCycle;
        }
        // Save cycle number
        c_DataStorageGeneral_->set_SequenceCycles(iTempCycle);
    }
    else{
        // Error occured parameter not known
        return EC_JSON_PARSER + EC_JP_PARA_NOT_KNOWN;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Analyzing incoming string, check if string is valid
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funCheckString(String strJSON, int iJSONLength, 
                                 int iCurrentPosition){
    // Initalize variables
    bool bErrorFlag = false;

    int iTempPosition = iCurrentPosition;

    // Check for string begin '"'
    if (strJSON[iCurrentPosition] == '\"'){
        // Check for string
        for (int x = iCurrentPosition; x < iJSONLength; x++){
            if(strJSON[x] == '\\'){
                iCurrentPosition += 1;

                if(strJSON[x] == '\"' or
                   strJSON[x] == '\\' or
                   strJSON[x] == '/' or
                   strJSON[x] == 'b' or
                   strJSON[x] == 'f' or
                   strJSON[x] == 'n' or
                   strJSON[x] == 'r' or
                   strJSON[x] == 't' or
                   strJSON[x] == 'u'){

                    iCurrentPosition += 1;

                    // Check for string end '"'
                    if(strJSON[iCurrentPosition] == '\"'){
                        break;
                    }
                }
                else{
                    bErrorFlag = true;  
                }
            }                             
            else{
                iCurrentPosition += 1;
                // Check for string end '"'
                if(strJSON[iCurrentPosition] == '\"'){
                    break;
                }
            }
        }
    }   
    else{
        bErrorFlag = true;
    }
              
    // Check if error has occured
    if (bErrorFlag == true){
        return iTempPosition;
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Analyzing incoming string, check if string contains a digit
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funCheckDigit(String strJSON, int iJSONLength, 
                                int iCurrentPosition){
    // Initalize variables
    bool bErrorFlag = false;

    for (int iPosition = iCurrentPosition; iPosition <  iJSONLength; iPosition++){
        // Check if 0-9 appears
        if(strJSON[iPosition] == '1' or
           strJSON[iPosition] == '2' or
           strJSON[iPosition] == '3' or
           strJSON[iPosition] == '4' or
           strJSON[iPosition] == '5' or
           strJSON[iPosition] == '6' or
           strJSON[iPosition] == '7' or
           strJSON[iPosition] == '8' or
           strJSON[iPosition] == '9' or
           strJSON[iPosition] == '0' ){
            if(iCurrentPosition < iJSONLength - 1){
                iCurrentPosition += 1;
            }
        }
        // Break if fraction or exponent begins
        else if (strJSON[iPosition] == '.' or
                 strJSON[iPosition] == 'E' or
                 strJSON[iPosition] == 'e' ){
            break;
        }
                
        // Next JSON structure begins
        else if (strJSON[iPosition] == ',' or
                 strJSON[iPosition] == '}' or
                 strJSON[iPosition] == ']'){
            break; 
        }   
        // Raise error if another character appears
        else{
            bErrorFlag = true;
            break;
        }
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Analyzing incoming string, check if string contains a number
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funCheckNumber(String strJSON, int iJSONLength, 
                                 int iCurrentPosition){
    // Initalize variables
    bool bErrorFlag = false;

    // Check if number startw with "-" or a number if something else appears
    // it's not a number
    if (strJSON[iCurrentPosition] == '-'){
        iCurrentPosition += 1;
    }
    else if(strJSON[iCurrentPosition] == '1' or
            strJSON[iCurrentPosition] == '2' or
            strJSON[iCurrentPosition] == '3' or
            strJSON[iCurrentPosition] == '4' or
            strJSON[iCurrentPosition] == '5' or
            strJSON[iCurrentPosition] == '6' or
            strJSON[iCurrentPosition] == '7' or
            strJSON[iCurrentPosition] == '8' or
            strJSON[iCurrentPosition] == '9' or
            strJSON[iCurrentPosition] == '0' ){
    }
    else {
            bErrorFlag = true;
    }
            
    // Check for digits
    if(strJSON[iCurrentPosition] == '1' or
       strJSON[iCurrentPosition] == '2' or
       strJSON[iCurrentPosition] == '3' or
       strJSON[iCurrentPosition] == '4' or
       strJSON[iCurrentPosition] == '5' or
       strJSON[iCurrentPosition] == '6' or
       strJSON[iCurrentPosition] == '7' or
       strJSON[iCurrentPosition] == '8' or
       strJSON[iCurrentPosition] == '9' ){
        
        // Enter checking Digit
        if(iCurrentPosition < iJSONLength - 1){
            iCurrentPosition += 1;
            iCurrentPosition = this->funCheckDigit(strJSON, 
                                                   iJSONLength, 
                                                   iCurrentPosition);
        }
        // Check if error occured
        if (bErrorFlag == true){
            return iCurrentPosition; 
        }       
    } 
    // Check for 0 (in case of decimals)
    else if (strJSON[iCurrentPosition] == '0'){
        iCurrentPosition += 1;  
    }
    // Return in error case
    else { 
        return iCurrentPosition;
    }
    // Check for fraction
    if (strJSON[iCurrentPosition] == '.'){
        // Enter checking Digit
        if(iCurrentPosition < iJSONLength - 1){
            iCurrentPosition += 1;
        }
        else{
            return iCurrentPosition;
        }             
        iCurrentPosition = this->funCheckDigit(strJSON, 
                                               iJSONLength, 
                                               iCurrentPosition);
        // Check if error occured
        if (bErrorFlag == true){
            return iCurrentPosition;
        }
    }
    // Check for exponent
    if (strJSON[iCurrentPosition] == 'E' or
        strJSON[iCurrentPosition] == 'e' ){
        iCurrentPosition += 1;
        if (strJSON[iCurrentPosition] == '+' or
            strJSON[iCurrentPosition] == '-' ){ 
        }                   
        else if(strJSON[iCurrentPosition] == '1' or
                strJSON[iCurrentPosition] == '2' or
                strJSON[iCurrentPosition] == '3' or
                strJSON[iCurrentPosition] == '4' or
                strJSON[iCurrentPosition] == '5' or
                strJSON[iCurrentPosition] == '6' or
                strJSON[iCurrentPosition] == '7' or
                strJSON[iCurrentPosition] == '8' or
                strJSON[iCurrentPosition] == '9' or
                strJSON[iCurrentPosition] == '0' ){
        }
        else{
            return iCurrentPosition;
        }
        // Enter checking Digit
        if (iCurrentPosition < iJSONLength - 1){
            iCurrentPosition += 1;
            iCurrentPosition = this->funCheckDigit(strJSON, 
                                                   iJSONLength, 
                                                   iCurrentPosition);
        }            
        // Check if error occured
        if (bErrorFlag == true){
            return iCurrentPosition;
        }
    }
    return iCurrentPosition;
}

/******************************************************************************
 * @brief Analyzing incoming string, check for unused whitespaces
 * @param strJSON: String containing JSON telegram
 * @param iJSONLength: Length of the JSON string
 * @param iCurrentPosition: Current position in the string
 * @return: Current position which was checked last
 *****************************************************************************/
int C_JSONParser::funCheckWhiteSpaces(String strJSON, int iJSONLength, 
                                      int iCurrentPosition){
    for (int iPosition = iCurrentPosition; iPosition < iJSONLength; iPosition++){
        // Check if next part in JSON telegram is a whitespace
        if(strJSON[iPosition] == ' '){
        }
        else{
            break;
        }
        iCurrentPosition += 1;
    }
    return iCurrentPosition;   
}

#endif /* json_parser_CPP */
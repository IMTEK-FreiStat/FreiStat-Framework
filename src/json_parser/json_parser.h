/******************************************************************************
 * @brief: Header file for the JSON parser, which translates the data, which 
 * should be sended into JSON format.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef json_parser_H
#define json_parser_H

// Include dependencies
#include "../data_storage/data_storage_general.h"
#include "../data_storage/data_storage_local.h"

/******************************************************************************
 * @brief: Abstract class for JSON Parser
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_JSONParser{
    private:
        // Object pointers
        C_DataSoftwareStorage * c_DataSoftwareStorage_;
        C_DataStorageGeneral * c_DataStorageGeneral_;
        C_DataStorageLocal * c_DataStorageLocal_;

        // Variables
        int iCommandNumber_;
        int iControlStatus_;
        
        char chrExperimentType_[4];

        String strCommandType_;
        

        // Methods
        int funCheckString(String, int, int);
        int funCheckWhiteSpaces(String, int, int);
        int funCheckDigit(String, int, int);
        int funCheckNumber(String, int, int);

        int funParseCommandTelegram(String, int, int);
        int funParseAcknowledgeTelegram(String, int, int);

        int funParseControl(String, int, int);
        int funParseExperimentType(String, int, int);
        int funParseExperimentParameters(String, int, int);
        int funParseSequenceControl(String, int, int);

        int funHandleCAParameter(String, String);
        int funHandleCVParameter(String, String);
        int funHandleNPVParameter(String, String);
        int funHandleDPVParameter(String, String);
        int funHandleLSVParameter(String, String);
        int funHandleOCPParameter(String, String);
        int funHandleSequenceParameter (String, String);

    public:
        // Constructor
        C_JSONParser();

        // Starting method
        void Begin(C_DataSoftwareStorage *);

        // Methods
        int funWrapObjectChar(char *, char*, bool);

        int funParseSerialData(String);

        String funWrapObjectString(String, bool);

};
#endif  /* json_parser_H */
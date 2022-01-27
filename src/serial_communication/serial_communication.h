/******************************************************************************
 * @brief: Header file for establishing communication and sending packages
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *  
 *****************************************************************************/

// Include guard
#ifndef serial_communication_H
#define serial_communication_H

// Include dependencies
#include <WiFi101.h>
#include <WiFiUdp.h>

// Include headers
#include "../json_parser/json_parser.h"

// Constant.cpp dependendent includes
#if WiFiEnabled || FREISTAT_STANDALONE
    #include <SD.h>
#endif

/******************************************************************************
 * @brief: Class for communication
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Communication{
    private:
        // Variables
        int iBaudrate_;
        
        char chrPrefix1_[10];
        char chrPrefix2_[10];
        char chrPrefix3_[10];
        char chrPrefix4_[10];
        char chrPrefix5_[10];
        char chrPrefix6_[10];

        char chrFilename[13];

        // Object pointers
        C_JSONParser * c_JSONParser_;
        C_DataSoftwareStorage * c_DataSoftwareStorage_;

        // WiFi class
        WiFiUDP Udp;
        IPAddress IPaddress;

        // Methods
        int funWriteSerial(char*);
        //int funWriteSerial(char* ,int);   DEPRECATED

        #if WiFiEnabled || FREISTAT_STANDALONE
            File dataFile;
            int iFlushCounter;
        #endif

    public:
        // Constructor
        C_Communication();
        
        // Starting method
        void Begin(int, C_DataSoftwareStorage *);

        // Methods
        void funEndCommunication();

        bool funDataAvailable();

        int funConstructPrefixes(char *); 
        int funEstablishCommunication();
        int funSendAcknowledgeTelegram();
        int funSendCommandTelegram(char *);
        int funSendExperimentData(S_DataContainer, int);
        int funSetupSDcard();
        int funStopSDcard();

        String funReadSerial();
};

#endif /* serial_communication_H */
/******************************************************************************
 * @brief: .Ino File of the FreiStat
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 * @copyright MIT Liscene
 * 
 *  Copyright (c) 2022 University of Freiburg, IMTEK
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 *****************************************************************************/

// Include guard
#ifndef FreiStat_INO
#define FreiStat_INO

// Include dependencies
#include "src/electrochemical_methods/electrochemical_method.h"
#if FREISTAT_STANDALONE
#include "src/interface/interface.h"
#endif

void setup(){
    // Only hardware related configurations.
    // Objects only get created in the loop() to prevent global variables and in-
    // consistencies in the program structure. 
}

void loop(){
    /***************************************************************************/
    // HARDWARE INITALIZATION
    // Create an object for the setup of the AD5940-Chip
    C_AD5940_Setup c_AD5940_Setup = C_AD5940_Setup();
    c_AD5940_Setup.Begin();  

    /***************************************************************************/
    // SOFTWARE INITALIZATION
    // Create an object data software handling in which all references and 
    // program related parameters are stored
    C_DataSoftwareStorage c_DataSoftwareStorage = C_DataSoftwareStorage();
    c_DataSoftwareStorage.Begin(&c_AD5940_Setup);  

    // Create an object to store the general experiment data
    C_DataStorageGeneral c_DataStorageGeneral = C_DataStorageGeneral();
    c_DataStorageGeneral.Begin(&c_DataSoftwareStorage);

    // Set reference to general data storage object
    c_DataSoftwareStorage.set_DataStorageGeneral(&c_DataStorageGeneral);
    
    // Create an object to store the local experiment data
    C_DataStorageLocal c_DataStorageLocal = C_DataStorageLocal();
    c_DataStorageLocal.Begin();

    // Set reference to local data storage object
    c_DataSoftwareStorage.set_DataStorageLocal(&c_DataStorageLocal);

    // Create an JSON parser object
    C_JSONParser c_JSONParser = C_JSONParser();
    c_JSONParser.Begin(&c_DataSoftwareStorage);

    // Set reference to objects
    c_DataSoftwareStorage.set_JSONParser(&c_JSONParser);

    // Create an object for managing the serial connection
    C_Communication c_Communication = C_Communication();
    c_Communication.Begin(BAUDRATE, &c_DataSoftwareStorage);

    // Set reference to objects
    c_DataSoftwareStorage.set_Communication(&c_Communication);

    // Create object implementing the chosen electrochemical method
    C_ElectrochemicalMethod c_ElectrochemicalMethod = C_ElectrochemicalMethod();

    // Setup of FreiStat completed
    // Update system status: 0 -> 1
    c_DataSoftwareStorage.set_SystemStatus(FREISTAT_WAITING);

    /***************************************************************************/
    // RUN FREISTAT
    // Loop forever
    while (true){
        // Intialize variables
        int iParseResult = 0;
        int iSequencePosition = 0;
        int iTrueSequenceLength = 0;

        // Check if FreiStat is configured for standalone mode
        if (FREISTAT_STANDALONE){
            #if FREISTAT_STANDALONE
            // Create interface class
            C_Interface c_Interface = C_Interface();
            c_Interface.begin();

            // Execute method depending on the selection
            switch (c_Interface.navigate())
            {
            case 0:
                c_DataStorageLocal.set_ExperimentType(CA);
                c_DataStorageLocal.set_BufferEntries(BUFFER_ENTRIES);
                for (int i=0; i<c_DataStorageLocal.get_BufferEntries(); i++){
                    c_DataStorageLocal.set_PotentialSteps(POTENTIAL_STEPS_F[i],i);
                    c_DataStorageLocal.set_PulseDurations(PULSE_LENGTHS_F[i],i);
                }
                break;
            case 1:
                c_DataStorageLocal.set_ExperimentType(LSV);
                break;
            case 2:
                c_DataStorageLocal.set_ExperimentType(CV);
                break;
            case 3:
                c_DataStorageLocal.set_ExperimentType(NPV);
                break;
            case 4:
                c_DataStorageLocal.set_ExperimentType(DPV);
                break;
            case 5:
                c_DataStorageLocal.set_ExperimentType(SWV);
                break;    
            default:
                break;
            }
            // Setup SD-card 
            c_Communication.funSetupSDcard();

            // Setup ec-method object according to the chosen method
            c_ElectrochemicalMethod.Begin(&c_DataSoftwareStorage);

            // Close SD-card file
            c_Communication.funStopSDcard();
            
            // Set system status to waiting state
            c_DataSoftwareStorage.set_SystemStatus(FREISTAT_WAITING);
            #endif
        }

        // Read from serial and interpret send data
        iParseResult = c_JSONParser.funParseSerialData(
            c_Communication.funReadSerial());

        // Check if parsing was successful and proceed
        if (iParseResult == EC_NO_ERROR + EC_JP_PARSING_SUC){
            // Check if sequence mode is enabled or not
            if (c_DataSoftwareStorage.get_SequenceStatus() == 
                FREISTAT_EXS_ENABLED){
                
                // Send acknowledge telegram
                c_Communication.funSendAcknowledgeTelegram();

                // Get pointer from local data storage object and save it,
                // sequence only operates with pointers and not objects
                C_DataStorageLocal * c_CurrentDataStorageLocal = 
                    &c_DataStorageLocal;
                    
                // Set reference of local data storage in data software storage
                // to current to let it be filled by the JSON parser
                c_DataSoftwareStorage.set_DataStorageLocal(
                    c_CurrentDataStorageLocal);

                // Save reference of current local data storage, which is later
                // used as basis for the sequence start
                C_DataStorageLocal * c_ReferenceDataStorageLocal = 
                    c_CurrentDataStorageLocal;

                // Reset system status to waiting state
                c_DataSoftwareStorage.set_SystemStatus(FREISTAT_WAITING);

                // Loop until sequence mode is disabled
                while(c_DataSoftwareStorage.get_SequenceStatus() == 
                      FREISTAT_EXS_ENABLED || 
                      c_DataSoftwareStorage.get_SystemStatus() != 
                      FREISTAT_EXP_STARTED){

                    // Read from serial and interpret send data
                    iParseResult = c_JSONParser.funParseSerialData(
                        c_Communication.funReadSerial());

                    // Check if parsing was successful and proceed
                    if (iParseResult == EC_NO_ERROR + EC_JP_PARSING_SUC){
                        if (c_DataSoftwareStorage.get_SystemStatus() == 
                            FREISTAT_WAITING){
                            // Send acknowledge telegram
                            c_Communication.funSendAcknowledgeTelegram();
                        }
                        else if (c_DataSoftwareStorage.get_SystemStatus() == 
                            FREISTAT_ExT){
                            // Send acknowledge telegram
                            c_Communication.funSendAcknowledgeTelegram();
                        }
                        else if (c_DataSoftwareStorage.get_SystemStatus() == 
                            FREISTAT_ExP){
                            // Send acknowledge telegram
                            c_Communication.funSendAcknowledgeTelegram();                        

                            // Increase true sequence counter
                            iTrueSequenceLength += 1;

                            // Instantiate new local data object
                            C_DataStorageLocal * c_NextDataStorageLocal = 
                                new C_DataStorageLocal();

                            // Call begin method to set it up
                            c_NextDataStorageLocal->Begin();

                            // Set reference in previous data object
                            c_CurrentDataStorageLocal->
                                set_NextDataStorage(c_NextDataStorageLocal);

                            // Make new object the current object
                            c_CurrentDataStorageLocal = c_NextDataStorageLocal;

                            // Update reference in data storage
                            c_DataSoftwareStorage.
                                set_DataStorageLocal(c_CurrentDataStorageLocal);
                        }
                    }
                }
                // Send acknowledge telegram
                c_Communication.funSendAcknowledgeTelegram();
                
                // Reset reference to start position in order to execute
                // sequence in correct order
                c_DataSoftwareStorage.set_DataStorageLocal(
                    c_ReferenceDataStorageLocal->get_NextDataStorage());   

                if (WiFiEnabled){
                    // Disable WiFi communication
                    c_Communication.funEndCommunication();
                    
                    // Setup SD-card 
                    c_Communication.funSetupSDcard();
                }

                // Sequence completely loaded. Start executing first experiment,
                // repeat until whole sequence is done for the defined cycles.
                while (iSequencePosition < 
                       c_DataStorageGeneral.get_SequenceCycles() *
                       c_DataStorageGeneral.get_SequenceLength()){

                    // Setup ec-method object according to the transmitted data
                    c_ElectrochemicalMethod.Begin(&c_DataSoftwareStorage);

                    // Check if experiment was terminated
                    if (c_DataSoftwareStorage.get_SystemStatus() == 
                        FREISTAT_WAITING){
                        break;
                    }

                    // Ec-method done - Move to next sequenece
                    // Increase sequence counter (break requirement)
                    iSequencePosition += 1;

                    // Check if new sequence cycle has begun
                    if (iSequencePosition % c_DataStorageGeneral.
                        get_SequenceLength() == 0){
                        // Jump to first sequence using the reference object
                        c_DataSoftwareStorage.set_DataStorageLocal(
                            c_ReferenceDataStorageLocal->get_NextDataStorage());  
                    }
                    else {
                        // Jump to next sequence using saved references
                        c_DataSoftwareStorage.set_DataStorageLocal(
                            c_DataSoftwareStorage.get_DataStorageLocal()->
                            get_NextDataStorage());   
                    }
                }

                if (WiFiEnabled){
                    // Restablish communication
                    c_Communication.funEstablishCommunication();
                    
                    // Close SD-card file
                    c_Communication.funStopSDcard();
                }

                // Send telegram that experiment is completed and that 
                // FreiStat stopped
                c_Communication.funSendCommandTelegram(FREISTAT_STOP_STR);

                c_DataSoftwareStorage.set_DataStorageLocal(c_ReferenceDataStorageLocal);

                // Delete chain of local data storage objects to free memory
                iSequencePosition = 0;
                c_DataStorageGeneral.set_SequenceLength(c_DataStorageGeneral.get_SequenceLength() + 2);

                c_DataSoftwareStorage.set_DataStorageLocal(c_ReferenceDataStorageLocal);

                
                while (c_DataStorageGeneral.get_SequenceLength() != 0){
                    if (iSequencePosition == c_DataStorageGeneral.
                        get_SequenceLength()){

                        delete c_DataSoftwareStorage.get_DataStorageLocal();

                        // Update sequence length
                        c_DataStorageGeneral.set_SequenceLength(c_DataStorageGeneral.get_SequenceLength() - 1); 

                        // Jump to first sequence using the reference object
                        c_DataSoftwareStorage.set_DataStorageLocal(
                            c_ReferenceDataStorageLocal->get_NextDataStorage());  

                        iSequencePosition = 0;
                    }
                    else {
                        // Jump to next sequence using saved references
                        c_DataSoftwareStorage.set_DataStorageLocal(
                            c_DataSoftwareStorage.get_DataStorageLocal()->
                            get_NextDataStorage());   
                    }
                    iSequencePosition += 1;
                }

                c_DataSoftwareStorage.set_DataStorageLocal(c_ReferenceDataStorageLocal);
                
                // Set system status to waiting state
                c_DataSoftwareStorage.set_SystemStatus(FREISTAT_WAITING);
            }
            // Single mode: only one ec-method
            else if (c_DataSoftwareStorage.get_SequenceStatus() == 
                FREISTAT_EXS_DISABLED){
                if (c_DataSoftwareStorage.get_SystemStatus() == FREISTAT_ExT){
                    // Send acknowledge telegram
                    c_Communication.funSendAcknowledgeTelegram();
                }
                else if (c_DataSoftwareStorage.get_SystemStatus() == 
                    FREISTAT_ExP){
                    // Send acknowledge telegram
                    c_Communication.funSendAcknowledgeTelegram();
                }
                else if (c_DataSoftwareStorage.get_SystemStatus() == 
                    FREISTAT_EXP_STARTED){
                    // Send acknowledge telegram
                    c_Communication.funSendAcknowledgeTelegram();

                    if (WiFiEnabled || FREISTAT_STANDALONE){
                        if (WiFiEnabled){
                            // Disable WiFi communication
                            c_Communication.funEndCommunication();
                        }
                        // Setup SD-card 
                        c_Communication.funSetupSDcard();
                    }

                    // Setup ec-method object according to the transmitted data
                    c_ElectrochemicalMethod.Begin(&c_DataSoftwareStorage);

                    if (WiFiEnabled || FREISTAT_STANDALONE){
                        if (WiFiEnabled){
                            // Restablish communication
                            c_Communication.funEstablishCommunication();
                        }
                        // Close SD-card file
                        c_Communication.funStopSDcard();
                    }

                    // Send telegram that experiment is completed and that 
                    // FreiStat stopped
                    c_Communication.funSendCommandTelegram(FREISTAT_STOP_STR);

                    // Set system status to waiting state
                    c_DataSoftwareStorage.set_SystemStatus(FREISTAT_WAITING);
                }   
            }
        }
        delay(10);
    }
}


void* operator new(size_t size)
{
    void* mem = malloc(size);
    return mem;
}

void operator delete(void* ptr)
{
    free(ptr);
}

void* operator new[] (size_t size)
{
    return (operator new)(size);
}

void operator delete[](void* ptr)
{
    return (operator delete)(ptr);
}

#endif /* FreiStat_INO */
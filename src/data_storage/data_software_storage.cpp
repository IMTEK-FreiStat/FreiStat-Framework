/******************************************************************************
 * @brief: Source file for the data storage which stores all required data for
 * the FreiStat and holds references to all other objects.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef data_software_storage_CPP
#define data_software_storage_CPP

// Include headers
#include "data_software_storage.h"

/******************************************************************************
 * @brief Default constructor of the class C_DataSoftwareStorage
 * 
 *****************************************************************************/ 
C_DataSoftwareStorage::C_DataSoftwareStorage(){}

/******************************************************************************
 * @brief Starting method for the class C_DataSoftwareStorage
 * 
 *****************************************************************************/
void C_DataSoftwareStorage::Begin(C_AD5940_Setup * c_AD5940_Setup){
    // Save reference to AD5940 setup object
    c_AD5940_Setup_ = c_AD5940_Setup;
    
    // Initialize system status
    iSystemStatus_ = FREISTAT_BOOTUP;

    // Initialize sequence status
    iSequenceStatus_ = FREISTAT_EXS_DISABLED;
}

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for saving reference pointer to the general data storage 
 * object
 * @param c_DataStorageGeneral: pointer to the genral data storage object
 *****************************************************************************/
void C_DataSoftwareStorage::set_DataStorageGeneral(C_DataStorageGeneral * 
        c_DataStorageGeneral){
    c_DataStorageGeneral_ = c_DataStorageGeneral;
}

/******************************************************************************
 * @brief Setter method for saving reference pointer to the local data storage 
 * object.
 * @param c_DataStorageLocal: pointer to the local data storage object
 *****************************************************************************/
void C_DataSoftwareStorage::set_DataStorageLocal(C_DataStorageLocal * 
        c_DataStorageLocal){
    c_DataStorageLocal_ = c_DataStorageLocal;
}

/******************************************************************************
 * @brief Setter method for saving reference pointer to JSON parser object
 * @param c_JSONParser: pointer to JSON parser object
 *****************************************************************************/
void C_DataSoftwareStorage::set_JSONParser(C_JSONParser * c_JSONParser){
    c_JSONParser_ = c_JSONParser;
}

/******************************************************************************
 * @brief Setter method for saving reference pointer to communication object
 * @param c_Communication: pointer to Communication object
 *****************************************************************************/
void C_DataSoftwareStorage::set_Communication(C_Communication * c_Communication){
    c_Communication_ = c_Communication;
}

/******************************************************************************
 * @brief Setter method for saving reference pointer to electrochemical method
 * object
 * @param c_ElectrochemicalMethod: pointer to electrochemical method object
 *****************************************************************************/
void C_DataSoftwareStorage::set_ElectrochemicalMethod(C_ElectrochemicalMethod * 
    c_ElectrochemicalMethod){
    c_ElectrochemicalMethod_ = c_ElectrochemicalMethod;
}

/******************************************************************************
 * @brief Method for changing the saved system status
 * @param iSystemStatus: encoded system status
 *****************************************************************************/
void C_DataSoftwareStorage::set_SystemStatus(int iSystemStatus){
    iSystemStatus_ = iSystemStatus;
}

/******************************************************************************
 * @brief Method for changing the saved sequence status
 * @param iSequenceStatus: encoded sequence status
 *****************************************************************************/
void C_DataSoftwareStorage::set_SequenceStatus(int iSequenceStatus){
    iSequenceStatus_ = iSequenceStatus;
}

/******************************************************************************
 * @brief Method for changing the saved control status
 * @param iControlStatus: encoded control status
 *****************************************************************************/
void C_DataSoftwareStorage::set_ControlStatus(int iControlStatus){
    iControlStatus_ = iControlStatus;
}

/******************************************************************************
 * Getter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Getter method for returning reference pointer to AD5940 setup object
 * @return: Returns pointer to AD5940 setup object
 *****************************************************************************/
C_AD5940_Setup * C_DataSoftwareStorage::get_AD5940Setup(){
    return c_AD5940_Setup_;
}

/******************************************************************************
 * @brief Getter method for returning reference pointer to the general data 
 * storage object
 * @return: Returns pointer to the general data storage object
 *****************************************************************************/
C_DataStorageGeneral * C_DataSoftwareStorage::get_DataStorageGeneral(){
    return c_DataStorageGeneral_;
}

/******************************************************************************
 * @brief Getter method for returning reference pointer to the local data 
 * storage object
 * @return: Returns pointer to the local data storage object
 *****************************************************************************/
C_DataStorageLocal * C_DataSoftwareStorage::get_DataStorageLocal(){
    return c_DataStorageLocal_;
}

/******************************************************************************
 * @brief Getter method for returning reference pointer to JSON parser object
 * @return: Returns pointer to JSON parser object
 *****************************************************************************/
C_JSONParser * C_DataSoftwareStorage::get_JSONParser(){
    return c_JSONParser_;
}

/******************************************************************************
 * @brief Getter method for returning reference pointer to communication object
 * @return: Returns pointer to communication object
 *****************************************************************************/
C_Communication * C_DataSoftwareStorage::get_Communication(){
    return c_Communication_;
}

/******************************************************************************
 * @brief Getter method for returning reference pointer to electrochemical 
 * method object
 * @return: Returns pointer to electrochemical method object
 *****************************************************************************/
C_ElectrochemicalMethod * C_DataSoftwareStorage::get_ElectrochemicalMethod(){
    return c_ElectrochemicalMethod_;
}

/******************************************************************************
 * @brief Method for returning current system status
 * @details The system status is encoded in an integer value, from which the 
 * following are defined:
 * System status : Meaning
 * 0 : FreiStat in boot up process
 * 1 : FreiStat in waiting state
 * 2 : Experiment type received, waiting for experiment parameters
 * 3 : Experiment parameters received, completing setup now
 * 4 : FreiStat ready for running experiment, waiting for start telegram
 * 5 : Experiment running
 * 6 : Experiment completed
 * 7 : Experiment canceled
 * @return: Returns encoded system status
 *****************************************************************************/
int C_DataSoftwareStorage::get_SystemStatus(){
    return iSystemStatus_;
}

/******************************************************************************
 * @brief Method for returning the saved sequence status
 * @details The sequenece status is encoded in an integer value, from which the 
 * following are defined:
 * Sequence status  :   Meaning
 * 0                :   Not Defiend
 * 1                :   Enabled
 * 2                :   Disabled
 * @return: encoded sequence status
 *****************************************************************************/
int C_DataSoftwareStorage::get_SequenceStatus(){
    return iSequenceStatus_;
}

/******************************************************************************
 * @brief Method for returning current control status
 * @details The control status is encoded in an integer value, from which the 
 * following are defined:
 * Control status   :   Meaning 
 * 0                :   Waiting
 * 1                :   Start
 * 2                :   Stopp
 * @return: Returns encoded system status
 *****************************************************************************/
int C_DataSoftwareStorage::get_ControlStatus(){
    return iSystemStatus_;
}

#endif  /* data_software_storage_CPP */
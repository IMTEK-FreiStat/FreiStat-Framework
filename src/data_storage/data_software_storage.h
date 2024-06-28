/******************************************************************************
 * @brief: Header file for the data storage which stores all required data for
 * the FreiStat and holds references to all other objects.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef data_software_storage_H
#define data_software_storage_H

// Include dependencies
#include "datatypes.cpp"
#include "../ad5941_FreiStat/ad5941_FreiStat_setup.h"
#include "logger.h"

// Forward declaration
class C_AD5940_Setup;
class C_Communication;
class C_DataStorageGeneral;
class C_DataStorageLocal;
class C_ElectrochemicalMethod;
class C_JSONParser;
class Logger;

/******************************************************************************
 * @brief: Abstract class for data storage.
 * @details: Object stores all data and references which are not related 
 * directly to measurement data.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_DataSoftwareStorage{
    private:
        // System status
        int iSystemStatus_;

        // Sequence status
        int iSequenceStatus_;

        // Control status
        int iControlStatus_;

        // Object pointers
        C_AD5940_Setup * c_AD5940_Setup_;
        C_Communication * c_Communication_;
        C_DataStorageGeneral * c_DataStorageGeneral_;
        C_DataStorageLocal * c_DataStorageLocal_;
        C_ElectrochemicalMethod * c_ElectrochemicalMethod_;
        C_JSONParser * c_JSONParser_;
        Logger  * logger;

    public:
        // Constructor
        C_DataSoftwareStorage();

        // Starting methods
        void Begin(C_AD5940_Setup *);

        // Setter
        void set_ControlStatus(int);
        void set_SequenceStatus(int);
        void set_SystemStatus(int);

        void set_Communication(C_Communication *);
        void set_DataStorageGeneral(C_DataStorageGeneral *);
        void set_DataStorageLocal(C_DataStorageLocal *);
        void set_ElectrochemicalMethod(C_ElectrochemicalMethod *);
        void set_JSONParser(C_JSONParser *);

        // Getter
        int get_ControlStatus();
        int get_SequenceStatus();
        int get_SystemStatus();

        C_AD5940_Setup * get_AD5940Setup();
        C_Communication * get_Communication();
        C_DataStorageGeneral * get_DataStorageGeneral();
        C_DataStorageLocal * get_DataStorageLocal();
        C_ElectrochemicalMethod * get_ElectrochemicalMethod();
        C_JSONParser * get_JSONParser();
};
#endif  /* data_software_storage_H */
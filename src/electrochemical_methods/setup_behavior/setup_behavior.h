/******************************************************************************
 * @brief: Header file containing the Superclass C_Setup from which all other
 * C_Setup_XX setup classes inherit.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_behavior_H
#define setup_behavior_H

// Include dependencies
#include <Arduino.h>
#include "../../data_storage/data_storage_general.h"
#include "../../data_storage/data_storage_local.h"

/******************************************************************************
 * @brief: Abstract class for setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_SetupBehavior{       
    protected:
        // Object references
        C_DataSoftwareStorage * c_DataSoftwareStorage_;
        C_DataStorageGeneral * c_DataStorageGeneral_;
        C_DataStorageLocal * c_DataStorageLocal_;

        // Methods
        int funCalibrateLPTIAResistor();
        int funGetDataPosition(int);
        
    public:
        // Constructor
        C_SetupBehavior();

        // Starting method
        virtual void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_behavior_H */
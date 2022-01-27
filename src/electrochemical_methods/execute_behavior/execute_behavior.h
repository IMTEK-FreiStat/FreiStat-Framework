/******************************************************************************
 * @brief: Header file containing the Superclass C_Execute from which all other
 * C_Execute_XX behavior classes inherit.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *  
 *****************************************************************************/

// Include guard
#ifndef execute_behavior_H
#define execute_behavior_H

// Include dependencies
#include "../../serial_communication/serial_communication.h"

/******************************************************************************
 * @brief: Abstract class for execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_ExecuteBehavior{
    protected:
        // Object references
        C_DataSoftwareStorage * c_DataSoftwareStorage_;
        C_DataStorageGeneral * c_DataStorageGeneral_;
        C_DataStorageLocal * c_DataStorageLocal_;
        
        // Variables
        char chrExperimentType_[4];

        // Methods
        int funGetDataPosition(int);

        int funGetExperimentTypeInt(char *);
        
    public:
        // Constructor
        C_ExecuteBehavior();

        // Starting method
        virtual void Begin(C_DataSoftwareStorage *);

};

#endif /* execute_behavior_H */
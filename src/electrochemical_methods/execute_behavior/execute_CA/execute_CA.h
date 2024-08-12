/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_CA which 
 * defines the behavior of executing an chronoamperometry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *   
 *****************************************************************************/

// Include guard
#ifndef execute_CA_H
#define execute_CA_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_CA behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Execute_CA : public C_ExecuteBehavior{
    private:
        // Variables
        bool bEosInterruptOccured_;
        
        uint32_t iStepCounter_;

        AFERefCfg_Type S_AFEReferenceBufferConfig_;

        // Methods
        int funConfigAfeReferenceBuffer();
        int funControlApplication(uint32_t);
        int funInterruptServiceRoutine();
        int funProcessExperimentData(uint32_t * , uint32_t );
        int funUpdateSequence();

    public:
        // Constructor
        C_Execute_CA();

        // Starting method
        int Begin(C_DataSoftwareStorage *);
};

#endif /* execute_CA_H */
/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_SWV which 
 * defines the behavior of executing an square wave voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *   
 *****************************************************************************/

// Include guard
#ifndef execute_SWV_H
#define execute_SWV_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_SWV behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Execute_SWV : public C_ExecuteBehavior{
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
        C_Execute_SWV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* execute_SWV_H */
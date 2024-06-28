/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_NPV which 
 * defines the behavior of executing an normal pulse voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 *   
 *****************************************************************************/

// Include guard
#ifndef execute_NPV_H
#define execute_NPV_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_NPV behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_Execute_NPV : public C_ExecuteBehavior{
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
        C_Execute_NPV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* execute_NPV_H */
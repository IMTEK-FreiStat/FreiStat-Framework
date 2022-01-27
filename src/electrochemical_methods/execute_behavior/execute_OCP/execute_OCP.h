/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_OCP which 
 * defines the behavior of measuring the open circuit potential
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_OCP_H
#define execute_OCP_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_OCP behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Execute_OCP : public C_ExecuteBehavior{
    private:
        // Variables
        int iAdcPgaGain_;

        float fAdcReferenceVoltage_;
        float fRtiaMagnitude_;

        // Methods
        int funControlApplication(uint32_t);
        int funInterruptServiceRoutine();
        int funProcessExperimentData(uint32_t * , uint32_t );
        int funUpdateSequence();

    public:
        // Constructor
        C_Execute_OCP();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* execute_OCP_H */
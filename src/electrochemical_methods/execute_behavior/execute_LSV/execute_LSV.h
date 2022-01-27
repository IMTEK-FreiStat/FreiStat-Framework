/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_LSV which 
 * defines the behavior of executing an linear sweep voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_LSV_H
#define execute_LSV_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_LSV behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Execute_LSV : public C_ExecuteBehavior{
    private:
        // Variables
        int iAdcPgaGain_;
        int iDacSeqBlock0Address_;
        int iDacSeqBlock1Address_;
        int iStepsPerBlock_;
        int iStepsToUpperVoltage_;

        float fAdcReferenceVoltage_;
        float fRtiaMagnitude_;

        // Methods
        int funControlApplication(uint32_t);
        int funInterruptServiceRoutine();
        int funPreCalulations();
        int funProcessExperimentData(uint32_t * , uint32_t );
        int funSequencerDACControl();
        int funUpdateDACRegister(uint32_t * );

    public:
        // Constructor
        C_Execute_LSV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* execute_LSV_H */
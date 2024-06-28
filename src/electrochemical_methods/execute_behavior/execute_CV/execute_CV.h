/******************************************************************************
 * @brief: Header file containing the subclass (C_Execute) C_Execute_CV which 
 * defines the behavior of executing an cyclic voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_CV_H
#define execute_CV_H

// Include dependencies
#include "../execute_behavior.h"

/******************************************************************************
 * @brief: Abstract class for execute_CV behavior which inherits from class
 * execute behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_Execute_CV : public C_ExecuteBehavior{
    private:
        // Variables
        int iAdcPgaGain_;
        int iDacSeqBlock0Address_;
        int iDacSeqBlock1Address_;
        int iStepsPerBlock_;
        int iStepsToLowerVoltage_;
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
        C_Execute_CV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* execute_CV_H */
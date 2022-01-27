/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_DPV which 
 * defines the behavior of setting up an differential pulse voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_DPV_H
#define setup_DPV_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_DPV behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_DPV : public C_SetupBehavior{
    private:
        // Methods
        int funInitDPV();
        int funSequencerExecuteSequence();
        int funSequencerInitializationSequence();
        
    public:
        // Constructor
        C_Setup_DPV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_DPV_H */
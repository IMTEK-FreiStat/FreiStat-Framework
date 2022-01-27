/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_DPV which 
 * defines the behavior of setting up an normal pulse voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_NPV_H
#define setup_NPV_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_NPV behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_NPV : public C_SetupBehavior{
    private:
        // Methods
        int funInitNPV();
        int funSequencerExecuteSequence();
        int funSequencerInitializationSequence();
        
    public:
        // Constructor
        C_Setup_NPV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_NPV_H */
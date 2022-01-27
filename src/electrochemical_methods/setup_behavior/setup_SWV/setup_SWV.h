/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_SWV which 
 * defines the behavior of setting up an square wave voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_SWV_H
#define setup_SWV_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_SWV behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_SWV : public C_SetupBehavior{
    private:
        // Methods
        int funInitSWV();
        int funSequencerExecuteSequence();
        int funSequencerInitializationSequence();
        
    public:
        // Constructor
        C_Setup_SWV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_SWV_H */
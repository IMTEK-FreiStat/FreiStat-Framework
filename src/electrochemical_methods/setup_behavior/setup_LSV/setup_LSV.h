/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_LSV which 
 * defines the behavior of setting up an linear sweep voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_LSV_H
#define setup_LSV_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_LSV behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_LSV : public C_SetupBehavior{
    private:
        // Methods
        int funInitLSV();

        int funSequencerADCControl();
        int funSequencerDACControl();
        int funSequencerInitializationSequence();

    public:
        // Constructor
        C_Setup_LSV();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_LSV_H */
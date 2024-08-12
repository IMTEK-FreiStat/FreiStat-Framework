/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_CV which 
 * defines the behavior of setting up an cyclic voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_CV_H
#define setup_CV_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_CV behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_CV : public C_SetupBehavior{
    private:
        // Methods
        int funInitCV();

        int funSequencerADCControl();
        int funSequencerDACControl();
        int funSequencerInitializationSequence();

    public:
        // Constructor
        C_Setup_CV();

        // Starting method
        int Begin(C_DataSoftwareStorage *);
};

#endif /* setup_CV_H */
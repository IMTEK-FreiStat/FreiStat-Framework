/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_OCP which 
 * defines the behavior of setting up the measurement of the open circuit
 * potential.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_OCP_H
#define setup_OCP_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_OCP behavior which inherits from class
 * setup behavior
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_Setup_OCP : public C_SetupBehavior{
    private:
        // Methods
        int funInitOCP();

        int funSequencerADCControl();
        int funSequencerInitializationSequence();

    public:
        // Constructor
        C_Setup_OCP();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};

#endif /* setup_OCP_H */
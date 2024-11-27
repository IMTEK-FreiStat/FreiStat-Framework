/******************************************************************************
 * @brief: Header file containing the subclass (C_Setup) C_Setup_EIS which 
 * defines the behavior of setting up an electrocemical impedance spectroscopy.
 * 
 * @author: Cedric Neumann
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_EIS_H
#define setup_EIS_H

// Include dependencies
#include "../setup_behavior.h"

/******************************************************************************
 * @brief: Abstract class for setup_EIS behavior which inherits from class
 * setup behavior
 * 
 * @author: Cedric Neumann
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Setup_EIS : public C_SetupBehavior{
    private:
        // Methods
        int funInitEIS();
        int funSequencerExecuteSequence();
        int funSequencerInitializationSequence();
        
    public:
        // Constructor
        C_Setup_EIS();

        // Starting method
        int Begin(C_DataSoftwareStorage *);
};

#endif /* setup_CV_H */
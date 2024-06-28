// Include guard
#ifndef setup_A_H
#define setup_A_H

// Include dependencies
#include "../setup_behavior.h"

class C_Setup_A : public C_SetupBehavior{
    private:
        // Methods
        int funInitA();
        int funSequencerExecuteSequence();
        int funSequencerInitializationSequence();
        
    public:
        // Constructor
        C_Setup_A();

        // Starting method
        void Begin(C_DataSoftwareStorage *);
};


#endif /* setup_C_H */

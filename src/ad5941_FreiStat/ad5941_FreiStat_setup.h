/******************************************************************************
 * @brief: Header file for the the setup of the AD5941/5940 chip
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *  
 *****************************************************************************/

// Include guard
#ifndef ad5940_setup_H
#define ad5940_setup_H

// Include dependencies
// Include C-files
extern "C" { 
#include <ad5940.h> 
}

// Include other dependencies
#include <SPI.h>
#include "../data_storage/constants.cpp"

/******************************************************************************
 * @brief: Abstract class for C_AD5940_Setup
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *  
 *****************************************************************************/
class C_AD5940_Setup{
    private:
        // Variables
        float fLFOSCFrequency_;

        AGPIOCfg_Type S_GPIOConfig_;
        CLKCfg_Type S_ClockConfig_;
        FIFOCfg_Type S_FiFoConfig_;
        LFOSCMeasure_Type S_LFOSCMeasure_;  
        SEQCfg_Type S_SequencerConfig_;

        // Static variables
        static bool bInterruptOccured_;

        // Methods
        void funAD5940_InitMCU();
        void funAD5940_Config();

        // Interrupt handler
        static void funExt_Int0_Handler();

    public:
        // Constructor 
        C_AD5940_Setup();

        // Starting method
        void Begin();

        // Setter
        void set_InterruptOccured(bool);

        // Getter
        bool get_InterruptOccured();

        float get_LFOSCFrequency();
};

#endif /* ad5940_setup_H */
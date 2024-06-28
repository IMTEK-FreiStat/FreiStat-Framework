/******************************************************************************
 * @brief: Header file for the data storage local, in which all experiment 
 * parameters are stored, which vary from experiment to experiment.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef data_storage_local_H
#define data_storage_local_H

// Include dependencies
#include "data_software_storage.h"

/******************************************************************************
 * @brief: Abstract class for data storag local
 * @details: Class that stores all varying experiment parameters. Measurements 
 * are not saved and directly send to the python program via serial port.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_DataStorageLocal{
    private:
        /**********************************************************************
         * Object references
         * 
         **********************************************************************/ 
        C_DataStorageLocal * c_nextDataStorage_;

        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        int     iExperimentState_;

        char    chrExperimentType_[4];
        
        /**********************************************************************
         * Experiment parameters
         * 
         * Type |       Parametername           |       Electrochemical method
         **********************************************************************/
        bool    bFixedWEPotential_;                     // LSV | CV |    | DPV | NPV | SWV
        bool    bSeqBlockUsed_;                         // LSV | CV |    | DPV | NPV | SWV

        int     iBufferEntries_;                        //     |    | CA |     |     |
        int     iCurrentStepNumber_;                    // LSV | CV | CA | DPV | NPV | SWV
        int     iCycle_;                                // LSV | CV | CA | DPV | NPV | SWV
        int     iDacCurrentBlock_;                      // LSV | CV |    | DPV | NPV | SWV
        int     iDacSeqBlock0Address_;                  // LSV | CV |    | DPV | NPV | SWV
        int     iDacSeqBlock1Address_;                  // LSV | CV |    | DPV | NPV | SWV
        int     iStepNumber_;                           // LSV | CV | CA |     |     |
        int     iStepsPerBlock_;                        //     | CV |    |     |     |

        float   fLowerVoltage_;                         // LSV | CV |    | DPV | NPV | SWV
        float   fScanrate_;                             // LSV | CV | CA | DPV | NPV | SWV
        float   fStartVoltage_;                         // LSV | CV |    | DPV | NPV | SWV
        float   fStepsize_;                             // LSV | CV |    |     |     |
        float   fStepsRemaining_;                       // LSV | CV | CA | DPV | NPV | SWV
        float   fUpperVoltage_;                         // LSV | CV |    | DPV | NPV | SWV
        float   fWePotentialHigh_;                      // LSV | CV | CA | DPV | NPV | SWV
        float   fWePotentialLow_;                       // LSV | CV | CA | DPV | NPV | SWV

        float   arrfPotentialSteps_[EXPERIMENT_BUFFER]; //     |    | CA | DPV | NPV | SWV
        float   arrfPulseDurations_[EXPERIMENT_BUFFER]; //     |    | CA | DPV | NPV | SWV

        /**********************************************************************
         * ADC - Analog digital converter
         * 
         **********************************************************************/
        bool    bAdcNotchFilter_;          

        int     iAdcPgaGain_;          
        int     iAdcOsrSinc2_;              
        int     iAdcOsrSinc3_;        

        /**********************************************************************
         * DAC - Digital analog converter
         * 
         **********************************************************************/
        bool    bDacIncrement_;

        int     iDacCurrentCode_;

        float   iDacIncrementPerStep_;

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/
        int     iLPTIARtiaSize_;
        
    public:
        // Constructor
        C_DataStorageLocal();

        // Starting method
        void Begin();

        /**********************************************************************
         * Setter methods
         **********************************************************************/
        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        void set_ExperimentState(int);    

        void set_ExperimentType(char *);      

        void set_NextDataStorage(C_DataStorageLocal *);
        
        /**********************************************************************
         * Experiment parameters
         * 
         **********************************************************************/ 
        void set_FixedWEPotential(bool);
        void set_SeqBlockUsed(bool);

        void set_BufferEntries(int);
        void set_CurrentStepNumber(int);
        void set_Cycle(int);
        void set_DacCurrentBlock(int);
        void set_DacSeqBlock0Address(int);
        void set_DacSeqBlock1Address(int);
        void set_StepNumber(int);
        void set_StepsPerBlock(int);

        void set_LowerVoltage(float);
        void set_Scanrate(float);
        void set_StartVoltage(float);
        void set_Stepsize(float);
        void set_StepsRemaining(float);
        void set_UpperVoltage(float);
        void set_WePotentialHigh(float);
        void set_WePotentialLow(float);

        void set_PotentialSteps(float, int);
        void set_PulseDurations(float, int);  

        /**********************************************************************
         * ADC - Analog digital converter
         * 
         **********************************************************************/
        void set_AdcNotchFilter(bool);

        void set_AdcPgaGain(int);
        void set_AdcOsrSinc2(int);
        void set_AdcOsrSinc3(int);

        /**********************************************************************
         * DAC - Digital analog converter
         * 
         **********************************************************************/
        void set_DacCurrentCode(int);
        void set_DacIncrementPerStep(float);
        void set_DacIncrement(bool);

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/        
        void set_LPTIARtiaSize(int);

        /**********************************************************************
         * Getter methods
         **********************************************************************/
        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        int get_ExperimentState();

        char * get_ExperimentType();
        
        C_DataStorageLocal * get_NextDataStorage();

        /**********************************************************************
         * Experiment parameters
         * 
         **********************************************************************/ 
        bool get_FixedWEPotential();
        bool get_SeqBlockUsed();

        int get_BufferEntries();
        int get_CurrentStepNumber();
        int get_Cycle();
        int get_DacCurrentBlock();
        int get_DacSeqBlock0Address();
        int get_DacSeqBlock1Address();
        int get_StepNumber();
        int get_StepsPerBlock();

        float get_LowerVoltage();
        float get_PotentialSteps(int);
        float get_PulseDurations(int);
        float get_Scanrate();
        float get_StartVoltage();
        float get_Stepsize();
        float get_StepsRemaining();
        float get_UpperVoltage();
        float get_WePotentialHigh();
        float get_WePotentialLow();

        /**********************************************************************
         * ADC - Analog digital converter
         * 
         **********************************************************************/
        bool get_AdcNotchFilter();

        int get_AdcPgaGain();
        int get_AdcOsrSinc2();
        int get_AdcOsrSinc3();

        /**********************************************************************
         * DAC - Digital analog converter
         * 
         **********************************************************************/
        bool get_DacIncrement();

        int get_DacCurrentCode();

        float get_DacIncrementPerStep();

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/        
        int get_LPTIARtiaSize();
};
#endif /* data_storage_local_H */
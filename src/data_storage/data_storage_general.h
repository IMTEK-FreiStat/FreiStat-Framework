/******************************************************************************
 * @brief: Header file for the general data storage, in which all experiment 
 * parameters which are identical for all experiments are stored.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef data_storage_general_H
#define data_storage_general_H

// Include dependencies
#include "data_software_storage.h"

/******************************************************************************
 * @brief: Abstract class for data storage general
 * @details: Class that stores general experiment parameters. Measurements are 
 * not saved and directly send to the python program via serial port.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_DataStorageGeneral{
    private:
        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        int     iCommandNumber_;
        int     iSendDataCounter_;

        /**********************************************************************
         * Experiment sequence variables
         * 
         **********************************************************************/
        int     iSequenceCycles_;
        int     iSequenceLength_;

        /**********************************************************************
         * Configuration parameters
         * 
         **********************************************************************/
        unsigned int iFiFoThreshold_;
        unsigned int iSeqStartAddress_;
        unsigned int iSeqMaxLength_;

        float   fADCReferenceVoltage_;
        float   fCalibrationResistorValue_;
        float   fLFOSCFrequency_;
        
        /**********************************************************************
         * Experiment container
         * 
         **********************************************************************/ 
        uint32_t    arruiSampleBuffer_[SAMPLE_BUFFER];   
        
        S_DataContainer S_ExperimentData_[TRANSMIT_BUFFER];

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/
        int     iLPAmpPowerMode_;
        int     iLPTIALoadSize_; 

        float   fLPTIARtiaSizeExternal_;

        fImpPol_Type fRtiaValue_;

        /**********************************************************************
         * Sequence
         * 
         **********************************************************************/
        SEQInfo_Type S_SequenceInfo_[AD5940_NUM_SEQ_COMMANDS];

    public:
        // Constructor
        C_DataStorageGeneral();

        // Starting method
        void Begin(C_DataSoftwareStorage * );

        /**********************************************************************
         * Setter methods
         **********************************************************************/
        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        void set_CommandNumber(int);   
        void set_SendDataCounter(int);

        /**********************************************************************
         * Experiment sequence variables
         * 
         **********************************************************************/
        void set_SequenceCycles(int);
        void set_SequenceLength(int);

        /**********************************************************************
         * Experiment container
         * 
         **********************************************************************/        
        void set_ExperimentData(S_DataContainer, int);

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/
        void set_LPAmpPowerMode(int);
        void set_LPTIALoadSize(int);

        void set_LPTIARtiaSizeExternal(float);

        void set_RtiaValue(fImpPol_Type);

        /**********************************************************************
         * Sequence
         * 
         **********************************************************************/
        void set_SequenceInfo(SEQInfo_Type, int);

        /**********************************************************************
         * Getter methods
         **********************************************************************/
        /**********************************************************************
         * General variables
         * 
         **********************************************************************/ 
        int get_CommandNumber();
        int get_SendDataCounter();

        /**********************************************************************
         * Experiment sequence variables
         * 
         **********************************************************************/        
        int get_SequenceCycles();
        int get_SequenceLength();

        /**********************************************************************
         * Configuration parameters
         * 
         **********************************************************************/
        unsigned int get_FiFoThreshold();
        unsigned int get_SeqMaxLength();
        unsigned int get_SeqStartAddress();

        float get_ADCReferenceVoltage();
        float get_CalibrationResistorValue();
        float get_LFOSCFrequency();
        
        /**********************************************************************
         * Experiment container
         * 
         **********************************************************************/ 
        uint32_t * get_SampleBuffer();
        
        S_DataContainer get_ExperimentData(int);

        /**********************************************************************
         * Low power TIA (Transimpedence amplifier)
         * 
         **********************************************************************/
        int get_LPAmpPowerMode();
        int get_LPTIALoadSize();

        float get_LPTIARtiaSizeExternal();

        fImpPol_Type get_RtiaValue();

        /**********************************************************************
         * Sequence
         * 
         **********************************************************************/
        SEQInfo_Type get_SequenceInfo(int);
        
};
#endif /* data_storage_general_H */
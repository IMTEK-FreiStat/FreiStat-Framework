/******************************************************************************
 * @brief: Source file for the general data storage, in which all experiment 
 * parameters which are identical for all experiments are stored.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef data_storage_general_CPP
#define data_storage_general_CPP

// Include header
#include "data_storage_general.h"

/******************************************************************************
 * @brief Default constructor of the class C_DataStorageGeneral
 *****************************************************************************/ 
C_DataStorageGeneral::C_DataStorageGeneral(){}

/******************************************************************************
 * @brief Starting method for the class C_DataStorageGeneral
 * @details Initialzing all experiment parameters and configuration parameters
 * with default values
 *****************************************************************************/
void C_DataStorageGeneral::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Initalizes system with default parameters
    // No command received yet
    iCommandNumber_ = FREISTAT_UNDEF_I;

    // Initializes configurations parameters
    iSequenceLength_ = 1;
    iSequenceCycles_ = 1;
    iSendDataCounter_ = 0;
    iFiFoThreshold_ = AD5940_FIFO_THRESHOLD;
    iLPTIALoadSize_ = LPTIARLOAD_SHORT;
    iLPAmpPowerMode_ = LPAMPPWR_NORM;
    iSeqStartAddress_ = AD5940_SEQ_START_ADDR;
    iSeqMaxLength_ = (AD5940_SEQ_MAX_ADDR - AD5940_SEQ_START_ADDR);

    fADCReferenceVoltage_ = AD5940_ADC_REF_VOLTAGE;
    fCalibrationResistorValue_ = AD5940_CAL_RESISTOR;
    fLFOSCFrequency_ = c_DataSoftwareStorage->get_AD5940Setup()->
                       get_LFOSCFrequency();
 }

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for saving the current command number
 * @param iCommandNumber: Command number of the JSON telegram 
 *****************************************************************************/
void C_DataStorageGeneral::set_CommandNumber(int iCommandNumber){
    iCommandNumber_ = iCommandNumber;
}

/******************************************************************************
 * @brief Setter method for saving the length of the experiment sequence
 * @param iSequenceLength: Length of the experiment sequence
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceLength(int iSequenceLength){
    iSequenceLength_ = iSequenceLength;
}

/******************************************************************************
 * @brief Setter method for saving the amount of cycles of the experiment 
 * sequence
 * @param iSequenceCycles: Amount of cycles the experiment sequence should be
 * be repeated
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceCycles(int iSequenceCycles){
    iSequenceCycles_ = iSequenceCycles;
}

/******************************************************************************
 * @brief Setter method for saving the size of the external LPTIA Rtia
 * resistor
 * @param fLPTIARtiaSizeExternal: Size of the external LPTIA Rtia in ohm
 *****************************************************************************/
void C_DataStorageGeneral::set_LPTIARtiaSizeExternal(float fLPTIARtiaSizeExternal){
    fLPTIARtiaSizeExternal_ = fLPTIARtiaSizeExternal;
}

/******************************************************************************
 * @brief Setter method for saving the power mode of the low power amplifier
 * resistor
 * @param iLPAmpPowerMode: Power mode of the low power amplifier
 *****************************************************************************/
void C_DataStorageGeneral::set_LPAmpPowerMode(int iLPAmpPowerMode){
    iLPAmpPowerMode_ = iLPAmpPowerMode;
}

/******************************************************************************
 * @brief Setter method for saving the values of the calibrated Rtia resistor
 * @param fRtiaValue: Data of the calibrated Rtia resistor
 *****************************************************************************/
void C_DataStorageGeneral::set_RtiaValue(fImpPol_Type fRtiaValue){
    fRtiaValue_ = fRtiaValue;
}

/******************************************************************************
 * @brief Setter method for saving the siza of the LPTIA load
 * @param iLPTIALoadSize: Value of the LPTIA load encoded as integer
 *****************************************************************************/
void C_DataStorageGeneral::set_LPTIALoadSize(int iLPTIALoadSize){
    iLPTIALoadSize_ = iLPTIALoadSize;
}

/******************************************************************************
 * @brief Setter method for saving the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceInfo(SEQInfo_Type S_SequenceInfo, 
                                            int iSequenceID){
    S_SequenceInfo_[iSequenceID] = S_SequenceInfo;
}

/******************************************************************************
 * @brief Setter method for saving the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
void C_DataStorageGeneral::set_SeqWaitAddr(uint32_t uiSeqWaitAddr, 
                                            int iSeqWaitAddrID){
    arruiSeqWaitAddr[iSeqWaitAddrID] = uiSeqWaitAddr;
}


/******************************************************************************
 * @brief Setter method for saving experiment data from specific position
 * in data storage
 * @param S_ExperimentData: Experiment data which should be stored
 * @param iPosition: Position from which data should be read
 *****************************************************************************/
void C_DataStorageGeneral::set_ExperimentData(S_DataContainer S_ExperimentData, 
                                              int iPosition){
    S_ExperimentData_[iPosition] = S_ExperimentData;
}

/******************************************************************************
 * @brief Setter method for saving experiment data from specific position
 * in data storage
 * @param S_ExperimentData: Experiment data which should be stored
 * @param iPosition: Position from which data should be read
 *****************************************************************************/
void C_DataStorageGeneral::set_ExperimentDataEIS(S_DataContainerEIS S_ExperimentDataEIS, 
                                              int iPosition){
    S_ExperimentDataEIS_[iPosition] = S_ExperimentDataEIS;
}

/******************************************************************************
 * @brief Setter method for latest position of send data
 * @param iSendDataCounter: Latest position of send data
 *****************************************************************************/
void C_DataStorageGeneral::set_SendDataCounter(int iSendDataCounter){
    iSendDataCounter_ = iSendDataCounter;
}

/******************************************************************************
 * Getter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Getter method for returning current command number
 * @return: Returns currently saved command number
 *****************************************************************************/
int C_DataStorageGeneral::get_CommandNumber(){
    return iCommandNumber_;
}

/******************************************************************************
 * @brief Getter method for returning the length of the experiment sequence
 * @return: Length of the experiment sequence
 *****************************************************************************/
int C_DataStorageGeneral::get_SequenceLength(){
    return iSequenceLength_;
}

/******************************************************************************
 * @brief Getter method for returning the amount of cycles of the experiment 
 * sequence
 * @return: Amount of cycles the experiment sequence should be be repeated
 *****************************************************************************/
int C_DataStorageGeneral::get_SequenceCycles(){
    return iSequenceCycles_;
}

/******************************************************************************
 * @brief Getter method for returning the defined FIFO threshold
 * @return: Returns defined FIFO threshold
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_FiFoThreshold(){
    return iFiFoThreshold_;
}

/******************************************************************************
 * @brief Getter method for returning the defined starting adress of the 
 * sequencer
 * @return: Returns defined squencer starting address
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_SeqStartAddress(){
    return iSeqStartAddress_;
}

/******************************************************************************
 * @brief Getter method for the defined maximum length of sequencer commands
 * @return: Returns defined maximum length of sequencer commands
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_SeqMaxLength(){
    return iSeqMaxLength_;
}

/******************************************************************************
 * @brief Getter method for the defined ADC/DAC reference voltage
 * @return: Returns defined ADC/DAC reference voltage
 *****************************************************************************/
float C_DataStorageGeneral::get_ADCReferenceVoltage(){
    return fADCReferenceVoltage_;
}

/******************************************************************************
 * @brief Getter method for the defined value of the reference resistor
 * @return: Returns defined value of the reference resistor
 *****************************************************************************/
float C_DataStorageGeneral::get_CalibrationResistorValue(){
    return fCalibrationResistorValue_;
}

/******************************************************************************
 * @brief Getter method for returning measured LFOSC frequency
 * @return: Returns measured LFOSC frequency
 *****************************************************************************/
float C_DataStorageGeneral::get_LFOSCFrequency(){
    return fLFOSCFrequency_;
}

/******************************************************************************
 * @brief Getter method for returning size of the external LPTIA Rtia
 * resistor
 * @return: Returns the size of the external LPTIA Rtia in ohm
 *****************************************************************************/
float C_DataStorageGeneral::get_LPTIARtiaSizeExternal(){
    return fLPTIARtiaSizeExternal_;
}

/******************************************************************************
 * @brief Getter method for returning the power mode of the low power amplifier
 * @return: Returns power mode of the low power amplifier
 *****************************************************************************/
int C_DataStorageGeneral::get_LPAmpPowerMode(){
    return iLPAmpPowerMode_;
}

/******************************************************************************
 * @brief Getter method for returning the values of the calibrated Rtia resistor
 * @returns: Data of the calibrated Rtia resistor
 *****************************************************************************/
fImpPol_Type C_DataStorageGeneral::get_RtiaValue(){
    return fRtiaValue_;
}

/******************************************************************************
 * @brief Getter method for returning the siza of the LPTIA load
 * @details: The following sizes for the load resistor are defined:
 * 
 * LPTIARLOAD_SHORT            0    0 ohm load 
 * LPTIARLOAD_10R              1    10 ohm load
 * LPTIARLOAD_30R              2    30 ohm load
 * LPTIARLOAD_50R              3    50 ohm load
 * LPTIARLOAD_100R             4    100 ohm load
 * LPTIARLOAD_1K6              5    Only available when Rtia setting >= 2 kOhm 
 * LPTIARLOAD_3K1              6    Only available when Rtia setting >= 4 kOhm
 * LPTIARLOAD_3K6              7    Only available when Rtia setting >= 4 kOhm
 * 
 * @return: Returns value of the LPTIA load encoded in an integer value
 *****************************************************************************/
int C_DataStorageGeneral::get_LPTIALoadSize(){
    return iLPTIALoadSize_;
}

/******************************************************************************
 * @brief Getter method for returning the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
SEQInfo_Type C_DataStorageGeneral::get_SequenceInfo(int iSequenceID){
    return S_SequenceInfo_[iSequenceID];
}

/******************************************************************************
 * @brief Setter method for saving the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
uint32_t C_DataStorageGeneral::get_SeqWaitAddr(int iSeqWaitAddrID){
    return arruiSeqWaitAddr[iSeqWaitAddrID];
}

/******************************************************************************
 * @brief Getter method for returning experiment data from specific position
 * in data storage
 * @param iPosition: Position from which data should be read
 * @returns: returns experiment data from specific position
 *****************************************************************************/
S_DataContainer C_DataStorageGeneral::get_ExperimentData(int iPosition){
    return S_ExperimentData_[iPosition];
}

/******************************************************************************
 * @brief Getter method for returning experiment data from specific position
 * in data storage
 * @param iPosition: Position from which data should be read
 * @returns: returns experiment data from specific position
 *****************************************************************************/
S_DataContainerEIS C_DataStorageGeneral::get_ExperimentDataEIS(int iPosition){
    return S_ExperimentDataEIS_[iPosition];
}

/******************************************************************************
 * @brief Getter method for returning last position of send data
 * @returns: Last position of send data
 *****************************************************************************/
int C_DataStorageGeneral::get_SendDataCounter(){
    return iSendDataCounter_;
}

/******************************************************************************
 * @brief Getter method for returning pointer to sample buffer
 * @returns: Returns pointer to sample buffer
 *****************************************************************************/
uint32_t * C_DataStorageGeneral::get_SampleBuffer(){
    return arruiSampleBuffer_;
}


void C_DataStorageGeneral::checkFrequency(float freq,  SEQInfo_Type S_SequenceInfo, 
    uint32_t arruiSeqWaitAddr[2])
{
  ADCFilterCfg_Type filter_cfg;
  DFTCfg_Type dft_cfg;
  HSDACCfg_Type hsdac_cfg;
  uint32_t WaitClks;
  ClksCalInfo_Type clks_cal;
  FreqParams_Type freq_params;
  uint32_t SeqCmdBuff[32];
  uint32_t SRAMAddr = 0;
  float fAdcClkFreq = 16000000;


  /* Step 1: Check Frequency */
  freq_params = AD5940_GetFreqParameters(freq);
  
    if(freq < 0.51){
                /* Update HSDAC update rate */
        hsdac_cfg.ExcitBufGain =EXCITBUFGAIN_2;
        hsdac_cfg.HsDacGain = HSDACGAIN_1;
        hsdac_cfg.HsDacUpdateRate = 0x1B;
        AD5940_HSDacCfgS(&hsdac_cfg);
        AD5940_HSRTIACfgS(HSTIARTIA_40K); //set as per load current range
        
        /*Update ADC rate */
        filter_cfg.ADCRate = ADCRATE_800KHZ;
        fAdcClkFreq = 16e6;
        
        /* Change clock to 16MHz oscillator */
        AD5940_HPModeEn(bFALSE);
        }
    else if(freq < 5 ){
        /* Update HSDAC update rate */
        hsdac_cfg.ExcitBufGain =EXCITBUFGAIN_2;// AppIMPCfg.ExcitBufGain;
        hsdac_cfg.HsDacGain = HSDACGAIN_1;//AppIMPCfg.HsDacGain;
        hsdac_cfg.HsDacUpdateRate = 0x1B;
        AD5940_HSDacCfgS(&hsdac_cfg);
        AD5940_HSRTIACfgS(HSTIARTIA_40K); //set as per load current range
        
        /*Update ADC rate */
        filter_cfg.ADCRate = ADCRATE_800KHZ;
        fAdcClkFreq = 16e6;
        
        /* Change clock to 16MHz oscillator */
        AD5940_HPModeEn(bFALSE);
        
    }
    else if(freq < 450){
        /* Update HSDAC update rate */
        hsdac_cfg.ExcitBufGain =EXCITBUFGAIN_2;
        hsdac_cfg.HsDacGain = HSDACGAIN_1;  
        
        hsdac_cfg.HsDacUpdateRate = 0x1B;
        AD5940_HSDacCfgS(&hsdac_cfg);
        AD5940_HSRTIACfgS(HSTIARTIA_5K); //set as per load current range
        
        /*Update ADC rate */
        filter_cfg.ADCRate = ADCRATE_800KHZ;
        fAdcClkFreq = 16e6;
        
        /* Change clock to 16MHz oscillator */
        AD5940_HPModeEn(bFALSE);
    }
    else if(freq<80000){
           /* Update HSDAC update rate */
        hsdac_cfg.ExcitBufGain =EXCITBUFGAIN_2;
        hsdac_cfg.HsDacGain = HSDACGAIN_1;
        hsdac_cfg.HsDacUpdateRate = 0x1B;
        AD5940_HSDacCfgS(&hsdac_cfg);
        AD5940_HSRTIACfgS(HSTIARTIA_5K); //set as per load current range
        
        /*Update ADC rate */
        filter_cfg.ADCRate = ADCRATE_800KHZ;
        fAdcClkFreq = 16e6;
        
        /* Change clock to 16MHz oscillator */
        AD5940_HPModeEn(bFALSE);
    }
        /* High power mode */
	else if(freq >= 80000){
		  /* Update HSDAC update rate */
        hsdac_cfg.ExcitBufGain = EXCITBUFGAIN_2;
        hsdac_cfg.HsDacGain = HSDACGAIN_1;
        hsdac_cfg.HsDacUpdateRate = 0x07;
        AD5940_HSDacCfgS(&hsdac_cfg);
        AD5940_HSRTIACfgS(HSTIARTIA_5K); //set as per load current range
        
        /*Update ADC rate */
        filter_cfg.ADCRate = ADCRATE_1P6MHZ;
        fAdcClkFreq = 32e6;
        
        /* Change clock to 32MHz oscillator */
        AD5940_HPModeEn(bTRUE);
	}
  
  /* Step 2: Adjust ADCFILTERCON and DFTCON to set optimumn SINC3, SINC2 and DFTNUM settings  */
  filter_cfg.ADCAvgNum = ADCAVGNUM_16;  /* Don't care because it's disabled */ 
  filter_cfg.ADCSinc2Osr = freq_params.ADCSinc2Osr;
  filter_cfg.ADCSinc3Osr = freq_params.ADCSinc3Osr;
  filter_cfg.BpSinc3 = bFALSE;
  filter_cfg.BpNotch = bTRUE;
  filter_cfg.Sinc2NotchEnable = bTRUE;
  dft_cfg.DftNum = freq_params.DftNum;
  dft_cfg.DftSrc = freq_params.DftSrc;
  dft_cfg.HanWinEn = bTRUE;
  AD5940_ADCFilterCfgS(&filter_cfg);
  AD5940_DFTCfgS(&dft_cfg);
  
  /* Step 3: Calculate clocks needed to get result to FIFO and update sequencer wait command */
  clks_cal.DataType = DATATYPE_DFT;
  clks_cal.DftSrc = freq_params.DftSrc;
  clks_cal.DataCount = 1L<<(freq_params.DftNum+2); /* 2^(DFTNUMBER+2) */
  clks_cal.ADCSinc2Osr = freq_params.ADCSinc2Osr;
  clks_cal.ADCSinc3Osr = freq_params.ADCSinc3Osr;
  clks_cal.ADCAvgNum = 0;
  clks_cal.RatioSys2AdcClk = AD5940_SYS_CLOCK_FREQ/ fAdcClkFreq;
  AD5940_ClksCalculate(&clks_cal, &WaitClks);		


  SRAMAddr = S_SequenceInfo.SeqRamAddr +  arruiSeqWaitAddr[0];

  
  SeqCmdBuff[0] =SEQ_WAIT(WaitClks/2);
  SeqCmdBuff[1] =SEQ_WAIT(WaitClks/2);
  
  AD5940_SEQCmdWrite(SRAMAddr, SeqCmdBuff, 2);   

  SRAMAddr = S_SequenceInfo.SeqRamAddr +  arruiSeqWaitAddr[1];
  
  SeqCmdBuff[0] =SEQ_WAIT(WaitClks/2);    
  SeqCmdBuff[1] =SEQ_WAIT(WaitClks/2);
  
  AD5940_SEQCmdWrite(SRAMAddr, SeqCmdBuff, 2);
}


#endif /* data_storage_general_CPP */
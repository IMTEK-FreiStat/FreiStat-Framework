/******************************************************************************
 * @brief: Source file containing the subclass (C_Setup) C_Setup_OCP which 
 * defines the behavior of measuring the open circuit potential
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_OCP_CPP
#define setup_OCP_CPP

// Include dependencies
#include "setup_OCP.h"

/******************************************************************************
 * @brief Constructor of the class C_Setup_OCP
 * 
 *****************************************************************************/ 
C_Setup_OCP::C_Setup_OCP(){}

/******************************************************************************
 * @brief Starting method for the class C_Setup_OCP
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
int C_Setup_OCP::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;
    
    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Set Rtia and Rtia Load to make inverting amplifier
    c_DataStorageLocal_->set_LPTIARtiaSize(LPTIARTIA_8K);

    c_DataStorageGeneral_->set_LPTIALoadSize(LPTIARLOAD_3K6);

    // Check if experiment parameters are valid
    return this->funInitOCP();
}

/******************************************************************************
 * @brief Check if stored parameters are valid for measuring open circuit 
 *        potential.
 * @details: Defined error codes:
 * Error code   :       Definition
 * 0            :       No error occured
 * 21001        :       Wake up of AFE failed
 * 21002        :       Sample buffer to small
 * 21003        :       Sequence doenst fit into SRAM of sequencer
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_OCP::funInitOCP(){
    // Initialize variables
    int iErrorCode = 0;

    FIFOCfg_Type S_FiFoConfig;
    SEQCfg_Type S_SeuqencerConfig;    
    SEQInfo_Type S_SequenceInfo;

    // Wakeup AFE by reading register, read is tried 10 times at most
    if (AD5940_WakeUp(10) > 10)
        // Error wakeup failed
        return EC_SETUP + EC_SE_WAKEUP_AFE_ERR;

    // Initializing the sequencer
    // Disable -> not used
    S_SeuqencerConfig.SeqBreakEn = bFALSE;

    // Clear sequencer count and crc checksum
    S_SeuqencerConfig.SeqCntCRCClr = bTRUE;

    // Disable sequencer
    S_SeuqencerConfig.SeqEnable = bFALSE;

    // Disable -> not used
    S_SeuqencerConfig.SeqIgnoreEn = bFALSE;

    // Set sequencer size to 4 kB
    S_SeuqencerConfig.SeqMemSize = SEQMEMSIZE_4KB;

    // Disable waiting timer after every command
    S_SeuqencerConfig.SeqWrTimer = 0;

    // Initialize sequencer
    AD5940_SEQCfg(&S_SeuqencerConfig);

    // Check if sample buffer has a valid size
    if (SAMPLE_BUFFER <= 0){
        return EC_SETUP + EC_SE_SAMPLE_BUFF_SIZE;
    }
        
    // Check if internal or external LPTIA Rtia is selected
    // External Rtia
    if (c_DataStorageLocal_->get_LPTIARtiaSize() == LPTIARTIA_OPEN){
        // Variable intialization
        fImpPol_Type RtiaValue;

        // Set magnitude to size of external resistor (only ohmic) -> Phase = 0
        RtiaValue.Magnitude = c_DataStorageGeneral_->get_LPTIARtiaSizeExternal();
        RtiaValue.Phase = 0;

        // Save values
        c_DataStorageGeneral_->set_RtiaValue(RtiaValue);
    }
    // Internal Rtia
    else {
        // Calibrating internal LPTIA Rtia resistor
        this->funCalibrateLPTIAResistor();
    }
    
    // Reconfigure FIFO, since the Rtia calibration can lead to data remnants
    // Disable FIFO
    AD5940_FIFOCtrlS(FIFOSRC_SINC3, bFALSE);

    // Enable FIFO
    S_FiFoConfig.FIFOEn = bTRUE;

    // Select Sinc2 + Notch Filter as FIFO data source
    S_FiFoConfig.FIFOSrc = FIFOSRC_SINC2NOTCH;

    // Get FIFO threshold
    S_FiFoConfig.FIFOThresh = c_DataStorageGeneral_->get_FiFoThreshold();

    // Set FIFO to FIFO mode instead of stream mode
    S_FiFoConfig.FIFOMode = FIFOMODE_FIFO;

    // Set FIFO size to 2 kB (4 kB for sequencer)
    S_FiFoConfig.FIFOSize = FIFOSIZE_2KB;

    // Configure FIFO
    AD5940_FIFOCfg(&S_FiFoConfig);

    // Reset all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    /*************************************************************************/
    // Generate OCP sequences (Init, ADC)
    // Initialize sequence generator with initalized values
    AD5940_SEQGenInit(c_DataStorageGeneral_->get_SampleBuffer(), SAMPLE_BUFFER);

    // Initialization sequence
    iErrorCode = this->funSequencerInitializationSequence();
    if (iErrorCode != EC_NO_ERROR){
        return iErrorCode;
    } 
    
    // ADC control sequence
    iErrorCode = this->funSequencerADCControl();
    if (iErrorCode != EC_NO_ERROR){
        return iErrorCode;
    }

    // Get intialization sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_1);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Enable sequencer
    AD5940_SEQCtrlS(bTRUE);                          

    // Trigger sequencer by writing in the register
    AD5940_SEQMmrTrig(S_SequenceInfo.SeqId);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_1);

    // Reset all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
    
    // Get ADC sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_2);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_2);

    // Disable sequencer
    AD5940_SEQCtrlS(bFALSE);
    AD5940_WriteReg(REG_AFE_SEQCNT, 0);
    
    // Enable sequencer
    AD5940_SEQCtrlS(bTRUE); 

    // Clear interrupt flag
    c_DataSoftwareStorage_->get_AD5940Setup()->set_InterruptOccured(false);

    // Set AFE to low power mode
    AD5940_AFEPwrBW(AFEPWR_LP, AFEBW_250KHZ);

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for generating the initialization sequence and writing the
 * commands to the SRAM
 * @details: Defined error codes:
 * Error code   :       Definition
 * 0            :       No error occured
 * 21001        :       Wake up of AFE failed
 * 21002        :       Sample buffer to small
 * 21003        :       Sequence doenst fit into SRAM of sequencer
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_OCP::funSequencerInitializationSequence(){
    // Initalize variables
    int iErrorCode = EC_NO_ERROR;

    const uint32_t *uiSequenceCommand;

    uint32_t uiSeqeuenceLength;

    // Define structs
    AFERefCfg_Type S_AFEReferenceBufferConfig;
    LPLoopCfg_Type S_LPLoopConfig;
    DSPCfg_Type S_DSPConfig;
    
    // Start sequence generator
    AD5940_SEQGenCtrl(bTRUE);

    // Disable all control signals of the AFE 
    AD5940_AFECtrlS(AFECTRL_ALL, bFALSE);

    // Disable high power band-gap
    S_AFEReferenceBufferConfig.HpBandgapEn = bTRUE;

    // Enable high power 1.1 V reference buffer
    S_AFEReferenceBufferConfig.Hp1V1BuffEn = bTRUE;

    // Enable high power 1.8 V reference buffer
    S_AFEReferenceBufferConfig.Hp1V8BuffEn = bTRUE;

    // Disable discharge of 1.1 V capacitor
    S_AFEReferenceBufferConfig.Disc1V1Cap = bFALSE;

    // Disable discharge of 1.8 V cpacitor
    S_AFEReferenceBufferConfig.Disc1V8Cap = bFALSE;
    
    // Disable thermal buffer
    S_AFEReferenceBufferConfig.Hp1V8ThemBuff = bFALSE;

    // Disable current limit for 1.8 V buffer
    S_AFEReferenceBufferConfig.Hp1V8Ilimit = bFALSE;

    // Disable 1.1 V reference buffer
    S_AFEReferenceBufferConfig.Lp1V1BuffEn = bFALSE;

    // Disable 1.8 V reference buffer                                                       
    S_AFEReferenceBufferConfig.Lp1V8BuffEn = bFALSE;
    
    // Enable low power band gap
    S_AFEReferenceBufferConfig.LpBandgapEn = bTRUE;

    // Enable 2.5 V reference buffer
    S_AFEReferenceBufferConfig.LpRefBufEn = bTRUE;

    // Disable boost buffer current
    S_AFEReferenceBufferConfig.LpRefBoostEn = bFALSE;

    // Configure reference buffer
    AD5940_REFCfgS(&S_AFEReferenceBufferConfig);
    
    /*************************************************************************/
    // Low power amplifier config
    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355
    S_LPLoopConfig.LpAmpCfg.LpAmpSel = LPAMP0;

    // Get low power amplifier mode
    S_LPLoopConfig.LpAmpCfg.LpAmpPwrMod = c_DataStorageGeneral_->
        get_LPAmpPowerMode();

    // Enable potential amplifier
    S_LPLoopConfig.LpAmpCfg.LpPaPwrEn = bTRUE;

    // Enable low power TIA amplifier
    S_LPLoopConfig.LpAmpCfg.LpTiaPwrEn = bTRUE;

    // Size of the first order RC resistor
    S_LPLoopConfig.LpAmpCfg.LpTiaRf = LPTIARF_20K;

    // Get stored LPTIA load size
    S_LPLoopConfig.LpAmpCfg.LpTiaRload = c_DataStorageGeneral_->
        get_LPTIALoadSize();
  
    // Get stored LPTIA Rtia size
    S_LPLoopConfig.LpAmpCfg.LpTiaRtia = c_DataStorageLocal_->
        get_LPTIARtiaSize();

    // Check if external resistor is used
    if (S_LPLoopConfig.LpAmpCfg.LpTiaRtia == LPTIARTIA_OPEN){
        // Close switches to support external resistor
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(8) | 
                                          LPTIASW(4) | 
                                          LPTIASW(5) | 
                                          LPTIASW(9);
    }                             
    else {
        // Close swtiches to support internal resistor
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(8) | 
                                          LPTIASW(4) ;          // removed switch 5
    }
    
    /*************************************************************************/
    // Low power DAC config
    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355
    S_LPLoopConfig.LpDacCfg.LpdacSel = LPDAC0;

    // Set potential of the working electrode to 1.1 V
    S_LPLoopConfig.LpDacCfg.DacData6Bit = 
        (uint32_t)((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 2 * 
        AD5940_6BIT_DAC_1LSB);

    // Set potential of the reference electrode to 1.1 V
    S_LPLoopConfig.LpDacCfg.DacData6Bit = 
        (uint32_t)((AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) / 2 * 
        AD5940_6BIT_DAC_1LSB);      

    // Disable data reset
    S_LPLoopConfig.LpDacCfg.DataRst = bFALSE;

    // Disable connection from DACS to amplifieres
    S_LPLoopConfig.LpDacCfg.LpDacSW = 0; 

    // Set reference for Low power DAC to internal 2.5 V
    S_LPLoopConfig.LpDacCfg.LpDacRef = LPDACREF_2P5;

    // Set data source to REG_AFE_LPDACDAT0DATA0
    S_LPLoopConfig.LpDacCfg.LpDacSrc = LPDACSRC_MMR;

    // Connect 12-Bit DAC to Vbias
    S_LPLoopConfig.LpDacCfg.LpDacVbiasMux = LPDACVBIAS_12BIT;

    // Connect 6-Bit DAC to Vzero
    S_LPLoopConfig.LpDacCfg.LpDacVzeroMux = LPDACVZERO_6BIT;

    // Power up register where 6-Bit and 12-Bit DAC write their data to.
    S_LPLoopConfig.LpDacCfg.PowerEn = bTRUE;

    // Configure low power loop (DAC and amplifier)
    AD5940_LPLoopCfgS(&S_LPLoopConfig);

    /*************************************************************************/
    // Configure digital signal processor (DSP)
    // Set all members of the structure to 0
    AD5940_StructInit(&S_DSPConfig, sizeof(S_DSPConfig));

    // Set negative input channel of ADC to LPTIA0 negative input mode
    S_DSPConfig.ADCBaseCfg.ADCMuxN = ADCMUXN_LPTIA0_N;

    // Set positive input channel of ADC to LPTIA0 positive input mode
    S_DSPConfig.ADCBaseCfg.ADCMuxP = ADCMUXP_LPTIA0_P;

    // Get gain of programmable gain amplifier
    S_DSPConfig.ADCBaseCfg.ADCPga = c_DataStorageLocal_->get_AdcPgaGain();

    // Select sampling rate according to ADC clock
    // Clock = 16 MHz -> 800 kHz sampling
    // Clock = 32 MHz -> 1.6 MHz sampling
    S_DSPConfig.ADCFilterCfg.ADCRate = ADCRATE_800KHZ;

    // Check Sinc3 filter oversampling rate
    if (c_DataStorageLocal_->get_AdcOsrSinc3() == ADCSINC3OSR_DISABLED){
        // Bypass Sinc3 filter
        S_DSPConfig.ADCFilterCfg.BpSinc3 = bTRUE;
    }
    else {
        // Disable bypass of Sinc3 filter
        S_DSPConfig.ADCFilterCfg.BpSinc3 = bFALSE;

        // Get oversampling rate for Sinc3 filter
        S_DSPConfig.ADCFilterCfg.ADCSinc3Osr = c_DataStorageLocal_->
            get_AdcOsrSinc3();        
    }

    // Check Sinc2 filter oversampling rate
    if (c_DataStorageLocal_->get_AdcOsrSinc2() == ADCSINC2OSR_DISABLED){
        // Disable Sinc2 filter + Notch filter
        S_DSPConfig.ADCFilterCfg.Sinc2NotchEnable = bFALSE;
    }
    else {
        // Enable Sinc2 filter + Notch filter
        S_DSPConfig.ADCFilterCfg.Sinc2NotchEnable = bTRUE;

        // Enable/ disable bypass of Notch filter
        S_DSPConfig.ADCFilterCfg.BpNotch = (BoolFlag)c_DataStorageLocal_->
                                                     get_AdcNotchFilter();
        // Get oversampling rate for Sinc2 filter
        S_DSPConfig.ADCFilterCfg.ADCSinc2Osr = c_DataStorageLocal_->
            get_AdcOsrSinc2(); 
    }

    // Config digital signal processor (DSP)
    AD5940_DSPCfgS(&S_DSPConfig);

    // Add cumston command -> Squence stop. This ensures the intialization 
    // sequence. Runs only one time
    AD5940_SEQGenInsert(SEQ_STOP());

    // Stop sequence generator
    AD5940_SEQGenCtrl(bFALSE);

    // Create sequence
    iErrorCode = AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSeqeuenceLength);

    if (iErrorCode == AD5940ERR_OK){
        // Get sequence info 
        SEQInfo_Type S_SequenceInfo = c_DataStorageGeneral_->
            get_SequenceInfo(SEQID_1);

        // Set all members of the structure to 0
        AD5940_StructInit(&S_SequenceInfo, sizeof(S_SequenceInfo));
        if (uiSeqeuenceLength >= c_DataStorageGeneral_->get_SeqMaxLength()){
            return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;
        }
            
        // Set sequence ID to 3
        S_SequenceInfo.SeqId = SEQID_1;

        // Get sequener start adress in SRAM
        S_SequenceInfo.SeqRamAddr = c_DataStorageGeneral_->get_SeqStartAddress();

        // Save pointer to sequencer commands stored in the MCU
        S_SequenceInfo.pSeqCmd = uiSequenceCommand;

        // Store length of the commands
        S_SequenceInfo.SeqLen = uiSeqeuenceLength;

        // Set write to SRAM flag
        S_SequenceInfo.WriteSRAM = bTRUE;

        // Save configuration
        c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_1);

        // Initialize sequence info
        AD5940_SEQInfoCfg(&S_SequenceInfo);
    }
    else {
        // Error occured while creating sequence
        return iErrorCode;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for generating the sequencer for the control of the ADC
 * sequence
 * @return: Returns error code encoded as integer
 *****************************************************************************/
int C_Setup_OCP::funSequencerADCControl(){
    // Variable initalization
    int iErrorCode = EC_NO_ERROR;

    const uint32_t *uiSequenceCommand;
    uint32_t uiSequenceLength;

    // Start the sequence generator
    AD5940_SEQGenCtrl(bTRUE);

    // Stop ADC
    AD5940_AFECtrlS(AFECTRL_ADCCNV | AFECTRL_ADCPWR, bFALSE);

    // Create custom interrupt 1
    AD5940_SEQGenInsert(SEQ_INT1());

    // Power up the ADC
    AD5940_AFECtrlS(AFECTRL_ADCPWR, bTRUE);

    // Wait for power up (1 / 16 MHz * 16 * 250 = 250 us)
    AD5940_SEQGenInsert(SEQ_WAIT(16 * 250));

    // Start ADC conversion
    AD5940_AFECtrlS(AFECTRL_ADCCNV, bTRUE);

    // Create sequence
    iErrorCode = AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSequenceLength);

    // Stop the sequence generator
    AD5940_SEQGenCtrl(bFALSE);

    // Check if error occured
    if (iErrorCode == AD5940ERR_OK){
        // Get sequence info 
        SEQInfo_Type S_SequenceInfo = c_DataStorageGeneral_->
            get_SequenceInfo(SEQID_2); 

        // Set all members of the structure to 0
        AD5940_StructInit(&S_SequenceInfo, sizeof(S_SequenceInfo));

        // Check if enough space for sequence is present
        if ((uiSequenceLength + c_DataStorageGeneral_->get_SequenceInfo(SEQID_1).
            SeqLen) >= c_DataStorageGeneral_->get_SeqMaxLength())
            return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;

        // Set sequence ID to 2            
        S_SequenceInfo.SeqId = SEQID_2;

        // Get sequener start adress in SRAM        
        S_SequenceInfo.SeqRamAddr = 
        c_DataStorageGeneral_->get_SequenceInfo(SEQID_1).SeqRamAddr + 
        c_DataStorageGeneral_->get_SequenceInfo(SEQID_1).SeqLen;

        // Save pointer to sequencer commands stored in the MCU        
        S_SequenceInfo.pSeqCmd = uiSequenceCommand;

        // Store length of the commands        
        S_SequenceInfo.SeqLen = uiSequenceLength;

        // Set write to SRAM flag
        S_SequenceInfo.WriteSRAM = bTRUE;

        // Save configuration
        c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_2); 

        // Initialize sequence info
        AD5940_SEQInfoCfg(&S_SequenceInfo);
    }
    else {
        // Error occured
        return iErrorCode;
    }
    return EC_NO_ERROR;
}

#endif /* setup_OCP_CPP */
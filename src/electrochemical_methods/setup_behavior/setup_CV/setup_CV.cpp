/******************************************************************************
 * @brief: Source file containing the subclass (C_Setup) C_Setup_CV which 
 * defines the behavior of setting up an cyclic voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_CV_CPP
#define setup_CV_CPP

// Include dependencies
#include "setup_CV.h"

/******************************************************************************
 * @brief Constructor of the class C_Setup_CV
 * 
 *****************************************************************************/ 
C_Setup_CV::C_Setup_CV(){}

/******************************************************************************
 * @brief Starting method for the class C_Setup_CV
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Setup_CV::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Check if experiment parameters are valid
    this->funInitCV();
}

/******************************************************************************
 * @brief Check if stored parameters are valid for cyclic voltammetry
 * @details: Defined error codes:
 * Error code   :       Definition
 * 0            :       No error occured
 * 21002        :       Sample buffer too small
 * 21001        :       Wake up of AFE failed
 * 21010        :       Turning potentials outside operation range
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_CV::funInitCV(){
    // Initialize variables
    int iErrorCode = 0;

    FIFOCfg_Type S_FiFoConfig;
    SEQCfg_Type S_SeuqencerConfig;    
    SEQInfo_Type S_SequenceInfo;

    // Get max. sweap range
    float fRange = (AD5940_MAX_DAC_OUTPUT - AD5940_MIN_DAC_OUTPUT) * 2;

    // Check if range of potentials is inside possible operation range
    if (fabs(c_DataStorageLocal_->get_UpperVoltage() - 
             c_DataStorageLocal_->get_LowerVoltage()) >= fRange){
        // Parameters out of range
        iErrorCode = EC_SETUP + EC_SE_TP_OOR;
        return iErrorCode;
    }
    /*************************************************************************/
    // Define operation range potentials of the working electrode
    // Check if potential of the working electrode is fixed or not
    if (c_DataStorageLocal_->get_FixedWEPotential() == true){
        // Set WE potential close to the upper voltage, since the potential of 
        // the cell is calculated as Vzero - Vbias (Vzero --> fWePotentialHigh)
        float fWePotentialHigh = c_DataStorageLocal_->get_UpperVoltage() + 
            50.0 + AD5940_MIN_DAC_OUTPUT;

        // Check if potential of the working electrode exceeds 2.4 V / 0.2 V
        if (fWePotentialHigh >= AD5940_MAX_DAC_OUTPUT){
            fWePotentialHigh = AD5940_MAX_DAC_OUTPUT;
        }
        else if(fWePotentialHigh <= 50.0 + 2 * AD5940_MIN_DAC_OUTPUT){
            fWePotentialHigh = 50.0 + 2 * AD5940_MIN_DAC_OUTPUT;
        }   
        
        c_DataStorageLocal_->set_WePotentialHigh(fWePotentialHigh);

        float fWePotentialLow = fWePotentialHigh;
        c_DataStorageLocal_->set_WePotentialLow(fWePotentialLow);

        // Operation range of 4.4 V is halfed to +/- 2.2 V
        fRange /= 2;
    }
    else {
        // Working potential can range from 0.4 V to 2.2 V
        c_DataStorageLocal_->set_WePotentialHigh(c_DataStorageLocal_->
            get_UpperVoltage() + 50.0 + AD5940_MIN_DAC_OUTPUT);
        c_DataStorageLocal_->set_WePotentialLow(AD5940_MIN_DAC_OUTPUT);
    }

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
    // Generate CV sequences (Init, DAC, ADC)
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

    // DAC control sequence
    iErrorCode = this->funSequencerDACControl();
    if (iErrorCode != EC_NO_ERROR){
        return iErrorCode;
    }

    // Get intialization sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_3);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Enable sequencer
    AD5940_SEQCtrlS(bTRUE);                          

    // Trigger sequencer by writing in the register
    AD5940_SEQMmrTrig(S_SequenceInfo.SeqId);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_3);

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

    // Get DAC sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_0);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info    
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_0);

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
 * 21002        :       Sample buffer too small
 * 21001        :       Wake up of AFE failed
 * 21010        :       Turning potentials outside operation range
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_CV::funSequencerInitializationSequence(){
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
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(2) | LPTIASW(4) | 
                                          LPTIASW(5) | LPTIASW(9);
    }                             
    else {
        // Close swtiches to support internal resistor
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(2) | LPTIASW(4) | LPTIASW(5);
    }
    
    /*************************************************************************/
    // Low power DAC config
    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355
    S_LPLoopConfig.LpDacCfg.LpdacSel = LPDAC0;

    // Set cell voltage (Voltage of the working electrode)
    if (c_DataStorageLocal_->get_StartVoltage() > 0){
        // Set potential of the working electrode to the high level
        S_LPLoopConfig.LpDacCfg.DacData6Bit = 
            (uint32_t)((c_DataStorageLocal_->get_WePotentialHigh() - 
            AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB);
    }
    else {
        // Set potential of the working electrode to the low level
        S_LPLoopConfig.LpDacCfg.DacData6Bit = 
            (uint32_t)((c_DataStorageLocal_->get_WePotentialLow() - 
            AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB);        
    }

    // Set potential of the reference electrode
    S_LPLoopConfig.LpDacCfg.DacData12Bit = 
        (uint32_t)(S_LPLoopConfig.LpDacCfg.DacData6Bit * 64 - 
        c_DataStorageLocal_->get_StartVoltage() / AD5940_12BIT_DAC_1LSB);

    // Disable data reset
    S_LPLoopConfig.LpDacCfg.DataRst = bFALSE;

    // Close switch LPDACSW0[4] and LPDACSW0[2]
    S_LPLoopConfig.LpDacCfg.LpDacSW = LPDACSW_VBIAS2LPPA | LPDACSW_VZERO2LPTIA;

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
            get_SequenceInfo(SEQID_3);

        // Set all members of the structure to 0
        AD5940_StructInit(&S_SequenceInfo, sizeof(S_SequenceInfo));
        if (uiSeqeuenceLength >= c_DataStorageGeneral_->get_SeqMaxLength()){
            return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;
        }
            
        // Set sequence ID to 3
        S_SequenceInfo.SeqId = SEQID_3;

        // Get sequener start adress in SRAM
        S_SequenceInfo.SeqRamAddr = c_DataStorageGeneral_->get_SeqStartAddress();

        // Save pointer to sequencer commands stored in the MCU
        S_SequenceInfo.pSeqCmd = uiSequenceCommand;

        // Store length of the commands
        S_SequenceInfo.SeqLen = uiSeqeuenceLength;

        // Set write to SRAM flag
        S_SequenceInfo.WriteSRAM = bTRUE;

        // Save configuration
        c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_3);

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
int C_Setup_CV::funSequencerADCControl(){
    // Initialize variables
    int iErrorCode = EC_NO_ERROR;

    const uint32_t *uiSequenceCommand;

    uint32_t uiSequenceLength;

    // Start the sequence generator
    AD5940_SEQGenCtrl(bTRUE);

    // Let Syncnextdevice register control GPIO pin 2
    AD5940_SEQGpioCtrlS(AGPIO_Pin2);

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
        if ((uiSequenceLength + c_DataStorageGeneral_->get_SequenceInfo(SEQID_3).
            SeqLen) >= c_DataStorageGeneral_->get_SeqMaxLength())
            return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;

        // Set sequence ID to 2            
        S_SequenceInfo.SeqId = SEQID_2;

        // Get sequener start adress in SRAM        
        S_SequenceInfo.SeqRamAddr = 
        c_DataStorageGeneral_->get_SequenceInfo(SEQID_3).SeqRamAddr + 
        c_DataStorageGeneral_->get_SequenceInfo(SEQID_3).SeqLen;

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

/******************************************************************************
 * @brief Method for generating the sequencer for the control of the DAC
 * sequence
 * @return: Returns error code encoded as integer
 *****************************************************************************/
int C_Setup_CV::funSequencerDACControl(){
    // Initialize variables
    int iDacMaxSeqLen;
    int iDacSeqBlock0Address;
    int iDacSeqBlock1Address;
    int iStepsRemaining;
    int iStepsPerBlock;

    SEQInfo_Type S_SequenceInfo;

    // Increment DAC at least one step to prevent plateaus. This calculation            
    // overwrites the step size if the step size is chosen to small
    if (c_DataStorageLocal_->get_Stepsize() < AD5940_12BIT_DAC_1LSB){
        c_DataStorageLocal_->set_DacIncrementPerStep(1);
    }
    else {
        // TODO: DEFINE FOR VARIABLE (TO DISABLE THIS OPTION)
        if (true == true){                                                                                              
            c_DataStorageLocal_->set_DacIncrementPerStep(int(
                c_DataStorageLocal_->get_Stepsize() / AD5940_12BIT_DAC_1LSB + 0.5));
        }        
        else {
            c_DataStorageLocal_->set_DacIncrementPerStep(
                c_DataStorageLocal_->get_Stepsize() / AD5940_12BIT_DAC_1LSB);
        }
    }

    // Calculate the total amount of steps between the peak voltages.
    // This calculation depends on the calulated stepsize
    c_DataStorageLocal_->set_StepNumber(((int)(fabsf
        (c_DataStorageLocal_->get_UpperVoltage() - 
         c_DataStorageLocal_->get_LowerVoltage()) / 
        (c_DataStorageLocal_->get_DacIncrementPerStep() * 
         AD5940_12BIT_DAC_1LSB))) * 2);

    // Determine if DAC must increment or decrement
    // Determine the amount of steps from the starting voltage to both
    // peak voltages
    if (c_DataStorageLocal_->get_StartVoltage() < 
            c_DataStorageLocal_->get_UpperVoltage()){
        c_DataStorageLocal_->set_DacIncrement(true);
    }
    else {
        c_DataStorageLocal_->set_DacIncrement(false);
    }
    
    // Save starting posistion of DAC
    c_DataStorageLocal_->set_DacCurrentCode(c_DataStorageLocal_->
                            get_StartVoltage() / AD5940_12BIT_DAC_1LSB);

    // Get the amount of steps remaining
    iStepsRemaining = c_DataStorageLocal_->get_StepNumber() * 
                      c_DataStorageLocal_->get_Cycle();
    
    // Save reamaining steps for the execute function
    c_DataStorageLocal_->set_StepsRemaining(iStepsRemaining);

    // Get the maximum length of the sequence available
    iDacMaxSeqLen = c_DataStorageGeneral_->get_SeqMaxLength() - 
                    c_DataStorageGeneral_->get_SequenceInfo(SEQID_2).SeqLen - 
                    c_DataStorageGeneral_->get_SequenceInfo(SEQID_3).SeqLen;

    // Check if SRAM of the sequencer has enough space for the commands
    if (iDacMaxSeqLen < AD5940_BUFFER_CV * AD5940_SIZE_SEQ_COMMAND)
        // Error not enough space for storing DAC sequencer commands
        return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;

    // Set amount of steps per block to one to ensure that the MCU updates
    // the DAC every step so no data accumulates.
    iStepsPerBlock = 1;

    // Save the steps per block for the execute function                                   
    c_DataStorageLocal_->set_StepsPerBlock(iStepsPerBlock);

    // Calculate current address of sequence block 0
    iDacSeqBlock0Address = c_DataStorageGeneral_->get_SequenceInfo(SEQID_2).SeqLen + 
                           c_DataStorageGeneral_->get_SequenceInfo(SEQID_2).SeqRamAddr;

    // Save the address of sequence block 0 for the execute function
    c_DataStorageLocal_->set_DacSeqBlock0Address(iDacSeqBlock0Address);

    // Calculate current address of sequence block 1
    iDacSeqBlock1Address = iDacSeqBlock0Address + (iStepsPerBlock + 1) * 
                           AD5940_BUFFER_CV;                                     

    // Save the address of sequence block 1 for the execute function
    c_DataStorageLocal_->set_DacSeqBlock1Address(iDacSeqBlock1Address);

    // Save currently used block
    c_DataStorageLocal_->set_DacCurrentBlock(CURRENT_BLOCK_0);

    // Set state of experiment to state 0
    c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_0);

    // Set position of the current step to 0 (Initialization)
    c_DataStorageLocal_->set_CurrentStepNumber(0);

    // Save currently saved block
    c_DataStorageLocal_->set_SeqBlockUsed(true);
        
    // Init first DAC sequence
    // Set sequence id to SEQID_0 (0)
    S_SequenceInfo.SeqId = SEQID_0;

    // Set sequence length to 6
    S_SequenceInfo.SeqLen = AD5940_BUFFER_CV;

    // Set start address to start address of block
    S_SequenceInfo.SeqRamAddr = iDacSeqBlock0Address;

    // Disable write to SRAM since we write with custom sequence above
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Save DAC sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_0);

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Set sequence id to SEQID_1 (1)
    S_SequenceInfo.SeqId = SEQID_1;

    // Set sequence length to 6
    S_SequenceInfo.SeqLen = AD5940_BUFFER_CV;

    // Set start address to start address of block
    S_SequenceInfo.SeqRamAddr = iDacSeqBlock1Address;

    // Disable write to SRAM since we write with custom sequence above
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Save DAC sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_1);

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    return EC_NO_ERROR;
}

#endif /* setup_CV_CPP */
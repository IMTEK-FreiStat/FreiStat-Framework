/******************************************************************************
 * @brief: Source file containing the subclass (C_Setup) C_Setup_DPV which 
 * defines the behavior of setting up an differential pulse voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *
 *****************************************************************************/

// Include guard
#ifndef setup_DPV_CPP
#define setup_DPV_CPP

// Include dependencies
#include "setup_DPV.h"

/******************************************************************************
 * @brief Constructor of the class C_Setup_DPV
 * 
 *****************************************************************************/ 
C_Setup_DPV::C_Setup_DPV(){}

/******************************************************************************
 * @brief Starting method for the class C_Setup_DPV
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Setup_DPV::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Initialize differntial pulse voltammetry
    this->funInitDPV();
}

/******************************************************************************
 * @brief Check if stored parameters are valid for differntial pulse voltammetry
 * @details: Defined error codes:
 * Error code   :       Definition
 * 0            :       No error occured
 * 21001        :       Wake up of AFE failed
 * 21002        :       Sample buffer to small
 * 21003        :       Sequence doenst fit into SRAM of sequencer
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_DPV::funInitDPV(){
    // Initialize variables
    int iErrorCode = 0;

    FIFOCfg_Type S_FiFoConfig;
    SEQCfg_Type S_SequencerConfig;    
    SEQInfo_Type S_SequenceInfo;

    // Wakeup AFE by reading register, read is tried 10 times at most
    if (AD5940_WakeUp(10) > 10){
        // Error wakeup failed
        return EC_SETUP + EC_SE_WAKEUP_AFE_ERR;        
    }

    // Initializing the sequencer
    // Disable -> not used
    S_SequencerConfig.SeqBreakEn = bFALSE;

    // Clear sequencer count and crc checksum
    S_SequencerConfig.SeqCntCRCClr = bTRUE;

    // Disable sequencer
    S_SequencerConfig.SeqEnable = bFALSE;

    // Disable -> not used
    S_SequencerConfig.SeqIgnoreEn = bFALSE;

    // Set sequencer size to 2 kB
    S_SequencerConfig.SeqMemSize = SEQMEMSIZE_2KB;

    // Disable waiting time after every command
    S_SequencerConfig.SeqWrTimer = 0;

    // Initialzie sequencer
    AD5940_SEQCfg(&S_SequencerConfig);

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

    // Set FIFO size to 4 kB (2 kB for sequencer)
    S_FiFoConfig.FIFOSize = FIFOSIZE_4KB;

    // Configure FIFO
    AD5940_FIFOCfg(&S_FiFoConfig);

    // Reset all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    // Set potential of the working electrode 
    c_DataStorageLocal_->set_WePotentialHigh(
        c_DataStorageLocal_->get_UpperVoltage() + 
        c_DataStorageLocal_->get_PotentialSteps(1) + 
        AD5940_MIN_DAC_OUTPUT + 50.0);

    c_DataStorageLocal_->set_WePotentialLow(
        c_DataStorageLocal_->get_UpperVoltage() + 
        c_DataStorageLocal_->get_PotentialSteps(1) + 
        AD5940_MIN_DAC_OUTPUT + 50.0);

    // Initialize lower potential (will be used to ramp up)
    c_DataStorageLocal_->set_LowerVoltage(c_DataStorageLocal_->get_StartVoltage());

    // Generate DPV sequences (Init, Execute)
    // Initialize sequence generator with initalized values
    AD5940_SEQGenInit(c_DataStorageGeneral_->get_SampleBuffer(), SAMPLE_BUFFER);
    
    // Initialization sequence
    iErrorCode = this->funSequencerInitializationSequence();
    if (iErrorCode != EC_NO_ERROR){
        return iErrorCode;
    } 
   
    // Execute sequence
    iErrorCode = this->funSequencerExecuteSequence();
    if (iErrorCode != EC_NO_ERROR){
        return iErrorCode;
    }

    // Get intialization sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_0);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Enable sequencer
    AD5940_SEQCtrlS(bTRUE);  

    // Trigger sequencer by writing in the register
    AD5940_SEQMmrTrig(S_SequenceInfo.SeqId);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_0);

    // Reset all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    // Get execute sequence
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_1);

    // Disable write to SRAM
    S_SequenceInfo.WriteSRAM = bFALSE;

    // Initialize sequence info
    AD5940_SEQInfoCfg(&S_SequenceInfo);

    // Save sequence info
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_1);
    
    // Get execute sequence
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
int C_Setup_DPV::funSequencerInitializationSequence(){
    // Initalize variables
    int iErrorCode = EC_NO_ERROR;

    const uint32_t *uiSequenceCommand;
    
    uint32_t uiSeqeuenceLength;

    // Define structs
    SEQInfo_Type S_SequenceInfo;
    AFERefCfg_Type S_AFEReferenceBufferConfig;
    LPLoopCfg_Type S_LPLoopConfig;
    HSLoopCfg_Type S_HSLoopConfig;
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
    S_AFEReferenceBufferConfig.Lp1V1BuffEn = bTRUE;

    // Disable 1.8 V reference buffer                                                       
    S_AFEReferenceBufferConfig.Lp1V8BuffEn = bTRUE;
    
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
    S_LPLoopConfig.LpAmpCfg.LpTiaRf = LPTIARF_1M;

    // Get stored LPTIA load size
    S_LPLoopConfig.LpAmpCfg.LpTiaRload = c_DataStorageGeneral_->
        get_LPTIALoadSize();
  
    // Get stored LPTIA Rtia size
    S_LPLoopConfig.LpAmpCfg.LpTiaRtia = c_DataStorageLocal_->
        get_LPTIARtiaSize();

    // Check if external resistor is used
    if (S_LPLoopConfig.LpAmpCfg.LpTiaRtia == LPTIARTIA_OPEN){
        // Close switches to support external resistor
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(2) | LPTIASW(4) | LPTIASW(5) | 
                                          LPTIASW(9) | LPTIASW(13);
    }                             
    else {
        // Close swtiches to support internal resistor
        S_LPLoopConfig.LpAmpCfg.LpTiaSW = LPTIASW(2) | LPTIASW(4) | LPTIASW(13);
    }

    /*************************************************************************/
    // Low power DAC config
    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355
    S_LPLoopConfig.LpDacCfg.LpdacSel = LPDAC0;

    // Set cell voltage (Voltage of the working electrode)
    S_LPLoopConfig.LpDacCfg.DacData6Bit = 
        (uint32_t)((c_DataStorageLocal_->get_WePotentialHigh() - 
                   AD5940_MIN_DAC_OUTPUT)/ AD5940_6BIT_DAC_1LSB);

    // Set potential of the reference electrode
    S_LPLoopConfig.LpDacCfg.DacData12Bit = 
        (uint32_t)(S_LPLoopConfig.LpDacCfg.DacData6Bit * 64 - 
                   (c_DataStorageLocal_->get_LowerVoltage()) /
                    AD5940_12BIT_DAC_1LSB);

    // Truncate if needed
    if (S_LPLoopConfig.LpDacCfg.DacData12Bit > 
        S_LPLoopConfig.LpDacCfg.DacData6Bit * 64)
        S_LPLoopConfig.LpDacCfg.DacData12Bit--;

    // Disable data reset
    S_LPLoopConfig.LpDacCfg.DataRst = bFALSE;

    // Close switch LPDACSW0[4] and LPDACSW0[2]
    S_LPLoopConfig.LpDacCfg.LpDacSW = LPDACSW_VBIAS2LPPA | LPDACSW_VZERO2LPTIA | 
                                      LPDACSW_VZERO2PIN;

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

    // Get oversampling rate for Sinc3 filter
    S_DSPConfig.ADCFilterCfg.ADCSinc3Osr = c_DataStorageLocal_->
        get_AdcOsrSinc3();

    // Select sampling rate according to ADC clock
    // Clock = 16 Mhz -> 800 kHz sampling
    // Clock = 32 Mhz -> 1.6 MHz sampling
    S_DSPConfig.ADCFilterCfg.ADCRate = ADCRATE_800KHZ;

    // Disable bypass of Sinc3 filter
    S_DSPConfig.ADCFilterCfg.BpSinc3 = bFALSE;

    // Enable Sinc2 filter + Notch filter
    S_DSPConfig.ADCFilterCfg.Sinc2NotchEnable = bTRUE;

    // Disable bypass of Notch filter
    S_DSPConfig.ADCFilterCfg.BpNotch = bFALSE;

    // Get oversampling rate for Sinc2 filter
    S_DSPConfig.ADCFilterCfg.ADCSinc2Osr = c_DataStorageLocal_->
        get_AdcOsrSinc2(); 

    // Config digital signal processor (DSP)
    AD5940_DSPCfgS(&S_DSPConfig);

    /*************************************************************************/
    // Configure high speed loop
    // Open all switches of the switch matrix
    S_HSLoopConfig.SWMatCfg.Dswitch = 0;
    S_HSLoopConfig.SWMatCfg.Pswitch = 0;
    S_HSLoopConfig.SWMatCfg.Nswitch = 0;
    S_HSLoopConfig.SWMatCfg.Tswitch = 0;

    // Configure high speed loop
    AD5940_HSLoopCfgS(&S_HSLoopConfig);

    // Turn on high reference power, Sinc2 + Notch-Filter and ADC
    AD5940_AFECtrlS(AFECTRL_HPREFPWR | AFECTRL_SINC2NOTCH | 
                    AFECTRL_ADCPWR, bTRUE);

    // Disable syncnextdevice
    AD5940_SEQGpioCtrlS(0);

    // Add cumston command -> Squence stop. This ensures the intialization 
    // sequence. Runs only one time
    AD5940_SEQGenInsert(SEQ_STOP()); 

    // Stop sequence generator
    AD5940_SEQGenCtrl(bFALSE);

    // Create sequence
    iErrorCode = AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSeqeuenceLength);

    if (iErrorCode == AD5940ERR_OK){
        // Get sequence info 
        S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_0);

        // Set all members of the structure to 0
        AD5940_StructInit(&S_SequenceInfo, sizeof(S_SequenceInfo));
        if (uiSeqeuenceLength >= c_DataStorageGeneral_->get_SeqMaxLength()){
            return EC_SETUP + EC_SE_SEQ_BUFF_SIZE;
        }
            
        // Set sequence ID to 0
        S_SequenceInfo.SeqId = SEQID_0;

        // Get sequener start adress in SRAM
        S_SequenceInfo.SeqRamAddr = c_DataStorageGeneral_->
            get_SeqStartAddress();

        // Save pointer to sequencer commands stored in the MCU
        S_SequenceInfo.pSeqCmd = uiSequenceCommand;

        // Store length of the commands
        S_SequenceInfo.SeqLen = uiSeqeuenceLength;

        // Save configuration
        c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_0);

        // Write to SRAM
        AD5940_SEQCmdWrite(S_SequenceInfo.SeqRamAddr, uiSequenceCommand, 
                           uiSeqeuenceLength);
    }
    else {
        // Error occured
        return iErrorCode;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for generating the execute sequence and writing the
 * commands to the SRAM
 * 
 * @return: Error code (Only no error)
 *****************************************************************************/
int C_Setup_DPV::funSequencerExecuteSequence(){
    // Initialize variables
    int iStepsRemaining = 0;

    const uint32_t *uiSequenceCommand;

    uint32_t uiCurrAddr = 0;
    uint32_t uiRegData = 0;
    uint32_t uiSequenceLength = 0;
    uint32_t uiVbiasCode = 0;
    uint32_t uiVzeroCode = 0;

    SEQInfo_Type S_SequenceInfo;

    S_DataContainer S_ExperimentData; 

    // Calculate total amount of steps and round up
    iStepsRemaining = (c_DataStorageLocal_->get_UpperVoltage() -
                       c_DataStorageLocal_->get_StartVoltage()) / 
                       c_DataStorageLocal_->get_PotentialSteps(0) + 0.5;

    // Save amount of steps
    c_DataStorageLocal_->set_StepsRemaining(iStepsRemaining);

    // Calculate DAC code
    uiVzeroCode = (c_DataStorageLocal_->get_WePotentialHigh() - 
                   AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB;
    uiVbiasCode = (uiVzeroCode * 64 - (c_DataStorageLocal_->get_LowerVoltage() / 
                   AD5940_12BIT_DAC_1LSB));

    // Ensure smooth transition when switching potential sign
    if (uiVbiasCode < (uiVzeroCode * 64)){
        uiVbiasCode--;
    }
            
    // Clip DAC code for the 6-Bit and 12-Bit DAC
    if (uiVbiasCode > 4095){
        uiVbiasCode = 4095;
    }
    if (uiVzeroCode > 64){
        uiVzeroCode = 64;
    }

    uiRegData = uiVzeroCode << 12 | uiVbiasCode;

    // Store the voltage value 
    S_ExperimentData.fVoltage = uiVzeroCode * AD5940_6BIT_DAC_1LSB - 
                                uiVbiasCode * AD5940_12BIT_DAC_1LSB;

    // Store experiment data at newest free position
    c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData, 
        this->funGetDataPosition(c_DataStorageLocal_->get_CurrentStepNumber()));

    // Set starting address for execute sequence
    uiCurrAddr = c_DataStorageGeneral_->get_SequenceInfo(SEQID_0).SeqRamAddr + 
                 c_DataStorageGeneral_->get_SequenceInfo(SEQID_0).SeqLen;

    // Define address blocks of sequences
    c_DataStorageLocal_->set_DacSeqBlock0Address(uiCurrAddr);
    c_DataStorageLocal_->set_DacSeqBlock1Address(uiCurrAddr + AD5940_BUFFER_DPV);

    // Initalize both sequences (SEQID_1 and SEQID_2)
    // Execute Sequence Block 1 (SEQID_1)    
    //Generate sequence
    AD5940_SEQGenCtrl(bTRUE);

    // Start ADC conversion 
    AD5940_AFECtrlS(AFECTRL_ADCCNV, bTRUE);

    // Wait for the defined sampling duration
    AD5940_SEQGenInsert(SEQ_WAIT(16 * 1000 * c_DataStorageLocal_->get_Scanrate()));

    // Stop ADC
    AD5940_AFECtrlS(AFECTRL_ADCCNV, bFALSE);

    // Create custom interrupt 1
    AD5940_SEQGenInsert(SEQ_INT1());

    // Write calculated DAC values into register
    AD5940_WriteReg(REG_AFE_LPDACDAT0, uiRegData);

    // Wait 10 clocks for DAC update
    AD5940_SEQGenInsert(SEQ_WAIT(10));

    // Jump to next sequence in the execute sequnence 
    AD5940_SEQGenInsert(SEQ_WR(REG_AFE_SEQ2INFO, ((uiCurrAddr + AD5940_BUFFER_DPV)
                        << BITP_AFE_SEQ2INFO_ADDR) | 
                        (AD5940_BUFFER_DPV << BITP_AFE_SEQ1INFO_LEN)));       

    // Create sequence
    AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSequenceLength);

    // Stop sequence generator
    AD5940_SEQGenCtrl(bFALSE);

    // Get stored sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_1);
    S_SequenceInfo.SeqId = SEQID_1;
    S_SequenceInfo.SeqRamAddr = uiCurrAddr;
    S_SequenceInfo.pSeqCmd = uiSequenceCommand;
    S_SequenceInfo.SeqLen = uiSequenceLength;

    // Save sequence info 
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_1);

    // Write command to SRAM
    AD5940_SEQCmdWrite(uiCurrAddr, uiSequenceCommand, uiSequenceLength);

    // Update current step
    c_DataStorageLocal_->set_CurrentStepNumber(
        c_DataStorageLocal_->get_CurrentStepNumber() + 1);

    // Save voltage of staircase
    c_DataStorageLocal_->set_LowerVoltage(c_DataStorageLocal_->
        get_LowerVoltage() + c_DataStorageLocal_->get_PotentialSteps(0));

    // Calculate DAC code
    uiVzeroCode = (c_DataStorageLocal_->get_WePotentialHigh() - 
                   AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB;
    uiVbiasCode = (uiVzeroCode * 64 - (c_DataStorageLocal_->get_LowerVoltage() + 
                   c_DataStorageLocal_->get_PotentialSteps(1)) / 
                   AD5940_12BIT_DAC_1LSB);

    // Ensure smooth transition when switching potential sign
    if (uiVbiasCode < (uiVzeroCode * 64)){
        uiVbiasCode--;
    }
            
    // Clip DAC code for the 6-Bit and 12-Bit DAC
    if (uiVbiasCode > 4095){
        uiVbiasCode = 4095;
    }
    if (uiVzeroCode > 64){
        uiVzeroCode = 64;
    }

    uiRegData = uiVzeroCode << 12 | uiVbiasCode;

    // Store the voltage value 
    S_ExperimentData.fVoltage = uiVzeroCode * AD5940_6BIT_DAC_1LSB - 
                                uiVbiasCode * AD5940_12BIT_DAC_1LSB;

    // Store experiment data at newest free position
    c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData, 
        this->funGetDataPosition(c_DataStorageLocal_->get_CurrentStepNumber()));

    // Execute Sequence Block 2 (SEQID_2)
    // Edit current address
    uiCurrAddr = uiCurrAddr + AD5940_BUFFER_DPV;

    //Generate sequence
    AD5940_SEQGenCtrl(bTRUE);

    // Start ADC conversion 
    AD5940_AFECtrlS(AFECTRL_ADCCNV, bTRUE);

    // Wait for the defined sampling duration
    AD5940_SEQGenInsert(SEQ_WAIT(16 * 1000 * c_DataStorageLocal_->get_Scanrate()));

    // Stop ADC
    AD5940_AFECtrlS(AFECTRL_ADCCNV, bFALSE);

    // Create custom interrupt 1
    AD5940_SEQGenInsert(SEQ_INT1());

    // Write calculated DAC values into register
    AD5940_WriteReg(REG_AFE_LPDACDAT0, uiRegData);

    // Wait 10 clocks for DAC update
    AD5940_SEQGenInsert(SEQ_WAIT(10));

    // Jump to next sequence in the execute sequnence 
    AD5940_SEQGenInsert(SEQ_WR(REG_AFE_SEQ1INFO, ((uiCurrAddr - AD5940_BUFFER_DPV)
                        << BITP_AFE_SEQ1INFO_ADDR) | 
                        (AD5940_BUFFER_DPV << BITP_AFE_SEQ2INFO_LEN)));       

    // Create sequence
    AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSequenceLength);

    // Stop sequence generator
    AD5940_SEQGenCtrl(bFALSE);

    // Get stored sequence info
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_2);
    S_SequenceInfo.SeqId = SEQID_2;
    S_SequenceInfo.SeqRamAddr = uiCurrAddr;
    S_SequenceInfo.pSeqCmd = uiSequenceCommand;
    S_SequenceInfo.SeqLen = uiSequenceLength;

    // Save sequence info 
    c_DataStorageGeneral_->set_SequenceInfo(S_SequenceInfo, SEQID_2);

    // Write command to SRAM
    AD5940_SEQCmdWrite(uiCurrAddr, uiSequenceCommand, uiSequenceLength);

    // Save currently used block
    c_DataStorageLocal_->set_DacCurrentBlock(CURRENT_BLOCK_0);

    // Save currently saved block
    c_DataStorageLocal_->set_SeqBlockUsed(false);

    return EC_NO_ERROR;
}

#endif /* setup_DPV_CPP */
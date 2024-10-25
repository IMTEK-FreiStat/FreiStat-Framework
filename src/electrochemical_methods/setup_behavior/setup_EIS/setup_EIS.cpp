/******************************************************************************
 * @brief: Source file containing the subclass (C_Setup) C_Setup_EIS which 
 * defines the behavior of setting up an chronoamperometry.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *
 *****************************************************************************/

// Include guard
#ifndef setup_EIS_CPP
#define setup_EIS_CPP

// Include dependencies
#include "setup_EIS.h"

/******************************************************************************
 * @brief Constructor of the class C_Setup_EIS
 * 
 *****************************************************************************/ 
C_Setup_EIS::C_Setup_EIS(){}

/******************************************************************************
 * @brief Starting method for the class C_Setup_EIS
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
int C_Setup_EIS::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Initialize chronoamperometry
    return this->funInitEIS();
}

/******************************************************************************
 * @brief Check if stored parameters are valid for chronoamperometry
 * @details: Defined error codes:
 * Error code   :       Definition
 * 0            :       No error occured
 * 21001        :       Wake up of AFE failed
 * 21002        :       Sample buffer to small
 * 21003        :       Sequence doenst fit into SRAM of sequencer
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Setup_EIS::funInitEIS(){
    // Initialize variables
    int iErrorCode = 0;

    FIFOCfg_Type S_FiFoConfig;
    SEQCfg_Type S_SequencerConfig;

    SEQInfo_Type S_SequenceInfo;
    AFERefCfg_Type S_AFE_ReConfig;
    HSLoopCfg_Type S_HSLoop_Config;
    DSPCfg_Type S_Dsp_Config;


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
    S_FiFoConfig.FIFOSrc = FIFOSRC_DFT;

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


    /*************************************************************************/
    // Generate EIS sequences (Init, Execute)
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

    // Disable sequencer
    AD5940_SEQCtrlS(bFALSE); 
    AD5940_WriteReg(REG_AFE_SEQCNT, 0);
    
    // Enable sequencer
    AD5940_SEQCtrlS(bTRUE); 

    // Clear interrupt flag
    c_DataSoftwareStorage_->get_AD5940Setup()->set_InterruptOccured(false);
    
    // Set AFE to low power mode
    AD5940_AFEPwrBW(AFEPWR_HP, AFEBW_250KHZ);

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
int C_Setup_EIS::funSequencerInitializationSequence(){
    // Initalize variables
    int iErrorCode = EC_NO_ERROR;

    const uint32_t *uiSequenceCommand;
    
    uint32_t uiSeqeuenceLength;

    // Define structs
    SEQInfo_Type S_SequenceInfo;
    AFERefCfg_Type S_AFEReferenceBufferConfig;
    HSLoopCfg_Type S_HSLoopConfig;
    DSPCfg_Type S_DSPConfig;
    SoftSweepCfg_Type S_Sweep_Config;

    float fNext_Frequency;

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
    
    // LP reference control - turned off to save power
    if (c_DataStorageLocal_->get_DcOffset() != 0.0){
        
        // Enable low power band gap
        S_AFEReferenceBufferConfig.LpBandgapEn = bTRUE;

        // Enable 2.5 V reference buffer
        S_AFEReferenceBufferConfig.LpRefBufEn = bTRUE;
    }
    else{
        // Disable low power band gap
        S_AFEReferenceBufferConfig.LpBandgapEn = bFALSE;

        // Disable 2.5 V reference buffer
        S_AFEReferenceBufferConfig.LpRefBufEn = bFALSE;
    }

    // Disable boost buffer current
    S_AFEReferenceBufferConfig.LpRefBoostEn = bFALSE;

    // Configure reference buffer
    AD5940_REFCfgS(&S_AFEReferenceBufferConfig);


    /**************************************************************************/
    // Config Sweep

    S_Sweep_Config.SweepEn = bTRUE;

    S_Sweep_Config.SweepStart = c_DataStorageLocal_->get_StartFrequency();
    S_Sweep_Config.SweepStop = c_DataStorageLocal_->get_StopFrequency();
    S_Sweep_Config.SweepPoints = c_DataStorageLocal_->get_NumberPoints();
    S_Sweep_Config.SweepLog = c_DataStorageLocal_->get_SweepTyp();
    c_DataStorageLocal_->set_CurrentFrequency(S_Sweep_Config.SweepStart);


    /*************************************************************************/
    // Configure high speed loop

    S_HSLoopConfig.HsDacCfg.ExcitBufGain = EXCITBUFGAIN_2;
    S_HSLoopConfig.HsDacCfg.HsDacGain = HSDACGAIN_1;
    S_HSLoopConfig.HsDacCfg.HsDacUpdateRate = 7;

    S_HSLoopConfig.HsTiaCfg.DiodeClose = bFALSE;

    // With dc offset
    if( c_DataStorageLocal_->get_DcOffset() != 0.0f){   
        S_HSLoopConfig.HsTiaCfg.HstiaBias = HSTIABIAS_VZERO0;
    }
    else {
        S_HSLoopConfig.HsTiaCfg.HstiaBias = HSTIABIAS_1P1;
    }
    S_HSLoopConfig.HsTiaCfg.HstiaCtia = 31; /* 31pF + 2pF */
    S_HSLoopConfig.HsTiaCfg.HstiaDeRload = HSTIADERLOAD_OPEN;
    S_HSLoopConfig.HsTiaCfg.HstiaDeRtia = HSTIADERTIA_OPEN;
    S_HSLoopConfig.HsTiaCfg.HstiaRtiaSel = HSTIARTIA_5K;

    S_HSLoopConfig.SWMatCfg.Dswitch = SWD_CE0;
    S_HSLoopConfig.SWMatCfg.Pswitch = SWP_CE0;
    S_HSLoopConfig.SWMatCfg.Nswitch = SWN_AIN1;
    S_HSLoopConfig.SWMatCfg.Tswitch = SWT_TRTIA|SWT_AIN1;

    S_HSLoopConfig.WgCfg.WgType = WGTYPE_SIN;
    S_HSLoopConfig.WgCfg.GainCalEn = bTRUE;
    S_HSLoopConfig.WgCfg.OffsetCalEn = bTRUE;
    //if(AppIMPCfg.SweepCfg.SweepEn == bTRUE)
    //{
    //    AppIMPCfg.FreqofData = AppIMPCfg.SweepCfg.SweepStart;
    //   AppIMPCfg.SweepCurrFreq = AppIMPCfg.SweepCfg.SweepStart;
    //      AD5940_SweepNext(&AppIMPCfg.SweepCfg, &AppIMPCfg.SweepNextFreq);
    //    sin_freq = AppIMPCfg.SweepCurrFreq;
    //}
    /*else
    {
        sin_freq = AppIMPCfg.SinFreq;
        AppIMPCfg.FreqofData = sin_freq;
    }*/

    AD5940_SweepNext(& S_Sweep_Config, &fNext_Frequency);

    S_HSLoopConfig.WgCfg.SinCfg.SinFreqWord = AD5940_WGFreqWordCal( c_DataStorageLocal_->get_CurrentFrequency(), AD5940_SYS_CLOCK_FREQ);
    S_HSLoopConfig.WgCfg.SinCfg.SinAmplitudeWord = (uint32_t)(c_DataStorageLocal_->get_AcAmplitude() / 800.0f*2047 + 0.5f);
    S_HSLoopConfig.WgCfg.SinCfg.SinPhaseWord = 0;

    AD5940_HSLoopCfgS(&S_HSLoopConfig);

    /*************************************************************************/
    // Low power amplifier config
    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355

     if (c_DataStorageLocal_->get_DcOffset() != 0.0){
        
        LPDACCfg_Type S_LPDAC_config;

        S_LPDAC_config.LpdacSel = LPDAC0;
        S_LPDAC_config.LpDacVbiasMux = LPDACVBIAS_12BIT; /* Use Vbias to tuning BiasVolt. */
        S_LPDAC_config.LpDacVzeroMux = LPDACVZERO_6BIT;  /* Vbias-Vzero = BiasVolt */
        S_LPDAC_config.DacData6Bit = 0x40>>1;            /* Set Vzero to middle scale. */
        if(c_DataStorageLocal_->get_DcOffset() < -1100.0f){
         c_DataStorageLocal_->set_DcOffset( -1100.0f + (2200.0f / 4095));
         }
        if(c_DataStorageLocal_->get_DcOffset() > 1100.0f){
             c_DataStorageLocal_->set_DcOffset(1100.0f - (2200.0f / 4095));
             }
        S_LPDAC_config.DacData12Bit = (uint16_t)((c_DataStorageLocal_->get_DcOffset() + 1100.0f)/(2200.0f / 4095));
        S_LPDAC_config.DataRst = bFALSE;      /* Do not reset data register */
        S_LPDAC_config.LpDacSW = LPDACSW_VBIAS2LPPA|LPDACSW_VBIAS2PIN|LPDACSW_VZERO2LPTIA|LPDACSW_VZERO2PIN|LPDACSW_VZERO2HSTIA;
        S_LPDAC_config.LpDacRef = LPDACREF_2P5;
        S_LPDAC_config.LpDacSrc = LPDACSRC_MMR;      /* Use MMR data, we use LPDAC to generate bias voltage for LPTIA - the Vzero */
        S_LPDAC_config.PowerEn = bTRUE;              /* Power up LPDAC */
        AD5940_LPDACCfgS(&S_LPDAC_config);
    }

    /*************************************************************************/
    // Configure digital signal processor (DSP)
    // Set all members of the structure to 0
    // TODO: check filter implementation 
    AD5940_StructInit(&S_DSPConfig, sizeof(S_DSPConfig));

    // Set negative input channel of ADC to LPTIA0 negative input mode
    S_DSPConfig.ADCBaseCfg.ADCMuxN = ADCMUXN_HSTIA_N;

    // Set positive input channel of ADC to LPTIA0 positive input mode
    S_DSPConfig.ADCBaseCfg.ADCMuxP = ADCMUXP_HSTIA_P;

    c_DataStorageLocal_->set_AdcPgaGain(ADCPGA_1);
    // Get gain of programmable gain amplifier
    S_DSPConfig.ADCBaseCfg.ADCPga = c_DataStorageLocal_->get_AdcPgaGain();

    // Select sampling rate according to ADC clock
    // Clock = 16 Mhz -> 800 kHz sampling
    // Clock = 32 Mhz -> 1.6 MHz sampling
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

    
    S_DSPConfig.DftCfg.DftNum = DFTNUM_16384;
    S_DSPConfig.DftCfg.DftSrc = DFTSRC_SINC3;
    S_DSPConfig.DftCfg.HanWinEn = bTRUE;

    // Config digital signal processor (DSP)
    AD5940_DSPCfgS(&S_DSPConfig);


   /* Enable all of them. They are automatically turned off during hibernate mode to save power */
  if(c_DataStorageLocal_->get_DcOffset() == 0.0f)
    AD5940_AFECtrlS(AFECTRL_HSTIAPWR|AFECTRL_INAMPPWR|AFECTRL_EXTBUFPWR|\
                AFECTRL_WG|AFECTRL_DACREFPWR|AFECTRL_HSDACPWR|\
                AFECTRL_SINC2NOTCH, bTRUE);
  else
    AD5940_AFECtrlS(AFECTRL_HSTIAPWR|AFECTRL_INAMPPWR|AFECTRL_EXTBUFPWR|\
                AFECTRL_WG|AFECTRL_DACREFPWR|AFECTRL_HSDACPWR|\
                AFECTRL_SINC2NOTCH|AFECTRL_DCBUFPWR, bTRUE);

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
            
        // Set sequence ID to 1
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
int C_Setup_EIS::funSequencerExecuteSequence(){
    // Initialize variables
    const uint32_t *uiSequenceCommand;

    uint32_t uiCurrAddr = 0;
    uint32_t uiRegData = 0;
    uint32_t uiSequenceLength = 0;
    uint32_t WaitClks;

    SEQInfo_Type S_SequenceInfo;
    SWMatrixCfg_Type S_SWMatrix_Config;
    ClksCalInfo_Type S_Clk_Info;

    S_Clk_Info.DataType = DATATYPE_DFT;
    S_Clk_Info.DftSrc = DFTSRC_SINC3;
    S_Clk_Info.DataCount = 1L<<(DFTNUM_16384+2); /* 2^(DFTNUMBER+2) */
    S_Clk_Info.ADCSinc2Osr = c_DataStorageLocal_->get_AdcOsrSinc2();
    S_Clk_Info.ADCSinc3Osr = c_DataStorageLocal_->get_AdcOsrSinc2();
    S_Clk_Info.ADCAvgNum = ADCAVGNUM_16;
    S_Clk_Info.RatioSys2AdcClk = 1;
    AD5940_ClksCalculate(&S_Clk_Info, &WaitClks);


    AD5940_SEQGenCtrl(bTRUE);
    AD5940_SEQGpioCtrlS(AGPIO_Pin2); /* Set GPIO1, clear others that under control */
    AD5940_SEQGenInsert(SEQ_WAIT(16*250));  /* @todo wait 250us? */
    S_SWMatrix_Config.Dswitch = SWD_RCAL0;
    S_SWMatrix_Config.Pswitch = SWP_RCAL0;
    S_SWMatrix_Config.Nswitch = SWN_RCAL1;
    S_SWMatrix_Config.Tswitch = SWT_RCAL1|SWT_TRTIA;
    AD5940_SWMatrixCfgS(&S_SWMatrix_Config);

    AD5940_AFECtrlS(AFECTRL_HSTIAPWR|AFECTRL_INAMPPWR|AFECTRL_EXTBUFPWR|\
                AFECTRL_WG|AFECTRL_DACREFPWR|AFECTRL_HSDACPWR|\
                AFECTRL_SINC2NOTCH, bTRUE);
    AD5940_AFECtrlS(AFECTRL_WG|AFECTRL_ADCPWR, bTRUE);  /* Enable Waveform generator */
    //delay for signal settling DFT_WAIT
    AD5940_SEQGenInsert(SEQ_WAIT(16*10));
    AD5940_AFECtrlS(AFECTRL_ADCCNV|AFECTRL_DFT, bTRUE);  /* Start ADC convert and DFT */
    AD5940_SEQGenInsert(SEQ_WAIT(WaitClks));
    //wait for first data ready
    AD5940_AFECtrlS(AFECTRL_ADCPWR|AFECTRL_ADCCNV|AFECTRL_DFT|AFECTRL_WG, bFALSE);  /* Stop ADC convert and DFT */

    
    /* Configure matrix for external Rz */
    S_SWMatrix_Config.Dswitch = SWD_CE0;
    S_SWMatrix_Config.Pswitch = SWP_CE0;
    S_SWMatrix_Config.Nswitch = SWN_AIN1;
    S_SWMatrix_Config.Tswitch = SWT_TRTIA|SWT_AIN1;
    AD5940_SWMatrixCfgS(&S_SWMatrix_Config);
    AD5940_AFECtrlS(AFECTRL_ADCPWR|AFECTRL_WG, bTRUE);  /* Enable Waveform generator */
    AD5940_SEQGenInsert(SEQ_WAIT(16*10));  //delay for signal settling DFT_WAIT
    AD5940_AFECtrlS(AFECTRL_ADCCNV|AFECTRL_DFT, bTRUE);  /* Start ADC convert and DFT */
    AD5940_SEQGenInsert(SEQ_WAIT(WaitClks));  /* wait for first data ready */
    AD5940_AFECtrlS(AFECTRL_ADCCNV|AFECTRL_DFT|AFECTRL_WG|AFECTRL_ADCPWR, bFALSE);  /* Stop ADC convert and DFT */
    AD5940_AFECtrlS(AFECTRL_HSTIAPWR|AFECTRL_INAMPPWR|AFECTRL_EXTBUFPWR|\
            AFECTRL_WG|AFECTRL_DACREFPWR|AFECTRL_HSDACPWR|\
            AFECTRL_SINC2NOTCH, bFALSE);
    AD5940_SEQGpioCtrlS(0); /* Clr GPIO1 */

    AD5940_EnterSleepS();/* Goto hibernate */

    /* Sequence end. */
    AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSequenceLength);
    AD5940_SEQGenCtrl(bFALSE); /* Stop sequencer generator */

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
    /* Write command to SRAM */




    return EC_NO_ERROR;
}

#endif /* setup_EIS_CPP */
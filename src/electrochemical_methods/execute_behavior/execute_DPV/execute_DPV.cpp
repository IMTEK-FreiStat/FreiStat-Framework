/******************************************************************************
 * @brief: Source file containing the subclass (C_Execute) C_Execute_DPV which 
 * defines the behavior of executing an differential pulse voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_DPV_CPP
#define execute_DPV_CPP

// Include dependencies
#include "execute_DPV.h"

/******************************************************************************
 * @brief Constructor of the class C_Execute_DPV
 * 
 *****************************************************************************/ 
C_Execute_DPV::C_Execute_DPV(){}

/******************************************************************************
 * @brief Starting method for the class C_Execute_DPV
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Execute_DPV::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Initialize variables
    bEosInterruptOccured_ = false;

    iStepCounter_ = 0;  

    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Get stored experiment type
    strncpy(chrExperimentType_, c_DataStorageLocal_->get_ExperimentType(), 
        sizeof(chrExperimentType_));

    // Convert char array with experiment type to integer
    int iExperimentType = funGetExperimentTypeInt(chrExperimentType_);

    // Preconfig AFE reference buffer
    this->funConfigAfeReferenceBuffer();

    // Get reference to serial communication object
    C_Communication * c_Communication = c_DataSoftwareStorage_->
        get_Communication();

    // Prepare telegram strucutre
    c_Communication->funConstructPrefixes(chrExperimentType_);

    // Set system status to experiment running
    c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_RUNNING);
    
    // Control the application
    this->funControlApplication(FREISTAT_START_TIMER);    

    // Loop while experiment is running
    while (c_DataSoftwareStorage_->get_SystemStatus() == FREISTAT_EXP_RUNNING){
        // Check if interrupt has occured
        if (c_DataSoftwareStorage_->get_AD5940Setup()->get_InterruptOccured()){
            // Clear interrupt flag
            c_DataSoftwareStorage_->get_AD5940Setup()->
                set_InterruptOccured(false);

            // Call interrupt service routine
            this->funInterruptServiceRoutine();
            
            // Get send data counter
            int iSendDataCounter = c_DataStorageGeneral_->get_SendDataCounter();

            // If data needs to be send, write to serial port
            if (iSendDataCounter + 1 < iStepCounter_){     
                S_DataContainer S_ExperimentData = c_DataStorageGeneral_->
                get_ExperimentData(this->funGetDataPosition(iSendDataCounter));

                // Write data to serial port       
                c_Communication->funSendExperimentData(
                    S_ExperimentData, iExperimentType);

                // Increase send data counter
                c_DataStorageGeneral_->set_SendDataCounter(iSendDataCounter + 1);
            }
        }
        // Check if experiment is completed
        // Check if end of sequence interrupt occured
        if (bEosInterruptOccured_ == true){
            // Get send data counter
            int iSendDataCounter = c_DataStorageGeneral_->get_SendDataCounter();

            // Check if step counter is equal to maximal amount of values and
            // send data counter is equal to step counter
            S_DataContainer S_ExperimentData = c_DataStorageGeneral_->
            get_ExperimentData(this->funGetDataPosition(iSendDataCounter));

            // Increase send data counter
            c_DataStorageGeneral_->set_SendDataCounter(iSendDataCounter + 1);

            if (iSendDataCounter == iStepCounter_){
                // Set system status to waiting state
                c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_COMPLETED);
            }
        }

        // Check if telegram was received
        if (c_Communication->funDataAvailable()){
            // Interpret send data
            c_DataSoftwareStorage_->get_JSONParser()->funParseSerialData(
                                    c_Communication->funReadSerial());
        }
    }
    // Control the application
    this->funControlApplication(FREISTAT_STOP_TIMER);   

    // Reset variables
    c_DataStorageGeneral_->set_SendDataCounter(0);
    c_DataStorageLocal_->set_CurrentStepNumber(0);
    c_DataStorageLocal_->set_StepNumber(0);
}

/******************************************************************************
 * @brief Method for implementing interrupt service routine for chrono-
 * amperometry
 * 
 * @return: Returns error code                                        
 *****************************************************************************/
int C_Execute_DPV::funInterruptServiceRoutine(){
    // Initalizing variables
    int iErrorCode = 0;

    uint32_t uiFiFoCount = 0;
    uint32_t uiInterruptFlag = 0;

    // Wake up AFE by reading register, try at most 10 times
    if (AD5940_WakeUp(10) > 10){  
        return EC_EXECUTE + EC_EX_WAKEUP_AFE_ERR;
    }
    
    // Lock SEQTRGSLP register to disable sleep mode of the AD5940
    AD5940_SleepKeyCtrlS(SLPKEY_LOCK);

    // Read interrupt flag from interrupt controller 0
    uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);

    // Loop until no interrupts are there which need to be handled
    // Reason for looping is that interrupts could occure while an interrupt is
    // still handled
    while (uiInterruptFlag != 0){
        // Custom interrupt 1
        if (uiInterruptFlag & AFEINTSRC_CUSTOMINT1){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_CUSTOMINT1);

            // Read amount of data which is currently stored in FIFO
            uiFiFoCount = AD5940_FIFOGetCnt();

            // Read data from FIFO and store in temporary buffer
            AD5940_FIFORd(c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);

            // Call the function to process data
            this->funProcessExperimentData(
                c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);

            // Create next sequence
            this->funUpdateSequence();           
            
            // Turn off ADC reference voltage to ensure unified starting
            // Conditions for every cycle 
            // Disable high power 1.8 V reference buffer
            S_AFEReferenceBufferConfig_.Hp1V8BuffEn = bFALSE;

            // Configure AFE reference buffer
            AD5940_REFCfgS(&S_AFEReferenceBufferConfig_);

            // Turn on ADC referenec voltage
            S_AFEReferenceBufferConfig_.Hp1V8BuffEn = bTRUE;

            // Configure AFE reference buffer
            AD5940_REFCfgS(&S_AFEReferenceBufferConfig_);
        }

        // FIFO threshold interrupt
        if (uiInterruptFlag & AFEINTSRC_DATAFIFOTHRESH){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_DATAFIFOTHRESH);

            // Read amount of data which is currently stored in FIFO
            uiFiFoCount = AD5940_FIFOGetCnt();

            // Read data from FIFO and store in temporary buffer
            AD5940_FIFORd(c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);
            
            // Call the function to process data
            this->funProcessExperimentData(
                c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);

        }
        // General purpose timer 1 interrupt
        if (uiInterruptFlag & AFEINTSRC_GPT1INT_TRYBRK){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_GPT1INT_TRYBRK);

            // Turn on LED on AD5940 board
            AD5940_AGPIOClr(AGPIO_Pin1);
        }
        // FIFO overflow interrupt
        if (uiInterruptFlag & AFEINTSRC_CMDFIFOOF)
        {
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_CMDFIFOOF);

            // Turn on LED on AD5940 board
            AD5940_AGPIOClr(AGPIO_Pin1);
        }
         // End of sequence interrupt
        if (uiInterruptFlag & AFEINTSRC_ENDSEQ){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_ENDSEQ);

            // Control the application
            this->funControlApplication(FREISTAT_STOP_TIMER);    

            //disable AFE
            AD5940_ShutDownS();

            // Set interrupt flag
            bEosInterruptOccured_ = true;
        }     
        // Update variable
        uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for processing data which was read from AD5940
 * 
 * @return: Returns error code
 *****************************************************************************/
int C_Execute_DPV::funProcessExperimentData(uint32_t * pData, 
                                           uint32_t uiCountData){
    // Intialize variables
    float fVoltage = 0;
        
    uint32_t iCountSamples = 0;
    uint64_t iSumSamples = 0;

    S_DataContainer S_ExperimentData;

    // Get ADC gain
    int iAdcPgaGain = c_DataStorageLocal_->get_AdcPgaGain();

    // Get reference voltage
    float fAdcReferenceVoltage = c_DataStorageGeneral_->get_ADCReferenceVoltage();

    // Get size of Rtia resistor
    float fRtiaMagnitude = c_DataStorageGeneral_->get_RtiaValue().Magnitude;

    // Get current step number
    int iCurrentStep = c_DataStorageLocal_->get_CurrentStepNumber();

    // Get experiment data at specific position
    S_ExperimentData = c_DataStorageGeneral_->get_ExperimentData(
                            this->funGetDataPosition(iCurrentStep - 1));

    // Loop to postprocess transmitted data
    for (int iData = 0; iData < uiCountData; iData++){
        // Sum up data 
        iSumSamples += pData[iData] & 0xffff;

        // Increase sample counter
        iCountSamples ++;
    }

    // Check if samples were collected
    if (iCountSamples > 0){
        // Turn it into a voltage (unit of ADCReferenceVoltage -> mV)
        fVoltage = AD5940_ADCCode2Volt((uint32_t)((float)iSumSamples / 
                                       iCountSamples + 0.5), iAdcPgaGain, 
                                       fAdcReferenceVoltage);

        S_DataContainer S_ExperimentData2 = 
            c_DataStorageGeneral_->get_ExperimentData(
            this->funGetDataPosition(iStepCounter_ - 1));

        // Calculate current in uA
        S_ExperimentData2.fCurrent = 1000.0f * fVoltage / fRtiaMagnitude;

        c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData2, 
                                                  this->funGetDataPosition(
                                                  iStepCounter_ - 1));        

        // Store cycle number
        S_ExperimentData.iCycle = 1 + c_DataStorageLocal_->get_StepNumber();       

        // Data point number
        S_ExperimentData.iMeasurmentPair = 1 + iStepCounter_;

        // Save time stamp
        S_ExperimentData.fTimeStamp = millis();

        // Save data
        c_DataStorageGeneral_->set_ExperimentData(
            S_ExperimentData, this->funGetDataPosition(iCurrentStep - 1)); 

        // Increase step counter
        iStepCounter_++;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for starting and stoping the CA sequence and the configuration
 * of the Wake-up timer which is used to time the sequence of different 
 * sequences of the chronoamperometry
 * 
 * @param uiCommand: Integer coded command to start and stop the CA
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Execute_DPV::funControlApplication(uint32_t uiCommand){
    // Initialize variables
    WUPTCfg_Type S_WakeUpTimer_Config;

    // Try to wake up AFE by reading at most 10 times
    if (AD5940_WakeUp(10) > 10)
        return EC_EXECUTE + EC_EX_WAKEUP_AFE_ERR;

    // Operate depending on the command
    switch (uiCommand){
        case FREISTAT_START_TIMER:{
            // Enable wake-up timer
            S_WakeUpTimer_Config.WuptEn = bTRUE;

            // Specifiy how many sequences are used (A = 1 | B = 2 | ...)
            S_WakeUpTimer_Config.WuptEndSeq = WUPTENDSEQ_B;

            // Define order and type of sequences
            S_WakeUpTimer_Config.WuptOrder[0] = SEQID_1;
            S_WakeUpTimer_Config.WuptOrder[1] = SEQID_2;

            // Define how long a sequence should run
            // = LFOSCFrequency (in Hz) * Time (in seconds)
            S_WakeUpTimer_Config.SeqxSleepTime[SEQID_1] = 1;
            S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_1] = 
                    (uint32_t)(c_DataStorageGeneral_->get_LFOSCFrequency() * 
                    (c_DataStorageLocal_->get_PulseDurations(0)) / 1000.0);
            S_WakeUpTimer_Config.SeqxSleepTime[SEQID_2] = 1;
            S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_2] = 
                    (uint32_t)(c_DataStorageGeneral_->get_LFOSCFrequency() * 
                    (c_DataStorageLocal_->get_PulseDurations(1)) / 1000.0);

            // Config wake-up timer
            AD5940_WUPTCfg(&S_WakeUpTimer_Config);        
            break;
        }
        case FREISTAT_STOP_TIMER:{
            // Stop wake up timer
            AD5940_WUPTCtrl(bFALSE);
            break;
        }
        default:
            break;
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for updating the measurement sequence
 * 
 * @returns: Error code encoded as integer
 *****************************************************************************/
int C_Execute_DPV::funUpdateSequence(){
    // Initialize variables
    bool bSeqBlockUsed = c_DataStorageLocal_->get_SeqBlockUsed();

    int iDacCurrentBlock = c_DataStorageLocal_->get_DacCurrentBlock();
    int iDacSeqBlock0Address = c_DataStorageLocal_->get_DacSeqBlock0Address();
    int iDacSeqBlock1Address = c_DataStorageLocal_->get_DacSeqBlock1Address();
    int iSRAMAddress;

    const uint32_t *uiSequenceCommand;

    uint32_t uiCurrAddr = 0;
    uint32_t uiVbiasCode = 0;
    uint32_t uiVzeroCode = 0;
    uint32_t uiSequenceLength = 0;

    uint32_t iCommandBuffer[AD5940_BUFFER_DPV];

    S_DataContainer S_ExperimentData; 

    // Assign address of block 0 or block 1 to current block address
    uiCurrAddr = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                    iDacSeqBlock0Address : iDacSeqBlock1Address;

    // Jump to next block
    iSRAMAddress = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                    iDacSeqBlock1Address : iDacSeqBlock0Address;

    if (iDacCurrentBlock == CURRENT_BLOCK_1){
        // Check if cycle is done
        if (c_DataStorageLocal_->get_StepsRemaining() <= 0 &&
            c_DataStorageLocal_->get_StepNumber() < 
            c_DataStorageLocal_->get_Cycle()){
            // Calculate amount of steps in cycle and round up
            int iStepsRemaining = (c_DataStorageLocal_->get_UpperVoltage() -
                                   c_DataStorageLocal_->get_StartVoltage()) / 
                                   c_DataStorageLocal_->get_PotentialSteps(0) + 
                                   1.5;
            
            // Save amount of steps
            c_DataStorageLocal_->set_StepsRemaining(iStepsRemaining);

            // Update step number
            c_DataStorageLocal_->set_StepNumber(c_DataStorageLocal_->
                get_StepNumber() + 1);
        }

        // Update remaining amount of steps
        c_DataStorageLocal_->set_StepsRemaining(c_DataStorageLocal_->
            get_StepsRemaining() - 1);

        // Save voltage of staircase
        c_DataStorageLocal_->set_LowerVoltage(c_DataStorageLocal_->
            get_LowerVoltage() + c_DataStorageLocal_->get_PotentialSteps(0));
    }

    // Calculate DAC code
    uiVzeroCode = (c_DataStorageLocal_->get_WePotentialHigh() - 
        AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB;

    if (iDacCurrentBlock == CURRENT_BLOCK_1){
        uiVbiasCode = (uiVzeroCode * 64 - ((c_DataStorageLocal_->
            get_LowerVoltage() + c_DataStorageLocal_->get_PotentialSteps(1)) / 
            AD5940_12BIT_DAC_1LSB));
    }
    else if (iDacCurrentBlock == CURRENT_BLOCK_0){
        if (c_DataStorageLocal_->get_StepsRemaining() <= 0){
            // Update start voltage
            c_DataStorageLocal_->set_LowerVoltage(c_DataStorageLocal_->get_StartVoltage());
        }
        uiVbiasCode = (uiVzeroCode * 64 - c_DataStorageLocal_->get_LowerVoltage() / 
            AD5940_12BIT_DAC_1LSB);
    }

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

    // Store the voltage value 
    S_ExperimentData.fVoltage = uiVzeroCode * AD5940_6BIT_DAC_1LSB - 
                                uiVbiasCode * AD5940_12BIT_DAC_1LSB;

    // Update current step
    c_DataStorageLocal_->set_CurrentStepNumber(c_DataStorageLocal_->
        get_CurrentStepNumber() + 1);

    // Store experiment data at newest free position
    c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData, 
        this->funGetDataPosition(c_DataStorageLocal_->get_CurrentStepNumber()));

    // Generate stop sequence if experiment is done
    if (c_DataStorageLocal_->get_StepNumber() >= c_DataStorageLocal_->get_Cycle()){
        // Read the current values of the AFE Control register
        uint32_t AfeControlRegister = AD5940_ReadReg(REG_AFE_AFECON);

        // Enbale ADC conversion bit
        AfeControlRegister &= ~AFECTRL_ADCCNV;
        iCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister); 
        iCommandBuffer[1] = SEQ_NOP();
        iCommandBuffer[2] = SEQ_NOP();
        iCommandBuffer[3] = SEQ_NOP();
        iCommandBuffer[4] = SEQ_NOP();
        iCommandBuffer[5] = SEQ_NOP();
        iCommandBuffer[6] = SEQ_STOP();

        // Write command to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, iCommandBuffer, AD5940_BUFFER_DPV);
    }
    else {
        // Read the current values of the AFE Control register
        uint32_t AfeControlRegister = AD5940_ReadReg(REG_AFE_AFECON);

        // Enbale ADC conversion bit
        AfeControlRegister |= AFECTRL_ADCCNV;
        iCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister); 
        iCommandBuffer[1] = SEQ_WAIT(16 * 1000 * c_DataStorageLocal_->get_Scanrate());

        // Clear ADC conversion bit
        AfeControlRegister &= ~AFECTRL_ADCCNV;
        iCommandBuffer[2] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister); 
        iCommandBuffer[3] = SEQ_INT1();

        iCommandBuffer[4] = SEQ_WR(REG_AFE_LPDACDAT0, uiVzeroCode << 12 | 
                            uiVbiasCode);
        iCommandBuffer[5] = SEQ_WAIT(10);
        iCommandBuffer[6] = SEQ_WR(bSeqBlockUsed ? REG_AFE_SEQ1INFO : 
                            REG_AFE_SEQ2INFO, ( iSRAMAddress << 
                            BITP_AFE_SEQ1INFO_ADDR) | (AD5940_BUFFER_DPV << 
                            BITP_AFE_SEQ1INFO_LEN));

        // Write command to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, iCommandBuffer, AD5940_BUFFER_DPV);
    }
    
    // Switch between block 0 and block 1
    iDacCurrentBlock = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                        CURRENT_BLOCK_1 : CURRENT_BLOCK_0; 
                        
    c_DataStorageLocal_->set_DacCurrentBlock(iDacCurrentBlock);

    // Switch from block 0 -> 1 or 1 -> 0
    bSeqBlockUsed = bSeqBlockUsed ? false : true;
    c_DataStorageLocal_->set_SeqBlockUsed(bSeqBlockUsed);

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Helper method for the configuration of the AFE reference buffer
 * 
 * @returns: Error code encoded as integer
 *****************************************************************************/
int C_Execute_DPV::funConfigAfeReferenceBuffer(){
    // Disable high power band-gap
    S_AFEReferenceBufferConfig_.HpBandgapEn = bTRUE;

    // Enable high power 1.1 V reference buffer
    S_AFEReferenceBufferConfig_.Hp1V1BuffEn = bTRUE;

    // Disable discharge of 1.1 V capacitor
    S_AFEReferenceBufferConfig_.Disc1V1Cap = bFALSE;

    // Disable discharge of 1.8 V cpacitor
    S_AFEReferenceBufferConfig_.Disc1V8Cap = bFALSE;
            
    // Disable thermal buffer
    S_AFEReferenceBufferConfig_.Hp1V8ThemBuff = bFALSE;

    // Disable current limit for 1.8 V buffer
    S_AFEReferenceBufferConfig_.Hp1V8Ilimit = bFALSE;

    // Disable 1.1 V reference buffer
    S_AFEReferenceBufferConfig_.Lp1V1BuffEn = bTRUE;

    // Disable 1.8 V reference buffer
    S_AFEReferenceBufferConfig_.Lp1V8BuffEn = bTRUE;
            
    // Enable low power band gap
    S_AFEReferenceBufferConfig_.LpBandgapEn = bTRUE;

    // Enable 2.5 V reference buffer
    S_AFEReferenceBufferConfig_.LpRefBufEn = bTRUE;

    // Disable boost buffer current
    S_AFEReferenceBufferConfig_.LpRefBoostEn = bFALSE;

    return EC_NO_ERROR;
}
#endif /* execute_DPV_CPP */
/******************************************************************************
 * @brief: Source file containing the subclass (C_Execute) C_Execute_CA which 
 * defines the behavior of executing an chronoamperometry
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_CA_CPP
#define execute_CA_CPP

// Include dependencies
#include "execute_CA.h"

/******************************************************************************
 * @brief Constructor of the class C_Execute_CA
 * 
 *****************************************************************************/ 
C_Execute_CA::C_Execute_CA(){}

/******************************************************************************
 * @brief Starting method for the class C_Execute_CA
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Execute_CA::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
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

    // Get reference to serial communication object
    C_Communication * c_Communication = c_DataSoftwareStorage_->
        get_Communication();

    // Prepare telegram strucutre
    c_Communication->funConstructPrefixes(chrExperimentType_);

    // Set system status to experiment running
    c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_RUNNING);

    // Control the application
    this->funControlApplication(FREISTAT_START_TIMER);    

    // Let system settle in 
    delay(20);   

    // Loop while experiment is running
    while (c_DataSoftwareStorage_->get_SystemStatus() == FREISTAT_EXP_RUNNING){
        // Check if interrupt has occured
        if (c_DataSoftwareStorage_->get_AD5940Setup()->get_InterruptOccured()){
            // Clear interrupt flag
            c_DataSoftwareStorage_->get_AD5940Setup()->
                set_InterruptOccured(false);

            // Call interrupt service routine
            this->funInterruptServiceRoutine();
        }
            
        // Get send data counter
        int iSendDataCounter = c_DataStorageGeneral_->get_SendDataCounter();

        // If data needs to be send, write to serial port
        if (iSendDataCounter + 2 < iStepCounter_){     
            S_DataContainer S_ExperimentData = c_DataStorageGeneral_->
            get_ExperimentData(this->funGetDataPosition(iSendDataCounter));

            if (S_ExperimentData.iCycle <= c_DataStorageLocal_->get_Cycle()){
                // Write data to serial port        
                c_Communication->funSendExperimentData(
                    S_ExperimentData, iExperimentType);  
            }

            // Increase send data counter
            c_DataStorageGeneral_->set_SendDataCounter(iSendDataCounter + 1);
        }
        
        // Check if experiment is completed
        // Check if end of sequence interrupt occured
        if (bEosInterruptOccured_ == true){
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
int C_Execute_CA::funInterruptServiceRoutine(){
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
    while (uiInterruptFlag != 0)
    {
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

            // Create next sequence
            this->funUpdateSequence();
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
int C_Execute_CA::funProcessExperimentData(uint32_t * pData, 
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
                    iCountSamples + 0.5), iAdcPgaGain, fAdcReferenceVoltage);

        S_DataContainer S_ExperimentData2 = 
            c_DataStorageGeneral_->get_ExperimentData(
            this->funGetDataPosition(iStepCounter_ - 2));

        // Calculate current in uA
        S_ExperimentData2.fCurrent = 1000.0f * fVoltage / fRtiaMagnitude;

        c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData2, 
            this->funGetDataPosition(iStepCounter_ - 2));        

        // Store voltage
        S_ExperimentData.fVoltage = c_DataStorageLocal_->
            get_PotentialSteps(iCurrentStep);

        // Store cycle number
        S_ExperimentData.iCycle = 1 + c_DataStorageLocal_->get_StepNumber();       

        // Data point number
        S_ExperimentData.iMeasurmentPair = 1 + iStepCounter_;

        // Save time stamp
        S_ExperimentData.fTimeStamp = millis();

        // Save data
        c_DataStorageGeneral_->set_ExperimentData(
            S_ExperimentData, this->funGetDataPosition(iStepCounter_)); 

        // Increase step counter
        iStepCounter_++;

        // Reset variables
        iSumSamples = 0;
        iCountSamples = 0;
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
int C_Execute_CA::funControlApplication(uint32_t uiCommand){
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
                    c_DataStorageLocal_->get_Scanrate() / 1000.0);
            S_WakeUpTimer_Config.SeqxSleepTime[SEQID_2] = 
                    S_WakeUpTimer_Config.SeqxSleepTime[SEQID_1];
            S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_2] = 
                    S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_1];

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
int C_Execute_CA::funUpdateSequence(){
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

    uint32_t iCommandBuffer[AD5940_BUFFER_CA];

    // Assign address of block 0 or block 1 to current block address
    uiCurrAddr = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                    iDacSeqBlock0Address : iDacSeqBlock1Address;

    // Jump to next block
    iSRAMAddress = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                    iDacSeqBlock1Address : iDacSeqBlock0Address;


    // Calculate values for one sampling step of chronoamperometry
    // Get current step number
    int iCurrentStep = c_DataStorageLocal_->get_CurrentStepNumber();

    // Check if current step has to be changed
    if (c_DataStorageLocal_->get_StepsRemaining() <= 0){
        // Check if new cycle begins
        if (iCurrentStep + 1 >= c_DataStorageLocal_->get_BufferEntries()){
            // Increment step number
            c_DataStorageLocal_->set_StepNumber(
                c_DataStorageLocal_->get_StepNumber() + 1);

            // Update current step
            c_DataStorageLocal_->set_CurrentStepNumber(0);

            // Set remaining pulse length
            c_DataStorageLocal_->set_StepsRemaining(
                c_DataStorageLocal_->get_PulseDurations(0));                
        }
        else {
            // Update current step
            c_DataStorageLocal_->set_CurrentStepNumber(iCurrentStep + 1);

            // Set remaining pulse length
            c_DataStorageLocal_->set_StepsRemaining(
                c_DataStorageLocal_->get_PulseDurations(iCurrentStep + 1));
        }

        // Get updated current step number
        iCurrentStep = c_DataStorageLocal_->get_CurrentStepNumber();

    }

    // Get sampling rate
    float fSamplingRate = c_DataStorageLocal_->get_Scanrate();

    // Calculate DAC code
    uiVzeroCode = (c_DataStorageLocal_->get_WePotentialHigh() - 
                   AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB;
    uiVbiasCode = (uiVzeroCode * 64 - 
                   (c_DataStorageLocal_->get_PotentialSteps(iCurrentStep) / 
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

    // Update remaining pulse length
    c_DataStorageLocal_->set_StepsRemaining(
        c_DataStorageLocal_->get_StepsRemaining() - fSamplingRate);

    // Generate stop sequence if experiment is done
    if (c_DataStorageLocal_->get_StepNumber() > c_DataStorageLocal_->get_Cycle()){
        // Enable sequencer
        AD5940_SEQGenCtrl(bTRUE);

        // Stop ADC
        AD5940_AFECtrlS(AFECTRL_ADCCNV, bFALSE);

        // Insert stop command
        AD5940_SEQGenInsert(SEQ_STOP());

        // Generate sequence
        AD5940_SEQGenFetchSeq(&uiSequenceCommand, &uiSequenceLength);

        // Disable sequence generator
        AD5940_SEQGenCtrl(bFALSE);

        // Write command to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, uiSequenceCommand, uiSequenceLength);
    }
    else {
        // Read the current values of the AFE Control register
        uint32_t AfeControlRegister = AD5940_ReadReg(REG_AFE_AFECON);
        // Clear ADC conversion bit
        AfeControlRegister &= ~AFECTRL_ADCCNV;
        iCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister); 
        iCommandBuffer[1] = SEQ_INT1();

        // Enbale ADC conversion bit
        AfeControlRegister |= AFECTRL_ADCCNV;
        iCommandBuffer[2] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister); 

        iCommandBuffer[3] = SEQ_WR(REG_AFE_LPDACDAT0, uiVzeroCode << 12 | uiVbiasCode);
        iCommandBuffer[4] = SEQ_WAIT(10);
        iCommandBuffer[5] = SEQ_WR(bSeqBlockUsed ? REG_AFE_SEQ1INFO : 
                            REG_AFE_SEQ2INFO, (iSRAMAddress << 
                            BITP_AFE_SEQ1INFO_ADDR) | (AD5940_BUFFER_CA << 
                            BITP_AFE_SEQ1INFO_LEN));

        // Write command to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, iCommandBuffer, AD5940_BUFFER_CA);
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

#endif /* execute_CA_CPP */
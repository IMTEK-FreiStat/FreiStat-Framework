/******************************************************************************
 * @brief: Source file containing the subclass (C_Execute) C_Execute_CV which 
 * defines the behavior of executing an cyclic voltammetry
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_CV_CPP
#define execute_CV_CPP

// Include dependencies
#include "execute_CV.h"

/******************************************************************************
 * @brief Constructor of the class C_Execute_CV
 * 
 *****************************************************************************/ 
C_Execute_CV::C_Execute_CV(){}

/******************************************************************************
 * @brief Starting method for the class C_Execute_CV
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
int C_Execute_CV::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Get current experiment type
    strncpy(chrExperimentType_, c_DataStorageLocal_->get_ExperimentType(), 
        sizeof(chrExperimentType_));

    // Convert char array with experiment type to integer
    int iExperimentType = this->funGetExperimentTypeInt(chrExperimentType_);

    // Get reference to serial communication object
    C_Communication * c_Communication = c_DataSoftwareStorage_->get_Communication();

    // Prepare telegram strucutre
    c_Communication->funConstructPrefixes(chrExperimentType_);

    // Do precalucations for the CV
    this->funPreCalulations();

    // Call first DAC sequence   
    this->funSequencerDACControl();

    // Set system status to experiment running
    c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_RUNNING);

    // Control the application
    this->funControlApplication(FREISTAT_START_TIMER);  

    // Loop while experiment is running
    while (c_DataSoftwareStorage_->get_SystemStatus() == FREISTAT_EXP_RUNNING){   
        // Check if interrupt has occured
        if (c_DataSoftwareStorage_->get_AD5940Setup()->get_InterruptOccured()){
            // Clear interrupt flag
            c_DataSoftwareStorage_->get_AD5940Setup()->set_InterruptOccured(false);
            
            // Call interrupt service routine
            this->funInterruptServiceRoutine();

            // Get send data counter
            int iSendDataCounter = c_DataStorageGeneral_->get_SendDataCounter();

            // If data needs to be send, write to serial port
            if (iSendDataCounter < c_DataStorageLocal_->
                get_CurrentStepNumber() - 1){   

                S_DataContainer S_ExperimentData = c_DataStorageGeneral_->
                get_ExperimentData(this->funGetDataPosition(iSendDataCounter));

                if (S_ExperimentData.iCycle != 0){
                    // Write data to serial port                                               
                    c_Communication->funSendExperimentData(
                        S_ExperimentData, iExperimentType);
                }

                // Increase send data counter
                c_DataStorageGeneral_->set_SendDataCounter(iSendDataCounter + 1);
            }
        }
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

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for implementing interrupt service routine for cyclic 
 * voltammetry
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Execute_CV::funInterruptServiceRoutine(){
    // Initalize variables
    uint32_t uiInterruptFlag = 0;
    uint32_t uiFiFoCount = 0;

    // Read interrupt flag from interrupt controller 0
    uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);

    // Loop until no interrupts are there which need to be handled
    // Reason for looping is that interrupts could occure while an interrupt is
    // still handled
    while (uiInterruptFlag != 0){
        // Custom interrupt 1 occured (New voltage step)
        if (uiInterruptFlag & AFEINTSRC_CUSTOMINT1){
            // Clear flag for custom interrupt 1
            AD5940_INTCClrFlag(AFEINTSRC_CUSTOMINT1);

            // Read amount of data which is currently stored in FIFO
            uiFiFoCount = AD5940_FIFOGetCnt();

            // Read data from FIFO and store in temporary buffer
            AD5940_FIFORd(c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);

            // Call function to process data 
            this->funProcessExperimentData(
                c_DataStorageGeneral_->get_SampleBuffer(), uiFiFoCount);     
            
            // Cals DAC sequence to make ne voltage step
            this->funSequencerDACControl();
        }
        // FIFO threshold interrupt
        if (uiInterruptFlag & AFEINTSRC_DATAFIFOTHRESH){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_DATAFIFOTHRESH);

            // Read amount of data which is currently stored in FIFO
            uiFiFoCount = AD5940_FIFOGetCnt();

            // Read data from FIFO and store in temporary buffer
            AD5940_FIFORd(c_DataStorageGeneral_->
                get_SampleBuffer(), uiFiFoCount);

            // Call the function to process data
            this->funProcessExperimentData(c_DataStorageGeneral_->
                get_SampleBuffer(), uiFiFoCount);
        }
        // General purpose timer 1 interrupt
        if (uiInterruptFlag & AFEINTSRC_GPT1INT_TRYBRK){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_GPT1INT_TRYBRK);

            // Turn on LED on AD5940 board
            AD5940_AGPIOClr(AGPIO_Pin1);
        }
        // FIFO overflow interrupt
        if (uiInterruptFlag & AFEINTSRC_CMDFIFOOF){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_CMDFIFOOF);

            // Turn on LED on AD5940 board
            AD5940_AGPIOClr(AGPIO_Pin1);
        }
        // End of sequence interrupt
        if (uiInterruptFlag & AFEINTSRC_ENDSEQ){
            // Reset interrupt flag
            AD5940_INTCClrFlag(AFEINTSRC_ENDSEQ);

            // Read amount of data which is currently stored in FIFO
            uiFiFoCount = AD5940_FIFOGetCnt();

            // Read data from FIFO and store in temporary buffer
            AD5940_FIFORd(c_DataStorageGeneral_->
                get_SampleBuffer(), uiFiFoCount);

            // Call the function to process data
            this->funProcessExperimentData(c_DataStorageGeneral_->
                get_SampleBuffer(), uiFiFoCount);

            // Stop the wake up Timer.
            this->funControlApplication(FREISTAT_STOP_TIMER); 

            // Update system and experiment status
            c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_COMPLETED);
            c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_0);

            // Turn of low power loop and set AFE to hibernate mode
            AD5940_ShutDownS();
        }

        // Update variable
        uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for processing data which was read from AD5941
 * @param pData : Buffer array which contains all recently read data which 
 *                hasn't been processed yet.
 * @param uiCountData : Size of the buffer array containing the data.
 * 
 * @return: Returns error code encoded as integer
 *****************************************************************************/
int C_Execute_CV::funProcessExperimentData(uint32_t * pData, 
                                           uint32_t uiCountData){             
    // Intialize variables
    float fVoltage = 0;

    uint64_t iSumSamples = 0;

    S_DataContainer S_ExperimentData;

    // Get current step number
    int iCurrentStep = c_DataStorageLocal_->get_CurrentStepNumber();

    // Get experiment data at specific position
    S_ExperimentData = c_DataStorageGeneral_->get_ExperimentData(
                            this->funGetDataPosition(iCurrentStep));

    // Loop to calculate sum of transmitted data
    for (int iData = 0; iData < uiCountData; iData++){
        // Sum up data 
        iSumSamples += (pData[iData] & 0xffff);
    }

    // Check if samples were collected
    if (uiCountData > 0){                                                            
        // Turn it into a voltage (unit of ADCReferenceVoltage -> mV)
        fVoltage = AD5940_ADCCode2Volt(uint32_t(float(iSumSamples / 
                    uiCountData) + 0.5), iAdcPgaGain_, fAdcReferenceVoltage_);

        S_DataContainer S_ExperimentData2 = c_DataStorageGeneral_->get_ExperimentData(
                        this->funGetDataPosition(iCurrentStep - 1));

        // Calculate current in uA
        S_ExperimentData2.fCurrent = 1000.0f * fVoltage / fRtiaMagnitude_;

        c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData2, 
            this->funGetDataPosition(iCurrentStep - 1));

        // Data point number
        S_ExperimentData.iMeasurmentPair = 1 + iCurrentStep;

        // Store cycle number
        S_ExperimentData.iCycle = 1 + iCurrentStep / 
            c_DataStorageLocal_->get_StepNumber();

        // Save time stamp
        S_ExperimentData.fTimeStamp = millis();

        // Save data
        c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData, 
            this->funGetDataPosition(iCurrentStep));

        // Increment current step number
        c_DataStorageLocal_->set_CurrentStepNumber(iCurrentStep + 1);
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for controlling the timing of the CV sequence
 * @param uiCommand: Command encoded as an integer for e.g., starting and 
 *                   stopping the CV.
 * 
 * @return: Error code encoded as integer 
 *****************************************************************************/
int C_Execute_CV::funControlApplication(uint32_t uiCommand){
    // Initialize variables
    WUPTCfg_Type S_WakeUpTimer_Config;

    // Try to wake up AFE by reading at most 10 times
    if (AD5940_WakeUp(10) > 10)
        return EC_EXECUTE + EC_EX_WAKEUP_AFE_ERR;

    // Operate depending on the command
    switch (uiCommand){
    case FREISTAT_START_TIMER:
    {
        // Enable wake-up timer
        S_WakeUpTimer_Config.WuptEn = bTRUE;

        // Specifiy how many sequences are used (A = 1 | B = 2 | ...)
        S_WakeUpTimer_Config.WuptEndSeq = WUPTENDSEQ_D;

        // Define order and type of sequences
        S_WakeUpTimer_Config.WuptOrder[0] = SEQID_0;
        S_WakeUpTimer_Config.WuptOrder[1] = SEQID_2;
        S_WakeUpTimer_Config.WuptOrder[2] = SEQID_1;
        S_WakeUpTimer_Config.WuptOrder[3] = SEQID_2;

        // Define how long a sequence should run
        // = LFOSCFrequency (in Hz) * Time (in seconds)
        // Sample delay shifts how much time the DAC and ADC should have        
        S_WakeUpTimer_Config.SeqxSleepTime[SEQID_0] = 1;
        S_WakeUpTimer_Config.SeqxSleepTime[SEQID_1] = 1;
        S_WakeUpTimer_Config.SeqxSleepTime[SEQID_2] = 1;

        S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_0] = 
            (uint32_t)(c_DataStorageGeneral_->get_LFOSCFrequency() * 
                c_DataStorageLocal_->get_Stepsize() / 
                c_DataStorageLocal_->get_Scanrate()) - 1 - 2;
        S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_1] = 
            S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_0];
        S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_2] = 1;

        // Config wake-up timer
        AD5940_WUPTCfg(&S_WakeUpTimer_Config);
        break;
    }
    case FREISTAT_STOP_TIMER:
    {
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
 * @brief Method for generating the sequencer for the control of the DAC
 * sequence
 * 
 * @return: Returns error code encoded as integer
 *****************************************************************************/
int C_Execute_CV::funSequencerDACControl(){
    // Initialize variables
    bool bFinalBlock;
    bool bSeqBlockUsed = c_DataStorageLocal_->get_SeqBlockUsed();

    int iDacCurrentBlock = c_DataStorageLocal_->get_DacCurrentBlock();
    int iSRAMAddress;
    int iStepsRemaining = c_DataStorageLocal_->get_StepsRemaining();
    int iStepsThisBlock;

    uint32_t uiCurrAddr;
    uint32_t uiDacData;

    uint32_t uiCommandBuffer[AD5940_BUFFER_CV];
    
    // Check if CV is done
    if (iStepsRemaining == 0)
        return AD5940ERR_OK;

    // Check if CV is in final block
    if (iStepsRemaining <= iStepsPerBlock_){
        // Set flag for final block
        bFinalBlock = true;

        // Set steps done this block to remaining steps
        iStepsThisBlock = iStepsRemaining;
    }
    else {
        // Reset flag for final block
        bFinalBlock = false;

        // Set steps done this block to steps per block
        iStepsThisBlock = iStepsPerBlock_;
    }

    // Update remaining steps
    iStepsRemaining -= iStepsThisBlock;
    c_DataStorageLocal_->set_StepsRemaining(iStepsRemaining);

    // Assign address of block 0 or block 1 to current block address
    if (iDacCurrentBlock == CURRENT_BLOCK_0){
        iSRAMAddress = iDacSeqBlock0Address_;
    }
    else {
        iSRAMAddress = iDacSeqBlock1Address_;
    }
    
    // Read the current values of the AFE Control register
    uint32_t AfeControlRegister = AD5940_ReadReg(REG_AFE_AFECON);

    //Clear the bits for ADC power (ADCEN) and ADC conversion to disable ADC
    AfeControlRegister &= ~(AFECTRL_ADCPWR | AFECTRL_ADCCNV);

    // Additional command if in the final block of CV
    if (bFinalBlock == true){
        uiCurrAddr = iSRAMAddress;

        // Jump to next sequence
        iSRAMAddress += AD5940_BUFFER_CV;

        // Update DAC register data
        this->funUpdateDACRegister(&uiDacData);

        // Collect commands for sequence
        uiCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister);
        uiCommandBuffer[1] = SEQ_INT1();
        uiCommandBuffer[2] = SEQ_WR(REG_AFE_LPDACDAT0, uiDacData);
        uiCommandBuffer[3] = SEQ_WAIT(10);
        uiCommandBuffer[4] = SEQ_WR(bSeqBlockUsed ? REG_AFE_SEQ1INFO : REG_AFE_SEQ0INFO,
                                    (iSRAMAddress << BITP_AFE_SEQ1INFO_ADDR) | 
                                    (AD5940_BUFFER_CV << BITP_AFE_SEQ1INFO_LEN));

        // Write sequencer commands to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, uiCommandBuffer, AD5940_BUFFER_CV);

        uiCurrAddr += AD5940_BUFFER_CV;
        // Stop sequencer
        // Collect commands for sequence
        uiCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister);
        uiCommandBuffer[1] = SEQ_NOP();
        uiCommandBuffer[2] = SEQ_NOP();
        uiCommandBuffer[3] = SEQ_NOP();
        uiCommandBuffer[4] = SEQ_STOP();

        // Write sequencer commands to SRAM. Sequencer is stopped -> 
        // End of sequence interrupt
        AD5940_SEQCmdWrite(uiCurrAddr, uiCommandBuffer, AD5940_BUFFER_CV);
    }
    // Not the final block of the CV
    else {
        // Jump to next block
        uiCurrAddr = iSRAMAddress;

        iSRAMAddress = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                        iDacSeqBlock1Address_ : iDacSeqBlock0Address_;

        // Update DAC register data
        this->funUpdateDACRegister(&uiDacData);

        // Collect commands for sequence
        uiCommandBuffer[0] = SEQ_WR(REG_AFE_AFECON, AfeControlRegister);
        uiCommandBuffer[1] = SEQ_INT1();
        uiCommandBuffer[2] = SEQ_WR(REG_AFE_LPDACDAT0, uiDacData);
        uiCommandBuffer[3] = SEQ_WAIT(10);
        uiCommandBuffer[4] = SEQ_WR(bSeqBlockUsed ? REG_AFE_SEQ1INFO : REG_AFE_SEQ0INFO,
                                    (iSRAMAddress << BITP_AFE_SEQ1INFO_ADDR) | 
                                    (AD5940_BUFFER_CV << BITP_AFE_SEQ1INFO_LEN));

        // Write sequencer commands to SRAM
        AD5940_SEQCmdWrite(uiCurrAddr, uiCommandBuffer, AD5940_BUFFER_CV);

        // Switch from block 0 -> 1 or 1 -> 0
        bSeqBlockUsed = bSeqBlockUsed ? false : true;
        c_DataStorageLocal_->set_SeqBlockUsed(bSeqBlockUsed);
    }

    // Switch between block 0 and block 1
    iDacCurrentBlock = (iDacCurrentBlock == CURRENT_BLOCK_0) ? 
                        CURRENT_BLOCK_1 : CURRENT_BLOCK_0; 

    c_DataStorageLocal_->set_DacCurrentBlock(iDacCurrentBlock);

    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for updating the values in the DAC register depending on the
 * current state of the experiment
 * @param ipDacData: Pointer where the value for the DAC is written to
 * 
 * @return: Returns error code.
 *****************************************************************************/
int C_Execute_CV::funUpdateDACRegister(uint32_t * ipDacData){
    // Initalize variables
    uint32_t iVbiasCode;
    uint32_t iVzeroCode;

    S_DataContainer S_ExperimentData; 

    // Get current state of the CV
    int iExperimentState = c_DataStorageLocal_->get_ExperimentState();
    
    // Update DAC depending on experiment state
    switch (iExperimentState){
    // Initalize sweaping direction
    case EC_METHOD_STATE_0:
        if (c_DataStorageLocal_->get_LowerVoltage() < 
            c_DataStorageLocal_->get_UpperVoltage()){
            // Set DAC to increment (ramp up)
            c_DataStorageLocal_->set_DacIncrement(true);
        }
        else {
            // Set DAC to decrement (ramp down)
            c_DataStorageLocal_->set_DacIncrement(false);
        }
        // Update ramp state: State 0 -> 1
        c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_1);
        break;

    // Sweaping from starting potential to first vertex
    case EC_METHOD_STATE_1:
        // Check sweep direction
        if (c_DataStorageLocal_->get_DacIncrement() == true){
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= iStepsToUpperVoltage_){
                // Update ramp state: State 1 -> 2
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_2);

                // Change sweep direction
                c_DataStorageLocal_->set_DacIncrement(false); 
            }
        }
        else {
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= iStepsToLowerVoltage_){
                // Update ramp state: State 1 -> 2
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_2);

                // Change sweep direction
                c_DataStorageLocal_->set_DacIncrement(true); 
            }
        }
        break;

    // Sweaping from first vertex to starting potential
    case EC_METHOD_STATE_2: 
        // Check sweep direction
        if (c_DataStorageLocal_->get_DacIncrement() == false){
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= iStepsToUpperVoltage_ * 2){
                // Update ramp state: State 2 -> 3
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_3);
            }
        }
        else {
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= iStepsToLowerVoltage_ * 2){
                // Update ramp state: State 2 -> 3
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_3);
            }
        }
        break;

    // Sweaping from starting potential to second vertex
    case EC_METHOD_STATE_3:
        // Check sweep direction
        if (c_DataStorageLocal_->get_DacIncrement() == false){
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= 
                (iStepsToUpperVoltage_ * 2 + iStepsToLowerVoltage_)){
                // Update ramp state: State 3 -> 4
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_4);

                // Change sweep direction
                c_DataStorageLocal_->set_DacIncrement(true); 
            }
        }
        else {
            if (c_DataStorageLocal_->get_CurrentStepNumber() % 
                c_DataStorageLocal_->get_StepNumber() >= 
                (iStepsToLowerVoltage_ * 2 + iStepsToUpperVoltage_)){
                // Update ramp state: State 3 -> 4
                c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_4);

                // Change sweep direction
                c_DataStorageLocal_->set_DacIncrement(false); 
            }
        }
        break;

    // Sweaping from second vertex to starting potential
    case EC_METHOD_STATE_4:
        if (c_DataStorageLocal_->get_CurrentStepNumber() % 
            c_DataStorageLocal_->get_StepNumber() == 
            (c_DataStorageLocal_->get_StepNumber() - 1)){
            // Update ramp state: State 4 -> 0
            c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_0);
        }    
        if (c_DataStorageLocal_->get_CurrentStepNumber() >= 
            c_DataStorageLocal_->get_StepNumber() * c_DataStorageLocal_->get_Cycle()){
            // Update ramp state: State 4 -> 5
            c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_5);
        }    
        break;

    // Stop sweaping
    case EC_METHOD_STATE_5:
        break;
    }

    // Check current DAC code
    if (c_DataStorageLocal_->get_DacCurrentCode() > 0){
        // Set potential of the WE to the high level                
        iVzeroCode = (uint32_t)((c_DataStorageLocal_->get_WePotentialHigh() - 
                      AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB);
    }
    else {   
        // Set potential of the WE to the low level
        iVzeroCode = (uint32_t)((c_DataStorageLocal_->get_WePotentialLow() - 
                      AD5940_MIN_DAC_OUTPUT) / AD5940_6BIT_DAC_1LSB);
    }

    // Calculate code of 12-Bit ADC/DAC
    iVbiasCode = (uint32_t)(iVzeroCode * 64 - c_DataStorageLocal_->
                    get_DacCurrentCode());

    // Ensure smooth transition when switching potential sign
    if (iVbiasCode < (iVzeroCode * 64)){
        iVbiasCode --;
    }
    
    // Clip DAC code for the 6-Bit and 12-Bit DAC
    if (iVbiasCode > 4095){
        iVbiasCode = 4095;
    }
    if (iVzeroCode > 64){
        iVzeroCode = 64;
    }

    // Update DAC code
    *ipDacData = (iVzeroCode << 12) | iVbiasCode;

    // Store the voltage value 
    S_ExperimentData.fVoltage = iVzeroCode * AD5940_6BIT_DAC_1LSB - 
                                iVbiasCode * AD5940_12BIT_DAC_1LSB;

    // Store experiment data at newest free position
    c_DataStorageGeneral_->set_ExperimentData(S_ExperimentData, 
        this->funGetDataPosition(c_DataStorageLocal_->get_CurrentStepNumber()));

    // Check for increment or decrement
    // Increase DAC code by increment per step
    if (c_DataStorageLocal_->get_DacIncrement()){
        c_DataStorageLocal_->set_DacCurrentCode(
            c_DataStorageLocal_->get_DacCurrentCode() + 
            c_DataStorageLocal_->get_DacIncrementPerStep());
    }
    // Decrease DAC code by increment per step
    else {
        c_DataStorageLocal_->set_DacCurrentCode(
            c_DataStorageLocal_->get_DacCurrentCode() - 
            c_DataStorageLocal_->get_DacIncrementPerStep());
    }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Helper method for conducting precalculations required for the
 *        execution of the CV.
 * @return: Returns error code.
 *****************************************************************************/
int C_Execute_CV::funPreCalulations(){
    // Get ADC gain
    iAdcPgaGain_ = c_DataStorageLocal_->get_AdcPgaGain();

    // Get reference voltage
    fAdcReferenceVoltage_ = c_DataStorageGeneral_->get_ADCReferenceVoltage();
    
    // Get size of Rtia resistor
    fRtiaMagnitude_ = c_DataStorageGeneral_->get_RtiaValue().Magnitude;
    
    iDacSeqBlock0Address_ = c_DataStorageLocal_->get_DacSeqBlock0Address();
    iDacSeqBlock1Address_ = c_DataStorageLocal_->get_DacSeqBlock1Address();
    iStepsPerBlock_ = c_DataStorageLocal_->get_StepsPerBlock();

    // Calculate steps from starting potential to upper turning potential
    // Check sweeping direction
    if (c_DataStorageLocal_->get_DacIncrement() == true){
        iStepsToUpperVoltage_ = (c_DataStorageLocal_->get_UpperVoltage() - 
                                c_DataStorageLocal_->get_StartVoltage()) / 
                                (c_DataStorageLocal_->get_DacIncrementPerStep() *
                                AD5940_12BIT_DAC_1LSB); 
    }
    else {
        // Sweeping down first (upper and lower vertex switched)
        iStepsToUpperVoltage_ = (c_DataStorageLocal_->get_LowerVoltage() - 
                                c_DataStorageLocal_->get_StartVoltage()) / 
                                (c_DataStorageLocal_->get_DacIncrementPerStep() *
                                AD5940_12BIT_DAC_1LSB);         
    }

    // Calculate steps from starting potential to lower turning potential
    iStepsToLowerVoltage_ = c_DataStorageLocal_->get_StepNumber() / 2 - 
                                                iStepsToUpperVoltage_;


    return EC_NO_ERROR;
}

#endif /* execute_CV_CPP */
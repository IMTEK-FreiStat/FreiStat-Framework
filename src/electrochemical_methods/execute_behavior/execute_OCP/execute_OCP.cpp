/******************************************************************************
 * @brief: Source file containing the subclass (C_Execute) C_Execute_OCP which 
 * defines the behavior of measuring the open circuit potential
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_OCP_CPP
#define execute_OCP_CPP

// Include dependencies
#include "execute_OCP.h"

/******************************************************************************
 * @brief Constructor of the class C_Execute_OCP
 * 
 *****************************************************************************/ 
C_Execute_OCP::C_Execute_OCP(){}

/******************************************************************************
 * @brief Starting method for the class C_Execute_OCP
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_Execute_OCP::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Save reference of data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;
    
    // Save reference of data storage object
    c_DataStorageGeneral_ = c_DataSoftwareStorage_->get_DataStorageGeneral();
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Get ADC gain
    iAdcPgaGain_ = c_DataStorageLocal_->get_AdcPgaGain();

    // Get reference voltage
    fAdcReferenceVoltage_ = c_DataStorageGeneral_->get_ADCReferenceVoltage();
    
    // Get size of Rtia resistor
    fRtiaMagnitude_ = c_DataStorageGeneral_->get_RtiaValue().Magnitude;

    // Get current experiment type
    strncpy(chrExperimentType_, c_DataStorageLocal_->get_ExperimentType(), 
        sizeof(chrExperimentType_));

    // Convert char array with experiment type to integer
    int iExperimentType = funGetExperimentTypeInt(chrExperimentType_);

    // Get reference to serial communication object
    C_Communication * c_Communication = c_DataSoftwareStorage_->get_Communication();

    // Prepare telegram strucutre
    c_Communication->funConstructPrefixes(chrExperimentType_);

    // Set system status to experiment running
    c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_RUNNING);

    // Control the application
    this->funControlApplication(FREISTAT_START_TIMER);  
    
    // Loop while experiment is running
    while (c_DataSoftwareStorage_->get_SystemStatus() == FREISTAT_EXP_RUNNING)
    {   
        // Check if interrupt has occured
        if (c_DataSoftwareStorage_->get_AD5940Setup()->get_InterruptOccured()){
            // Clear interrupt flag
            c_DataSoftwareStorage_->get_AD5940Setup()->set_InterruptOccured(false);

            // Call interrupt service routine
            this->funInterruptServiceRoutine();

            // If data needs to be send, write to serial port
            if (c_DataStorageGeneral_->get_SendDataCounter() < 
                c_DataStorageLocal_->get_CurrentStepNumber()){     

                // Get send data counter
                int iSendDataCounter = c_DataStorageGeneral_->
                    get_SendDataCounter();

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
}

/******************************************************************************
 * @brief Method for implementing interrupt service routine for cyclic 
 * voltammetry
 * 
 * @return: Returns error code                                        
 *****************************************************************************/
int C_Execute_OCP::funInterruptServiceRoutine(){
    // Initalizing variables
    uint32_t uiInterruptFlag;
    uint32_t uiFiFoCount;

    // Read interrupt flag from interrupt controller 0
    uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);

    // Loop until no interrupts are there which need to be handled
    // Reason for looping is that interrupts could occure while an interrupt is
    // still handled
    while (uiInterruptFlag != 0)
    {
        Serial.print(uiInterruptFlag);
        Serial.print("\n");
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
 * 
 * @param pData : Buffer array which contains all recently read data which 
 *                hasn't been processed yet.
 * @param uiCountData : Size of the buffer array containing the data.
 * 
 * @return: Returns error code encoded as integer
 *****************************************************************************/
int C_Execute_OCP::funProcessExperimentData(uint32_t * pData, 
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
        Serial.print(pData[iData] & 0xffff);
        Serial.print("\n");
        iSumSamples += (pData[iData] & 0xffff);
    }

    // Check if samples were collected
    if (uiCountData > 0){                                                            
        // Turn it into a voltage (unit of ADCReferenceVoltage -> mV)
        S_ExperimentData.fVoltage = AD5940_ADCCode2Volt(uint32_t(float(
            iSumSamples / uiCountData) + 0.5), iAdcPgaGain_, 
            fAdcReferenceVoltage_);

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
 * 
 * @param uiCommand: Command encoded as an integer for e.g., starting and 
 *                   stopping the CV.
 * 
 * @return: Error code encoded as integer 
 *****************************************************************************/
int C_Execute_OCP::funControlApplication(uint32_t uiCommand){
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
        S_WakeUpTimer_Config.WuptEndSeq = WUPTENDSEQ_A;

        // Define order and type of sequences
        S_WakeUpTimer_Config.WuptOrder[0] = SEQID_2;

        // Define how long a sequence should run
        // = LFOSCFrequency (in Hz) * Time (in seconds)
        // Sample delay shifts how much time the DAC and ADC should have        
        S_WakeUpTimer_Config.SeqxSleepTime[SEQID_2] = 1;

        S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_2] = 
            (uint32_t)(c_DataStorageGeneral_->get_LFOSCFrequency() * 
            c_DataStorageLocal_->get_Scanrate() / 1000.0);

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

#endif /* execute_OCP_CPP */
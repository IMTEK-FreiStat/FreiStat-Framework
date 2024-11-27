/******************************************************************************
 * @brief: Source file containing the subclass (C_Execute) C_Execute_EIS which 
 * defines the behavior of executing an electrocemical impedance spectroscopy.
 * 
 * @author: Cedric Neumann
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_EIS_CPP
#define execute_EIS_CPP

// Include dependencies
#include "execute_EIS.h"

/******************************************************************************
 * @brief Constructor of the class C_Execute_EIS
 * 
 *****************************************************************************/ 
C_Execute_EIS::C_Execute_EIS(){}

/******************************************************************************
 * @brief Starting method for the class C_Execute_EIS
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
int C_Execute_EIS::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
   
    // Initialize variables
    bEosInterruptOccured_ = false;
    int iErrorCode = 0;
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
    iErrorCode= this->funControlApplication(FREISTAT_START_TIMER); 
    
    // Loop while experiment is running
    while (c_DataSoftwareStorage_->get_SystemStatus() == FREISTAT_EXP_RUNNING){
        // Check if interrupt has occured
        if (c_DataSoftwareStorage_->get_AD5940Setup()->get_InterruptOccured()){
            // Clear interrupt flag
            c_DataSoftwareStorage_->get_AD5940Setup()->
                set_InterruptOccured(false);

            // Call interrupt service routine
           iErrorCode = this->funInterruptServiceRoutine();

           // Check for error 
           if (iErrorCode != 0) {
            return iErrorCode;
           }
        }
            
        // Get send data counter
        int iSendDataCounter = c_DataStorageGeneral_->get_SendDataCounter();

        // If data needs to be send, write to serial port
        if (iStepCounter_ > iSendDataCounter ){
            S_DataContainerEIS S_ExperimentDataEIS = c_DataStorageGeneral_->
            get_ExperimentDataEIS(this->funGetDataPosition(iSendDataCounter));


            // Write data to serial port       
                c_Communication->funSendExperimentDataEIS(
                    S_ExperimentDataEIS, iExperimentType);

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

    return 0;
}

/******************************************************************************
 * @brief Method for implementing interrupt service routine for chrono-
 * amperometry
 * 
 * @return: Returns error code                                        
 *****************************************************************************/
int C_Execute_EIS::funInterruptServiceRoutine(){
    // Initalizing variables
    uint32_t uiFiFoCount = 0;
    uint32_t uiInterruptFlag = 0;

    // Read interrupt flag from interrupt controller 0
    uiInterruptFlag = AD5940_INTCGetFlag(AFEINTC_0);

    // Loop until no interrupts are there which need to be handled
    // Reason for looping is that interrupts could occure while an interrupt is
    // still handled
    while (uiInterruptFlag != 0){
        // Custom interrupt 1
        if (uiInterruptFlag & AFEINTSRC_CUSTOMINT1){
            // Reset interrupt flag
       
            // Try to wake up AFE by reading at most 10 times
            if(AD5940_WakeUp(10) > 10){
                return EC_EXECUTE + EC_EX_WAKEUP_AFE_ERR;
                }

            //Prohibit AFE to enter sleep mode.
            AD5940_SleepKeyCtrlS(SLPKEY_LOCK); 

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
            
            if ( c_DataStorageLocal_->get_NumberPoints() - 1 <= iStepCounter_){  

            //disable AFE
            AD5940_ShutDownS();

            // Set interrupt flag
            bEosInterruptOccured_ = true;
            }
     
        }

        // FIFO threshold interrupt
          if (uiInterruptFlag & AFEINTSRC_DATAFIFOTHRESH){

            if(AD5940_WakeUp(10) > 10)  /* Wakeup AFE by read register, read 10 times at most */
                return AD5940ERR_WAKEUP;  /* Wakeup Failed */
            AD5940_SleepKeyCtrlS(SLPKEY_LOCK);  /* Prohibit AFE to enter sleep mode. */

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

            AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Allow AFE to enter sleep mode. */

             if ( c_DataStorageLocal_->get_NumberPoints() - 1 <= iStepCounter_){

                //disable AFE
                AD5940_ShutDownS();

                // Set interrupt flag
                bEosInterruptOccured_ = true;
            }
     
            
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
int C_Execute_EIS::funProcessExperimentData(uint32_t * pData, 
                                           uint32_t uiCountData){
    // Intialize variables
     S_DataContainerEIS S_ExperimentDataEIS;
    uint32_t ImpResCount = uiCountData/4;


    // Convert DFT result to int32_t type 
    for(uint32_t i=0; i<uiCountData; i++)
    {
        pData[i] &= 0x3ffff; 
        if(pData[i]&(1L<<17)) /* Bit17 is sign bit */
        {
        pData[i] |= 0xfffc0000; /* Data is 18bit in two's complement, bit17 is the sign bit */
        }
    }

    iImpCar_Type * pSrcData = (iImpCar_Type*)pData;

    for(uint32_t i=0; i<ImpResCount; i++)
    {
        iImpCar_Type *pDftRcal, *pDftRz;

        pDftRcal = pSrcData++;
        pDftRz = pSrcData++;
        float RzMag,RzPhase;
        float RcalMag, RcalPhase;
        
        //Calculate RcalMag, RcalPhase, RzMag and RzPhase
        RcalMag = sqrt((float)pDftRcal->Real*pDftRcal->Real+(float)pDftRcal->Image*pDftRcal->Image);
        RcalPhase = atan2(-pDftRcal->Image,pDftRcal->Real);
        RzMag = sqrt((float)pDftRz->Real*pDftRz->Real+(float)pDftRz->Image*pDftRz->Image);
        RzPhase = atan2(-pDftRz->Image,pDftRz->Real);

        // Calculate output RzMag and RzPhase 
        RzMag = RcalMag/RzMag* AD5940_CAL_RESISTOR;
        RzPhase = RcalPhase - RzPhase;
        
        // Store frequency, magnitue and phase into the export struct
        S_ExperimentDataEIS.fFrequency = c_DataStorageLocal_->get_CurrentFrequency();
        S_ExperimentDataEIS.Magnitude = RzMag;
        S_ExperimentDataEIS.Phase = RzPhase;

        // Data point number
        S_ExperimentDataEIS.iMeasurmentPair = 1 + iStepCounter_;

        // Save data
        c_DataStorageGeneral_->set_ExperimentDataEIS(
                S_ExperimentDataEIS, this->funGetDataPosition(iStepCounter_));
                iStepCounter_ ++; 
  }
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for starting and stoping the EIS sequence and the configuration
 * of the Wake-up timer which is used to time the sequence of different 
 * sequences of the chronoamperometry
 * 
 * @param uiCommand: Integer coded command to start and stop the EIS
 * 
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_Execute_EIS::funControlApplication(uint32_t uiCommand){
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
            S_WakeUpTimer_Config.WuptEndSeq = WUPTENDSEQ_A;

            // Define order and type of sequences
            S_WakeUpTimer_Config.WuptOrder[0] = SEQID_1;

            // Define how long a sequence should run
            // = LFOSCFrequency (in Hz) * Time (in seconds)
            S_WakeUpTimer_Config.SeqxSleepTime[SEQID_1] = 4;
            S_WakeUpTimer_Config.SeqxWakeupTime[SEQID_1] = (uint32_t)1600-4;

            // Config wake-up timer
            AD5940_WUPTCfg(&S_WakeUpTimer_Config);    
            break;
        }
        case FREISTAT_STOP_TIMER:{
            // Stop wake up timer
            AD5940_WUPTCtrl(bFALSE);
            // Update system and experiment status
            c_DataSoftwareStorage_->set_SystemStatus(FREISTAT_EXP_COMPLETED);
            c_DataStorageLocal_->set_ExperimentState(EC_METHOD_STATE_0);
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
int C_Execute_EIS::funUpdateSequence(){

    // Initialize variables
    float fNextFrequency;
    uint32_t arruiSeqWaitAddr[2];
    SoftSweepCfg_Type S_Sweep_Config;
    SEQInfo_Type S_SequenceInfo;

    // Configer Sweep struct
    S_Sweep_Config.SweepEn = bTRUE;
    S_Sweep_Config.SweepIndex = iStepCounter_;
    S_Sweep_Config.SweepStart = c_DataStorageLocal_->get_StartFrequency();
    S_Sweep_Config.SweepStop = c_DataStorageLocal_->get_StopFrequency();
    S_Sweep_Config.SweepPoints =  c_DataStorageLocal_->get_NumberPoints();
    S_Sweep_Config.SweepLog = c_DataStorageLocal_->get_SweepTyp();

    // Calculating next frequency
    AD5940_SweepNext(&S_Sweep_Config, &fNextFrequency);

    // Calculating frequency word for the wavegenerator
    AD5940_WGFreqCtrlS(fNextFrequency, AD5940_SYS_CLOCK_FREQ);

    // Update current frequency
    c_DataStorageLocal_->set_CurrentFrequency(fNextFrequency);
    
    // Get execute sequence
    S_SequenceInfo = c_DataStorageGeneral_->get_SequenceInfo(SEQID_1);

    // Get sequence wait adresses
    arruiSeqWaitAddr[0] = c_DataStorageGeneral_->get_SeqWaitAddr(0);
    arruiSeqWaitAddr[1] = c_DataStorageGeneral_->get_SeqWaitAddr(1);

    // Check filtersettings and wait command for new frequency
    c_DataStorageGeneral_->checkFrequency(fNextFrequency,
                                        S_SequenceInfo, arruiSeqWaitAddr);
    return EC_NO_ERROR;
}

#endif /* execute_EIS_CPP */
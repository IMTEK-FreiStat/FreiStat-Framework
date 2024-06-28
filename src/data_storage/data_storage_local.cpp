/******************************************************************************
 * @brief: Source file for the data storage local, in which all experiment 
 * parameters are stored, which vary from experiment to experiment.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef data_storage_local_CPP
#define data_storage_local_CPP

// Include header
#include "data_storage_local.h"

/******************************************************************************
 * @brief Default constructor of the class C_DataStorageLocal
 *****************************************************************************/ 
C_DataStorageLocal::C_DataStorageLocal(){}

/******************************************************************************
 * @brief Starting method for the class C_DataStorageLocal
 * @details Initialzing all experiment parameters and configuration parameters
 * with default values
 *****************************************************************************/
void C_DataStorageLocal::Begin(){
    // Initialize experiment parameters with default values
    bFixedWEPotential_ = true;

    iAdcPgaGain_ = ADCPGA_1P5;
    iAdcOsrSinc3_ = ADCSINC3OSR_4;
    iAdcOsrSinc2_ = ADCSINC2OSR_667;
    iCurrentStepNumber_ = 0;
    iCycle_ = CYCLE_I;
    iLPTIARtiaSize_ = LPTIARTIA_20K;
    iStepNumber_ = 0;

    fStartVoltage_ = START_POTENTIAL_F;
    fLowerVoltage_ = LOWER_POTENTIAL_F;
    fUpperVoltage_ = UPPER_POTENTIAL_F;
    fStepsize_ = STEP_SIZE_F;
    fScanrate_ = SCAN_RATE_F;
    fWePotentialHigh_ = AD5940_MAX_DAC_OUTPUT;
    fWePotentialLow_ = AD5940_MIN_DAC_OUTPUT;

    char chrExperimentType_[] = {UNDEFINED_EC_METHOD};

}

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for reference to next data object
 * @param c_nextDataStorage: reference to next data object
 *****************************************************************************/
void C_DataStorageLocal::set_NextDataStorage(
        C_DataStorageLocal * c_nextDataStorage){
    c_nextDataStorage_ = c_nextDataStorage;
}

/******************************************************************************
 * @brief Setter method for saving the current experiment type
 * @param chrExperimentType: Experiment type of the JSON telegram 
 *****************************************************************************/
 void C_DataStorageLocal::set_ExperimentType(char * chrExperimentType){
    strncpy(chrExperimentType_ ,chrExperimentType, sizeof(chrExperimentType_));
}

/******************************************************************************
 * @brief Setter method for saving the start potential
 * @param fStartVoltage: Start potential
 *****************************************************************************/
 void C_DataStorageLocal::set_StartVoltage(float fStartVoltage){
    fStartVoltage_ = fStartVoltage;
}

/******************************************************************************
 * @brief Setter method for saving the lower potential
 * @param fLowerVoltage: Lower potential
 *****************************************************************************/
 void C_DataStorageLocal::set_LowerVoltage(float fLowerVoltage){
    fLowerVoltage_ = fLowerVoltage;
}

/******************************************************************************
 * @brief Setter method for saving the upper potential
 * @param fUpperVoltage: Upper potential
 *****************************************************************************/
 void C_DataStorageLocal::set_UpperVoltage(float fUpperVoltage){
    fUpperVoltage_ = fUpperVoltage;
}

/******************************************************************************
 * @brief Setter method for saving the stepsize of the ADC
 * @param fStepsize: Stepsize in mV
 *****************************************************************************/
 void C_DataStorageLocal::set_Stepsize(float fStepsize){
    fStepsize_ = fStepsize;
}

/******************************************************************************
 * @brief Setter method for saving the scan rate
 * @param fScanrate: Scan rate in mV/s. Determines in combination with the step
 * size the time between measurements
 * 
 * t = fStepsize / fScanrate
 * 
 *****************************************************************************/
 void C_DataStorageLocal::set_Scanrate(float fScanrate){
    fScanrate_ = fScanrate;
}

/******************************************************************************
 * @brief Setter method for saving the number of cycles
 * @param iCycle: Number of cycles the experiment should be conducted
 *****************************************************************************/
 void C_DataStorageLocal::set_Cycle(int iCycle){
    iCycle_ = iCycle;
}

/******************************************************************************
 * @brief Setter method for saving the upper voltage limit of the working 
 * electrode
 * @param fWePotentialHigh: Upper voltage limit of the working electrode
 *****************************************************************************/
 void C_DataStorageLocal::set_WePotentialHigh(float fWePotentialHigh){
    fWePotentialHigh_ = fWePotentialHigh;
}

/******************************************************************************
 * @brief Setter method for saving the lower voltage limit of the working 
 * electrode
 * @param fWePotentialLow: Lower voltage limit of the working electrode
 *****************************************************************************/
 void C_DataStorageLocal::set_WePotentialLow(float fWePotentialLow){
    fWePotentialLow_ = fWePotentialLow;
}

/******************************************************************************
 * @brief Setter method for option of bypassing notch filter (50 Hz/ 60 Hz)
 * @param bAdcNotchFilter: Variable defining if notch filter is bypassed
 *****************************************************************************/
 void C_DataStorageLocal::set_AdcNotchFilter(bool bAdcNotchFilter){
    bAdcNotchFilter_ = bAdcNotchFilter;
}

/******************************************************************************
 * @brief Setter method for saving the gain of des programmable gain
 * amplifier
 * @param iAdcPgaGain: Gain of the programmable gain amplifier encoded as an int
 *****************************************************************************/
 void C_DataStorageLocal::set_AdcPgaGain(int iAdcPgaGain){
    iAdcPgaGain_ = iAdcPgaGain;
}

/******************************************************************************
 * @brief Setter method for saving the oversampling rate of the Sinc2 filter
 * 
 * @param iAdcPgaGain: Oversampling rate of the Sinc2 filter encoded as an int
 *****************************************************************************/
 void C_DataStorageLocal::set_AdcOsrSinc2(int iAdcOsrSinc2){
    iAdcOsrSinc2_ = iAdcOsrSinc2;
}

/******************************************************************************
 * @brief Setter method for saving the oversampling rate of the Sinc3 filter
 * 
 * @param iAdcPgaGain: Oversampling rate of the Sinc3 filter encoded as an int
 *****************************************************************************/
 void C_DataStorageLocal::set_AdcOsrSinc3(int iAdcOsrSinc3){
    iAdcOsrSinc3_ = iAdcOsrSinc3;
}

/******************************************************************************
 * @brief Setter method for saving the calculated DAC steps per increment
 * @param iDacIncrementPerStep: DAC steps per increment
 *****************************************************************************/
void C_DataStorageLocal::set_DacIncrementPerStep(float iDacIncrementPerStep){
    iDacIncrementPerStep_ = iDacIncrementPerStep;
}

/******************************************************************************
 * @brief Setter method for saving the calculated total amount of steps
 * for the chosen electrochemical method
 * @param iStepNumber: Total number of steps 
 *****************************************************************************/
void C_DataStorageLocal::set_StepNumber(int iStepNumber){
    iStepNumber_ = iStepNumber;
}

/******************************************************************************
 * @brief Setter method for saving the direction of increment/ decrement of the
 * DAC
 * @param bDacIncrement: Is DAC incrementing or decrementing
 *****************************************************************************/
void C_DataStorageLocal::set_DacIncrement(bool bDacIncrement){
    bDacIncrement_ = bDacIncrement;
}

/******************************************************************************
 * @brief Setter method for saving the current position of the DAC
 * @param iDacCurrentCode: Current position of the DAC
 *****************************************************************************/
void C_DataStorageLocal::set_DacCurrentCode(int iDacCurrentCode){
    iDacCurrentCode_ = iDacCurrentCode;
}

/******************************************************************************
 * @brief Getter method for saving the current state of the ec-method
 * @param iExperimentState: The state of the ec-method
 *****************************************************************************/
void C_DataStorageLocal::set_ExperimentState(int iExperimentState){
    iExperimentState_ = iExperimentState;
}

/******************************************************************************
 * @brief Setter method for saving current step of the ADC/ DAC
 * @param iCurrentStepNumber: Current step of the ADC/ DAC
 *****************************************************************************/
void C_DataStorageLocal::set_CurrentStepNumber(int iCurrentStepNumber){
    iCurrentStepNumber_ = iCurrentStepNumber;
}

/******************************************************************************
 * @brief Setter method for saving the potential step at a specific position
 * in data storage
 * @param fPotentialStep: Potential step which should be stored
 * @param iPosition: Position from which data should be read
 *****************************************************************************/
void C_DataStorageLocal::set_PotentialSteps(float fPotentialStep, int iPosition){
    arrfPotentialSteps_[iPosition] = fPotentialStep;
}

/******************************************************************************
 * @brief Setter method for saving the pulse duration at a specific position
 * in data storage
 * @param fPulseDuration: Pulse duration which should be stored
 * @param iPosition: Position from which data should be read
 *****************************************************************************/
void C_DataStorageLocal::set_PulseDurations(float fPulseDuration, int iPosition){
    arrfPulseDurations_[iPosition] = fPulseDuration;
}

/******************************************************************************
 * @brief Setter method for saving the remaining steps of the ec-method
 * @param fStepsRemaining: Remaining steps of the ec-method
 *****************************************************************************/
void C_DataStorageLocal::set_StepsRemaining(float fStepsRemaining){
    fStepsRemaining_ = fStepsRemaining;
}

/******************************************************************************
 * @brief Setter method for saving the steps which the ADC does per block
 * @param iStepsPerBlock: Steps which the ADC does per block
 *****************************************************************************/
void C_DataStorageLocal::set_StepsPerBlock(int iStepsPerBlock){
    iStepsPerBlock_ = iStepsPerBlock;
}

/******************************************************************************
 * @brief Setter method for saving which block is used
 * @param bSeqBlockUsed: used block (0 or 1)
 *****************************************************************************/
void C_DataStorageLocal::set_SeqBlockUsed(bool bSeqBlockUsed){
    bSeqBlockUsed_ = bSeqBlockUsed;
}

/******************************************************************************
 * @brief Setter method for saving the currently used block (0 or 1)
 * @param iDacCurrentBlock: the currently used block (0 or 1)
 *****************************************************************************/
void C_DataStorageLocal::set_DacCurrentBlock(int iDacCurrentBlock){
    iDacCurrentBlock_ = iDacCurrentBlock;
}

/******************************************************************************
 * @brief Setter method for saving the address of block 0 of the DAC 
 * sequence
 * @param iDacSeqBlock0Address: address of block 0 of the DAC sequence
 *****************************************************************************/
void C_DataStorageLocal::set_DacSeqBlock0Address(int iDacSeqBlock0Address){
    iDacSeqBlock0Address_ = iDacSeqBlock0Address;
}

/******************************************************************************
 * @brief Setter method for saving the address of block 1 of the DAC 
 * sequence
 * @param iDacSeqBlock1Address: address of block 1 of the DAC sequence
 *****************************************************************************/
void C_DataStorageLocal::set_DacSeqBlock1Address(int iDacSeqBlock1Address){
    iDacSeqBlock1Address_ = iDacSeqBlock1Address;
}

/******************************************************************************
 * @brief Setter method for saving if potential of the working electrode is
 * fixed or variable
 * @param bFixedWEPotential: Returns bool flag for the fixation of the 
 * we-potential 
 *****************************************************************************/
void C_DataStorageLocal::set_FixedWEPotential(bool bFixedWEPotential){
    bFixedWEPotential_ = bFixedWEPotential;
}

/******************************************************************************
 * @brief Setter method for saving amount of entries in buffer arrays
 * @param iBufferEntries: Amount of buffer entries in data storage arrays
 *****************************************************************************/
void C_DataStorageLocal::set_BufferEntries(int iBufferEntries){
    iBufferEntries_ = iBufferEntries;
}

/******************************************************************************
 * Getter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Getter method for reference to next data object
 * @returns: reference to next data object
 *****************************************************************************/
C_DataStorageLocal *  C_DataStorageLocal::get_NextDataStorage(){
    return c_nextDataStorage_;
}

/******************************************************************************
 * @brief Getter method for returning current experiment type
 * @return: Returns currently saved experiment type
 *****************************************************************************/
char * C_DataStorageLocal::get_ExperimentType(){
    return chrExperimentType_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved starting potential
 * @return: Returns currently saved starting potential
 *****************************************************************************/
float C_DataStorageLocal::get_StartVoltage(){
    return fStartVoltage_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved lower voltage
 * @return: Returns currently saved lower voltage
 *****************************************************************************/
float C_DataStorageLocal::get_LowerVoltage(){
    return fLowerVoltage_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved upper voltage
 * @return: Returns currently saved upper voltage
 *****************************************************************************/
float C_DataStorageLocal::get_UpperVoltage(){
    return fUpperVoltage_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved step size
 * @return: Returns currently saved step size
 *****************************************************************************/
float C_DataStorageLocal::get_Stepsize(){
    return fStepsize_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved scan rate
 * @return: Returns currently saved scan rate
 *****************************************************************************/
float C_DataStorageLocal::get_Scanrate(){
    return fScanrate_;
}

/******************************************************************************
 * @brief Getter method for returning currently saved numbers of cycles
 * @return: Returns currently saved numbers of cycles
 *****************************************************************************/
int C_DataStorageLocal::get_Cycle(){
    return iCycle_;
}

/******************************************************************************
 * @brief Getter method for returning the upper voltage limit of the working
 * electrode
 * @return: Returns the upper voltage limit of the working electrode in mV
 *****************************************************************************/
float C_DataStorageLocal::get_WePotentialHigh(){
    return fWePotentialHigh_;
}

/******************************************************************************
 * @brief Getter method for returning the lower voltage limit of the working
 * electrode
 * @return: Returns the lower voltage limit of the working electrode in mV
 *****************************************************************************/
float C_DataStorageLocal::get_WePotentialLow(){
    return fWePotentialLow_;
}

/******************************************************************************
 * @brief Getter method for the option of bypassing notch filter (50 Hz/ 60 Hz)
 * @returns: Variable defining if notch filter is bypassed
 *****************************************************************************/
 bool C_DataStorageLocal::get_AdcNotchFilter(){
    return bAdcNotchFilter_;
}

/******************************************************************************
 * @brief Getter method for returning the gain of des programmable gain
 * amplifier
 * @details: The following gain configurations for the ADC Gain are defined:
 * 
 * ADCPGA_1                    0     ADC PGA Gain of 1 
 * ADCPGA_1P5                  1     ADC PGA Gain of 1.5 
 * ADCPGA_2                    2     ADC PGA Gain of 2 
 * ADCPGA_4                    3     ADC PGA Gain of 4 
 * ADCPGA_9                    4     ADC PGA Gain of 9 
 * 
 * @returns: Returns the gain of the programmable gain amplifier encoded as an 
 * int
 *****************************************************************************/
 int C_DataStorageLocal::get_AdcPgaGain(){
    return iAdcPgaGain_;
}

/******************************************************************************
 * @brief Getter method for returning the oversampling rate of the Sinc2
 * filter
 * @details: The following size are defined for the oversampling rate of the
 *           Sinc2 filter:
 * 
 * ADCSINC2OSR_22               0       ADC SINC2 OSR 22   
 * ADCSINC2OSR_44               1       ADC SINC2 OSR 44   
 * ADCSINC2OSR_89               2       ADC SINC2 OSR 89   
 * ADCSINC2OSR_178              3       ADC SINC2 OSR 178  
 * ADCSINC2OSR_267              4       ADC SINC2 OSR 267  
 * ADCSINC2OSR_533              5       ADC SINC2 OSR 533  
 * ADCSINC2OSR_640              6       ADC SINC2 OSR 640  
 * ADCSINC2OSR_667              7       ADC SINC2 OSR 667  
 * ADCSINC2OSR_800              8       ADC SINC2 OSR 800  
 * ADCSINC2OSR_889              9       ADC SINC2 OSR 889  
 * ADCSINC2OSR_1067             10      ADC SINC2 OSR 1067 
 * ADCSINC2OSR_1333             11      ADC SINC2 OSR 1333 
 * 
 * @returns: Returns the oversampling ratoe of the Sinc2 filter encoded as an
 * int
 *****************************************************************************/
 int C_DataStorageLocal::get_AdcOsrSinc2(){
    return iAdcOsrSinc2_;
}

/******************************************************************************
 * @brief Getter method for returning the oversampling rate of the Sinc3
 * filter
 * @details: The following size are defined for the oversampling rate of the
 *           Sinc3 filter:
 * 
 * ADCSINC3OSR_5                0       ADC SINC2 OSR 5   
 * ADCSINC3OSR_4                1       ADC SINC2 OSR 4   
 * ADCSINC3OSR_2                2       ADC SINC2 OSR 2   
 * 
 * @returns: Returns the oversampling ratoe of the Sinc3 filter encoded as an
 * int
 *****************************************************************************/
 int C_DataStorageLocal::get_AdcOsrSinc3(){
    return iAdcOsrSinc3_;
}

/******************************************************************************
 * @brief Setter method for saving the the size of the LPTIA Rtia resistor
 * @param iLPTIARtiaSize: Value of the LPTIA Rtia resistor in ohm
 *****************************************************************************/
void C_DataStorageLocal::set_LPTIARtiaSize(int iLPTIARtiaSize){
    // Check if iLPTIARtiaSize is already encoded
    if (iLPTIARtiaSize >= 0 && iLPTIARtiaSize <= 26){
        iLPTIARtiaSize_ = iLPTIARtiaSize;
    }
    else {
        // Check the value of iLPTIARtiaSize
        switch (iLPTIARtiaSize){
        case 0:
            iLPTIARtiaSize_ = LPTIARTIA_OPEN;
            break;
        case 200:
            iLPTIARtiaSize_ = LPTIARTIA_200R;
            break;
        case 1000:
            iLPTIARtiaSize_ = LPTIARTIA_1K;
            break;
        case 2000:
            iLPTIARtiaSize_ = LPTIARTIA_2K;
            break;
        case 3000:
            iLPTIARtiaSize_ = LPTIARTIA_3K;
            break;
        case 4000:
            iLPTIARtiaSize_ = LPTIARTIA_4K;
            break;
        case 6000:
            iLPTIARtiaSize_ = LPTIARTIA_6K;
            break;
        case 8000:
            iLPTIARtiaSize_ = LPTIARTIA_8K;
            break;
        case 10000:
            iLPTIARtiaSize_ = LPTIARTIA_10K;
            break;
        case 12000:
            iLPTIARtiaSize_ = LPTIARTIA_12K;
            break;
        case 16000:
            iLPTIARtiaSize_ = LPTIARTIA_16K;
            break;
        case 20000:
            iLPTIARtiaSize_ = LPTIARTIA_20K;
            break;
        case 24000:
            iLPTIARtiaSize_ = LPTIARTIA_24K;
            break;
        case 30000:
            iLPTIARtiaSize_ = LPTIARTIA_30K;
            break;
        case 32000:
            iLPTIARtiaSize_ = LPTIARTIA_32K;
            break;
        case 40000:
            iLPTIARtiaSize_ = LPTIARTIA_40K;
            break;
        case 48000:
            iLPTIARtiaSize_ = LPTIARTIA_48K;
            break;
        case 64000:
            iLPTIARtiaSize_ = LPTIARTIA_64K;
            break;
        case 85000:
            iLPTIARtiaSize_ = LPTIARTIA_85K;
            break;
        case 96000:
            iLPTIARtiaSize_ = LPTIARTIA_96K;
            break;
        case 100000:
            iLPTIARtiaSize_ = LPTIARTIA_100K;
            break;
        case 120000:
            iLPTIARtiaSize_ = LPTIARTIA_120K;
            break;
        case 128000:
            iLPTIARtiaSize_ = LPTIARTIA_128K;
            break;
        case 160000:
            iLPTIARtiaSize_ = LPTIARTIA_160K;
            break;
        case 196000:
            iLPTIARtiaSize_ = LPTIARTIA_196K;
            break;
        case 256000:
            iLPTIARtiaSize_ = LPTIARTIA_256K;
            break;
        case 512000:
            iLPTIARtiaSize_ = LPTIARTIA_512K;
            break;
        default:
            // Value not found, initialize with default value
            iLPTIARtiaSize_ = LPTIARTIA_20K;
            break;
        }
    }    
}

/******************************************************************************
 * @brief Getter method for returning the calculated DAC steps per increment
 * @returns: DAC steps per increment
 *****************************************************************************/
float C_DataStorageLocal::get_DacIncrementPerStep(){
    return iDacIncrementPerStep_;
}

/******************************************************************************
 * @brief Getter method for returning the calculated total amount of steps
 * for the chosen electrochemical method
 * @returns: Total number of steps 
 *****************************************************************************/
int C_DataStorageLocal::get_StepNumber(){
    return iStepNumber_;
}

/******************************************************************************
 * @brief Getter method for returning the direction of increment/ decrement of 
 * the DAC.
 * @return: Returns status of DAC (incrementing or decrementing)
 *****************************************************************************/
bool C_DataStorageLocal::get_DacIncrement(){
    return bDacIncrement_;
}

/******************************************************************************
 * @brief Getter method for returning the current position of the DAC
 * @returns: Current position of the DAC
 *****************************************************************************/
int C_DataStorageLocal::get_DacCurrentCode(){
    return iDacCurrentCode_;
}

/******************************************************************************
 * @brief Getter method for returning the size of the LPTIA Rtia encoded as an
 * integer value which matches the define in the AD5940.h
 * electrode
 * @details: The following sizes of the low power transimpedance amplifier 
 *           resistor ore defined:
 * 
 *  LPTIARTIA_OPEN              0           Disconnect LPTIA internal Rtia
 *  LPTIARTIA_200R              1           200Ohm internal Rtia
 *  LPTIARTIA_1K                2           1 kOhm
 *  LPTIARTIA_2K                3           2 kOhm
 *  LPTIARTIA_3K                4           3 kOhm
 *  LPTIARTIA_4K                5           4 kOhm
 *  LPTIARTIA_6K                6           6 kOhm
 *  LPTIARTIA_8K                7           8 kOhm
 *  LPTIARTIA_10K               8           10 kOhm
 *  LPTIARTIA_12K               9           12 kOhm
 *  LPTIARTIA_16K               10          16 kOhm
 *  LPTIARTIA_20K               11          20 kOhm
 *  LPTIARTIA_24K               12          24 kOhm
 *  LPTIARTIA_30K               13          30 kOhm
 *  LPTIARTIA_32K               14          32 kOhm
 *  LPTIARTIA_40K               15          40 kOhm
 *  LPTIARTIA_48K               16          48 kOhm
 *  LPTIARTIA_64K               17          64 kOhm
 *  LPTIARTIA_85K               18          85 kOhm
 *  LPTIARTIA_96K               19          96 kOhm
 *  LPTIARTIA_100K              20          100 kOhm
 *  LPTIARTIA_120K              21          120 kOhm
 *  LPTIARTIA_128K              22          128 kOhm
 *  LPTIARTIA_160K              23          160 kOhm
 *  LPTIARTIA_196K              24          196 kOhm
 *  LPTIARTIA_256K              25          256 kOhm
 *  LPTIARTIA_512K              26          512 kOhm
 * 
 * @return: Returns the size of the LPTIA Rtia encoded as an int
  *****************************************************************************/
int C_DataStorageLocal::get_LPTIARtiaSize(){
    return iLPTIARtiaSize_;
}

/******************************************************************************
 * @brief Getter method for returning the current state of the ec-method
 * @returns: The state of the ec-method
 *****************************************************************************/
int C_DataStorageLocal::get_ExperimentState(){
    return iExperimentState_;
}

/******************************************************************************
 * @brief Getter method for returning current step of the ADC/ DAC
 * @returns: Current step of the ADC/ DAC
 *****************************************************************************/
int C_DataStorageLocal::get_CurrentStepNumber(){
    return iCurrentStepNumber_;
}

/******************************************************************************
 * @brief Getter method for returning the potential step at a specific position
 * in data storage
 * @param iPosition: Position from which data should be read
 * @returns: Potential step which should be read
 *****************************************************************************/
float C_DataStorageLocal::get_PotentialSteps(int iPosition){
    return arrfPotentialSteps_[iPosition];
}

/******************************************************************************
 * @brief Getter method for returning the pulse duration at a specific position
 * in data storage
 * @param iPosition: Position from which data should be read
 * @returns: Pulse duration which should be read
 *****************************************************************************/
float C_DataStorageLocal::get_PulseDurations(int iPosition){
    return arrfPulseDurations_[iPosition];
}

/******************************************************************************
 * @brief Getter method for returning the remaining steps of the ec-method
 * @returns: returning the remaining steps of the ec-method
 *****************************************************************************/
float C_DataStorageLocal::get_StepsRemaining(){
    return fStepsRemaining_;
}

/******************************************************************************
 * @brief Getter method for returning the steps which the ADC does per block
 * @returns: returning the steps which the ADC does per block
 *****************************************************************************/
int C_DataStorageLocal::get_StepsPerBlock(){
    return iStepsPerBlock_;
}

/******************************************************************************
 * @brief Getter method for returning which block is used
 * @returns: Used block (0 or 1)
 *****************************************************************************/
bool C_DataStorageLocal::get_SeqBlockUsed(){
    return bSeqBlockUsed_;
}

/******************************************************************************
 * @brief Getter method for returning the currently used block (0 or 1)
 * @returns: returning the currently used block (0 or 1)
 *****************************************************************************/
int C_DataStorageLocal::get_DacCurrentBlock(){
    return iDacCurrentBlock_;
}

/******************************************************************************
 * @brief Getter method for returning the address of block 0 of the DAC 
 * sequence
 * @returns: address of block 0 of the DAC sequence
 *****************************************************************************/
int C_DataStorageLocal::get_DacSeqBlock0Address(){
    return iDacSeqBlock0Address_;
}

/******************************************************************************
 * @brief Getter method for returning the address of block 1 of the DAC 
 * sequence
 * @returns: address of block 1 of the DAC sequence
 *****************************************************************************/
int C_DataStorageLocal::get_DacSeqBlock1Address(){
    return iDacSeqBlock1Address_;
}

/******************************************************************************
 * @brief Getter method for returning if potential of the working electrode is
 * fixed or variable
 * @returns: Returns bool flag for the fixation of the we-potential 
 *****************************************************************************/
bool C_DataStorageLocal::get_FixedWEPotential(){
    return bFixedWEPotential_;
}

/******************************************************************************
 * @brief Getter method for returning amount of entries in buffer arrays
 * @returns: Amount of buffer entries in data storage arrays
 *****************************************************************************/
int C_DataStorageLocal::get_BufferEntries(){
    return iBufferEntries_;
}

#endif /* data_storage_local_CPP */
/******************************************************************************
 * @brief: Source file for the general data storage, in which all experiment 
 * parameters which are identical for all experiments are stored.
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef data_storage_general_CPP
#define data_storage_general_CPP

// Include header
#include "data_storage_general.h"

/******************************************************************************
 * @brief Default constructor of the class C_DataStorageGeneral
 *****************************************************************************/ 
C_DataStorageGeneral::C_DataStorageGeneral(){}

/******************************************************************************
 * @brief Starting method for the class C_DataStorageGeneral
 * @details Initialzing all experiment parameters and configuration parameters
 * with default values
 *****************************************************************************/
void C_DataStorageGeneral::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    // Initalizes system with default parameters
    // No command received yet
    iCommandNumber_ = FREISTAT_UNDEF_I;

    // Initializes configurations parameters
    iSequenceLength_ = 1;
    iSequenceCycles_ = 1;
    iSendDataCounter_ = 0;
    iFiFoThreshold_ = AD5940_FIFO_THRESHOLD;
    iLPTIALoadSize_ = LPTIARLOAD_SHORT;
    iLPAmpPowerMode_ = LPAMPPWR_NORM;
    iSeqStartAddress_ = AD5940_SEQ_START_ADDR;
    iSeqMaxLength_ = (AD5940_SEQ_MAX_ADDR - AD5940_SEQ_START_ADDR);

    fADCReferenceVoltage_ = AD5940_ADC_REF_VOLTAGE;
    fCalibrationResistorValue_ = AD5940_CAL_RESISTOR;
    fLFOSCFrequency_ = c_DataSoftwareStorage->get_AD5940Setup()->
                       get_LFOSCFrequency();
 }

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for saving the current command number
 * @param iCommandNumber: Command number of the JSON telegram 
 *****************************************************************************/
void C_DataStorageGeneral::set_CommandNumber(int iCommandNumber){
    iCommandNumber_ = iCommandNumber;
}

/******************************************************************************
 * @brief Setter method for saving the length of the experiment sequence
 * @param iSequenceLength: Length of the experiment sequence
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceLength(int iSequenceLength){
    iSequenceLength_ = iSequenceLength;
}

/******************************************************************************
 * @brief Setter method for saving the amount of cycles of the experiment 
 * sequence
 * @param iSequenceCycles: Amount of cycles the experiment sequence should be
 * be repeated
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceCycles(int iSequenceCycles){
    iSequenceCycles_ = iSequenceCycles;
}

/******************************************************************************
 * @brief Setter method for saving the size of the external LPTIA Rtia
 * resistor
 * @param fLPTIARtiaSizeExternal: Size of the external LPTIA Rtia in ohm
 *****************************************************************************/
void C_DataStorageGeneral::set_LPTIARtiaSizeExternal(float fLPTIARtiaSizeExternal){
    fLPTIARtiaSizeExternal_ = fLPTIARtiaSizeExternal;
}

/******************************************************************************
 * @brief Setter method for saving the power mode of the low power amplifier
 * resistor
 * @param iLPAmpPowerMode: Power mode of the low power amplifier
 *****************************************************************************/
void C_DataStorageGeneral::set_LPAmpPowerMode(int iLPAmpPowerMode){
    iLPAmpPowerMode_ = iLPAmpPowerMode;
}

/******************************************************************************
 * @brief Setter method for saving the values of the calibrated Rtia resistor
 * @param fRtiaValue: Data of the calibrated Rtia resistor
 *****************************************************************************/
void C_DataStorageGeneral::set_RtiaValue(fImpPol_Type fRtiaValue){
    fRtiaValue_ = fRtiaValue;
}

/******************************************************************************
 * @brief Setter method for saving the siza of the LPTIA load
 * @param iLPTIALoadSize: Value of the LPTIA load encoded as integer
 *****************************************************************************/
void C_DataStorageGeneral::set_LPTIALoadSize(int iLPTIALoadSize){
    iLPTIALoadSize_ = iLPTIALoadSize;
}

/******************************************************************************
 * @brief Setter method for saving the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
void C_DataStorageGeneral::set_SequenceInfo(SEQInfo_Type S_SequenceInfo, 
                                            int iSequenceID){
    S_SequenceInfo_[iSequenceID] = S_SequenceInfo;
}

/******************************************************************************
 * @brief Setter method for saving experiment data from specific position
 * in data storage
 * @param S_ExperimentData: Experiment data which should be stored
 * @param iPosition: Position from which data should be read
 *****************************************************************************/
void C_DataStorageGeneral::set_ExperimentData(S_DataContainer S_ExperimentData, 
                                              int iPosition){
    S_ExperimentData_[iPosition] = S_ExperimentData;
}

/******************************************************************************
 * @brief Setter method for latest position of send data
 * @param iSendDataCounter: Latest position of send data
 *****************************************************************************/
void C_DataStorageGeneral::set_SendDataCounter(int iSendDataCounter){
    iSendDataCounter_ = iSendDataCounter;
}

/******************************************************************************
 * Getter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Getter method for returning current command number
 * @return: Returns currently saved command number
 *****************************************************************************/
int C_DataStorageGeneral::get_CommandNumber(){
    return iCommandNumber_;
}

/******************************************************************************
 * @brief Getter method for returning the length of the experiment sequence
 * @return: Length of the experiment sequence
 *****************************************************************************/
int C_DataStorageGeneral::get_SequenceLength(){
    return iSequenceLength_;
}

/******************************************************************************
 * @brief Getter method for returning the amount of cycles of the experiment 
 * sequence
 * @return: Amount of cycles the experiment sequence should be be repeated
 *****************************************************************************/
int C_DataStorageGeneral::get_SequenceCycles(){
    return iSequenceCycles_;
}

/******************************************************************************
 * @brief Getter method for returning the defined FIFO threshold
 * @return: Returns defined FIFO threshold
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_FiFoThreshold(){
    return iFiFoThreshold_;
}

/******************************************************************************
 * @brief Getter method for returning the defined starting adress of the 
 * sequencer
 * @return: Returns defined squencer starting address
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_SeqStartAddress(){
    return iSeqStartAddress_;
}

/******************************************************************************
 * @brief Getter method for the defined maximum length of sequencer commands
 * @return: Returns defined maximum length of sequencer commands
 *****************************************************************************/
unsigned int C_DataStorageGeneral::get_SeqMaxLength(){
    return iSeqMaxLength_;
}

/******************************************************************************
 * @brief Getter method for the defined ADC/DAC reference voltage
 * @return: Returns defined ADC/DAC reference voltage
 *****************************************************************************/
float C_DataStorageGeneral::get_ADCReferenceVoltage(){
    return fADCReferenceVoltage_;
}

/******************************************************************************
 * @brief Getter method for the defined value of the reference resistor
 * @return: Returns defined value of the reference resistor
 *****************************************************************************/
float C_DataStorageGeneral::get_CalibrationResistorValue(){
    return fCalibrationResistorValue_;
}

/******************************************************************************
 * @brief Getter method for returning measured LFOSC frequency
 * @return: Returns measured LFOSC frequency
 *****************************************************************************/
float C_DataStorageGeneral::get_LFOSCFrequency(){
    return fLFOSCFrequency_;
}

/******************************************************************************
 * @brief Getter method for returning size of the external LPTIA Rtia
 * resistor
 * @return: Returns the size of the external LPTIA Rtia in ohm
 *****************************************************************************/
float C_DataStorageGeneral::get_LPTIARtiaSizeExternal(){
    return fLPTIARtiaSizeExternal_;
}

/******************************************************************************
 * @brief Getter method for returning the power mode of the low power amplifier
 * @return: Returns power mode of the low power amplifier
 *****************************************************************************/
int C_DataStorageGeneral::get_LPAmpPowerMode(){
    return iLPAmpPowerMode_;
}

/******************************************************************************
 * @brief Getter method for returning the values of the calibrated Rtia resistor
 * @returns: Data of the calibrated Rtia resistor
 *****************************************************************************/
fImpPol_Type C_DataStorageGeneral::get_RtiaValue(){
    return fRtiaValue_;
}

/******************************************************************************
 * @brief Getter method for returning the siza of the LPTIA load
 * @details: The following sizes for the load resistor are defined:
 * 
 * LPTIARLOAD_SHORT            0    0 ohm load 
 * LPTIARLOAD_10R              1    10 ohm load
 * LPTIARLOAD_30R              2    30 ohm load
 * LPTIARLOAD_50R              3    50 ohm load
 * LPTIARLOAD_100R             4    100 ohm load
 * LPTIARLOAD_1K6              5    Only available when Rtia setting >= 2 kOhm 
 * LPTIARLOAD_3K1              6    Only available when Rtia setting >= 4 kOhm
 * LPTIARLOAD_3K6              7    Only available when Rtia setting >= 4 kOhm
 * 
 * @return: Returns value of the LPTIA load encoded in an integer value
 *****************************************************************************/
int C_DataStorageGeneral::get_LPTIALoadSize(){
    return iLPTIALoadSize_;
}

/******************************************************************************
 * @brief Getter method for returning the Sequence info
 * @returns: Sequence info
 *****************************************************************************/
SEQInfo_Type C_DataStorageGeneral::get_SequenceInfo(int iSequenceID){
    return S_SequenceInfo_[iSequenceID];
}

/******************************************************************************
 * @brief Getter method for returning experiment data from specific position
 * in data storage
 * @param iPosition: Position from which data should be read
 * @returns: returns experiment data from specific position
 *****************************************************************************/
S_DataContainer C_DataStorageGeneral::get_ExperimentData(int iPosition){
    return S_ExperimentData_[iPosition];
}

/******************************************************************************
 * @brief Getter method for returning last position of send data
 * @returns: Last position of send data
 *****************************************************************************/
int C_DataStorageGeneral::get_SendDataCounter(){
    return iSendDataCounter_;
}

/******************************************************************************
 * @brief Getter method for returning pointer to sample buffer
 * @returns: Returns pointer to sample buffer
 *****************************************************************************/
uint32_t * C_DataStorageGeneral::get_SampleBuffer(){
    return arruiSampleBuffer_;
}

#endif /* data_storage_general_CPP */
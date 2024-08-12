/******************************************************************************
 * @brief: Source file containing the Superclass C_Setup from which all other
 * C_Setup_XX setup classes inherit.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef setup_behavior_CPP
#define setup_behavior_CPP

// Include dependencies
#include "setup_behavior.h"

/******************************************************************************
 * @brief Constructor of the super class C_SetupBehavior
 * 
 *****************************************************************************/ 
C_SetupBehavior::C_SetupBehavior(){}

/******************************************************************************
 * @brief Starting method for the super class C_SetupBehavior
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
int C_SetupBehavior::Begin(C_DataSoftwareStorage *){
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Method for calibrating internal LPTIA resistor
 *
 * @return: Error code encoded as integer
 *****************************************************************************/
int C_SetupBehavior::funCalibrateLPTIAResistor(){
    // Variable initialization
    // Calibration result
    fImpPol_Type S_RtiaCalibrationValue;
    // Internal LPTIA Rtia calibration structure
    LPRTIACal_Type S_LPTIARtiaCalibration;

    // Set all members of the structure to 0
    AD5940_StructInit(&S_LPTIARtiaCalibration, sizeof(S_LPTIARtiaCalibration));

    // Chose LPAMP0 because LPAMP1 is only available on ADuCM355
    S_LPTIARtiaCalibration.LpAmpSel = LPAMP0;

    // Return result in magnitude and phase (bTrue) or in real and imaginary
    // part (bFalse)
    S_LPTIARtiaCalibration.bPolarResult = bTRUE;

    // Set frequency of AD5940
    S_LPTIARtiaCalibration.AdcClkFreq = AD5940_SYS_CLOCK_FREQ;

    // Set frequency of the MCU
    S_LPTIARtiaCalibration.SysClkFreq = FEATHER_M0_SYS_CLOCK_FREQ;

    // Set Sinc3 output filter to oversampling rate of 4
    S_LPTIARtiaCalibration.ADCSinc3Osr = ADCSINC3OSR_4;

    // Set Sinc2 output filter to oversampling rate of 22
    S_LPTIARtiaCalibration.ADCSinc2Osr =  ADCSINC2OSR_22;

    // Use at most 2048 DFT (Discrete Fourier Transform) values 
    S_LPTIARtiaCalibration.DftCfg.DftNum = DFTNUM_2048;

    // Sinc + Notch filter 
    S_LPTIARtiaCalibration.DftCfg.DftSrc = DFTSRC_SINC2NOTCH;

    // Use Hann filter smoothing
    S_LPTIARtiaCalibration.DftCfg.HanWinEn = bTRUE;

    // Use 3 periods of signals
    // TODO values unclear
    S_LPTIARtiaCalibration.fFreq = AD5940_SYS_CLOCK_FREQ / 4 / 22 / 2048 * 3;

    // Value of the calibration resistor
    S_LPTIARtiaCalibration.fRcal = AD5940_CAL_RESISTOR;

    // Get size of the LPTIA resistor
    S_LPTIARtiaCalibration.LpTiaRtia = c_DataStorageLocal_->get_LPTIARtiaSize();

    // Get power mode of the low power amplifier
    S_LPTIARtiaCalibration.LpAmpPwrMod = c_DataStorageGeneral_->get_LPAmpPowerMode();

    // No external capacitor connecteted
    S_LPTIARtiaCalibration.bWithCtia = bFALSE;

    // Measuring the internal resistor of the LPTIA with the HSTIA
    AD5940_LPRtiaCal(&S_LPTIARtiaCalibration, &S_RtiaCalibrationValue);

    // Save calibrated value
    c_DataStorageGeneral_->set_RtiaValue(S_RtiaCalibrationValue);
    
    return EC_NO_ERROR;
}

/******************************************************************************
 * @brief Helper method calculating the position of the data in the
 * experiment data array
 * 
 * @param iStepNumber: Current stepnumber, which needs to be translated into the
 *                     correct array position.
 * 
 * Example: SAMPLE BUFFER = 100
 * -> Experiment Data array [0] - [99]
 * 
 * iStepNumber = 0          ->          Array[0]
 * iStepNumber = 1          ->          Array[1]
 * iStepNumber = 99         ->          Array[99]
 * iStepNumber = 100        ->          Array[0]
 * iStepNumber = 603        ->          Array[3]
 * 
 * @return: position in the experiment data array
 *****************************************************************************/
int C_SetupBehavior::funGetDataPosition(int iStepNumber){
    // Check if step number is divisible by the SAMPLE BUFFER
    if (iStepNumber % TRANSMIT_BUFFER == 0){
        return 0;
    }
    else {
        return iStepNumber % TRANSMIT_BUFFER;
    }
}

#endif /* setup_behavior_CPP */
/******************************************************************************
 * @brief: Source file containing the Superclass C_Execute from which all other
 * C_Execute_XX behavior classes inherit.
 * 
 * @author: Mark Jasper
 * @version: V 1.6.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef execute_behavior_CPP
#define execute_behavior_CPP

// Include dependencies
#include "execute_behavior.h"

/******************************************************************************
 * @brief Constructor of the class C_ExecuteBehavior
 * 
 *****************************************************************************/ 
C_ExecuteBehavior::C_ExecuteBehavior(){}

/******************************************************************************
 * @brief Starting method for the super class C_ExecuteBehavior
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * 
 *****************************************************************************/
void C_ExecuteBehavior::Begin(C_DataSoftwareStorage *){}

/******************************************************************************
 * @brief Helper method calculating the position of the data in the
 * experiment data array
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
int C_ExecuteBehavior::funGetDataPosition(int iStepNumber){
    // Check if step number is divisible by the SAMPLE BUFFER
    if (iStepNumber % TRANSMIT_BUFFER == 0){
        return 0;
    }
    else {
        return iStepNumber % TRANSMIT_BUFFER;
    }
}

/******************************************************************************
 * @brief Helper method for converting char array containing experiment type 
 *        into integer for faster calculations.
 * 
 * @return: Experiment type encoded as integer
 *****************************************************************************/
int C_ExecuteBehavior::funGetExperimentTypeInt(char * chrExperimentType){
    // Initialize variables
    int iExperimentType = UNDEFINED_EC_METHOD_I;

    if (strcmp(chrExperimentType, SEQUENCE) == 0){
        iExperimentType = SEQUENCE_I;
    }
    else if (strcmp(chrExperimentType, OCP) == 0){
        iExperimentType = OCP_I;
    }
    else if (strcmp(chrExperimentType, CA) == 0){
        iExperimentType = CA_I;
    }   
    else if (strcmp(chrExperimentType, LSV) == 0){
        iExperimentType = LSV_I;
    }
    else if (strcmp(chrExperimentType, CV) == 0){
        iExperimentType = CV_I;
    }
    else if (strcmp(chrExperimentType, NPV) == 0){
        iExperimentType = NPV_I;
    }
    else if (strcmp(chrExperimentType, DPV) == 0){
        iExperimentType = DPV_I;
    }
    else if (strcmp(chrExperimentType, SWV) == 0){
        iExperimentType = SWV_I;
    }
    return iExperimentType;
}

#endif /* execute_behavior_CPP */
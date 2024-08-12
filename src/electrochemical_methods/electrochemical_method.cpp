/******************************************************************************
 * @brief: Source file for the electrochemical method class, which creates the
 * objects required for the chosen electrochemical method dynamically
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef electrochemical_method_CPP
#define electrochemical_method_CPP

// Include headers
#include "electrochemical_method.h"

/******************************************************************************
 * @brief Constructor of the class C_ElectrochemicalMethod
 * 
 *****************************************************************************/ 
C_ElectrochemicalMethod::C_ElectrochemicalMethod(){}

/******************************************************************************
 * @brief Starting method for the class C_ElectrochemicalMethod
 * @param c_DataSoftwareStorage: Reference to data software storage object
 * @return: return status of executing electrochemical method
 *****************************************************************************/
int C_ElectrochemicalMethod::Begin(C_DataSoftwareStorage * c_DataSoftwareStorage){
    int iErrorCode = 0;

    // Save reference to data software storage object
    c_DataSoftwareStorage_ = c_DataSoftwareStorage;

    // Get reference to data object
    c_DataStorageLocal_ = c_DataSoftwareStorage_->get_DataStorageLocal();

    // Get stored experiment type
    strncpy(chrExperimentType_, c_DataStorageLocal_->get_ExperimentType(), 
        sizeof(chrExperimentType_));

    // Check for experiment type
    // No experiment type stored
    if (strcmp(chrExperimentType_, UNDEFINED_EC_METHOD) == 0){
        // EC-Method unknown
        return EC_SETUP + EC_SE_EC_METHOD_UKNOWN;
    }
    // Chronoamperometry
    else if (strcmp(chrExperimentType_, CA) == 0 && ENABLE_CA){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_CA();
        c_ExecuteBehavior_ = new C_Execute_CA();
    }
    // Open circuit potential
    else if (strcmp(chrExperimentType_, OCP) == 0 && ENABLE_OCP){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_OCP();
        c_ExecuteBehavior_ = new C_Execute_OCP();
    }
    // Linear sweep voltammetry
    else if (strcmp(chrExperimentType_, LSV) == 0 && ENABLE_LSV){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_LSV();
        c_ExecuteBehavior_ = new C_Execute_LSV();
    }
    // Cyclic voltammetry
    else if (strcmp(chrExperimentType_, CV) == 0 && ENABLE_CV){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_CV();
        c_ExecuteBehavior_ = new C_Execute_CV();
    }
    // Normal pulse voltammetry
    else if (strcmp(chrExperimentType_, NPV) == 0 && ENABLE_NPV){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_NPV();
        c_ExecuteBehavior_ = new C_Execute_NPV();
    }
    // Differential pulse voltammetry
    else if (strcmp(chrExperimentType_, DPV) == 0 && ENABLE_DPV){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_DPV();
        c_ExecuteBehavior_ = new C_Execute_DPV();
    }
    // Square wave voltammetry
    else if (strcmp(chrExperimentType_, SWV) == 0 && ENABLE_SWV){
        // Create behavior objects, save references and execute starting method
        c_SetupBehavior_ = new C_Setup_SWV();
        c_ExecuteBehavior_ = new C_Execute_SWV();
    }
    // Selected method not enabled
    else {
        return EC_SETUP + EC_SE_METHOD_DISABLED;
    }

    iErrorCode = c_SetupBehavior_->Begin(c_DataSoftwareStorage_);
    if (iErrorCode != 0) {
        return iErrorCode;
    }
    
    iErrorCode = c_ExecuteBehavior_->Begin(c_DataSoftwareStorage_);
    if (iErrorCode != 0) {
        return iErrorCode;
    }

    delete c_SetupBehavior_;
    delete c_ExecuteBehavior_;

    return EC_NO_ERROR;
}

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for changing execute behavior of ex-method object
 * @param c_ExecuteBehavior: Reference to execute behavior method
 *****************************************************************************/
 void C_ElectrochemicalMethod::set_ExecuteBehavior(C_ExecuteBehavior 
        * c_ExecuteBehavior){
    c_ExecuteBehavior_ = c_ExecuteBehavior;
}

/******************************************************************************
 * @brief Setter method for changing setup behavior of ex-method object
 * @param c_SetupBehavior: Reference to setup behavior method
 *****************************************************************************/
 void C_ElectrochemicalMethod::set_SetupBehavior(C_SetupBehavior 
        * c_SetupBehavior){
    c_SetupBehavior_ = c_SetupBehavior;
}

#endif /* electrochemical_method_CPP */ 
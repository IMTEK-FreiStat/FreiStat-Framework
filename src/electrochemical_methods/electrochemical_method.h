/******************************************************************************
 * @brief: Header file for the electrochemical method class, which creates the
 * objects required for the chosen electrochemical method dynamically
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/

// Include guard
#ifndef electrochemical_method_H
#define electrochemical_method_H

// Include dependencies


// Amperometry
#include "execute_behavior/execute_A/execute_A.h"
#include "setup_behavior/setup_A/setup_A.h"

// Chronoamperometry
#include "execute_behavior/execute_CA/execute_CA.h"
#include "setup_behavior/setup_CA/setup_CA.h"

// Open circuit potential
#include "execute_behavior/execute_OCP/execute_OCP.h"
#include "setup_behavior/setup_OCP/setup_OCP.h"

// Linear sweep voltammetry
#include "execute_behavior/execute_LSV/execute_LSV.h"
#include "setup_behavior/setup_LSV/setup_LSV.h"

// Cyclic voltammetry
#include "execute_behavior/execute_CV/execute_CV.h"
#include "setup_behavior/setup_CV/setup_CV.h"

// Normal pulse voltammetry
#include "execute_behavior/execute_NPV/execute_NPV.h"
#include "setup_behavior/setup_NPV/setup_NPV.h"

// Differential pulse voltammetry
#include "execute_behavior/execute_DPV/execute_DPV.h"
#include "setup_behavior/setup_DPV/setup_DPV.h"

// Square wave voltammetry
#include "execute_behavior/execute_SWV/execute_SWV.h"
#include "setup_behavior/setup_SWV/setup_SWV.h"

/******************************************************************************
 * @brief: Abstract class for electrochemical method class
 * 
 * @author: Mark Jasper
 * @version: V 1.5.0
 * @date: 13.09.2021
 * 
 *****************************************************************************/
class C_ElectrochemicalMethod{
    private:
        // Object references
        C_DataSoftwareStorage * c_DataSoftwareStorage_;
        C_DataStorageLocal * c_DataStorageLocal_;
        C_ExecuteBehavior * c_ExecuteBehavior_;
        C_SetupBehavior * c_SetupBehavior_;

        // Variables
        char chrExperimentType_[4];

    public:
        // Constructor
        C_ElectrochemicalMethod();

        // Starting method
        int Begin(C_DataSoftwareStorage *);

        // Methods
        void setup();

        // Setter 
        void set_ExecuteBehavior(C_ExecuteBehavior *); 
        void set_SetupBehavior(C_SetupBehavior *);

};

#endif  /* electrochemical_method_H */
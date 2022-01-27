/******************************************************************************
 * @brief: File containing all defined structs which are used in the software,
 * except datatypes of the AD5940.h library
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guards
#ifndef datatypes_CPP
#define datatypes_CPP

// Struct containing the experiment data
struct S_DataContainer{
    int iCycle;
    int iMeasurmentPair;
    float fVoltage;
    float fCurrent;
    float fTimeStamp;
};

#endif /* datatypes_CPP */
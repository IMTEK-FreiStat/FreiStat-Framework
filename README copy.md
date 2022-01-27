<img src="FreiStat-Framework.jpg" width="400">

# FreiStat - Potentiostat
Contributors: 
- Mark Jasper
- David Bill 

Supervision:
- Jochen Kieninger


Contacts: 
- Mark Jasper, mark.jasper@imtek.uni-freiburg.de
- Dr. Jochen Kieninger, kieninger@imtek.uni-freiburg.de

[Laboratory for Sensors](https://www.imtek.de/laboratories/sensors/sensors_home?set_language=en), IMTEK, University of Freiburg

<img src="FreiStat.jpg" width="400">

## About FreiStat
The FreiStat is an open-source small scale potentiostat on the basis of the 
AD5941 potentiostat chip of Analog Devices Inc. and an Adafruit Feather M0. 

The FreiStat supports communication over serial USB, WiFi or as a standalone
microcontroller software.

## About
This repository contains the Python framework and the corresponding software for
the microcontroller in C++ to run the FreiStat for different electrochemical
methods.
Is is compatible to all boards with the same formfactor as the Adafruit M0
series and can be compiled using the Arduino IDE or an Arduino extension.
Furthermore, is the code compatible to the AD5940/AD5941 as long as the hardware 
is connected in the same way.

## Hardware
The hardware of the FreiStat consists of the following components:
* Adafruit Feather M0 Wifi
* Custom-PCB for the AD5941 of Analog Devices Inc. with the following Interface:
    - AD5941 GPIO0 connected to A1 (Interrupt Pin)
    - AD5941 GPIO1 connected to A2
    - AD5941 GPIO2 connected to A3
    - AD5941 Reset connected to A4
    - SPI (CS line connected to A5)

## Software
### Microcontroller requirements
* Download the [library files](https://github.com/analogdevicesinc/ad5940libusing) ("ad5940.c" and "ad5940.h")
  with for example the external library installer of the Ardunio IDE.
* Open FreiStat/src/ad5940_FreiStat/ad5940_library_extension
* Go to the definition of the "ad5940.h"
* Select the chip you are using by uncommenting the right line in the "ad5940.h"
```
/** 
 * Select the correct chip.
 * Recommend to define this in your compiler.
 * */
//#define CHIPSEL_M355      /**< ADuCM355 */
#define CHIPSEL_594X      /**< AD5940 or AD5941 */
```

### Python library requirements
* Requirements are installed automatically when installing the package via pip install
* Requirements can be installed manually by executing:
  "pip install requirements.txt"

## Electrochemical methods
The FreiStat software allows for running different electrochemical methods by 
sending JSON-telegrams to the microcontroller. The supported ec-methods are:
* Chronoamperometry (CA) 
* Linear Sweep Voltammetry (LSV)
* Cyclic Voltammetry (CV)
* Normal Pulse Voltammetry (NPV)
* Differential Pulse Voltammetery (DPV)
* Square Wave Voltammetery (SWV)
* Arbitrary sequences of the above stated electrochemical methods

### JSON-telegrams
The FreiStat operates by transmitting JSON-telegrams bidirectional.
The following telegrams are defined:

1. Command telegrams:
    1. Sending the experiment type                      PL  -> MCU
        - CV example: {"C":1,"ExT": "CV"}
        - CA example: {"C":1,"ExT": "CA"}

    2. Sending the experiment parameters                PL  -> MCU
        - CV example: {"C":2,"ExP":{"pSP":500.4395,"pLP":-620.0,"pUP":1050.0,"pSZ":2.0,"pSR":200.0,"pC":1,"pLRS":11,"pFWP":1,"pMF":0,"pOS2":1,"pOS3":2}}
        - CA example: {"C":2,"ExP": {"pPS":[-100.1953, 100.1953, -200.1221],"pPL":[1000.0, 500.0, 1000.0],"pSAR":5.0,"pC":3,"pLRS":3,"pMF":0,"pOS2":7,"pOS3":1}}

    3. Sending operating commands: Start/ Stop/ Wait    PL <-> MCU
        - Start: {"C":3,"ExC": "Start"}
        - Stop: {"C":3,"ExC":"Stop"}

    4. Sending enable/ disable of sequence mode         PL  -> MCU
        - Enable sequence: {"C":4,"ExS": "SE"}
        - Disable sequence: {"C":4,"ExS": "SD"}

2. Acknowledge telegrams:
    - Example: {"A":1}

3. Data telegrams:
    - Example: {"R":1,"M":{"D":180,"V":889.40173,"C":8.88046,"T":14451}}

PL  = Pyhon Library
MCU = Microcontroller

### Parametrization of the electrochemical methods
All ec-methods are parametrized in the Python library when calling the facade
of the chosen method.

All parameters are explained in the function calls of the methods, but the 
different parameters of the different ec-methods are also additionally listed
below.

General limits of operation:

- FixedWEPotential: True -> Voltage range = 2.1 V
- FixedWEPotential: Flase -> Voltage range = ±2.0 V
- Max. current = ±4.5 mA 

### Chronoamperometry
Chronoamperometry with a defined sampling rate of values, independent of the 
chosen pulse length. After the sampling time is over, the data is send in a JSON 
frame (see section about JSON-telegrams) to the Python library  where it is 
stored and displayed.

Parameters:
- Potential_Steps : List of potential steps in V (limited to 50 values)
- Pulse_Lengths : List of pulse lenghts in s (need to match length of potential step list)
- Sampling_Rate : Sampling rate in s
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- Sampling rate > 3 ms
- Amount of distinct steps = 50

### Linear Sweep Voltammetry
Linear sweep voltammetry with current averaging. After each voltage step the data is
send in a JSON frame (see section about JSON-telegrams) to the Python library 
where it is stored and displayed. 
Supports sweeping up or down, depending on the chosen values for the StartVoltage
and StopVoltage.

Parameters:
- StartVoltage : Starting potential in V at which the LSV should start sweeping
- StopVoltage : Stop potential in V at which the LSV should stop sweeping
- Stepsize : Stepsize in V (min 537 uV)
- Scanrate : Scanrate in V/s 
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- Stepsize / Scanrate > 2.875 ms


### Cyclic Voltammetry
Cyclic voltammetry with current averaging. After each voltage step the data is
send in a JSON frame (see section about JSON-telegrams) to the Python library 
where it is stored and displayed.
Supports sweeping up or down, depending on the chosen values for the FirstVertex and SecondVertex: 
- Sweeping down : FirstVertex < SecondVertex
- Sweeping up : SecondVertex < FirstVertex


Parameters:
- StartVoltage : Starting potential in V at which the CV should start ramping up
- FirstVertex : Potential of the first vertex in V
- SecondVertex : Potential of the second vertex in V
- Stepsize : Stepsize in V (min 537 uV)
- Scanrate : Scanrate in V/s 
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- Stepsize / Scanrate > 2.875 ms


### Normal Pulse Voltammetry
After each voltage step the data is send in a JSON frame (see section about JSON-telegrams) to the Python library where it is stored and displayed.

Parameters:
- BaseVoltage : Base Potential which the FreiStat should apply in V
- StartVoltage : Potential of the first peak in V
- StopVoltage : Potential of the last peak in V
- DeltaV_Staircase : Increament per peak in V
- Pulse_Lengths [Tau' Tau] : Pulse lengths in s
- Sampling_Duration : Time defining how long data should be sampled at the end of each step in s.
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- Tau and Tau' > 3 ms
- Sinc2_Oversampling * Sinc3_Oversampling / 800 000 Hz < Sampling_Duration

### Differential Pulse Voltammetry
After each voltage step the data is send in a JSON frame (see section about JSON-telegrams) to the Python library where it is stored and displayed.

Parameters:
- StartVoltage : Potential at which the DPV should start in V
- StopVoltage : Potential at which the DPV should stop in V
- DeltaV_Staircase : Increament of the underlying staircase in V
- DeltaV_Peak : Size of the peak in V
- Pulse_Lengths [Tau' Tau] : Pulse lengths in s
- Sampling_Duration : Time defining how long data should be sampled at the end of each step in s.
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- Tau and Tau' > 3 ms
- Sinc2_Oversampling * Sinc3_Oversampling / 800 000 Hz < Sampling_Duration

### Square Wave Voltammetry
After each voltage step the data is send in a JSON frame (see section about JSON-telegrams) to the Python library where it is stored and displayed.

Parameters:
- StartVoltage : Potential at which the SWV should start in V
- StopVoltage : Potential at which the SWV should stop in V
- DeltaV_Staircase : Increament of the underlying staircase in V
- DeltaV_Peak : Size of the peak in V
- DutyCycle : Length of the duty cycle in s
- Sampling_Duration : Time defining how long data should be sampled at the end of each step in s.
- Cycle : Amount of cycles
- CurrentRange : Current range at which the potentiostat should operate in A
- FixedWEPotential : Use a fixed potential for the working electrode
- MainsFilter : Actiavete the 50/ 60 Hz mains filter
- Sinc2_Oversampling : Oversampling rate for Sinc2
- Sinc3_Oversampling : Oversampling rate for Sinc3
- EnableOptimizer : Enable the optimzier which tunes the parameters
- LowPerformanceMode : Disabling the plotting of the data

Limits of operation:

- DutyCycle / 2 > 3 ms
- Sinc2_Oversampling * Sinc3_Oversampling / 800 000 Hz < Sampling_Duration

### Sequence of electrochemical methods
The sequnece mode of the FreiStat allows for chaining different electrochemical.
methods after each other which are then executed in order for a defined amount
of cycles.

Parameters:
- SequenceCycles : Amount of cycles

Limits of operation:
- The sequence can't exceed a length of 35 due to the limited memory of the
  Microcontroller

## How to use the code
* Follow the instructions for the reqiurements of the microcontroller and the
  Python library
* Compile and upload the microcontroller code using the Arduino IDE or a similar
  program using the Arduino Extension.
* Include Python library into your project
* Import a ec-method into your project 
  ```
  from FreiStat.Methods.run_chronoamperometry import Run_CA

  Run_CA().start(...)
  ```
* Run your Python script with the parameters you want to use (see
  Examples.py for how to implement each method)

## Acknowledgement
The code is loosely based on the ideas of the example code of the AD5941 
[example repository](https://github.com/analogdevicesinc/ad5940-examples), of 
Analog Devices Inc.
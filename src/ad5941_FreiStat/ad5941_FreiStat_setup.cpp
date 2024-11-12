/******************************************************************************
 * @brief: Source file for the the setup of the AD5941/5940 chip.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/

// Include guard
#ifndef ad5940_FreiStat_setup_CPP
#define ad5940_FreiStat_setup_CPP

// Include header
#include "ad5941_FreiStat_setup.h"

// Define static variable
bool C_AD5940_Setup::bInterruptOccured_ = false;

/******************************************************************************
 * @brief Constructor of the class C_AD5940_Setup
 * 
 *****************************************************************************/ 
C_AD5940_Setup::C_AD5940_Setup(){}

/******************************************************************************
 * @brief Starting method for the class C_AD5940_Setup
 * 
 *****************************************************************************/
void C_AD5940_Setup::Begin(){
    // Intialize variables
    bInterruptOccured_ = false;

    // Intialize SPI, GPIOs and Interrupts
    this->funAD5940_InitMCU();

    // Configure the AD5940 chip
    this->funAD5940_Config();
}

/******************************************************************************
 * @brief Setup method for initializing SPI and GPIOs
 * @details This method is used to initialize the SPI connection between the
 * MCU and the AD5940 chip. Furthermore the General Purpose Input/ Outputs
 * (GPIOs) are intializied:
 * 
 * Pins intialized   :       Meaning/ Purpose
 * SPI_CS_AD5940_Pin : Analog Output A1 of MCU is connected to GPIO0 of AD594x       
 * AD5940_ResetPin   : Analog Output A4 of MCU is connected to Reset pin of AD594x
 * AD5940_IntPin     : Analog Output A5 of MCU is connected to CS pin of AD594x    
 * 
 *****************************************************************************/
void C_AD5940_Setup::funAD5940_InitMCU(){
    // Start SPI connection
    SPI.begin();

    // Initialize pins of the MCU
    // Set analog pin 2 as input to allow AD5940 to control the connected LED
    pinMode(FEATHER_M0_AFE_SLEEP_LED, INPUT_PULLUP); 

    // SPI chip select pin
    pinMode(FEATHER_M0_SPI_CS_Pin, OUTPUT);
    
    // AD5940 reset pin
    pinMode(FEATHER_M0_ResetPin, OUTPUT);

    // Initialize GPIO
    // AD5940 interrupt pin to feather m0 interrupt input
    // GPIO0, GPIO3 and GPIO5 can be configured for the INT0 output
    // GPIO4 and GPIO7 can be configured for the INT1 output
    pinMode(FEATHER_M0_IntPin, INPUT_PULLUP);

    // Attach ISR to pin A1 with falling edge
    // Falling edge is used since we use INPUT_PULLUP as configuration for A1
    attachInterrupt(digitalPinToInterrupt(FEATHER_M0_IntPin), 
                    this->funExt_Int0_Handler, 
                    FALLING);

    // Set chip select to enable the AD5940 to receive SPI commands when the
    // signal is pulled down to low later
    AD5940_CsSet();

    // Set reset pin for the AD5940 (Beware that the reset pin is negated!)
    AD5940_RstSet();
}

/******************************************************************************
 * @brief Setup method for configuring the AD5940
 * @details This method is used to configure the Analog Front End (AFE). The
 * configuration is separated into different sections:
 * 
 * 1. Configure system clock
 * 2. Configure FIFO
 * 3. Configure sequencer
 * 4. Configure interrupt controller (interrupt pins where defined before)
 * 5. Configure GPIOs to match hardware configuration
 * 6. Calibrate LFOSC frequency (LFOSC = low frequency oscillator) 
 * 
 *****************************************************************************/
void C_AD5940_Setup::funAD5940_Config(){
    // Hardware reset of the AD5940 by clearing and setting the previous defined
    // resetpin
    AD5940_HWReset();

    // Reinitialize the AD5940 after each hardware reset
    AD5940_Initialize();

    // Begin configuration of the AD5940/ AD5941

    /**************************************************************************/
    // 1. Configure system clock
    // Enable the internal 16 MHz / 32 MHz oscillator
    S_ClockConfig_.HFOSCEn = bTRUE;

    // Set the high frequency oscillator to 16 MHz 
    S_ClockConfig_.HfOSC32MHzMode = bFALSE; 

    // Enable the internal 32 KHz oscillator
    S_ClockConfig_.LFOSCEn = bTRUE;

    // Disable the usage of an external 16 MHz or 32 MHz oscillator
    S_ClockConfig_.HFXTALEn = bFALSE;

    // Set source of system clock to the internal high frequency oscillator
    S_ClockConfig_.SysClkSrc = SYSCLKSRC_HFOSC;  

    // Set system clock devider according to chosen frequency
    S_ClockConfig_.SysClkDiv = SYSCLKDIV_1;

    // Set the clock sourse of the ADC/DAC converters to the internal 
    // high frequency oscillator
    S_ClockConfig_.ADCCLkSrc = ADCCLKSRC_HFOSC;

    // Set system clock devider for ADC/DAC according to chosen frequency
    S_ClockConfig_.ADCClkDiv = ADCCLKDIV_1;

    // Configure the system clock
    AD5940_CLKCfg(&S_ClockConfig_);

    /**************************************************************************/
    // 2. Configure FIFO
    // Set FIFO to FIFO mode (instead of Stream mode)
    // FIFO mode will discard new data if FIFO is full
    // Stream mode will discard old data if FIFO is full
    S_FiFoConfig_.FIFOMode = FIFOMODE_FIFO;

    // Define FIFO size. Possible sizes are 2kB, 4kB, 6kB 
    // Keep in mind that the sequencer and the FIFO share the same SRAM (6kB)
    S_FiFoConfig_.FIFOSize = FIFOSIZE_2KB; 

    // Set the data source for the FIFO to the SINC3 Filter (50 Hz/ 60 Hz 
    // surpression) which gets the data from the ADC
    S_FiFoConfig_.FIFOSrc = FIFOSRC_SINC3;

    // Set FIFO threshold to 100 (Default). This value is not neccessary since 
    // the software doesn't use the FIFO interrupt.
    S_FiFoConfig_.FIFOThresh = AD5940_FIFO_THRESHOLD;          

    // Enable FIFO
    S_FiFoConfig_.FIFOEn = bTRUE;

    // Configure the FIFO 
    AD5940_FIFOCfg(&S_FiFoConfig_);
    
    /**************************************************************************/
    // 3. Configure sequencer
    // Allocate remaining SRAM to sequencer (4 kB)
    S_SequencerConfig_.SeqMemSize = SEQMEMSIZE_4KB;     

    // Clear count and CRC register of the sequencer
    S_SequencerConfig_.SeqCntCRCClr = bTRUE;    

    // Set wait time of the sequencer to 0 clock ticks
    S_SequencerConfig_.SeqWrTimer = 0;          

    // Disable sequencer. Sequencer gets enabled when data is written into it. 
    S_SequencerConfig_.SeqEnable = bFALSE;   

    // Initialize unused parameters
    S_SequencerConfig_.SeqBreakEn = bFALSE;
    S_SequencerConfig_.SeqIgnoreEn = bFALSE;

    // Configure the sequencer
    AD5940_SEQCfg(&S_SequencerConfig_);

    /**************************************************************************/
    // 4. Configure interrupt controller
    // Configure interrupt controller 0
    // Enable the following interrupts:
    AD5940_INTCCfg(AFEINTC_0, 
                   AFEINTSRC_DATAFIFOTHRESH | /* FIFO threshold reached     */
                   AFEINTSRC_DATAFIFOOF |     /* FIFO overflow              */
                   AFEINTSRC_CUSTOMINT0 |     /* Custom interrupt source 0  */
                   AFEINTSRC_CUSTOMINT1 |     /* Custom interrupt source 1  */
                   AFEINTSRC_GPT1INT_TRYBRK | /* General purpose timer 1    */
                   AFEINTSRC_ENDSEQ,          /* End of sequnece interrupt  */
                   bTRUE);
   
    // Clear all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    // Configure interrupt controller 1
    // Enable all interrupts so we can use the interrupt controller 1 to test if
    // our interrupts work properly.
    AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);

    // Clear all interrupt flags
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    /**************************************************************************/
    // 5. Configure GPIOs to match hardware configuration
    // Define functionality of the GPIO pins
    S_GPIOConfig_.FuncSet = GP0_INT  |  /* Interrupt pin for InterController 0*/
                            GP1_GPIO |  /* Indicates sleeping state of AD5940 */
                            GP2_SYNC;   /* Sequencer controls GPIO2 output level        
                                           and shows sampling process */

    // Disable input of all pins. Communication is soly done via SPI
    S_GPIOConfig_.InputEnSet = 0;

    // Enable output of GPIO 0, GPIO 1 and GPIO 2
    S_GPIOConfig_.OutputEnSet = AGPIO_Pin0 | 
                                AGPIO_Pin1 | 
                                AGPIO_Pin2;

    // Set output of GPIO 1 to high to turn off LED 
    // (Other side of LED is connected to Pullup_Input)
    S_GPIOConfig_.OutVal = AGPIO_Pin1;

    // Diable pullup/ pulldown on all GPIOs
    S_GPIOConfig_.PullEnSet = 0;

    // Configure the GPIOs
    AD5940_AGPIOCfg(&S_GPIOConfig_);

    /**************************************************************************/
    // 6. Calibrate LFOSC frequency
    // Manufacturer note: calibrate LFOSC using system clock.  The system clock 
    // accuracy decides measurement accuracy. Use XTAL to get better result.
    // Define time for calibration of LFOSC
    S_LFOSCMeasure_.CalDuration = AD5940_LFOSC_CAL_TIME;

    // Since sequencer is empty, use first adress for calibration commands
    S_LFOSCMeasure_.CalSeqAddr = 0;        

    // Check system frequency at SYS_CLOCK definition section of this method
    S_LFOSCMeasure_.SystemClkFreq = FEATHER_M0_SYS_CLOCK_FREQ / 3;

    // Measure LFSOC frequendy and store it in variable
    AD5940_LFOSCMeasure(&S_LFOSCMeasure_, &fLFOSCFrequency_);

    // Unlock SEQTRGSLP register to enable sleep mode of the AD5940
    AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);                                
}

/******************************************************************************
 * @brief Interrupt handler for analog pin A5
 *****************************************************************************/
void C_AD5940_Setup::funExt_Int0_Handler(){
    // Set interrupt flag
    bInterruptOccured_ = true;
}

/******************************************************************************
 * Setter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Setter method for setting interrupt flag
 * @param bInterruptOccured: Set/ reset interrupt flag
 *****************************************************************************/
void C_AD5940_Setup::set_InterruptOccured(bool bInterruptOccured){
    bInterruptOccured_ = bInterruptOccured;
}

/******************************************************************************
 * Getter methods
 *****************************************************************************/
/******************************************************************************
 * @brief Getter method for returning measured LFOSC frequency
 * @return: fLFOSCFrequency_
 *****************************************************************************/
float C_AD5940_Setup::get_LFOSCFrequency(){
    return fLFOSCFrequency_;
}

/******************************************************************************
 * @brief Getter method for returning interrupt flag
 * @return: interrupt flag
 *****************************************************************************/
bool C_AD5940_Setup::get_InterruptOccured(){
    return bInterruptOccured_;
}
#endif /* ad5940_FreiStat_setup_CPP */
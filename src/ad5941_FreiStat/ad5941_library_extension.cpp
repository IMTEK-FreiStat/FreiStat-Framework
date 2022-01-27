/******************************************************************************
 * @brief: C++-file containing required functions which need to be implemented
 * as an extension to enable work with the AD5941/5940 library from Analog 
 * Devides Inc.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 * As defined in the library of Anlog Devices Inc. are the following functions
 * required in order for the library to work properly.
 * 
 *  AD5940_CsSet
 *  AD5940_CsClr
 *  AD5940_RstSet
 *  AD5940_RstClr
 *  AD5940_Delay10us
 *  AD5940_ReadWriteNBytes
 * 
 *****************************************************************************/

// Include guard
#ifndef ad5940_library_extension_C
#define ad5940_library_extension_C

// Include dependencies
// Include C-files
extern "C" { 
#include <ad5940.h> 
}

// Include other dependencies
#include <SPI.h>
#include "../data_storage/constants.cpp"

static const SPISettings SPISetting(FEATHER_M0_SYS_CLOCK_FREQ / 4, MSBFIRST, SPI_MODE0);

/******************************************************************************
 * @brief Method to deselect the chip select function of the AD5940
 * 
 *****************************************************************************/
void AD5940_CsSet(void){
  digitalWrite(FEATHER_M0_SPI_CS_Pin, HIGH);
}

/******************************************************************************
 * @brief Method to select the chip select function of the AD5940
 * 
 *****************************************************************************/
void AD5940_CsClr(void){
  digitalWrite(FEATHER_M0_SPI_CS_Pin, LOW);
}

/******************************************************************************
 * @brief Method to reset the AD5940
 * 
 *****************************************************************************/
void AD5940_RstSet(void){
  digitalWrite(FEATHER_M0_ResetPin, HIGH);
}

/******************************************************************************
 * @brief Method to clear the reset on the AD5940
 * 
 *****************************************************************************/
void AD5940_RstClr(void){
  digitalWrite(FEATHER_M0_ResetPin, LOW);
}

/******************************************************************************
 * @brief Method to for delaying the AD5940 for a multiple time of 10 micro
 * seconds
 * @details Check if time is larger than 1638.3
 * According to the Arduino specification the largest values for 
 * delayMicroseconds() to be accurate is 16383 micro seconds. For larger
 * values delay() in combination with delayMicroseconds() is used.
 * 
 * @param time: multiplier which defines the amount of times the AD5940 should
 * be delay by 10 micro seconds
 * 
 *****************************************************************************/
void AD5940_Delay10us(uint32_t iTime){
  // Value is smaller threshold
  if (iTime < 1638){
    delayMicroseconds(iTime * 10);
  }
  // Value is larger than threshold
  else if (iTime >= 1638){
    uint32_t iTimeDelayMicro = iTime % 1000;
    uint32_t iTimeDelay = iTime - iTimeDelayMicro;
    delay(iTimeDelay / 100);
    delayMicroseconds(iTimeDelayMicro * 10);
  }
}

/******************************************************************************
 * @brief Method to to write and read data from the SPI port
 * @details: The timing of the SPI must coincide with the frequencies of the
 * MCU and the AD5940.
 * System clock frequency for the AD5940 was chosen as 16 MHz.
 * System clock frequency for the MCU is for the Adafruit Feather M0 48 MHz.
 * 
 * For the SPI to work the clock must be equal to the system clock of the MCU
 * devided by a multiply of 2, while staying below the system clock of the
 * AD5940. 
 * The highest possible value is therfore 12 MHz.
 * 
 * Therefore every 83.3333 ns an SPI clock pulse is send.
 * To send one byte 666.666 ns are required.
 * 
 * As send mode the most significant bit is send first: MSBFIRST
 * 
 * The SPI operates in mode 0: 
 * https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase
 * 
 * @param pSendBuffer: Send buffer holding all data that should be send via SPI
 * to the AD5940
 * @param pRecvBuff: Receive buffer storing all data transmitted by the AD5940
 * to the MCU via SPI.
 * @param length: Length of transmitted data
 * 
 *****************************************************************************/
void AD5940_ReadWriteNBytes(unsigned char *pSendBuffer, 
                            unsigned char *pRecvBuff, 
                            unsigned long length){
  // Begin SPI transaction with previously descripted settings
  SPI.beginTransaction(SPISetting);
  
  // Repeat sending until length of data is reached
  for (int i = 0; i < length; i++)
  {
    // Transfer data bidirectional, which means that data is read and written
    // at the same time.
    *pRecvBuff++ = SPI.transfer(*pSendBuffer++);
  }
  // End transaction
  SPI.endTransaction();       
}

#endif /* ad5940_library_extension_C */
"""
Module containing different encoder functions which are used in the library.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging

# Import internal dependencies
from ..Data_storage.constants import *

def _encode_Bool_Flag(bBoolFlag : bool) -> int:
    """
    Description
    -----------
    Method encoding a bool value into an integer flag.

    Parameters
    ----------
    `bBoolFlag` : bool
        Boolean flag which should be converted into an integer value

    Return
    ------
    `iEncodedFlag` : int
        Boolean flag encoded as integer (0 : True | 1 : False)

    """
    if (bBoolFlag == True):
        return 1
    elif (bBoolFlag == False):
        return 0

def _encode_Sinc_Oversampling_Rate(strFilterName: str, 
                                   iOversampling: int,
                                   logger : logging.Logger) -> int:
    """
    Description
    -----------
    Method encoding the oversampling rate of the chosen filter in an integer
    value which can be interpreted by the FreiStat.
        
    Parameters
    ----------
    `strFilterName` : string 
        Name of the filter which should be encoded
        Defiend: [Sinc2, Sinc3]

    `iOversampling` : int
        Chosen oversampling rate

    `logger` : logging.Logger
        Logger which should be used in the library

    Return
    ------
    `iOversampling` : int
        Oversampling rate encoded as integer value.

    """
    # Check if oversampling value is negative
    if (iOversampling < 0):
        logger.warning("Oversampling rate is negative." + 
                        " Using positive value instead")

        # Set value positive
        iOversampling = -iOversampling

    # Check which filter was chosen
    if (strFilterName == "Sinc2"):
        # Create a list with the defined Sinc2 oversampling rates
        listOversamplingRates: list = [ADCSINC2OSR_22_VALUE,
                                       ADCSINC2OSR_44_VALUE,
                                       ADCSINC2OSR_89_VALUE,
                                       ADCSINC2OSR_178_VALUE,
                                       ADCSINC2OSR_267_VALUE,
                                       ADCSINC2OSR_533_VALUE,
                                       ADCSINC2OSR_640_VALUE,
                                       ADCSINC2OSR_667_VALUE,
                                       ADCSINC2OSR_800_VALUE,
                                       ADCSINC2OSR_889_VALUE,
                                       ADCSINC2OSR_1067_VALUE,
                                       ADCSINC2OSR_1333_VALUE]

        # Create a list with defined Sinc2 oversampling rates integers
        listOversamplingIntegers: list = [ADCSINC2OSR_22,
                                          ADCSINC2OSR_44,
                                          ADCSINC2OSR_89,
                                          ADCSINC2OSR_178,
                                          ADCSINC2OSR_267,
                                          ADCSINC2OSR_533,
                                          ADCSINC2OSR_640,
                                          ADCSINC2OSR_667,
                                          ADCSINC2OSR_800,
                                          ADCSINC2OSR_889,
                                          ADCSINC2OSR_1067,
                                          ADCSINC2OSR_1333]

        # Check if oversampling rate is between 22 and 1333
        if (iOversampling < ADCSINC2OSR_22_VALUE):
            logger.warning("Oversampling rate below lower bound." +
                            " Oversampling rate set to 22.")
            return ADCSINC2OSR_22

        elif (iOversampling > ADCSINC2OSR_1333_VALUE):
            logger.warning("Oversampling rate above uppper bound." +
                            " Oversampling rate set to 1333")
            return ADCSINC2OSR_1333

    elif (strFilterName == "Sinc3"):
        # Create a list with the defined Sinc3 oversampling rates
        listOversamplingRates: list = [ADCSINC3OSR_DISABLED_VALUE,
                                       ADCSINC3OSR_5_VALUE,
                                       ADCSINC3OSR_4_VALUE,
                                       ADCSINC3OSR_2_VALUE]

        # Create a list with defined Sinc3 oversampling rates integers
        listOversamplingIntegers: list = [ADCSINC3OSR_DISABLED,
                                          ADCSINC3OSR_5,
                                          ADCSINC3OSR_4,
                                          ADCSINC3OSR_2]

        # Check if oversampling rate is between 0 and 5
        if (iOversampling < ADCSINC3OSR_DISABLED_VALUE):
            logger.warning("Oversampling rate below lower bound." +
                            " Disable Sinc3 filter.")
            return ADCSINC3OSR_DISABLED

        elif (iOversampling > ADCSINC3OSR_5_VALUE):
            logger.warning("Oversampling rate above uppper bound." +
                            " Oversampling rate set to 5 (not recommended")
            return ADCSINC3OSR_5
        
    else:
        # Error: Defined filter not known
        return -1

    #  Check if value is corresponding to a oversampling
    for i in range (len(listOversamplingRates)):
        if (iOversampling == listOversamplingRates[i]):
            return listOversamplingIntegers[i]

    # Find nearest value for given oversampling rate
    for i in range (len(listOversamplingRates) - 1):
        if (iOversampling >= listOversamplingRates[i] and 
            iOversampling <= listOversamplingRates[i + 1]):

            iLowerDifferenz = iOversampling - listOversamplingRates[i]
            iUpperDifferenz = listOversamplingRates[i + 1] - iOversampling

            if (iLowerDifferenz < iUpperDifferenz):
                logger.warning("Exact value not possible (" + 
                                str(iOversampling) + 
                                "), used closest value (" +
                                str(listOversamplingRates[i]) + 
                                ") instead.")
                return listOversamplingIntegers[i]                    
            else:
                logger.warning("Exact value not possible (" + 
                                str(iOversampling) + 
                                "), used closest value (" +
                                str(listOversamplingRates[i + 1]) + 
                                ") instead.")
                return listOversamplingIntegers[i + 1]

def _encode_LPTIA_Resistor_Size(iLPTiaResistorSize : int,
                                logger : logging.Logger) -> int:
    """
    Description
    -----------
    Method encoding the size of the low power transimpendance amplifier
    resistor into an integer value which can be interpreted by the FreiStat.
        
    Parameters
    ----------
    `iLPTiaResistorSize` : int 
        Size of the LPTia resisitor in ohm. Range 200 - 512000 ohm.

    `logger` : logging.Logger
        Logger which should be used in the library

    Return
    ------
    `iLPTiaSize` : int
        Size of the LPTia encoded as integer value. Range: 0-26

    """
    # Check if LPTIA resistor should be disabled
    if (iLPTiaResistorSize == LPTIARTIA_OPEN_VALUE):
        logger.warning("LPTIA resistor is set to 0 Ohm.")
            
        return LPTIARTIA_OPEN

    # Check if resistor value is negative
    if (iLPTiaResistorSize < 0):
        logger.warning("LPTIA resistor value is negative." + 
                        " Using positive value instead")

        # Set value positive
        iLPTiaResistorSize = -iLPTiaResistorSize
        
    # Check if resistor is between 200 and 512000 ohm
    if (iLPTiaResistorSize < LPTIARTIA_200R_VALUE):
        logger.warning("LPTIA resistor value below lower bound." +
                        " LPTIA resistor value set to 200 Ohm")
        return LPTIARTIA_200R

    elif (iLPTiaResistorSize > LPTIARTIA_512K_VALUE):
        logger.warning("LPTIA resistor value above uppper bound." +
                        " LPTIA resistor value set to 512 kOhm")
        return LPTIARTIA_512K


    # Create a list with defined LPTIA Rtia values
    listLPTiaResistorSizes: list = [LPTIARTIA_200R_VALUE,
                                    LPTIARTIA_1K_VALUE,
                                    LPTIARTIA_2K_VALUE,
                                    LPTIARTIA_3K_VALUE,
                                    LPTIARTIA_4K_VALUE,
                                    LPTIARTIA_6K_VALUE,
                                    LPTIARTIA_8K_VALUE,
                                    LPTIARTIA_10K_VALUE,
                                    LPTIARTIA_12K_VALUE,
                                    LPTIARTIA_16K_VALUE,
                                    LPTIARTIA_20K_VALUE,
                                    LPTIARTIA_24K_VALUE,
                                    LPTIARTIA_30K_VALUE,
                                    LPTIARTIA_32K_VALUE,
                                    LPTIARTIA_40K_VALUE,
                                    LPTIARTIA_48K_VALUE,
                                    LPTIARTIA_64K_VALUE,
                                    LPTIARTIA_85K_VALUE,
                                    LPTIARTIA_96K_VALUE,
                                    LPTIARTIA_100K_VALUE,
                                    LPTIARTIA_120K_VALUE,
                                    LPTIARTIA_128K_VALUE,
                                    LPTIARTIA_160K_VALUE,
                                    LPTIARTIA_196K_VALUE,
                                    LPTIARTIA_256K_VALUE,
                                    LPTIARTIA_512K_VALUE
    ]

    # Create a list with defined LPTIA Rtia integers
    listLPTiaResistorInteger: list = [LPTIARTIA_200R,
                                      LPTIARTIA_1K,
                                      LPTIARTIA_2K,
                                      LPTIARTIA_3K,
                                      LPTIARTIA_4K,
                                      LPTIARTIA_6K,
                                      LPTIARTIA_8K,
                                      LPTIARTIA_10K,
                                      LPTIARTIA_12K,
                                      LPTIARTIA_16K,
                                      LPTIARTIA_20K,
                                      LPTIARTIA_24K,
                                      LPTIARTIA_30K,
                                      LPTIARTIA_32K,
                                      LPTIARTIA_40K,
                                      LPTIARTIA_48K,
                                      LPTIARTIA_64K,
                                      LPTIARTIA_85K,
                                      LPTIARTIA_96K,
                                      LPTIARTIA_100K,
                                      LPTIARTIA_120K,
                                      LPTIARTIA_128K,
                                      LPTIARTIA_160K,
                                      LPTIARTIA_196K,
                                      LPTIARTIA_256K,
                                      LPTIARTIA_512K
    ]

    #  Check if value is corresponding to a defined value for Rtia
    for i in range (len(listLPTiaResistorSizes)):
        if (iLPTiaResistorSize == listLPTiaResistorSizes[i]):
            return listLPTiaResistorInteger[i]

    # Find nearest value for given resistor size
    for i in range (len(listLPTiaResistorSizes) - 1):
        if (iLPTiaResistorSize >= listLPTiaResistorSizes[i] and 
            iLPTiaResistorSize <= listLPTiaResistorSizes[i + 1]):

            iLowerDifferenz = iLPTiaResistorSize - listLPTiaResistorSizes[i]
            iUpperDifferenz = listLPTiaResistorSizes[i + 1] - iLPTiaResistorSize

            if (iLowerDifferenz < iUpperDifferenz):
                logger.warning("Exact value not possible (" + 
                                str(round(iLPTiaResistorSize)) + 
                                " Ohm), used closest value (" +
                                str(listLPTiaResistorSizes[i]) + 
                                " Ohm) instead.")
                iLPTiaResistorSize = listLPTiaResistorInteger[i]                    
            else:
                logger.warning("Exact value not possible (" + 
                                str(round(iLPTiaResistorSize)) + 
                                " Ohm), used closest value (" +
                                str(listLPTiaResistorSizes[i + 1]) + 
                                " Ohm) instead.")
                iLPTiaResistorSize = listLPTiaResistorInteger[i + 1]

            return iLPTiaResistorSize
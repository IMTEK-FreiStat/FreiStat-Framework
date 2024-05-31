"""
Module containing different decoder functions which are used in the library.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies

# Import internal dependencies
from ..data_storage.constants import *


def _decode_SincXOSR(iSincXOSR: int, strSincName: str) -> int:
    """
    Description
    -----------
    Method decoding the integer encoded oversampling rates back into the actual
    values.

    Parameters
    ----------
    `iSincXOSR` : int
        Integer encoded oversampling rate of Sinc2 / Sinc3 filter

    `strSincName` : string
        String containing the name abbreviation of the according sinc filter

    Return
    ------
    `iSincXOSRSize` : int
        Decoded oversampling rate of Sinc2 / Sinc3 filter

    """
    # Check if Sinc2 oder Sinc3 filter is used
    if strSincName == SINC2_OVERSAMPLING:
        # Initialize list
        listOversamplingRates: list = [
            ADCSINC2OSR_22_VALUE,
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
            ADCSINC2OSR_1333_VALUE,
        ]
        # Return decoded value
        return listOversamplingRates[iSincXOSR]

    elif strSincName == SINC3_OVERSAMPLING:
        # Initialize list
        listOversamplingRates: list = [
            ADCSINC3OSR_5_VALUE,
            ADCSINC3OSR_4_VALUE,
            ADCSINC3OSR_2_VALUE,
        ]
        # Return decoded value
        return listOversamplingRates[iSincXOSR]
    else:
        # Error occured
        return -1


def _decode_LPTIA_Resistor_Size(iLPTiaSize: int) -> int:
    """
    Description
    -----------
    Method decoding the size of the low power transimpendance amplifier
    resistor from an integer value back into the value in ohm.

    Parameters
    ----------
    `iLPTiaSize` : int
        Size of the LPTia encoded as integer value. Range: 0-26

    Return
    ------
    `iLPTiaResistorSize` : int
        Size of the LPTia resisitor in ohm. Range 200 - 512000 ohm.

    """
    # Create list with defined LPTIA Rtia values
    listLPTiaResistorSizes: list = [
        LPTIARTIA_200R_VALUE,
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
        LPTIARTIA_512K_VALUE,
    ]

    # Create list with defined LPTIA Rtia integers
    listLPTiaResistorInteger: list = [
        LPTIARTIA_200R,
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
        LPTIARTIA_512K,
    ]

    # Fint the corresponding value of the Rtia
    for i in range(len(listLPTiaResistorInteger)):
        if iLPTiaSize == listLPTiaResistorInteger[i]:
            return listLPTiaResistorSizes[i]

"""
Module containing the dictionaries for the framework

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import internal dependencies
from .constants import *

dic_configParameters = {
    SEQUENCE_LENGTH: ["Sequence Length", "Length of the sequence"],
    BASE_POTENTIAL: ["Base Potential", "Base potential in mV"],
    START_POTENTIAL: ["Start potential", "Starting potential in mV"],
    STOP_POTENTIAL: ["Stop potential", "Stop potential in mV"],
    LOWER_POTENTIAL: ["Lower turning potential", "Lower potential in mV"],
    UPPER_POTENTIAL: ["Upper turning potential", "Upper potential in mV"],
    POTENTIAL_STEPS: ["Potential steps", "Potential steps in mV"],
    PULSE_LENGTH: ["Pulse lengths", "Pulse lengths in ms"],
    SAMPLING_RATE: ["Sampling rate", "Sampling rate in ms"],
    SAMPLING_DURATION: ["Sampling duration", "Sampling duration in ms"],
    STEP_SIZE: ["Step size", "Stepsize in mV"],
    SCAN_RATE: ["Scan rate", "Scanrate in mV/s"],
    DELTA_V_STAIRCASE: ["Delta V staircase", "Delta V staircase in mV"],
    DELTA_V_PEAK: ["Delta V peak", "Delta V peak in mV"],
    CYCLE: ["Cycles", "Number of cycles"],
    LPTIA_RTIA_SIZE: ["Current range", "Rtia resistor size encoded as integer"],
    FIXED_WE_POTENTIAL: [
        "Fixed WE potential",
        "Working electrode using fixed potential (1: True | 0: False)",
    ],
    MAINS_FILTER: [
        "Mains filter",
        "50 Hz/ 60 Hz mains filter enabled (1: True | 0 : False)",
    ],
    SINC2_OVERSAMPLING: [
        "Sinc2 Oversampling rate",
        "Oversampling rate sinc2 filter encoded as integer",
    ],
    SINC3_OVERSAMPLING: [
        "Sinc3 Oversampling rate",
        "Oversampling rate sinc3 filter encoded as integer",
    ],
}

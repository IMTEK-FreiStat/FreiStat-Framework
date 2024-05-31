"""
Module implementing a class for handling the data required to run the
elctrochemical method. 

The class handles the following variables:

Experiment type         : `self._strElectrochemicalMethod`
Experiment parameters   : `self._listExperimentParameters`
Experiment data         : `self._listStoredData`

These are accessed by :
Experiment type         : `save_ExperimentType`      | `get_ExperimentType`
Experiment parameters   : `save_ExperimentParameters`| `get_ExperimentParameters`
Experiment data         : `append_Data`              | `get_StoredData`
                          `set_StoredData`

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Include dependencies

# Include internal dependencies


class DataStorage:
    """
    Descirption
    -----------
    Class in which all experiment data is stored for one electrochemical method.

    """

    def __init__(self) -> None:
        """
        Descirption
        -----------
        Constructor of class DataStorage

        """
        # Initalize class variable
        self._strElectrochemicalMethod: str = ""
        self._listStoredData: list = []
        self._listExperimentParameters: list = []

    def save_ExperimentParameters(self, listExperimentParameters: list) -> None:
        """
        Descirption
        -----------
        Save experiment parameters in the data object.

        Parameters
        ----------
        `listExperimentParameters` : list
            List which contains every experiment parameter required for the
            chosen electrochemical method

        """
        self._listExperimentParameters = listExperimentParameters

    def save_ExperimentType(self, strExperimentType: str) -> None:
        """
        Descirption
        -----------
        Save experiment type in the data object.

        Parameters
        ----------
        `strExperimentType` : string
            String containing electrochemical method of the experiment

        """
        self._strElectrochemicalMethod = strExperimentType

    def append_Data(self, listTemp: list) -> None:
        """
        Descirption
        -----------
        Extend data in the data storage by appending a new list of arbitrary
        data.

        Parameters
        ----------
        `listTemp` : list
            List with new data which should be appended to the existing data

        """
        self._listStoredData.append(listTemp)

    # Setter methods
    def set_StoredData(self, listStoredData: list) -> None:
        """
        Descirption
        -----------
        Overwrites stored experiment data with new data. Use this method with
        caution, since the currently stored data is lost and can't be restored.

        Parameters
        ----------
        `listStoredData` : list
            List which consists of experiment data entries

        """
        self._listStoredData = listStoredData

    # Getter methods
    def get_StoredData(self) -> list:
        """
        Descirption
        -----------
        Return the stored data as a list.

        Return
        ------
        `listStoredData` : list
            Retrun list which consists of all data aquired during an experiment

        """
        return self._listStoredData

    def get_ExperimentParameters(self) -> list:
        """
        Descirption
        -----------
        Get stored experiment parameters.

        Return
        ------
        `listExperimentParameters` : list
            Return list cotaining all experiment parameters

        """
        return self._listExperimentParameters

    def get_ExperimentType(self) -> str:
        """
        Descirption
        -----------
        Get stored experiment type.

        Return
        ------
        `strElectrochemicalMethod` : string
            Return string containing experiment type

        """
        return self._strElectrochemicalMethod

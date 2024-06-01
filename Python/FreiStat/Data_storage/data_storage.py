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
        self._electrochemical_method = ""
        self._stored_data = []
        self._experiment_parameters = []

    def save_experiment_parameters(self, experiment_parameters: list) -> None:
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
        self._experiment_parameters = experiment_parameters

    def save_experiment_type(self, experiment_type: str) -> None:
        """
        Descirption
        -----------
        Save experiment type in the data object.

        Parameters
        ----------
        `strExperimentType` : string
            String containing electrochemical method of the experiment

        """
        self._electrochemical_method = experiment_type

    def append_data(self, temp: list) -> None:
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
        self._stored_data.append(temp)

    # Setter methods
    def set_stored_data(self, stored_data: list) -> None:
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
        self._stored_data = stored_data

    # Getter methods
    def get_stored_data(self) -> list:
        """
        Descirption
        -----------
        Return the stored data as a list.

        Return
        ------
        `listStoredData` : list
            Retrun list which consists of all data aquired during an experiment

        """
        return self._stored_data

    def get_experiment_parameters(self) -> list:
        """
        Descirption
        -----------
        Get stored experiment parameters.

        Return
        ------
        `listExperimentParameters` : list
            Return list cotaining all experiment parameters

        """
        return self._experiment_parameters

    def get_experiment_type(self) -> str:
        """
        Descirption
        -----------
        Get stored experiment type.

        Return
        ------
        `strElectrochemicalMethod` : string
            Return string containing experiment type

        """
        return self._electrochemical_method

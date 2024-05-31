"""
Module implementing a class for handling all data operations while running 
the Python library of FreiStat.

The dataobjects are stored in a list (`self._listDataObject`) which can be
extended by calling `create_DataObject()`. The new data object is the appended
at the end of the list

Example:
self._listDataObject = []
self.create_DataObject()

-> self._listDataObject = [DatObj1]

self.create_DataObject()

-> self._listDataObject = [DatObj1, DatObj2]

The dataobject can be changed by calling on of the following 3 methods:

`move_first_DataObject` : Move to the object at list position 0
`move_next_DataObject`  : Move to the next object in the list. If the last
                          element is reached it jumps back to the first object
                          in the list. If the last object is a sequence element, 
                          it is skipped and directly jumped to the first one.
                          This is done to prevent that the sequence data is send
                          to the microcontroller and also that no data is stored
                          in this object.
`move_previous_DataObject` : Same as `move_next_DataObject`, except the logic
                             works in the opposite direction.

--------------------------------------------------------------------------------

The data inside the pointed to data object can be accessed in different ways:

`append_StoredData`         : Add new list of data to internal data list of the
                              data storage object.
`save_ExperimentParmeters`  : Save the experiment parameters in list format in
                              the data storage object.
`save_ExperimentType`       : Save experiment type as string in data object.

All these function have also a corresponding get__-method the get access to the
corresponding data.

--------------------------------------------------------------------------------

The datahandling can also export the stored data in several forms:

`export_ExperimentParameters_csv`   : Export experiment parameters as csv
`export_Data_csv`                   : Export experiment data as csv
`export_DataStorage`                : Export the data inside the datastorage
                                      object.

`import_DataStorage`                : Import the data inside the datastorage
                                      object. 

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

import csv
import glob
import os
import pickle
import time
from typing import List, Optional

from .constants import SEQUENCE
from .data_software_storage import DataSoftwareStorage
from .data_storage import DataStorage
from .dictionaries import (
    CA,
    CV,
    DPV,
    EC_DATASTORAGE,
    EC_DS_EXPORT_SETUP_ERR,
    EC_DS_METHOD_UNKOWN,
    FREISTAT_CA_LABEL,
    FREISTAT_CA_LABEL_SEQ,
    FREISTAT_CORE_DATA_FOLDER,
    FREISTAT_CORE_OBJECT_FOLDER,
    FREISTAT_CSV_EXPORT,
    FREISTAT_CV_LABEL,
    FREISTAT_CV_LABEL_SEQ,
    FREISTAT_DATA,
    FREISTAT_DATA_STORAGE,
    FREISTAT_EXPERIMENT_PARAMETERS,
    FREISTAT_OBJECT_EXPORT,
    FREISTAT_SEQUENCE_POSITION,
    LSV,
    NPV,
    SWV,
    dic_configParameters,
)


class DataHandling:
    """
    Descirption
    -----------
    Class which handles all operations regarding the stored data.

    There is only one instance of this class exisitng, regardless of the amount
    of DataStorage objects, since the data is stored in a list format, which is
    accessed in form of a ring structure from outside.

    """

    def __init__(self, data_software_storage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of class DataHandling

        Parameters
        ----------
        `dataSoftwareStorage` : DataSoftwareStorage
            Reference to the data software storage object

        """
        # Safe data software storage reference and set own reference
        self._data_software_storage = data_software_storage
        self._data_software_storage.set_data_handling(self)

        # Initialize class variables
        self._data_objects: List[DataStorage] = []
        self._current_data_object = 0

        self._base_directory = os.getcwd()
        self._working_directory = ["", ""]

        self._output_file = ""

    def create_data_object(self) -> None:
        """
        Description
        -----------
        Create new data object for a electrochemical method and append it to the
        list of data storage objects.

        Set reference to the newest object in the list.

        """
        # Create data object and append it to the list
        self._data_objects.append(DataStorage())

        # Set reference to this new Data object
        self._current_data_object = len(self._data_objects) - 1

    def move_first_data_object(self) -> None:
        """
        Description
        -----------
        Move to the first stored data object in the list.

        """
        # Move to first entry in the list
        self._current_data_object = 0

    def move_next_data_object(self) -> None:
        """
        Description
        -----------
        Move to the next stored data object in the list. If the last object is
        reached, jump to the first entry. If the last stored object is a
        sequence object, skip it.

        """
        # Check if last element is sequence element
        if (
            self._data_objects[len(self._data_objects) - 1].get_experiment_type()
            == SEQUENCE
        ):
            upper_boundary = 2
        else:
            upper_boundary = 1

        # Check if the second last data object is reached
        # (Last element contains information regarding sequence and is skipped)
        if self._current_data_object == len(self._data_objects) - upper_boundary:
            # Jump to first object
            self._current_data_object = 0
        else:
            # Jump to next object
            self._current_data_object += 1

    def move_previous_data_object(self) -> None:
        """
        Description
        -----------
        Move to the previous stored data object in the list. If the first object
        is reached, jump to the last entry. If the last stored object is a
        sequence object, skip it.

        """
        # Check if last element is sequence element
        if (
            self._data_objects[len(self._data_objects) - 1].get_experiment_type()
            == SEQUENCE
        ):
            upper_boundary = 2
        else:
            upper_boundary = 1

        # Check if first data object is reached
        if self._current_data_object == 0:
            # Jump to second last object (see `move_next_DataObject`)
            self._current_data_object = len(self._data_objects) - upper_boundary
        else:
            # Jump to previous object
            self._current_data_object -= 1

    def setup_export_files(self) -> None:
        """
        Description
        -----------
        Setup for exporting stored data. This method creates all required
        folders and saves the references to these subdictionaries.

        """
        # Initialize variables
        folder_name = ""

        # Storing csv-files
        # Create directory for storing all experiment data
        self._create_work_directory(FREISTAT_CSV_EXPORT, FREISTAT_CORE_DATA_FOLDER)

        # Create sub directory for the current day (YEAR-MONTH-DAY)
        folder_name = time.strftime("%y_%m_%d")
        self._create_work_directory(FREISTAT_CSV_EXPORT, folder_name)

        # Create sub directory for the current experiment
        # (HOUR-MINUTE-SECOND)
        folder_name = time.strftime("%H_%M_%S")
        self._create_work_directory(FREISTAT_CSV_EXPORT, folder_name)

        # Change back to back to base directory
        os.chdir(self._base_directory)

        # Storing object-files
        # Create directory for storing the data storage object
        self._create_work_directory(FREISTAT_OBJECT_EXPORT, FREISTAT_CORE_OBJECT_FOLDER)

        # Create sub directory for the current day (YEAR-MONTH-DAY)
        folder_name = time.strftime("%y_%m_%d")
        self._create_work_directory(FREISTAT_OBJECT_EXPORT, folder_name)

        # Create sub directory for the current experiment
        # (HOUR-MINUTE-SECOND)
        folder_name = time.strftime("%H_%M_%S")
        self._create_work_directory(FREISTAT_OBJECT_EXPORT, folder_name)

        # Change back to back to base directory
        os.chdir(self._base_directory)

    def export_data_csv(self, stored_data: list) -> str:
        """
        Description
        -----------
        Exporting stored experiment data as a csv file at the previously defined
        (`setup_ExportFiles()`) folder locations.

        Parameters
        ----------
        `listStoredData` : list
            List containing all stored data which should be exported in a csv
            file.

        Return
        ------
        `strExportPath` : string
            System path where the experiment data is stored

        """
        # Initialize variables
        error_code = 0

        experiment_type = ""

        # Get current experiment type
        experiment_type = self._data_objects[
            self._current_data_object
        ].get_experiment_type()

        # Check if setup was called
        if self._working_directory[FREISTAT_CSV_EXPORT] == "":
            error_code = EC_DATASTORAGE + EC_DS_EXPORT_SETUP_ERR
            return str(error_code)

        # Change working directory
        os.chdir(self._working_directory[FREISTAT_CSV_EXPORT])

        # Save export path
        export_path = os.getcwd()

        # Create csv-file
        self._output_file = (
            FREISTAT_DATA
            + "_"
            + FREISTAT_SEQUENCE_POSITION
            + str(self._current_data_object)
            + "_"
            + experiment_type
            + ".csv"
        )

        # Open writer who is used to write into csv file
        with open(self._output_file, "w", newline="", encoding="utf-8") as csv_file:
            # Write header line
            writer = csv.writer(csv_file)

            # Check if there is data to write
            if len(self.get_stored_data()) > 0:
                # Check which electrochemical method should be exported
                if experiment_type in (LSV, CV, NPV, SWV, DPV):
                    # Check if sequence or single method should be exported
                    if len(self.get_stored_data()[0]) <= 5:
                        # Write header in csv file
                        writer.writerow(FREISTAT_CV_LABEL)
                    else:
                        # Write header in csv file
                        writer.writerow(FREISTAT_CV_LABEL_SEQ)

                    # Loop over every entry
                    for data in stored_data:
                        # Write new row
                        writer.writerow(data)

                elif experiment_type == CA:
                    # Check if sequence or single method should be exported
                    if len(self.get_stored_data()[0]) <= 5:
                        # Write header in csv file
                        writer.writerow(FREISTAT_CA_LABEL)
                    else:
                        # Write header in csv file
                        writer.writerow(FREISTAT_CA_LABEL_SEQ)

                    # Loop over every entry
                    for data in stored_data:
                        # Write new row
                        writer.writerow(data)
                else:
                    # Method not known
                    error_code = EC_DATASTORAGE + EC_DS_METHOD_UNKOWN
                    return str(error_code)

        # Return file-path location of the exported csv-file
        return export_path

    def export_experiment_parameters_csv(
        self, experiment_type: str, stored_parameters: list
    ) -> Optional[str]:
        """
        Description
        -----------
        Exporting stored experiment data as a csv file at the previously defined
        (`setup_ExportFiles()`) folder locations.

        Parameters
        ----------
        `strExperimentType` : str
            Experiment type encoded as string

        `listStoredParameters` : list
            List containing all stored data which should be exported in a csv
            file

        """
        # Initialzie variables
        errorcode = 0

        # Check if setup was called
        if self._working_directory[FREISTAT_CSV_EXPORT] == "":
            errorcode = EC_DATASTORAGE + EC_DS_EXPORT_SETUP_ERR
            return str(errorcode)

        # Change working directory
        os.chdir(self._working_directory[FREISTAT_CSV_EXPORT])

        # Create csv-file
        self._output_file = (
            FREISTAT_EXPERIMENT_PARAMETERS
            + "_"
            + FREISTAT_SEQUENCE_POSITION
            + str(self._current_data_object)
            + "_"
            + experiment_type
            + ".csv"
        )

        # Write header line
        with open(self._output_file, "w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)

            # Write experiment type
            writer.writerow(["Electrochemical method", experiment_type])

            # Loop over every entry
            for parameters in stored_parameters:
                # Write new row
                writer.writerow([dic_configParameters[parameters[0]][1], parameters[1]])

        return None

    def _create_work_directory(self, directory_select: int, name: str):
        """
        Description
        -----------
        Helper method for checking and creating working directory to store csv
        and object files. This method is called by `setup_ExportFiles()` and
        doesn't need to be accessed from outside this class.

        Parameters
        ----------
        `iDirectorySelect` : int
            Integer determinating if path of persisent data objects or
            measurements should be chosen.

        `strName` : str
            Name of the folder of the working directory

        """
        # Check if work directory is already created
        self._working_directory[directory_select] = os.getcwd()
        if not os.path.exists(name):
            # If folder doesn't exist, create it
            os.mkdir(name)

        # Change working directory to new folder
        self._working_directory[directory_select] += "\\" + name + "\\"
        os.chdir(self._working_directory[directory_select])

    def export_data_storage(self) -> None:
        """
        Description
        -----------
        Exporting data storage object to ensure data persistence in case of a
        software crash.

        """
        # Change working directory
        os.chdir(self._working_directory[FREISTAT_OBJECT_EXPORT])

        # Get current experiment type
        experiment_type = self._data_objects[
            self._current_data_object
        ].get_experiment_type()

        # Overwrite existing external object with newest iteration
        with open(
            FREISTAT_DATA_STORAGE
            + "_"
            + FREISTAT_SEQUENCE_POSITION
            + str(self._current_data_object)
            + "_"
            + experiment_type,
            "wb",
        ) as output:
            # Write data object as data file into the chosen directory.
            pickle.dump(
                self._data_objects[self._current_data_object].get_stored_data(),
                output,
                pickle.HIGHEST_PROTOCOL,
            )

    def import_data_storage(self, path: str) -> None:
        """
        Description
        -----------
        Import data storage object to ensure data persistence in case of a
        software crash

        Parameters
        ----------
        `strPath` : string
            String containg path to the data which should be imported.

        TODO 01.09.2021 -> changed export need to change import as well:
        "note : load loop until EOFERROR"

        """
        # Initialzie variables
        data = []

        # Change working directory
        os.chdir(path)

        # Get all files in the folder
        data = glob.glob(FREISTAT_DATA_STORAGE + "*")

        print(data)

        # Loop over every found file
        for entry in data:
            # Create Data storage
            self.create_data_object()

            # Fill new datastorage with the external data
            with open(entry, "rb") as exernal_data:
                self._data_objects[self._current_data_object].set_stored_data(
                    pickle.load(exernal_data)
                )

            # Export data as csv
            with open(
                entry + "_recovery.csv", "w", newline="", encoding="utf-8"
            ) as csv_file:
                # Write header line
                writer = csv.writer(csv_file)

                stored_data = self._data_objects[
                    self._current_data_object
                ].get_stored_data()
                # Loop over every entry
                for s_data in stored_data:
                    # Write new row
                    writer.writerow(s_data)

    def append_stored_data(self, temp: list) -> None:
        """
        Description
        -----------
        Append current data object with new arbitrary list of data.

        Parameters
        ----------
        `listTemp` : list
            List with new data which should be appended to the existing data

        """
        # Add list of data to the current referenced data object
        self._data_objects[self._current_data_object].append_data(temp)

    def save_experiment_parmeters(self, experiment_parameters: list) -> None:
        """
        Description
        -----------
        Save experiment parameters in current data object.

        Parameters
        ----------
        `listExperimentParameters` : list
            List which contains every experiment parameter required for the
            chosen electrochemical method

        """
        # Add list of experiment parameters to the current referenced data
        # object
        self._data_objects[self._current_data_object].save_experiment_parameters(
            experiment_parameters
        )

    def save_experiment_type(self, experiment_type: str) -> None:
        """
        Description
        -----------
        Save experiment type in current data object.

        Parameters
        ----------
        `strExperimentType` : string
            String containing electrochemical method of the experiment

        """
        # Save experiment type in the current referenced data object
        self._data_objects[self._current_data_object].save_experiment_type(
            experiment_type
        )

    # Getter methods
    def get_stored_data(self) -> list:
        """
        Description
        -----------
        Read data from currently referenced data object.

        Return
        ------
        `listStoredData` : list
            List containing all data currently stored in the currently
            referenced data storage object

        """
        return self._data_objects[self._current_data_object].get_stored_data()

    def get_experiment_parameters(self) -> list:
        """
        Description
        -----------
        Get experiment parameters from currently referenced data object.

        Return
        ------
        `listExperimentParameters` : list
            List cotaining all experiment parameters

        """
        return self._data_objects[self._current_data_object].get_experiment_parameters()

    def get_experiment_type(self) -> str:
        """
        Description
        -----------
        Get experiment type from currently referenced data object.

        Return
        ------
        `strExperimentType` : string
            String containing experiment type

        """
        return self._data_objects[self._current_data_object].get_experiment_type()

    def get_sequence_length(self) -> int:
        """
        Description
        -----------
        Get length of the experiment sequence (amount of stored data objects).

        Return
        ------
        `LengthListDataObject` : int
            Integer containing the length of the current experiment sequence

        """
        return len(self._data_objects)

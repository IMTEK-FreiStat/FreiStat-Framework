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

# Import dependencies
import csv
import os
import glob
import pickle
import time

# Import internal dependencies
from .constants import *
from .data_storage import DataStorage
from .data_software_storage import DataSoftwareStorage

class DataHandling:
    """
    Descirption
    -----------
    Class which handles all operations regarding the stored data.

    There is only one instance of this class exisitng, regardless of the amount
    of DataStorage objects, since the data is stored in a list format, which is
    accessed in form of a ring structure from outside.

    """

    def __init__(self, dataSoftwareStorage: DataSoftwareStorage) -> None:
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
        self._dataSoftwareStorage = dataSoftwareStorage
        self._dataSoftwareStorage.setDataHandling(self)

        # Initialize class variables
        self._listDataObject : list = []
        self._currentDataObject : int = 0

        self._baseDirectory : str = os.getcwd()
        self._workingDirectory : list = ["", ""]

    def create_DataObject(self) -> None:
        """
        Description
        -----------
        Create new data object for a electrochemical method and append it to the
        list of data storage objects. 

        Set reference to the newest object in the list.

        """
        # Create data object and append it to the list
        self._listDataObject.append(DataStorage())

        # Set reference to this new Data object
        self._currentDataObject = len(self._listDataObject) - 1

    def move_first_DataObject(self) -> None:
        """
        Description
        -----------
        Move to the first stored data object in the list.

        """
        # Move to first entry in the list
        self._currentDataObject = 0

    def move_next_DataObject(self) -> None:
        """
        Description
        -----------
        Move to the next stored data object in the list. If the last object is
        reached, jump to the first entry. If the last stored object is a 
        sequence object, skip it.

        """
        # Check if last element is sequence element
        if (self._listDataObject[len(self._listDataObject) - 1]. \
            get_ExperimentType() == SEQUENCE):
            iUpperBoundary = 2
        else :
            iUpperBoundary = 1

        # Check if the second last data object is reached 
        # (Last element contains information regarding sequence and is skipped)
        if (self._currentDataObject == len(self._listDataObject) - 
            iUpperBoundary):
            # Jump to first object
            self._currentDataObject = 0
        else :
            # Jump to next object
            self._currentDataObject += 1

    def move_previous_DataObject(self) -> None:
        """
        Description
        -----------
        Move to the previous stored data object in the list. If the first object 
        is reached, jump to the last entry. If the last stored object is a 
        sequence object, skip it.

        """
        # Check if last element is sequence element
        if (self._listDataObject[len(self._listDataObject) - 1]. \
            get_ExperimentType() == SEQUENCE):
            iUpperBoundary = 2
        else :
            iUpperBoundary = 1

        # Check if first data object is reached
        if (self._currentDataObject == 0):
            # Jump to second last object (see `move_next_DataObject`)
            self._currentDataObject = len(self._listDataObject) - iUpperBoundary
        else :
            # Jump to previous object
            self._currentDataObject -= 1

    def setup_ExportFiles(self) -> None:
        """
        Description
        -----------
        Setup for exporting stored data. This method creates all required
        folders and saves the references to these subdictionaries.

        """
        # Initialize variables
        strFolderName : str = ""

        # Storing csv-files
        # Create directory for storing all experiment data
        self._create_WorkDirectory(FREISTAT_CSV_EXPORT, 
                                   FREISTAT_CORE_DATA_FOLDER)

        # Create sub directory for the current day (YEAR-MONTH-DAY)
        strFolderName = time.strftime("%y_%m_%d")
        self._create_WorkDirectory(FREISTAT_CSV_EXPORT, strFolderName)

        # Create sub directory for the current experiment
        # (HOUR-MINUTE-SECOND)
        strFolderName = time.strftime("%H_%M_%S")
        self._create_WorkDirectory(FREISTAT_CSV_EXPORT, strFolderName)

        # Change back to back to base directory
        os.chdir(self._baseDirectory)

        # Storing object-files
        # Create directory for storing the data storage object
        self._create_WorkDirectory(FREISTAT_OBJECT_EXPORT, 
                                   FREISTAT_CORE_OBJECT_FOLDER)
        
        # Create sub directory for the current day (YEAR-MONTH-DAY)
        strFolderName = time.strftime("%y_%m_%d")
        self._create_WorkDirectory(FREISTAT_OBJECT_EXPORT, strFolderName)

        # Create sub directory for the current experiment
        # (HOUR-MINUTE-SECOND)
        strFolderName = time.strftime("%H_%M_%S")
        self._create_WorkDirectory(FREISTAT_OBJECT_EXPORT, strFolderName)

        # Change back to back to base directory
        os.chdir(self._baseDirectory)

    def export_Data_csv(self, listStoredData : list) -> str:    
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
        iErrorcode : int = 0

        strExperimentType : str = ""
        
        # Get current experiment type
        strExperimentType = self._listDataObject[self._currentDataObject]. \
                                get_ExperimentType()

        # Check if setup was called
        if (self._workingDirectory[FREISTAT_CSV_EXPORT] == ""):
            iErrorcode = EC_DATASTORAGE + EC_DS_EXPORT_SETUP_ERR
            return str(iErrorcode)

        # Change working directory
        os.chdir(self._workingDirectory[FREISTAT_CSV_EXPORT])

        # Save export path
        strExportPath = os.getcwd()

        # Create csv-file
        self._outputFile = FREISTAT_DATA + "_" + FREISTAT_SEQUENCE_POSITION + \
            str(self._currentDataObject) + "_" + strExperimentType + ".csv"

        # Open writer who is used to write into csv file
        with open(self._outputFile, 'w', newline = '', encoding = "utf-8") as \
            csvFile:
            # Write header line
            writer = csv.writer(csvFile)

            # Check which electrochemical method should be exported
            if (strExperimentType == LSV or
                strExperimentType == CV or
                strExperimentType == NPV or
                strExperimentType == SWV or
                strExperimentType == DPV):
                # Check if sequence or single method should be exported
                if (len(self.get_StoredData()[0]) <= 5):
                    # Write header in csv file
                    writer.writerow(FREISTAT_CV_LABEL)
                else :
                    # Write header in csv file
                    writer.writerow(FREISTAT_CV_LABEL_SEQ)                    

                # Loop over every entry
                for iEntry in range(len(listStoredData)):
                    # Write new row
                    writer.writerow(listStoredData[iEntry])

            elif (strExperimentType == CA):
                # Check if sequence or single method should be exported
                if (len(self.get_StoredData()[0]) <= 5):
                    # Write header in csv file
                    writer.writerow(FREISTAT_CA_LABEL)
                else :
                    # Write header in csv file
                    writer.writerow(FREISTAT_CA_LABEL_SEQ)  

                # Loop over every entry
                for iEntry in range(len(listStoredData)):
                    # Write new row
                    writer.writerow(listStoredData[iEntry])
            else :
                # Method not known 
                iErrorcode = EC_DATASTORAGE + EC_DS_METHOD_UNKOWN
                return str(iErrorcode)
            
        # Close file writer
        csvFile.close

        # Return file-path location of the exported csv-file
        return strExportPath

    def export_ExperimentParameters_csv(self, listStoredParameters : list) -> None:    
        """
        Description
        -----------
        Exporting stored experiment data as a csv file at the previously defined
        (`setup_ExportFiles()`) folder locations.

        Parameters
        ----------
        `listStoredParameters` : list
            List containing all stored data which should be exported in a csv
            file

        """
        # Initialzie variables
        iErrorcode : int = 0

        strExperimentType : str = ""
        
        # Get current experiment type
        strExperimentType = self._listDataObject[self._currentDataObject]. \
                                get_ExperimentType()

        # Check if setup was called
        if (self._workingDirectory[FREISTAT_CSV_EXPORT] == ""):
            iErrorcode = EC_DATASTORAGE + EC_DS_EXPORT_SETUP_ERR
            return str(iErrorcode)

        # Change working directory
        os.chdir(self._workingDirectory[FREISTAT_CSV_EXPORT])

        # Create csv-file
        self._outputFile = FREISTAT_EXPERIMENT_PARAMETERS + "_" + \
            FREISTAT_SEQUENCE_POSITION + str(self._currentDataObject) + \
            "_" + strExperimentType + ".csv"

        # Write header line
        with open(self._outputFile, 'w', newline = '', encoding = "utf-8") as \
            csvFile:
            writer = csv.writer(csvFile)

            # Check which electrochemical method should be exported and
            # intialize reference list
            if (strExperimentType == CA):
                listReferenceList = [                
                    [POTENTIAL_STEPS, POTENTIAL_STEPS_TEXT],
                    [PULSE_LENGTH, PULSE_LENGTH_TEXT],
                    [SAMPLING_RATE, SAMPLING_RATE_TEXT],
                    [CYCLE, CYCLE_TEXT],
                    [LPTIA_RTIA_SIZE, LPTIA_RTIA_SIZE_TEXT],
                    [MAINS_FILTER, MAINS_FILTER_TEXT],
                    [SINC2_OVERSAMPLING, SINC2_OVERSAMPLING_TEXT],
                    [SINC3_OVERSAMPLING, SINC3_OVERSAMPLING_TEXT]
                ]

            elif (strExperimentType == LSV):
                listReferenceList = [
                    [START_POTENTIAL, START_POTENTIAL_TEXT],
                    [STOP_POTENTIAL, STOP_POTENTIAL],
                    [STEP_SIZE, STEP_SIZE_TEXT],
                    [SCAN_RATE, SCAN_RATE_TEXT],
                    [CYCLE, CYCLE_TEXT],
                    [LPTIA_RTIA_SIZE, LPTIA_RTIA_SIZE_TEXT],
                    [FIXED_WE_POTENTIAL, FIXED_WE_POTENTIAL_TEXT],
                    [MAINS_FILTER, MAINS_FILTER_TEXT],
                    [SINC2_OVERSAMPLING, SINC2_OVERSAMPLING_TEXT],
                    [SINC3_OVERSAMPLING, SINC3_OVERSAMPLING_TEXT]
                ]

            elif (strExperimentType == CV):
                listReferenceList = [
                    [START_POTENTIAL, START_POTENTIAL_TEXT],
                    [LOWER_POTENTIAL, LOWER_POTENTIAL_TEXT],
                    [UPPER_POTENTIAL, UPPER_POTENTIAL_TEXT],
                    [STEP_SIZE, STEP_SIZE_TEXT],
                    [SCAN_RATE, SCAN_RATE_TEXT],
                    [CYCLE, CYCLE_TEXT],
                    [LPTIA_RTIA_SIZE, LPTIA_RTIA_SIZE_TEXT],
                    [FIXED_WE_POTENTIAL, FIXED_WE_POTENTIAL_TEXT],
                    [MAINS_FILTER, MAINS_FILTER_TEXT],
                    [SINC2_OVERSAMPLING, SINC2_OVERSAMPLING_TEXT],
                    [SINC3_OVERSAMPLING, SINC3_OVERSAMPLING_TEXT]
                ]   

            elif (strExperimentType == NPV):
                listReferenceList = [
                    [BASE_POTENTIAL, BASE_POTENTIAL_TEXT],
                    [START_POTENTIAL, START_POTENTIAL_TEXT],
                    [STOP_POTENTIAL, STOP_POTENTIAL_TEXT],
                    [DELTA_V_STAIRCASE, DELTA_V_STAIRCASE_TEXT],
                    [PULSE_LENGTH,PULSE_LENGTH_TEXT],
                    [SAMPLING_DURATION, SAMPLING_DURATION_TEXT],
                    [CYCLE, CYCLE_TEXT],
                    [LPTIA_RTIA_SIZE, LPTIA_RTIA_SIZE_TEXT],
                    [FIXED_WE_POTENTIAL, FIXED_WE_POTENTIAL_TEXT],
                    [MAINS_FILTER, MAINS_FILTER_TEXT],
                    [SINC2_OVERSAMPLING, SINC2_OVERSAMPLING_TEXT],
                    [SINC3_OVERSAMPLING, SINC3_OVERSAMPLING_TEXT]
                ]     

            elif (strExperimentType == DPV or
                  strExperimentType == SWV):
                listReferenceList = [
                    [START_POTENTIAL, START_POTENTIAL_TEXT],
                    [STOP_POTENTIAL, STOP_POTENTIAL_TEXT],
                    [DELTA_V_STAIRCASE, DELTA_V_STAIRCASE_TEXT],
                    [DELTA_V_PEAK, DELTA_V_PEAK_TEXT],
                    [PULSE_LENGTH,PULSE_LENGTH_TEXT],
                    [SAMPLING_DURATION, SAMPLING_DURATION_TEXT],
                    [CYCLE, CYCLE_TEXT],
                    [LPTIA_RTIA_SIZE, LPTIA_RTIA_SIZE_TEXT],
                    [FIXED_WE_POTENTIAL, FIXED_WE_POTENTIAL_TEXT],
                    [MAINS_FILTER, MAINS_FILTER_TEXT],
                    [SINC2_OVERSAMPLING, SINC2_OVERSAMPLING_TEXT],
                    [SINC3_OVERSAMPLING, SINC3_OVERSAMPLING_TEXT]
                ]             

            # Loop over every entry 
            for iEntry in range(len(listStoredParameters)):
                # Check if entry is found
                if(listStoredParameters[iEntry][0] == 
                    listReferenceList[iEntry][0]):
                    # Write new row
                    writer.writerow([listReferenceList[iEntry][1], 
                                        listStoredParameters[iEntry][1]])
                
        # Close file
        csvFile.close

    def _create_WorkDirectory(self, iDirectorySelect: int, strName: str):
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
        self._workingDirectory[iDirectorySelect] = os.getcwd()
        if not os.path.exists(strName):
            # If folder doesn't exist, create it
            os.mkdir(strName)

        # Change working directory to new folder
        self._workingDirectory[iDirectorySelect] += "\\" + strName + "\\"
        os.chdir(self._workingDirectory[iDirectorySelect])

    def export_JSON(self) -> None:
        """
        Description
        -----------
        Exporting stored experiment data as a JSON file

        """
        # TODO planned feature
        
    def export_DataStorage(self) -> None:
        """
        Description
        -----------
        Exporting data storage object to ensure data persistence in case of a 
        software crash.
        
        """
        # Initialzie variables
        iErrorcode : int = 0

        # Check if setup was called
        if (self._workingDirectory[FREISTAT_CSV_EXPORT] == ""):
            iErrorcode = EC_DATASTORAGE + EC_DS_EXPORT_SETUP_ERR
            # TODO return value

        # Change working directory
        os.chdir(self._workingDirectory[FREISTAT_OBJECT_EXPORT])

        # Get current experiment type
        strExperimentType = self._listDataObject[self._currentDataObject]. \
                                get_ExperimentType()

        # Overwrite existing external object with newest iteration
        with open(FREISTAT_DATA_STORAGE + "_" + FREISTAT_SEQUENCE_POSITION + \
            str(self._currentDataObject) + "_" + strExperimentType, "wb") \
            as output:
            # Write data object as data file into the chosen directory.
            pickle.dump(self._listDataObject[self._currentDataObject].get_StoredData(), output, 
                        pickle.HIGHEST_PROTOCOL)

        # Close output writer
        output.close

    def import_DataStorage(self, strPath : str) -> None:
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
        listData : list = []

        # Change working directory
        os.chdir(strPath)

        # Get all files in the folder
        listData = glob.glob(FREISTAT_DATA_STORAGE + '*')

        print(listData)
        
        # Loop over every found file 
        for iIndex in range(len(listData)):
            # Create Data storage
            self.create_DataObject()            
            
            # Fill new datastorage with the external data
            with open(listData[iIndex], "rb") as input:
                self._listDataObject[self._currentDataObject]. \
                    set_StoredData(pickle.load(input))   

            # Close input reader
            input.close      

            # Export data as csv
            with open(listData[iIndex] + "_recovery.csv", 'w', newline = '', 
                encoding = "utf-8") as csvFile:
                # Write header line
                writer = csv.writer(csvFile)                

                listStoredData = self._listDataObject[self._currentDataObject].get_StoredData()
                # Loop over every entry
                for iEntry in range(len(listStoredData)):
                    # Write new row
                    writer.writerow(listStoredData[iEntry])            
    
    def append_StoredData(self, listTemp: list) -> None:
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
        self._listDataObject[self._currentDataObject].append_Data(listTemp)

    def save_ExperimentParmeters(self, listExperimentParameters: list) -> None:
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
        self._listDataObject[self._currentDataObject]. \
            save_ExperimentParameters(listExperimentParameters)

    def save_ExperimentType(self, strExperimentType: str) -> None:
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
        self._listDataObject[self._currentDataObject]. \
            save_ExperimentType(strExperimentType)

    # Getter methods
    def get_StoredData(self) -> list:
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
        return self._listDataObject[self._currentDataObject].get_StoredData()

    def get_ExperimentParameters(self) -> list:
        """
        Description
        -----------
        Get experiment parameters from currently referenced data object.
        
        Return
        ------
        `listExperimentParameters` : list
            List cotaining all experiment parameters

        """
        return self._listDataObject[self._currentDataObject]. \
            get_ExperimentParameters()

    def get_ExperimentType(self) -> str:
        """
        Description
        -----------
        Get experiment type from currently referenced data object.
        
        Return
        ------
        `strExperimentType` : string
            String containing experiment type

        """
        return  self._listDataObject[self._currentDataObject]. \
            get_ExperimentType()

    def get_SequenceLength(self) -> int:
        """
        Description
        -----------
        Get length of the experiment sequence (amount of stored data objects).

        Return
        ------
        `LengthListDataObject` : int
            Integer containing the length of the current experiment sequence
            
        """
        return len(self._listDataObject)
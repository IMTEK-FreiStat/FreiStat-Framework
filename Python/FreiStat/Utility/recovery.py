"""
Module implementing a class for recovering data from the exported data storage
and export the data as csv.

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
from ..data_storage.data_software_storage import DataSoftwareStorage
from ..data_storage.data_handling import DataHandling


class Recovery_Data:
    """
    Description
    -----------
    Class which implements a recovery tool to import the recovery data and
    export it as csv.

    """

    def start_Recovery(self, Path: str) -> None:
        """
        Description
        -----------
        Method for loading the exported data into the recovery class

        Parameters
        ----------
        `Path` : string
            String containing the path to the folder of the data files which
            should be recovered.

        """
        # Create instance of DataSoftwareStorage
        self._DataSoftwareStorage = DataSoftwareStorage()

        # Create instance of DataHandling
        self._DataHandling = DataHandling(self._DataSoftwareStorage)

        # Import external data
        self._DataHandling.import_data_storage(Path)

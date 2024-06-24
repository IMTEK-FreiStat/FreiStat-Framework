"""
Module implementing a class which creates setup and execute behavior given
a certain electrochemical method, which is passed as a string.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import multiprocessing as mp
from multiprocessing.queues import Queue

# Import internal dependencies
from ..Data_storage.constants import *
from ..Data_storage.data_software_storage import DataSoftwareStorage

from .Setup_behavior.setup_ca import SetupCA
from .Execute_behavior.execute_ca import ExecuteCA

from .Setup_behavior.setup_a import SetupA
from .Execute_behavior.execute_a import ExecuteA

from .Setup_behavior.setup_ocp import SetupOCP
from .Execute_behavior.execute_ocp import ExecuteOCP

from .Setup_behavior.setup_lsv import SetupLSV

from .Setup_behavior.setup_cv import SetupCV
from .Execute_behavior.execute_cv import ExecuteCV

from .Setup_behavior.setup_npv import SetupNPV
#from .Execute_behavior.execute_npv import ExecuteNPV

from .Setup_behavior.setup_dpv import SetupDPV
from .Execute_behavior.execute_dpv import ExecuteDPV

from .Setup_behavior.setup_swv import SetupSWV
#from .Execute_behavior.execute_swv import ExecuteSWV

from .Setup_behavior.setup_sequence import SetupSequence
from .Execute_behavior.execute_sequence import ExecuteSequence

class ElectrochemicalMethod:
    """
    Description
    -----------
    Class which is called when running a electrochemical method. The behavior
    of this class is changed dynamically according to the choosen method. 

    """

    def __init__(self, 
                 ElectrochemicalMethod: str, 
                 dataSoftwareStorage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Constructor of the class ElectrochemicalMethod

        Parameters
        ----------
        `ElectrochemicalMethod` : string
            String containing the abbreviation for the choosen electrochmical
            method

        `dataSoftwareStorage`: DataSoftwareStorage
            Reference to DataSoftwareStorage object

        """
        # Set reference to object storing all software related parameters
        self._dataSoftwareStorage = dataSoftwareStorage

        # Define setup and execute behavior for running a sequence of
        # electrochemical methods
        if (ElectrochemicalMethod == SEQUENCE):
            self._setupBehavior = SetupSequence(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteSequence(self._dataSoftwareStorage)

        # Define setup and execute behavior for running chronoamperometry
        if(ElectrochemicalMethod == A):
            self._setupBehavior = SetupA(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteA(self._dataSoftwareStorage)            

        # Define setup and execute behavior for running chronoamperometry
        if(ElectrochemicalMethod == CA):
            self._setupBehavior = SetupCA(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteCA(self._dataSoftwareStorage)

        # Define setup and execute behavior for running ocp measurement
        if(ElectrochemicalMethod == OCP):
            self._setupBehavior = SetupOCP(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteOCP(self._dataSoftwareStorage)

        # Define setup and execute behavior for running linear sweep voltammetry
        if(ElectrochemicalMethod == LSV):
            self._setupBehavior = SetupLSV(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteCV(self._dataSoftwareStorage)

        # Define setup and execute behavior for running cyclic voltammetry
        if(ElectrochemicalMethod == CV):
            self._setupBehavior = SetupCV(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteCV(self._dataSoftwareStorage)

        # Define setup and execute behavior for running normal pulse voltammetry
        if(ElectrochemicalMethod == NPV):
            self._setupBehavior = SetupNPV(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteDPV(self._dataSoftwareStorage)

        # Define setup and execute behavior for running differential pulse
        # voltammetry
        if(ElectrochemicalMethod == DPV):
            self._setupBehavior = SetupDPV(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteDPV(self._dataSoftwareStorage)

        # Define setup and execute behavior for running square wave voltammetry
        if(ElectrochemicalMethod == SWV):
            self._setupBehavior = SetupSWV(self._dataSoftwareStorage)
            self._executeBehavior = ExecuteDPV(self._dataSoftwareStorage)

    def setup(self, ExperimentParamters: list) -> int:
        """
        Description
        -----------
        Setup method running defined setup behavior.

        Parameters
        ----------
        `ExperimentParamters` : list
            List containing all experiment parameters. Changes depending on the
            choosen ec-method.

        Return
        ------
        ErrorCode : Int
            Return error code which encodes informations about setup progress

        """
        return self._setupBehavior.setup(ExperimentParamters)

    def execute(self, 
                dataQueue : Queue,
                event : mp.Event(),
                iTelegrams : int = 3,
                bEnableReading : bool = True,
                bPorgressiveMesurement : bool = False) -> None:
        """
        Description
        -----------
        Execute method running defined execute behavior.

        Parameters
        ----------
        `dataQueue` : Queue
            Data queue which is used as a pipe between processes

        `event` : Event
            Multiprocessing event to indicate termination event

        `iTelegrams` : int 
            Number of telegrams which should be send by the ec-method

        `bEnableReading` : bool
            Flag indicating if incoming data should be read or not

        `bPorgressiveMesurement` : bool
            Flag indicating if measurement should be displayed continously or
            if each cycle should start at time = 0

        """
        self._executeBehavior.execute(dataQueue = dataQueue, event= event,
            iTelegrams= iTelegrams, bEnableReading= bEnableReading,
            bPorgressiveMesurement= bPorgressiveMesurement)
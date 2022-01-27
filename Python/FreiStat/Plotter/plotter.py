"""
Module implementing the plotter class which is used to display the measurement
results at runtime.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
from matplotlib.animation import FuncAnimation
from matplotlib.figure import Figure, SubplotParams
import matplotlib.pyplot as plt
from tkinter.ttk import Style

# Import internal dependencies
from ..Data_storage.constants import *
from ..Utility.decoder import _decode_LPTIA_Resistor_Size

class Plotter:
    """
    Descirption
    -----------
    Class which plots the data and updates the plot during the experiment in a
    loop. This is the reason why this class acts a central point of the
    application.

    """
    def __init__(self, strMethod : str, listTempExperimentParameters : list,
                 strMode : str, process) -> None:
        """
        Description
        -----------
        Constructor of class Plotter.

        Parameters
        ----------
        `strMethod` : str
            String containing the experiment type

        `listTempExperimentParameters` : list
            List containing the experiment parameters of the choosen ec-method

        `strMode` : string
            String defining in which mode the FreiStat plotter should be used
            Defined: "standalone", "backend"

        `process` : process
            Reference to the process, which is used to read the data from the
            serial connection.

        """
        # Initalize class variables
        self._iCycle : int = 0
        self._iDataCounter : int = 0
        self._iDataPoint : int = 2
        self._iLine2DCounter : int = 0
        self._iMethodCounter : int = 0

        self._strCurrentMethod : str = UNDEFIEND
        self._strMode : str = strMode

        self._lines : list = []
        self._listAxes : list = []
        self._listFig : list = []
        self._listfCurrent : list = []
        self._listfTime : list = []
        self._listfVoltage : list = []
        self._listStoredData : list = []

        self._process = process

        # Store experiment type
        self._experimentType = strMethod

        # Store experiment parameters
        self._experimentParameters = listTempExperimentParameters

        # Setup live plot
        self._subplotParams = SubplotParams(top= 0.95, right= 0.8)
        self._fig, (self._ax) = plt.subplots(1, 1, subplotpars= self._subplotParams) 
        self._line, = self._ax.plot([], [])

        # Set title of live feed window
        self._fig.canvas.manager.set_window_title(PLOT_WINDOW_TITLE_MAIN)

        # Enable grid
        self._ax.grid()

        # Check if FreiStat is used as backend
        if (self._strMode == FREISTAT_BACKEND):
            self._StyleConfig = Style()

        # Calculate amount of Datapoints and time per datapoint
        self._iDataPoints = self._calculateDatapoints(self._experimentType, 
            self._experimentParameters)
        self._fStepTime = self._calculateStepTime(self._experimentType, 
            self._experimentParameters)

        # Check if a sequence should be plotted or only a single method
        # Sequence should be plotted
        if (self._experimentParameters[0][0] == SEQUENCE_LENGTH):
            # Result plots
            # Create window for every method in the sequence
            for iIndex in range(self._experimentParameters[0][1]):
                self._subplotParams = SubplotParams(top= 0.95, right= 0.75)
                fig, (ax) = plt.subplots(1, 1, figsize=[6.4, 3.6], 
                                         subplotpars= self._subplotParams) 
                self._listFig.append(fig)
                self._listAxes.append(ax)

                # Set title of the different windows
                self._listFig[iIndex].canvas.manager.set_window_title(
                    self._experimentParameters[2][iIndex][0] + ": " + 
                    PLOT_WINDOW_SEQUENCE + ": " + str(iIndex))

                # Enable grid
                self._listAxes[iIndex].grid()
            
            # Create first level structure for the amount of methods in the
            # sequence
            for iIndex in range(self._experimentParameters[0][1] * 
                                self._experimentParameters[1][1]):
                self._listfTime.append([])
                self._listfCurrent.append([])
                self._listfVoltage.append([])

            # Create second level structure for the amount of cycles in each of
            # the methods in the sequence
            for iIndex in range(self._experimentParameters[0][1] * 
                                self._experimentParameters[1][1]):
                for iIndex2 in range(self._experimentParameters[2][iIndex % 
                    self._experimentParameters[0][1]][1]): 

                    self._listfTime[iIndex].append([])
                    self._listfCurrent[iIndex].append([])
                    self._listfVoltage[iIndex].append([])

            # Determine the maximum amount of cycles
            for iIndex in range(self._experimentParameters[0][1]):  
                # Check if cycle in experiment parameters is larger than stored
                if (self._iLine2DCounter < self._experimentParameters[2][iIndex][1]):
                    self._iLine2DCounter = self._experimentParameters[2][iIndex][1]

            # Check if self._iLine2DCounter is larger than "10" and limit 
            # maximum amount of lines in the plot
            if (self._iLine2DCounter > PLOT_DISPLAY_CYCLES):
                self._iLine2DCounter = PLOT_DISPLAY_CYCLES

            # Create required amount of Line2D objects
            for iIndex in range(self._iLine2DCounter):
                self._lines.append(self._ax.plot([],[])[0])

            # Disable plotting of sequence
            self._bPlotSequence : bool = True

        # Single method should be plotted
        else :
            # Create lines depending on the electrochemical method
            if (self._experimentType == CV or 
                self._experimentType == LSV):

                # Create entry for every cycle to separate data
                for iIndex in range(self._experimentParameters[5][1]):
                    self._listfTime.append([])
                    self._listfCurrent.append([])
                    self._listfVoltage.append([])

                self._iLine2DCounter = self._experimentParameters[5][1]

            elif (self._experimentType == NPV or
                  self._experimentType == DPV or
                  self._experimentType == SWV):

                # Create entry for every cycle to separate data
                for iIndex in range(self._experimentParameters[6][1]):
                    self._listfTime.append([])
                    self._listfCurrent.append([])
                    self._listfVoltage.append([])

                self._iLine2DCounter = self._experimentParameters[6][1]

            elif (self._experimentType == CA):

                # Create entry for every cycle to separate data
                for iIndex in range (self._experimentParameters[3][1]):
                    self._listfTime.append([])
                    self._listfCurrent.append([])
                    self._listfVoltage.append([])

                self._iLine2DCounter = self._experimentParameters[3][1]

            elif (self._experimentType == OCP):
                # Create entry for every cycle to separate data
                for iIndex in range (self._experimentParameters[2][1]):
                    self._listfTime.append([])
                    self._listfVoltage.append([])

                self._iLine2DCounter = self._experimentParameters[3][1]

            # Check if self._iLine2DCounter is larger than "10" and limit 
            # maximum amount of lines in the plot
            if (self._iLine2DCounter > PLOT_DISPLAY_CYCLES):
                self._iLine2DCounter = PLOT_DISPLAY_CYCLES

            # Create required amount of Line2D objects
            for iIndex in range(self._iLine2DCounter):
                self._lines.append(self._ax.plot([],[])[0])

            # Disable plotting of sequence
            self._bPlotSequence : bool = False

    def initPlot(self) -> None:
        """
        Description
        -----------
        Initialize plot window

        Deploy different labels and window definitions depending on the chosen 
        electrochemical method

        """    
        # Open circuit potential
        if (self._experimentType == OCP):
            self._initPlot_OCP(self._experimentParameters)

        # Chronoamperometry
        elif (self._experimentType == CA):
            self._initPlot_CA(self._experimentParameters)

        # Linear sweep voltammetry
        elif (self._experimentType == LSV):
            self._initPlot_LSV(self._experimentParameters)

        # Cyclic voltammetry
        elif (self._experimentType == CV):
            self._initPlot_CV(self._experimentParameters)

        # Normal pulse voltammetry
        elif (self._experimentType == NPV):
            self._initPlot_NPV(self._experimentParameters)
        
        # Differential pulse voltammetry
        elif (self._experimentType == DPV):
            self._initPlot_DPV(self._experimentParameters)

        # Square wave voltammetry
        elif (self._experimentType == SWV):
            self._initPlot_DPV(self._experimentParameters)

    def _initPlot_OCP(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize OCP plots for measuring
        the open circuit potential.

        TODO: Function not done

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        # Define window in x-direction
        self._ax.set_xlim([0 - PADDING_TIME_S, 0 + PADDING_TIME_S])

        # Define window in y-direction                    
        self._ax.set_ylim([0 - PADDING_VOLTAGE_MV, 0 + PADDING_VOLTAGE_MV])

        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_OCP_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_OCP_Y_LABEL)

    def _initPlot_CA(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize CA plots

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        # Intialize variable
        fLimitLeft : float = 0
        fLimitRight : float = 0
        fMaxPotential : float = 0
        fMinPotential : float = 0
        
        # Calculate experiment duration
        for iStep in range(len(listExperimentParameters[1][1])):
            fLimitRight += listExperimentParameters[1][1][iStep]
            
        fLimitRight += PADDING_TIME_S
        fLimitLeft = -PADDING_TIME_S

        # Define window in x-direction         
        self._ax.set_xlim([fLimitLeft, fLimitRight])

        # Search for max and min potential
        for iStep in range(len(listExperimentParameters[0][1])):
            if (fMaxPotential < listExperimentParameters[0][1][iStep]):
                fMaxPotential = listExperimentParameters[0][1][iStep]
            if (fMinPotential > listExperimentParameters[0][1][iStep]):
                fMinPotential = listExperimentParameters[0][1][iStep]

        # Decode LPTIA Rtia size
        iLPTIARtiaSize : int = _decode_LPTIA_Resistor_Size(
                                    listExperimentParameters[4][1])

        # Define window in y-direction                    
        self._ax.set_ylim([-0.9 * 1e6 / iLPTIARtiaSize - PADDING_CURRENT_UA, 
                            0.9 * 1e6 / iLPTIARtiaSize + PADDING_CURRENT_UA])
        
        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_CA_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_CA_Y_LABEL)    

    def _initPlot_LSV(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize LSV plots

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        fStartingPotential : float = listExperimentParameters[0][1]
        fStopPotential : float = listExperimentParameters[1][1]

        # Check which potential is lower
        if (fStartingPotential < fStopPotential):
            fLowerPotential = fStartingPotential
            fUpperPotential = fStopPotential
        else : 
            fLowerPotential = fStopPotential
            fUpperPotential = fStartingPotential
            
        # Define window in x-direction
        self._ax.set_xlim([fLowerPotential - PADDING_VOLTAGE_MV, 
                           fUpperPotential + PADDING_VOLTAGE_MV])

        # Decode LPTIA Rtia size
        iLPTIARtiaSize : int = _decode_LPTIA_Resistor_Size(
                                    listExperimentParameters[5][1])

        # Define window in y-direction                    
        self._ax.set_ylim([-0.9 * 1e6 / iLPTIARtiaSize - PADDING_CURRENT_UA, 
                            0.9 * 1e6 / iLPTIARtiaSize + PADDING_CURRENT_UA])

        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_CV_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_CV_Y_LABEL)

    def _initPlot_CV(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize plots of the CV family.
        This contains at the moment:
            - Cyclic voltammetry

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        fStartingPotential : float = listExperimentParameters[0][1]
        fLowerPotential : float = listExperimentParameters[1][1]
        fUpperPotential : float = listExperimentParameters[2][1]

        fLimitRight : float = max([fStartingPotential, fUpperPotential, 
                                   fLowerPotential]) + PADDING_VOLTAGE_MV            
        fLimitLeft : float = min([fStartingPotential, fUpperPotential, 
                                  fLowerPotential]) - PADDING_VOLTAGE_MV
            
        # Define window in x-direction
        self._ax.set_xlim([fLimitLeft, fLimitRight])

        # Decode LPTIA Rtia size
        iLPTIARtiaSize : int = _decode_LPTIA_Resistor_Size(
                                    listExperimentParameters[6][1])

        # Define window in y-direction                    
        self._ax.set_ylim([-0.9 * 1e6 / iLPTIARtiaSize - PADDING_CURRENT_UA, 
                            0.9 * 1e6 / iLPTIARtiaSize + PADDING_CURRENT_UA])

        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_CV_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_CV_Y_LABEL)

    def _initPlot_DPV(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize plots of the CV family.
        This contains at the moment:
            - Differential pulse voltammetry
            - Square wave voltammetry

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        fStartingPotential : float = listExperimentParameters[0][1]
        fStopPotential : float = listExperimentParameters[1][1]
        fDeltaV_peak : float = listExperimentParameters[3][1]

        fLimitRight : float = fStopPotential + fDeltaV_peak + PADDING_VOLTAGE_MV            
        fLimitLeft : float =  fStartingPotential - fDeltaV_peak - \
                              PADDING_VOLTAGE_MV
            
        # Define window in x-direction
        self._ax.set_xlim([fLimitLeft, fLimitRight])

        # Decode LPTIA Rtia size
        iLPTIARtiaSize : int = _decode_LPTIA_Resistor_Size(
                                    listExperimentParameters[6][1])

        # Define window in y-direction                    
        self._ax.set_ylim([-0.9 * 1e6 / iLPTIARtiaSize - PADDING_CURRENT_UA, 
                            0.9 * 1e6 / iLPTIARtiaSize + PADDING_CURRENT_UA])

        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_CV_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_CV_Y_LABEL)

    def _initPlot_NPV(self, listExperimentParameters : list) -> None:
        """
        Description
        -----------
        Sub method of the initPlot method to initialize plots of the CV family.
        This contains at the moment:
            - Normal pulse voltammetry

        Parameters
        ----------
        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        """
        fStartPotential : float = listExperimentParameters[1][1]
        fStopPotential : float = listExperimentParameters[2][1]
        fDeltaV_peak : float = listExperimentParameters[3][1]

        fLimitRight : float = fStopPotential + fDeltaV_peak + PADDING_VOLTAGE_MV            
        fLimitLeft : float =  fStartPotential - PADDING_VOLTAGE_MV
            
        # Define window in x-direction
        self._ax.set_xlim([fLimitLeft, fLimitRight])

        # Decode LPTIA Rtia size
        iLPTIARtiaSize : int = _decode_LPTIA_Resistor_Size(
                                    listExperimentParameters[6][1])

        # Define window in y-direction                    
        self._ax.set_ylim([-0.9 * 1e6 / iLPTIARtiaSize - PADDING_CURRENT_UA, 
                            0.9 * 1e6 / iLPTIARtiaSize + PADDING_CURRENT_UA])

        # Load labels from constants.py
        self._ax.axes.set_xlabel(PLOT_CV_X_LABEL)
        self._ax.axes.set_ylabel(PLOT_CV_Y_LABEL)

    def _initAnimate(self):
        """
        Description
        -----------
        Initialize the animation object

        Return
        ------
        `_lines`: list
            Reference to the 2D lines which is used to draw the data in the plot

        """
        return self._lines

    def _updateAnimate(self, frame, dataQueue) :
        """
        Description
        -----------
        Update the plot with new data depending on the choosen electrochemical 
        method.

        Return
        ------
        `_line`: Line2D
            Reference to the 2D line which is used to draw the data in the plot

        """      
        # Check if data is available in the queue
        while (dataQueue.empty() == False):
            listTemp : list = dataQueue.get()
            self._listStoredData.append(listTemp)

            self._insertDataInOutput(listTemp)

        # Check if sequence or single method should be plotted
        if (self._bPlotSequence == False):
            # Open circuit potential
            if (self._experimentType == OCP):
                # Update data lists
                for iData in range(len(self._listStoredData)):
                    # Pop oldest data from list
                    listCurrentData = self._listStoredData.pop(0)
                    
                    # Check cycle
                    self._iCycle = listCurrentData[iData][0]

                    # Append data to data lists
                    self._listfVoltage[self._iCycle - 1].append(
                        listCurrentData[iData][2])
                    self._listfTime[self._iCycle - 1].append(
                        listCurrentData[iData][3])

                # Update displayed data
                if (len(self._listfVoltage[self._iCycle - 1]) > 0):
                    self._lines[(self._iCycle - 1)]. \
                        set_data(self._listfTime[self._iCycle - 1], 
                        self._listfVoltage[self._iCycle - 1])

            # Cyclic voltammetry        | Linear sweep voltammetry 
            elif (self._experimentType == CV or 
                  self._experimentType == LSV):
                # Update data lists
                for iData in range(len(self._listStoredData)):
                    # Pop oldest data from list
                    listCurrentData = self._listStoredData.pop(0)

                    # Check cycle
                    self._iCycle = listCurrentData[0]

                    # Append data to data lists
                    self._listfVoltage[self._iCycle - 1].append(
                        listCurrentData[2])
                    self._listfCurrent[self._iCycle - 1].append(
                        listCurrentData[3])

                # Update displayed data
                if (len(self._listfVoltage[self._iCycle - 1]) > 0):
                    # Update Current vs Voltage plot
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                        set_data(self._listfVoltage[self._iCycle - 1], 
                                 self._listfCurrent[self._iCycle - 1])

                    # Set label of current Line2D which is plotted
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                        set_label(PLOT_CYCLE_NAME + ": " + str(self._iCycle))

                    # Update legend
                    self._ax.legend(title= PLOT_LEGEND_NAME, 
                                    bbox_to_anchor=(1.05, 1), 
                                    loc='upper left')

            # Chronoamperometry
            elif (self._experimentType == CA):
                # Update data lists
                for iData in range(len(self._listStoredData)):
                    # Pop oldest data from list
                    listCurrentData = self._listStoredData.pop(0)

                    # Check if cycle changed
                    if(self._iCycle != listCurrentData[0]):
                        # Update displayed data
                        self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                            set_data(self._listfTime[self._iCycle - 1], 
                                    self._listfCurrent[self._iCycle - 1])  

                    # Check cycle
                    self._iCycle = listCurrentData[0]

                    # Append data to data lists
                    self._listfCurrent[self._iCycle - 1].append(
                        listCurrentData[3])
                    self._listfTime[self._iCycle - 1].append(
                        listCurrentData[4])

                # Update displayed data
                if (len(self._listfCurrent[self._iCycle - 1]) > 0):
                    # Update Current vs time plot
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                        set_data(self._listfTime[self._iCycle - 1], 
                                 self._listfCurrent[self._iCycle - 1])  

                    # Set label of current Line2D which is plotted
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                        set_label(PLOT_CYCLE_NAME + ": " + str(self._iCycle))

                    # Update legend
                    self._ax.legend(title= PLOT_LEGEND_NAME, 
                                    bbox_to_anchor=(1.05, 1), 
                                    loc='upper left')

            # Square wave voltammetry   | Differential pulse voltammetry
            # Normal pulse voltammetry
            elif (self._experimentType == DPV or 
                  self._experimentType == SWV or
                  self._experimentType == NPV):
                # Update data lists
                while(len(self._listStoredData) >= 2):         
                    # Pop oldest data from list
                    listCurrentDataFirst = self._listStoredData.pop(0)
                    listCurrentDataSecond = self._listStoredData.pop(0) 

                    # Check cycle
                    self._iCycle = listCurrentDataFirst[0]
                    
                    # Append data to data lists
                    if(self._experimentType == NPV):
                        self._listfVoltage[self._iCycle - 1].append(
                            listCurrentDataSecond[2])
                        self._listfCurrent[self._iCycle - 1].append(
                            listCurrentDataSecond[3])

                    if(self._experimentType == DPV):
                        self._listfVoltage[self._iCycle - 1].append(
                            listCurrentDataFirst[2])
                        self._listfCurrent[self._iCycle - 1].append(
                            listCurrentDataSecond[3] - 
                            listCurrentDataFirst[3])

                    if(self._experimentType == SWV):
                        self._listfVoltage[self._iCycle - 1].append(
                            listCurrentDataSecond[2])
                        self._listfCurrent[self._iCycle - 1].append(
                            listCurrentDataFirst[3] - 
                            listCurrentDataSecond[3])

                # Update displayed data
                if (len(self._listfVoltage[self._iCycle - 1]) > 0):
                    # Update current vs voltage plot
                    for iIndex in range(self._iCycle):
                        self._lines[iIndex % self._iLine2DCounter]. \
                        set_data(self._listfVoltage[iIndex], 
                                 self._listfCurrent[iIndex])  

                    # Set label of current Line2D which is plotted
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                        set_label(PLOT_CYCLE_NAME + ": " + str(self._iCycle))

                    # Update legend
                    self._ax.legend(title= PLOT_LEGEND_NAME, 
                                    bbox_to_anchor=(1.05, 1), 
                                    loc='upper left')

        # Plot sequence
        elif (self._bPlotSequence == True):
            # Update data lists
            while(len(self._listStoredData) >= 2):   
                # Pop oldest data from list
                listCurrentDataFirst = self._listStoredData.pop(0)

                # Check if method has changed
                if (self._iDataPoint > listCurrentDataFirst[2]):
                    # Reset data in live plot
                    for iIndex in range(len(self._lines)):
                        self._lines[iIndex].set_data(0,0)
                        self._lines[iIndex].set_label("")

                    # Get the experiment parameters
                    listTempExpParameters = self._experimentParameters[2][
                        self._iMethodCounter % self._experimentParameters[0][1]][2]

                    # TODO

                    self._iDataPoints = self._calculateDatapoints(listCurrentDataFirst[8], listTempExpParameters)
                    self._fStepTime = self._calculateStepTime(listCurrentDataFirst[8], listTempExpParameters)


                    # Reconfig axes
                    if (listCurrentDataFirst[8] == OCP):
                        self._initPlot_OCP(listTempExpParameters)

                    elif (listCurrentDataFirst[8] == LSV):
                        self._initPlot_LSV(listTempExpParameters)

                    elif (listCurrentDataFirst[8] == CV):
                        self._initPlot_CV(listTempExpParameters)

                    elif (listCurrentDataFirst[8] == DPV or
                          listCurrentDataFirst[8] == SWV):
                        self._initPlot_DPV(listTempExpParameters)

                    elif (listCurrentDataFirst[8] == NPV):
                        self._initPlot_NPV(listTempExpParameters)

                    elif (listCurrentDataFirst[8] == CA):
                        self._initPlot_CA(listTempExpParameters)

                    # Set data for lines object depending on the current method
                    if (self._strCurrentMethod == CV or 
                        self._strCurrentMethod == LSV or 
                        self._strCurrentMethod == NPV or
                        self._strCurrentMethod == DPV or
                        self._strCurrentMethod == SWV):
                        # Update data for last result plot with new data
                        tempLine2D = self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].plot(
                            self._listfVoltage[self._iMethodCounter - 1]
                                              [self._iCycle - 1], 
                            self._listfCurrent[self._iMethodCounter - 1]
                                              [self._iCycle - 1])

                        # Add labels
                        self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].set_xlabel(
                            PLOT_CV_X_LABEL)
                        self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].set_ylabel(
                            PLOT_CV_Y_LABEL)

                    elif (self._strCurrentMethod == CA):
                        # Update data for last result plot with new data
                        tempLine2D = self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].plot(
                            self._listfTime[self._iMethodCounter - 1]
                                           [self._iCycle - 1], 
                            self._listfCurrent[self._iMethodCounter - 1]
                                              [self._iCycle - 1])

                        # Add labels
                        self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].set_xlabel(
                            PLOT_CA_X_LABEL)
                        self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].set_ylabel(
                            PLOT_CA_Y_LABEL)

                    if (self._strCurrentMethod != UNDEFIEND):
                        # Set label of current Line2D which is plotted
                        tempLine2D[0].set_label(PLOT_SEQUENCE_CYCLE_NAME + ": " + 
                            str(int((self._iMethodCounter - 1) / 
                            self._experimentParameters[0][1]) + 1) + " " +
                            PLOT_CYCLE_NAME + ": " + 
                            str(self._experimentParameters[2]
                            [(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]][1]))

                        # Update legend
                        self._listAxes[(self._iMethodCounter - 1) % 
                            self._experimentParameters[0][1]].legend(
                            title= PLOT_LEGEND_NAME, bbox_to_anchor=(1.05, 1), 
                            loc='upper left')

                    # Update drawing in result plot
                    self._listFig[(self._iMethodCounter - 1) % 
                        self._experimentParameters[0][1]].canvas.draw()

                    # Reset cycle
                    self._iCycle = 0

                    # Temporary save current method
                    self._strCurrentMethod = listCurrentDataFirst[8]

                    # Increase method counter
                    self._iMethodCounter += 1

                self._iDataPoint = listCurrentDataFirst[2] 

                # Check if cycle changed
                if (self._iCycle != listCurrentDataFirst[1]):
                    # Temporary save current cycle
                    self._iCycle = listCurrentDataFirst[1]

                # Append data to data lists
                if (self._strCurrentMethod == OCP or
                    self._strCurrentMethod == CA or
                    self._strCurrentMethod == LSV or
                    self._strCurrentMethod == CV):
                    self._listfVoltage[self._iMethodCounter - 1][self._iCycle - 1]. \
                        append(listCurrentDataFirst[3])
                    self._listfCurrent[self._iMethodCounter - 1][self._iCycle - 1]. \
                        append(listCurrentDataFirst[4])
                    self._listfTime[self._iMethodCounter - 1][self._iCycle - 1]. \
                        append(listCurrentDataFirst[5])

                elif (self._strCurrentMethod == NPV):
                    listCurrentDataSecond = self._listStoredData.pop(0)
                    self._listfVoltage[self._iMethodCounter - 1] \
                        [self._iCycle - 1].append(
                        listCurrentDataSecond[3])
                    self._listfCurrent[self._iMethodCounter - 1] \
                        [self._iCycle - 1].append(
                        listCurrentDataSecond[4])
                    self._listfTime[self._iMethodCounter - 1] \
                        [self._iCycle - 1].append(
                        listCurrentDataSecond[5])

                elif (self._strCurrentMethod == DPV):
                    listCurrentDataSecond = self._listStoredData.pop(0)
                    self._listfVoltage[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataFirst[3])
                    self._listfCurrent[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataSecond[4] - 
                        listCurrentDataFirst[4])
                    self._listfTime[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataFirst[5])

                elif (self._strCurrentMethod == SWV):
                    listCurrentDataSecond = self._listStoredData.pop(0)
                    self._listfVoltage[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataSecond[3])
                    self._listfCurrent[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataFirst[4] - 
                        listCurrentDataSecond[4])
                    self._listfTime[self._iMethodCounter - 1][self._iCycle - 1].append(
                        listCurrentDataSecond[5])

                elif (listCurrentDataFirst[8] == UNDEFIEND):
                    plt.close(self._fig)
                    pass

            # Update displayed data
            if (self._iCycle > 0 and 
                len(self._listfVoltage[self._iMethodCounter - 1][self._iCycle - 1]) > 0 and 
                len(self._listfCurrent[self._iMethodCounter - 1][self._iCycle - 1]) > 0 and 
                len(self._listfTime[self._iMethodCounter - 1][self._iCycle - 1]) > 0):

                # Set data for lines object depending on the current method
                if (self._strCurrentMethod == CV or 
                    self._strCurrentMethod == LSV):
                    # Update current vs voltage plot
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter].set_data(
                        self._listfVoltage[self._iMethodCounter - 1]
                                          [self._iCycle - 1], 
                        self._listfCurrent[self._iMethodCounter - 1]
                                          [self._iCycle - 1])
                elif (self._strCurrentMethod == NPV or
                      self._strCurrentMethod == SWV or
                      self._strCurrentMethod == DPV ):
                    for iIndex in range(self._iCycle):
                        self._lines[iIndex % self._iLine2DCounter].set_data(
                            self._listfVoltage[self._iMethodCounter - 1]
                                              [iIndex], 
                            self._listfCurrent[self._iMethodCounter - 1]
                                              [iIndex])  
                elif (self._strCurrentMethod == CA):
                    # Update current vs time plot
                    self._lines[(self._iCycle - 1) % self._iLine2DCounter].set_data(
                        self._listfTime[self._iMethodCounter - 1]
                                       [self._iCycle - 1], 
                        self._listfCurrent[self._iMethodCounter - 1]
                                          [self._iCycle - 1])

                # Set label of current Line2D which is plotted
                self._lines[(self._iCycle - 1) % self._iLine2DCounter]. \
                    set_label(PLOT_CYCLE_NAME + ": " + str(self._iCycle))

                # Update legend
                self._ax.legend(title= PLOT_LEGEND_NAME, 
                                bbox_to_anchor=(1.05, 1), 
                                loc='upper left')

        return self._lines        

    def T_Animate(self, dataQueue) -> None:
        """
        Description
        -----------
        Show the plot and start dynamically updating the plot

        `dataQueue` : Queue
            Data queue used as pipe between the different processes

        """
        self._animate = FuncAnimation(self._fig, 
                                      self._updateAnimate,  
                                      fargs= (dataQueue,),
                                      frames= self.dataGenerator,
                                      init_func=self._initAnimate,
                                      interval= 2,
                                      repeat= False, 
                                      blit=False)

        # Show plot if FreiStat is not used in a GUI   
        if (self._strMode == FREISTAT_STANDALONE):
            plt.show()

    def T_Print(self, iInterval : int, dataQueue) -> None:
        """
        Description
        -----------
        Alternative method to the T_Animate method, which only prints the data
        to the specified output

        Parameters
        ----------
        `iInterval` : int
            Time at which the data should be updated in ms

        `dataQueue` : Queue
            Data queue used as pipe between the different processes

        """
        self._animate = FuncAnimation(self._fig, 
                                      self._outputData,  
                                      fargs= (dataQueue,),
                                      frames= self.dataGenerator,
                                      init_func=self._initAnimate,
                                      interval= iInterval,
                                      repeat= False, 
                                      blit=False)

    def _outputData(self, frame, dataQueue) :
        """
        Description
        -----------
        Method called from the func animation if only the data should be 
        outputted to the specified source

        Parameters
        ----------
        `dataQueue` : Queue
            Data queue used as pipe between the different processes
            
        """
        while (dataQueue.empty() == False):
            listTemp : list = dataQueue.get()
            self._insertDataInOutput(listTemp)

    def _insertDataInOutput(self, listData):
        """
        Description
        -----------
        Method for printing the experiment results

        Parameters
        ----------
        `listData` : List
            List containing the last data point
            
        """
        # Check for sequence mode
        if (self._bPlotSequence == False):
            if (self._strMode == FREISTAT_BACKEND):
                self._listBox.insert("end",
                    "Cycle: " + str(listData[0]) + "\t - " +
                    "Datapoint:  " + str(listData[1]) + "\t - \t" + 
                    "Voltage: " + str(listData[2]) + " mV\t\t - \t" +
                    "Current: " + str(listData[3]) + " \u03BCA\t\t - \t" +
                    "Time: " + str(listData[4]) + " ms")
                self._listBox.yview("end")

                # Calulate Experiment runtime as countdown
                iRuntime : int = int((self._iDataPoints - listData[1]) * self._fStepTime)
                iDays : int = int(iRuntime / 86400)
                iHours : int = int((iRuntime - iDays * 86400) / 3600)
                iMinutes: int = int((iRuntime - iHours * 3600) / 60)
                self._StyleConfig.configure(self._progressBar['style'], 
                    text= "{0:02d}:{1:02d}:{2:02d}:{3:02d}           ".format(
                        iDays, iHours, iMinutes, iRuntime % 60))
                self._progressBar['value'] = 100/ self._iDataPoints * listData[1]

            elif (self._strMode == FREISTAT_STANDALONE):
                print("Cycle: " + str(listData[0]) + "\t - " +
                    "Datapoint:  " + str(listData[1]) + "\t - \t" + 
                    "Voltage: " + str(listData[2]) + " mV\t\t - \t" +
                    "Current: " + str(listData[3]) + " \u03BCA\t\t - \t" +
                    "Time: " + str(listData[4]) + " ms")

        else :
            # Check if data is empty and return
            if (listData[8] == UNDEFIEND):
                return

            if (self._strMode == FREISTAT_BACKEND):
                self._listBox.insert("end",
                    "Sequence cycle: " + str(listData[0]) + "\t - " +
                    "Cycle: " + str(listData[1]) + "\t - " +
                    "Datapoint:  " + str(listData[2]) + "\t - \t" + 
                    "Voltage: " + str(listData[3]) + " mV\t\t - \t" +
                    "Current: " + str(listData[4]) + " \u03BCA\t\t - \t" +
                    "Time: " + str(listData[5]) + " ms")
                self._listBox.yview("end")

                # Calulate Experiment runtime as countdown
                iRuntime : int = int((self._iDataPoints - listData[2]) * self._fStepTime)
                
                iDays : int = int(iRuntime / 86400)
                iHours : int = int((iRuntime - iDays * 86400) / 3600)
                iMinutes: int = int((iRuntime - iHours * 3600) / 60)
                self._StyleConfig.configure(self._progressBar['style'], 
                    text= "{0:02d}:{1:02d}:{2:02d}:{3:02d}           ".format(
                        iDays, iHours, iMinutes, iRuntime % 60))
                self._progressBar['value'] = 100/ self._iDataPoints * listData[2]

            elif (self._strMode == FREISTAT_STANDALONE):
                print("Sequence cycle:" + str(listData[0]) + "\t - " +
                    "Cycle: " + str(listData[1]) + "\t - " +
                    "Datapoint:  " + str(listData[2]) + "\t - \t" + 
                    "Voltage: " + str(listData[3]) + " mV\t\t - \t" +
                    "Current: " + str(listData[4]) + " \u03BCA\t\t - \t" +
                    "Time: " + str(listData[5]) + " ms")

    def dataGenerator(self):
        """
        Description
        -----------
        Generator, which is used to generate frames for the func animate.
        
        """
        # Intialize variables
        iGeneratorTicks = 0

        # Look until process, which handles the data of the serial port is done
        while(self._process.is_alive()):
            # Update to next tick
            iGeneratorTicks += 1
            yield iGeneratorTicks

    def attachEvent(self, pressEvent) -> None:
        """
        Description
        -----------
        Method to attack press event to matplot window

        Parameters
        ----------  
        pressEvent : Event
            Event which should be executable in the live plot.

        """
        # Attach press event to window
        self._fig.canvas.mpl_connect('key_press_event', 
            lambda event: pressEvent(event))

        for iIndex in range(len(self._listFig)):
            self._listFig[iIndex].canvas.mpl_connect('key_press_event', 
                lambda event: pressEvent(event))

    def _calculateStepTime(self, strMethod : str, 
                           listExperimentParameters : list) -> float:
        """
        Description
        -----------
        Calculate the rough duration of the experiment

        Parameters
        ----------
        `strMethod` : str
            String containing the experiment type

        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        Return
        ------
        `fStepTime` : float
            Time needed for one datapoint

        """   
        # Intialize variables
        fStepTime : float = 0.0

        if (strMethod == SEQUENCE):
            fStepTime = self._calculateStepTime(listExperimentParameters[2][0][0],
                listExperimentParameters[2][0][2])
        elif (strMethod == CA):
            fStepTime =  listExperimentParameters[2][1] / 1000
        elif (strMethod == LSV):
            fStepTime = listExperimentParameters[2][1] / \
                        listExperimentParameters[3][1]
        elif (strMethod == CV):
            fStepTime = listExperimentParameters[3][1] / \
                        listExperimentParameters[4][1]  
        elif (strMethod == NPV or strMethod == DPV or strMethod == SWV):
            fStepTime = (listExperimentParameters[4][1][0] + \
                         listExperimentParameters[4][1][1]) / 2000
        return fStepTime

    def _calculateDatapoints(self, strMethod : str, 
                             listExperimentParameters : list) -> int:
        """
        Description
        -----------
        Calculate the amount of datapoints in the experiment

        Parameters
        ----------
        `strMethod` : str
            String containing the experiment type

        `listExperimentParameters` : list
            List containing the experiment parameters for the specific method

        Return
        ------
        `iDatapoints` : int
            Datapoints measured during the experiment

        """      
        # Intialize variables
        iDatapoints : int = 0

        if (strMethod == SEQUENCE):
            iDatapoints = self._calculateStepTime(listExperimentParameters[2][0][0],
                listExperimentParameters[2][0][2])
        elif (strMethod == CA):
            fDuration : float = 0.0
            for iIndex in range(len(listExperimentParameters[1][1])):
                fDuration += listExperimentParameters[1][1][iIndex]
            iDatapoints = fDuration / listExperimentParameters[2][1] * \
                          listExperimentParameters[3][1]  
            print(iDatapoints)
        elif (strMethod == LSV):
            iDatapoints = int(abs(listExperimentParameters[0][1] - 
                                  listExperimentParameters[1][1]) / 
                              listExperimentParameters[2][1] * 
                              listExperimentParameters[4][1])
        elif (strMethod == CV):
            iDatapoints = int(abs(listExperimentParameters[1][1] - 
                                  listExperimentParameters[2][1]) * 2 / 
                              listExperimentParameters[3][1] * 
                              listExperimentParameters[5][1])
        elif (strMethod == NPV):
            iDatapoints = int(abs(listExperimentParameters[1][1] - 
                                  listExperimentParameters[2][1]) / \
                              listExperimentParameters[3][1] * \
                              listExperimentParameters[6][1] * 2)
        elif (strMethod == DPV or strMethod == SWV):
            iDatapoints = int(abs(listExperimentParameters[0][1] - 
                                  listExperimentParameters[1][1]) / \
                              listExperimentParameters[2][1] * \
                              listExperimentParameters[6][1] * 2)
        return iDatapoints

    def set_listBox(self, listBox) -> None :
        """
        Description
        -----------
        Setter method for saving the reference to the list box element

        Parameters
        ----------
        `listBox` : ListBox
            Tkinter list box element working as a terminal

        """
        self._listBox = listBox

    def set_progressBar(self, progressBar) -> None:
        """
        Description
        -----------
        Setter method for saving the reference to the progress bar element

        Parameters
        ----------
        `progressBar` : ProgressBar
            Tkinter progress bar element working as a terminal

        """      
        self._progressBar = progressBar  

    def get_figure(self):
        """
        Description
        -----------
        Getter method returning reference to the live feed.

        Return
        ------
        `LiveFeed` : Figure
            Matplotlib figure of the live feed

        """
        return self._fig

    def get_listfigures(self):
        """
        Description
        -----------
        Getter method returning the reference to the static plots

        Return
        ------
        `listFigures` : list
            List of Matplotlib figures

        """
        return self._listFig

    def get_animate(self):
        """
        Description
        -----------
        Getter method returning reference to the animate function

        Return
        ------
        `animate` : FuncAnimation
            Matplotlib FuncAnimation which is used to display the live feed

        """
        return self._animate
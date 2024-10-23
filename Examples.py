"""
Python script implementing different examples on how to use the FreiStat Python
library for the different kinds of electrochemical methods.

The example contains the following examples, which can be selected by changing
the inital value of iExperiment:
iExperiment : EC-Method
0           : Sequence: CV - CA - LSV
1           : Chronoamperometry (CA)
2           : Open circuit potential (OCP)   TODO not working (Hardware restriction)
3           : Linear sweep voltammetry (LSV)
4           : Cyclic voltammetry (CV)
5           : Normal pulse voltammetry (NPV)
6           : Differential pulse voltammetry (DPV)
7           : Square wave voltammetry (SWV)

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import internal dependencies
from Python.FreiStat.Data_storage.constants import *
from Python.FreiStat.Methods.run_chronoamperometry import Run_CA
from Python.FreiStat.Methods.run_open_circuit_potential import Run_OCP
from Python.FreiStat.Methods.run_linear_sweep_voltammetry import Run_LSV
from Python.FreiStat.Methods.run_cyclic_voltammetry import Run_CV
from Python.FreiStat.Methods.run_normal_pulse_voltammetry import Run_NPV
from Python.FreiStat.Methods.run_differential_pulse_voltammetry import Run_DPV
from Python.FreiStat.Methods.run_square_wave_voltammetry import Run_SWV
from Python.FreiStat.Methods.run_electrochemical_impedance_spectroscopy import Run_EIS

from Python.FreiStat.Methods.run_sequence import Run_Sequence

from Python.FreiStat.Utility.recovery import Recovery_Data

def ExampleImplementation():
    """
    Example method implementing the different electrochemical methods which are
    supported by the FreiStat software.
    
    """
    # Change this parameter to switch between the different electrochemical
    # methods (see top for more information)
    iExperiment : int = 8

    if (iExperiment == 0):
        # Run sequence on FreiStat
        C_Run_Sequence = Run_Sequence(EnableOptimizer= False)

        # 1. Entry in sequence: Add CV
        C_Run_Sequence.add_CV(StartVoltage= 0.5,
                              FirstVertex= -0.62,
                              SecondVertex= 1.05,
                              Stepsize= 0.005,
                              Scanrate= 0.6,
                              Cycle= 3,
                              CurrentRange= 45e-6,
                              FixedWEPotential= True,
                              MainsFilter = False,
                              Sinc2_Oversampling = 667,
                              Sinc3_Oversampling = 2)

        # 2. Entry in sequence: Add CA
        C_Run_Sequence.add_CA(Potential_Steps= [-0.1, 0.05, 1],
                              Pulse_Lengths=[0.3, 0.5, 0.4],
                              Sampling_Rate= 0.02,
                              Cycle= 5,
                              CurrentRange= 15e-6, 
                              MainsFilter = False,
                              Sinc2_Oversampling = 667,
                              Sinc3_Oversampling = 2,
                              Progressive_Measurement = False)
        
        # 3. Entry in sequence: Add LSV
        C_Run_Sequence.add_LSV(StartVoltage= 0.08,
                               StopVoltage= 0.2,
                               Stepsize= 0.001,
                               Scanrate= 0.2,
                               Cycle= 3,
                               CurrentRange= 15e-6,
                               FixedWEPotential= True,
                               MainsFilter = False,
                               Sinc2_Oversampling = 150,
                               Sinc3_Oversampling = 2)
        
        # 4. Entry in sequence: Add NPV
        C_Run_Sequence.add_NPV(BaseVoltage= 0.3,
                               StartVoltage= 0.5,
                               StopVoltage= 1.025,
                               DeltaV_Staircase= 0.02,
                               Pulse_Lengths= [0.03, 0.07],
                               Sampling_Duration= 0.01,
                               Cycle= 2,
                               CurrentRange= 45e-6,
                               FixedWEPotential= True,
                               MainsFilter = False,
                               Sinc2_Oversampling = 25,
                               Sinc3_Oversampling = 2)

        # 5. Entry in sequence: Add DPV
        C_Run_Sequence.add_DPV(StartVoltage= 0.0,
                               StopVoltage= 1.4,
                               DeltaV_Staircase= 0.01,
                               DeltaV_Peak= 0.02,
                               Pulse_Lengths= [0.01, 0.01],
                               Sampling_Duration= 0.005,
                               Cycle= 4,
                               CurrentRange= 45e-6,
                               FixedWEPotential= True,
                               MainsFilter = False,
                               Sinc2_Oversampling = 25,
                               Sinc3_Oversampling = 2)

        # 6. Entry in sequence: Add SWV
        C_Run_Sequence.add_SWV(StartVoltage= 0.0,
                               StopVoltage= 1.4,
                               DeltaV_Staircase= 0.05,
                               DeltaV_Peak= 0.2,
                               DutyCycle= 0.1,
                               Sampling_Duration= 0.005,
                               Cycle= 3,
                               CurrentRange= 45e-6,
                               FixedWEPotential= True,
                               MainsFilter = False,
                               Sinc2_Oversampling = 25,
                               Sinc3_Oversampling = 2)
        
        strExportpath = C_Run_Sequence.start(SequenceCycles= 3,
                                             LowPerformanceMode= False)

    elif (iExperiment == 1):
        # Run chronoamperometry on FreiStat
        strExportpath = Run_CA().start(Potential_Steps= [0.5, -.5],
                                       Pulse_Lengths=[5, 5],
                                       Sampling_Rate= 0.01,
                                       Cycle= 3,
                                       CurrentRange= 45e-5, 
                                       MainsFilter= False,
                                       Sinc2_Oversampling = 22,
                                       Sinc3_Oversampling = 2,
                                       LowPerformanceMode= False,
                                       EnableOptimizer= False,
                                       Progressive_Measurement = False)
    elif (iExperiment == 2):
        # Run chronoamperometry on FreiStat
        strExportpath = Run_OCP().start(Measurement_Length= 200.0,
                                        Sampling_Rate= 10.0,
                                        Cycle= 3,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 667,
                                        Sinc3_Oversampling = 4,
                                        LowPerformanceMode= False)

    elif (iExperiment == 3):
        # Run linear sweep voltammetry on FreiStat
        strExportpath = Run_LSV().start(StartVoltage= 0.08,
                                        StopVoltage= 0.4,
                                        Stepsize= 0.001,
                                        Scanrate= 0.2,
                                        Cycle= 2,
                                        CurrentRange= 45e-6,
                                        FixedWEPotential= True,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 150,
                                        Sinc3_Oversampling = 2,
                                        LowPerformanceMode= False)

    elif (iExperiment == 4):
        # Run cyclic voltammetry on FreiStat
        strExportpath = Run_CV().start(StartVoltage= 0.5,
                                       FirstVertex= -0.22,
                                       SecondVertex= 0.55,
                                       Stepsize= 0.002,
                                       Scanrate= 0.8,
                                       Cycle= 1,
                                       CurrentRange= 45e-6,
                                       FixedWEPotential= True,
                                       MainsFilter = True,
                                       Sinc2_Oversampling = 222,
                                       Sinc3_Oversampling = 2,
                                       EnableOptimizer= True,
                                       LowPerformanceMode= False)

    elif (iExperiment == 5):
        # Run normal pulse voltammetry on FreiStat
        strExportpath = Run_NPV().start(BaseVoltage= 0.3,
                                        StartVoltage= 0.5,
                                        StopVoltage= 1.025,
                                        DeltaV_Staircase= 0.02,
                                        Pulse_Lengths= [0.03, 0.07],
                                        Sampling_Duration= 0.01,
                                        Cycle= 1,
                                        CurrentRange= 45e-6,
                                        FixedWEPotential= True,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 25,
                                        Sinc3_Oversampling = 4,
                                        LowPerformanceMode= False)

    elif (iExperiment == 6):
        # Run differential pulse voltammetry on FreiStat
        strExportpath = Run_DPV().start(StartVoltage= 0.0,
                                        StopVoltage= 1.4,
                                        DeltaV_Staircase= 0.05,
                                        DeltaV_Peak= 0.2,
                                        Pulse_Lengths= [0.4, 0.6],
                                        Sampling_Duration= 0.04,
                                        Cycle= 1,
                                        CurrentRange= 45e-6,
                                        FixedWEPotential= True,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 22,
                                        Sinc3_Oversampling = 4,
                                        EnableOptimizer= True,
                                        LowPerformanceMode= False)

    elif (iExperiment == 7):
        # Run square wave voltammetry on FreiStat
        strExportpath = Run_SWV().start(StartVoltage= 0.0,
                                        StopVoltage= 1.4,
                                        DeltaV_Staircase= 0.05,
                                        DeltaV_Peak= 0.2,
                                        DutyCycle= 0.1,
                                        Sampling_Duration= 0.01,
                                        Cycle= 1,
                                        CurrentRange= 45e-6,
                                        FixedWEPotential= True,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 100,
                                        Sinc3_Oversampling = 4,
                                        LowPerformanceMode= False)

    elif (iExperiment == 8):
        # Run electrochemical impedance spectroscopy

        strExportpath = Run_EIS().start(start_frequency= 1,
                                        stop_frequency= 100,
                                        ac_amplitude= 0.4,
                                        dc_offset = 0.0,
                                        num_points= 100,
                                        sweep_typ= True,
                                        MainsFilter = False,
                                        Sinc2_Oversampling = 22,
                                        Sinc3_Oversampling = 4,
                                        EnableOptimizer= False,
                                        LowPerformanceMode= False
                                        )

    # Show file-path of the collected data in the terminal
    print(strExportpath)

# Run example implementation if this script is the main file
if __name__ == '__main__':
    ExampleImplementation()
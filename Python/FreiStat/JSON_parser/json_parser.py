"""
Module for parsing JSON strings into internal data structure of the FreiStat
Software.

"""

__author__ = "Mark Jasper"
__contact__ = "University of Freiburg, IMTEK, Jochen Kieninger"
__credits__ = "Mark Jasper"

__version__ = "1.0.0"
__maintainer__ = "Mark Jasper"
__email__ = "mark.jasper@imtek.uni-freiburg.de, kieninger@imtek.uni-freiburg.de"

# Import dependencies
import logging
from typing import Union

# Import internal dependencies
from ..Data_storage.data_software_storage import DataSoftwareStorage

class JSON_Parser:
    """
    Description
    -----------
    Class for parsing incoming and outgoing data from or into JSON format.

    """

    def __init__(self) -> None:
        """
        Description
        -----------
        Constructor of class JSON_Parser.

        """

    def __init__(self, dataSoftwareStorage: DataSoftwareStorage) -> None:
        """
        Description
        -----------
        Overloaded constructor of class JSON_Parser.

        Parameters
        ----------
        `dataSoftwareStorage` : DataSoftwareStorage
            Reference to data software storage object

        """
        # Save reference to data storage object and set own reference
        self._dataSoftwareStorage = dataSoftwareStorage
        self._dataSoftwareStorage.setJSON_Parser(self)

    def parse_JSON_string(self, listJSONdata: list, strJSON: str) -> \
        Union[int, bool, list]:
        """
        Description
        -----------
        Parse input string into list structure.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string in JSON format

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Initialize variables
        iJSON_Length : int = strJSON.__len__()
        iCurrentPosition :int = 0

        # Call check object method to start parsing, since object is the origin
        # of the JSON-telegrams used by FreiStat
        iCurrentPosition, bErrorFlag, listJSONdata = self._check_Object(
            listJSONdata, strJSON, iJSON_Length, iCurrentPosition)

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_WhiteSpaces(self,
                           listJSONdata: list,
                           strJSON: str, 
                           iJSON_Length: int, 
                           iCurrentPosition: int) -> \
                           Union[int, list]:
        """
        Description
        -----------
        Checks if the next positions in the string are whitespaces  \n
        (space, linefeed, carriage return, horizontal tab).

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking  

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_WhiteSpaces\n")

        # Skip through all whitespaces
        for iPosition in range(iCurrentPosition,iJSON_Length):
            # Check for whitespace
            if(strJSON[iPosition] == " "):
                pass
            else:
                # TODO implement the other types of whitespaces
                break

            # Move to next position in string
            iCurrentPosition += 1

        # Return union
        return iCurrentPosition, listJSONdata   

    def _check_String(self, 
                      listJSONdata: list,
                      strJSON: str, 
                      iJSON_Length: int, 
                      iCurrentPosition: int) -> \
                      Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a valid JSON string.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_String\n")

        # Initalize variables
        bErrorFlag : bool = False

        # Check for string begin '"'
        if (strJSON[iCurrentPosition] == "\""):
            # Check for string
            for x in range(iCurrentPosition,iJSON_Length):
                if(strJSON[x] == "\\"):
                    # Move to the next position in the string
                    iCurrentPosition += 1

                    if(strJSON[x] == "\"" or
                       strJSON[x] == "\\" or
                       strJSON[x] == "\/" or
                       strJSON[x] == "b" or
                       strJSON[x] == "f" or
                       strJSON[x] == "n" or
                       strJSON[x] == "r" or
                       strJSON[x] == "t" or
                       strJSON[x] == "u"):

                        # Move to the next position in the string
                        iCurrentPosition += 1

                        # Check for string end '"'
                        if(strJSON[iCurrentPosition] == "\""):
                            break
                        elif (strJSON[iCurrentPosition] == "\\"):
                            pass
                    else:
                        bErrorFlag = True                       
                else:
                    # Move to the next position in the string
                    iCurrentPosition += 1

                    # Check for string end '"'
                    if(strJSON[iCurrentPosition] == "\""):
                        break    
        else:
            bErrorFlag = True  

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Digit(self, 
                     listJSONdata: list,
                     strJSON: str, 
                     iJSON_Length: int, 
                     iCurrentPosition: int) -> \
                     Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string are digits 0-9.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Digit\n")

        # Initalize variables
        bErrorFlag : bool = False
        
        for iPosition in range(iCurrentPosition,iJSON_Length):
                # Check if 0-9 appears
                if(strJSON[iPosition] in {"1", "2", "3", "4", "5", 
                                          "6", "7", "8", "9", "0"}):
                    if(iCurrentPosition < iJSON_Length - 1):
                        # Move to the next position in the string
                        iCurrentPosition += 1
                    pass
                # Break if fraction or exponent begins
                elif (strJSON[iPosition] in {".", "E", "e"}):
                    break
                # Next JSON structure begins
                elif (strJSON[iPosition] in {",", "}","]"}):
                    break      
                # Raise error if another character appears
                else:
                    bErrorFlag = True
                    break

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Number(self, 
                      listJSONdata: list,
                      strJSON: str, 
                      iJSON_Length: int, 
                      iCurrentPosition: int) -> \
                      Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a valid JSON number.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON`: string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Number\n")

        # Initalize variables
        bErrorFlag : bool = False

        # Check if number startw with "-" or a number if something else appears
        # it's not a number
        if (strJSON[iCurrentPosition] == "-"):
            iCurrentPosition += 1
        elif(strJSON[iCurrentPosition] in {"1", "2", "3", "4", "5", 
                                          "6", "7", "8", "9", "0"}):
            pass
        else:
            bErrorFlag = True
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata

        # Check for digits
        if (strJSON[iCurrentPosition] in {"1", "2", "3", "4", "5", 
                                          "6", "7", "8", "9"}):
            # Enter checking Digit
            if(iCurrentPosition < iJSON_Length - 1):
                # Move to the next position in the string
                iCurrentPosition += 1    

                # Call _check_Digit method
                iCurrentPosition, bErrorFlag, listJSONdata \
                = self._check_Digit(listJSONdata, 
                                    strJSON, 
                                    iJSON_Length, 
                                    iCurrentPosition)
                # Check if error occured
                if (bErrorFlag == True):
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata                                                            
            else:
                pass
            
        # Check for 0 (in case of decimals)
        elif (strJSON[iCurrentPosition] == "0"):
            # Move to the next position in the string
            iCurrentPosition += 1

        # Return in error case
        else:
            bErrorFlag = True
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata
        
        # Check for fraction
        if (strJSON[iCurrentPosition] == "."):
            # Enter checking Digit
            if(iCurrentPosition < iJSON_Length - 1):
                # Move to the next position in the string
                iCurrentPosition += 1
            else:
                bErrorFlag = True
                # Return union
                return iCurrentPosition, bErrorFlag, listJSONdata    

            # Call _check_Digit            
            iCurrentPosition, bErrorFlag, listJSONdata \
            = self._check_Digit(listJSONdata, 
                                strJSON, 
                                iJSON_Length, 
                                iCurrentPosition)
            # Check if error occured
            if (bErrorFlag == True):
                # Return union
                return iCurrentPosition, bErrorFlag, listJSONdata

        # Check for exponent
        if (strJSON[iCurrentPosition] in {"E", "e"}):
            # Move to the next position in the string
            iCurrentPosition += 1

            # Check for sign
            if (strJSON[iCurrentPosition] in {"+", "-"}):
                pass
            elif (strJSON[iCurrentPosition] in {"1", "2", "3", "4", "5", 
                                          "6", "7", "8", "9", "0"}):
                pass
            else:
                bErrorFlag = True
                # Return union
                return iCurrentPosition, bErrorFlag, listJSONdata

            # Enter checking Digit
            if (iCurrentPosition < iJSON_Length - 1):
                # Move to the next position in the string
                iCurrentPosition += 1

                # Call _check_Digit method
                iCurrentPosition, bErrorFlag, listJSONdata \
                = self._check_Digit(listJSONdata, 
                                    strJSON, 
                                    iJSON_Length, 
                                    iCurrentPosition)            
            # Check if error occured
            if (bErrorFlag == True):
                # Return union
                return iCurrentPosition, bErrorFlag, listJSONdata

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Array(self, 
                     listJSONdata: list,
                     strJSON: str, 
                     iJSON_Length: int, 
                     iCurrentPosition: int) -> \
                     Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a valid JSON array.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Array\n")

        # Initalize variables
        bErrorFlag : bool = False

        if(strJSON[iCurrentPosition] == "["):
            # Move to the next position in the string
            iCurrentPosition += 1
            iTempCurrentPosition = iCurrentPosition

            # Call _check_WhiteSpaces method
            iCurrentPosition, listJSONdata \
            = self._check_WhiteSpaces(listJSONdata, 
                                      strJSON, 
                                      iJSON_Length, 
                                      iCurrentPosition)

            if (iTempCurrentPosition == iCurrentPosition):
                # Call _check_Value method
                iCurrentPosition, bErrorFlag, listJSONdata \
                = self._check_Value(listJSONdata, 
                                    strJSON, 
                                    iJSON_Length, 
                                    iCurrentPosition)

                # Check if error has occured
                if (bErrorFlag == True):
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata

                if (iCurrentPosition < iJSON_Length - 1):
                    # Move to the next position in the string
                    iCurrentPosition += 1

                # Check for ","
                for iPosition in range(iCurrentPosition, iJSON_Length):
                    if (iPosition < iCurrentPosition):
                        pass
                    else:
                        if (strJSON[iCurrentPosition] == ","):
                            # Move to the next position in the string
                            iCurrentPosition += 1

                            # Call _check_Value method
                            iCurrentPosition, bErrorFlag, listJSONdata  \
                            = self._check_Value(listJSONdata, 
                                                strJSON, 
                                                iJSON_Length, 
                                                iCurrentPosition)
                            
                            # Check if error has occured
                            if (bErrorFlag == True):
                                # Return union
                                return iCurrentPosition, bErrorFlag, listJSONdata

                        elif (strJSON[iCurrentPosition] == "]"):
                            if (iCurrentPosition < iJSON_Length - 1):
                                # Move to the next position in the string
                                iCurrentPosition += 1
                            break         
                        else:
                            bErrorFlag = True
                            # Return union
                            return iCurrentPosition, bErrorFlag, listJSONdata                
            else: 
                if(strJSON[iCurrentPosition] == "]"):
                    if (iCurrentPosition < iJSON_Length - 1):
                        # Move to the next position in the string
                        iCurrentPosition += 1
                    pass
                else: 
                    bErrorFlag = True
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata                                     
        else:
            bErrorFlag = True
        
        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Bool(self, 
                    listJSONdata: list,
                    strJSON: str, 
                    iJSON_Length: int, 
                    iCurrentPosition: int) -> \
                    Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a boolean
        (True, False, Null).

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Bool\n")

        # Initalize variables
        bErrorFlag : bool = False

        if (strJSON[iCurrentPosition] == "t" or "f" or "n"):
            # Check fpr "true"
            if (strJSON[iCurrentPosition] == "t" and
                iCurrentPosition + 3 <= iJSON_Length - 1):
                if(strJSON[iCurrentPosition+1] == "r" and
                   strJSON[iCurrentPosition+2] == "u" and 
                   strJSON[iCurrentPosition+3] == "e"):
                   # Skip the literal
                   iCurrentPosition += 3
                else:
                    bErrorFlag = True 

            # Check for "false"
            elif (strJSON[iCurrentPosition] == "f" and
                  iCurrentPosition + 4 <= iJSON_Length - 1):
                if(strJSON[iCurrentPosition+1] == "a" and
                   strJSON[iCurrentPosition+2] == "l" and
                   strJSON[iCurrentPosition+3] == "s" and  
                   strJSON[iCurrentPosition+4] == "e"):
                    # Skip the literal
                   iCurrentPosition += 4
                else:
                    bErrorFlag = True
                    
            # Check for "null"
            elif (strJSON[iCurrentPosition] == "n"and
                  iCurrentPosition + 3 <= iJSON_Length - 1):
                if(strJSON[iCurrentPosition+1] == "u" and
                   strJSON[iCurrentPosition+2] == "l" and 
                   strJSON[iCurrentPosition+3] == "l"):
                   # Skip the literal
                   iCurrentPosition += 3
                else:
                    bErrorFlag = True
            else:
                bErrorFlag = True     

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Value(self, 
                     listJSONdata: list,
                     strJSON: str, 
                     iJSON_Length: int, 
                     iCurrentPosition: int) -> \
                     Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a valid JSON value.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Value\n")

        # Initalize variables
        bErrorFlag : bool = False
        bErrorFlags : list = [False, False, False, False, False]

        # Check for whitespace before value
        iCurrentPosition, listJSONdata \
        = self._check_WhiteSpaces(listJSONdata, 
                                  strJSON, 
                                  iJSON_Length, 
                                  iCurrentPosition)

        # Either check for one of the following:
        # string, number, object, array, boolean
        # If all fail return error

        # Check for JSON object
        listTempJSONdata: list = []
        iCurrentPosition, bErrorFlag, listTempJSONdata \
        = self._check_Object(listTempJSONdata, 
                             strJSON, 
                             iJSON_Length, 
                             iCurrentPosition)

        # Check if error has occured
        if (bErrorFlag == True):
            bErrorFlag = False
            bErrorFlags[0] = True
        elif(bErrorFlag == False):
            listJSONdata.append(listTempJSONdata)
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata                       

        # Safe iCurrentPosition
        iTempPosition = iCurrentPosition

        # Check for JSON string
        iCurrentPosition, bErrorFlag, listJSONdata \
        = self._check_String(listJSONdata, 
                             strJSON, 
                             iJSON_Length, 
                             iCurrentPosition)        

        # Check if error has occured
        if (bErrorFlag == True):
            bErrorFlag = False
            bErrorFlags[1] = True  
        elif(bErrorFlag == False):
            if (iTempPosition != iCurrentPosition):
                listJSONdata.append(strJSON[iTempPosition:iCurrentPosition])
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata     

        # Safe iCurrentPosition
        iTempPosition = iCurrentPosition

        # Check for number
        iCurrentPosition, bErrorFlag, listJSONdata \
        = self._check_Number(listJSONdata, 
                             strJSON, 
                             iJSON_Length, 
                             iCurrentPosition) 
        
        # Check if error has occured
        if (bErrorFlag == True):
            bErrorFlag = False
            bErrorFlags[2] = True  
        elif(bErrorFlag == False):
            if (iTempPosition != iCurrentPosition):
                listJSONdata.append(strJSON[iTempPosition:iCurrentPosition])
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata     

        # Check for Array
        iCurrentPosition, bErrorFlag, listJSONdata \
        = self._check_Array(listJSONdata, 
                            strJSON, 
                            iJSON_Length, 
                            iCurrentPosition) 
        # Check if error has occured
        if (bErrorFlag == True):
            bErrorFlag = False
            bErrorFlags[3] = True 
        elif(bErrorFlag == False):
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata                  

        #check if next 4/5 values make up the words false, true or null
        iCurrentPosition, bErrorFlag, listJSONdata \
        = self._check_Bool(listJSONdata, 
                           strJSON, 
                           iJSON_Length, 
                           iCurrentPosition)  

        # Check if error has occured
        if (bErrorFlag == True):
            bErrorFlag = False
            bErrorFlags[4] = True  
        elif(bErrorFlag == False):
            if (iTempPosition != iCurrentPosition):
                listJSONdata.append(strJSON[iTempPosition:iCurrentPosition])
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata     

        # Check for whitespace after value
        iCurrentPosition, listJSONdata \
        = self._check_WhiteSpaces(listJSONdata, 
                                  strJSON, 
                                  iJSON_Length, 
                                  iCurrentPosition)

        # Check if error has occured
        if (bErrorFlags == [True, True, True, True, True]):
            bErrorFlag = True
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata
        
        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata

    def _check_Object(self, 
                      listJSONdata: list,
                      strJSON: str, 
                      iJSON_Length: int, 
                      iCurrentPosition: int) -> \
                      Union[int, bool, list]:
        """
        Description
        -----------
        Checks if the next characters of the string build a valid JSON object.

        Parameters
        ----------
        `listJSONdata` : list
            List structure containing the data from the JSON file

        `strJSON` : string
            Input string which should be checked

        `iJSON_Length` : int
            Length of the JSON string which should be checked

        `iCurrentPosition` : int
            Current position at which the method should start checking

        Return
        ------
        `listJSONdata` : list
            Modified list structure containing the data from the JSON file

        `iCurrentPosition` : int
            Current position at which the method stopped checking

        `bErrorFlag` : bool
            Error flag which indicates, that an error has occured

        """
        # Logging function call if --log = INFO
        logging.info(str(iCurrentPosition) + "_check_Object\n")

        # Initalize variables
        bErrorFlag: bool = False

        # Check for open bracket
        if(strJSON[iCurrentPosition] == "{"):
            # Move to the next position in the string
            iCurrentPosition += 1
        else:
            bErrorFlag = True
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata
        
        # Check for whitespace 
        iCurrentPosition, listJSONdata \
        = self._check_WhiteSpaces(listJSONdata, 
                                  strJSON, 
                                  iJSON_Length, 
                                  iCurrentPosition)

        # Check for object end
        if(strJSON[iCurrentPosition] == "}"):
            if (iCurrentPosition < iJSON_Length - 1):
                # Move to the next position in the string
                iCurrentPosition += 1
            # Return union
            return iCurrentPosition, bErrorFlag, listJSONdata
        
        for iPosition in range(iCurrentPosition,iJSON_Length):
            if (iPosition < iCurrentPosition):
                pass
            else:
                # Safe iCurrentPosition
                iTempPosition = iCurrentPosition

                listTempJSONdata: list = []

                # Check for JSON string
                iCurrentPosition, bErrorFlag, listTempJSONdata \
                = self._check_String(listTempJSONdata, 
                                     strJSON, 
                                     iJSON_Length, 
                                     iCurrentPosition)                                                
                
                # Check if error has occured
                if (bErrorFlag == True):
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata

                # Move to the next position in the string
                iCurrentPosition += 1

                # Safe object name if an object was recognized
                if (iTempPosition != iCurrentPosition):
                    listTempJSONdata.append(strJSON[
                        iTempPosition:iCurrentPosition])            

                # Check for whitespace 
                iCurrentPosition, listJSONdata \
                = self._check_WhiteSpaces(listJSONdata, 
                                          strJSON, 
                                          iJSON_Length, 
                                          iCurrentPosition)        

                # Check for ":"
                if(strJSON[iCurrentPosition] == ":"):
                    # Move to the next position in the string
                    iCurrentPosition += 1
                else:
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata  

                # Check for value
                iCurrentPosition, bErrorFlag, listTempJSONdata \
                = self._check_Value(listTempJSONdata, 
                                    strJSON, 
                                    iJSON_Length, 
                                    iCurrentPosition)

                # Check if error has occured
                if (bErrorFlag == True):
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata
                
                if(strJSON[iCurrentPosition] == "\""):
                    # Move to the next position in the string
                    iCurrentPosition += 1

                listJSONdata.append(listTempJSONdata)
                # Check for "," or "}"
                if (strJSON[iCurrentPosition] == ","):
                    # Move to the next position in the string
                    iCurrentPosition += 1
                elif (strJSON[iCurrentPosition] == "}"):
                    if (iCurrentPosition < iJSON_Length - 1):
                        # Move to the next position in the string
                        iCurrentPosition += 1
                    break
                else:
                    bErrorFlag = True
                    # Return union
                    return iCurrentPosition, bErrorFlag, listJSONdata         

                # Check for whitespace 
                iCurrentPosition, listJSONdata \
                = self._check_WhiteSpaces(listJSONdata, 
                                          strJSON, 
                                          iJSON_Length, 
                                          iCurrentPosition)

        # Return union
        return iCurrentPosition, bErrorFlag, listJSONdata  

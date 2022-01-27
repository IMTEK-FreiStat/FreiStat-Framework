/******************************************************************************
 * @brief: Source file for the interface extension for the FreiStat in 
 * standalone mode.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 *
 *****************************************************************************/

// Include guard
#ifndef interface_CPP
#define interface_CPP

// Include headers
#include "interface.h"

/******************************************************************************
 * @brief Constructor of the class C_Interface
 * 
 *****************************************************************************/ 
C_Interface::C_Interface(){}

/******************************************************************************
 * @brief Setup method for the display of the interface
 * @details Contains the definition and the start-up of the display
 * 
 *****************************************************************************/ 
void C_Interface::begin(){
	// Intialize interface text
	strcpy(chrarrEcMethods[0], CA);
	strcpy(chrarrEcMethods[1], LSV);
	strcpy(chrarrEcMethods[2], CV);
	strcpy(chrarrEcMethods[3], NPV);
	strcpy(chrarrEcMethods[4], DPV);
	strcpy(chrarrEcMethods[5], SWV);

	// Initialize variables
	iSelectedMethod = 0;
	iCursorCurrentPos = 0;
	iCursorPreviousPos = 0;
	iCurrentMenuPoints = 6;

	// Button setup
	pinMode(BUTTON_A, INPUT_PULLUP);
	pinMode(BUTTON_B, INPUT_PULLUP);
	pinMode(BUTTON_C, INPUT_PULLUP);

	// Create an object for the display and safe the reference
	display = Adafruit_SSD1306(128, 32, &Wire);
	
	// Start display (Adress: 0x3C - can't be changed)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	// Call the default font method to set the appearence of the text to default
	defaultFont();
	
	// Create the base frame which is displayed as a menu
	createFrame(4, 0);

	// Set start cursor
	display.setCursor(0,0);
	display.print("|>|");
	display.display();
}

/******************************************************************************
 * @brief Method for setting the text appearence to default
 * @details Changes the textsize and the color of the text to default
 * 
 *****************************************************************************/ 
void C_Interface::defaultFont(){
	// Define the font and color of the display text
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);

	// Display changes
	display.display();
}

/******************************************************************************
 * @brief Method for creating the base frame which is displayed in the display as a menu
 * @details See documentation in the header file for more information
 * @param iMenuPoints: The amount of menu points which should be displayed
 * @param iStartPosition: First position which should be displayed
 * 
 *****************************************************************************/ 
void C_Interface::createFrame(int8_t iMenuPoints, int8_t iStartPosition){
	// Reset display
	display.clearDisplay();

	// Create menu frame
	for (int i = 0; i <= iMenuPoints - 1; i++)
	{
		display.setCursor(0,i * 8);
		display.print("| |" );   
		display.setCursor(18,i * 8);
		display.print(chrarrEcMethods[i + iStartPosition]);    
	}
	// Display changes
	display.display();
}

/******************************************************************************
 * @brief Method for navigating inside the menu
 * @return: Current cursor position
 *****************************************************************************/ 
int8_t C_Interface::navigate(){
	while(digitalRead(BUTTON_C)){
		// Check if Button A has been pressed
		if (!digitalRead(BUTTON_A)){
			if (iSelectedMethod >= iCurrentMenuPoints - 2){
				createFrame(4, iSelectedMethod - (iCurrentMenuPoints - 4) - 2);
				moveCursor(iCursorCurrentPos);
				iSelectedMethod -= 1;
			}
			else if (iSelectedMethod > 0){
				moveCursor(iSelectedMethod - 1);
				iSelectedMethod -= 1;
			}
		}

		// Check if Button B has been pressed
		if (!digitalRead(BUTTON_B)){
			if (iSelectedMethod <= iCurrentMenuPoints - 4){
				iSelectedMethod += 1;
				moveCursor(iSelectedMethod);
				
			}
			else if (iSelectedMethod < iCurrentMenuPoints - 1){
				createFrame(4, iSelectedMethod - (iCurrentMenuPoints - 4));
				moveCursor(iCursorCurrentPos);
				iSelectedMethod += 1;
			}
		}
		// Wait 100 ms
		delay(100);
	}
	// Reset display and print info
	display.clearDisplay();
	display.setCursor(0, 8);
	display.print("Experiment Running" );  
	// Display changes
	display.display();
	return iSelectedMethod;
}

/******************************************************************************
 * @brief Move cursor to set position
 * @details Changes the position of the cursor to the position given by the 
 * committed parameter.
 * The 6 equals the width of one letter (5) + 1 and the 8 the height (7) + 1
 * 
 * @param cursorPosition: The position the cursor should point to
 * 
 *****************************************************************************/ 
void C_Interface::moveCursor(int8_t cursorPosition){
	// Update current and previous cursor position
	iCursorPreviousPos = iCursorCurrentPos;
	iCursorCurrentPos = cursorPosition;

	// Remove cursor from old position
	display.setCursor(6, iCursorPreviousPos * 8);
	display.setTextColor(SSD1306_BLACK);
	display.print(">");
	
	// Display cursor at new position
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(6, iCursorCurrentPos  * 8);
	display.print(">");

	// Display changes
	display.display();
}

/******************************************************************************
 * @brief Check if menu is in bound
 * @details Check if the given planned cursor position is possible and return a fixed value
 * @return : Bounded value
 *****************************************************************************/ 
int8_t C_Interface::menubounds(int8_t cursorPosition){
	// Check if given parameter is negative
	if(cursorPosition < 0 )
	{
		cursorPosition = abs(cursorPosition);
	}

	// Check if given parameter is out of range
	if(cursorPosition > iCurrentMenuPoints)
	{
		cursorPosition = iCurrentMenuPoints;
	}

	return cursorPosition;
}
#endif /* interface_CPP */
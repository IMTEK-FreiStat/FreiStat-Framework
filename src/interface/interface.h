/******************************************************************************
 * @brief: Header file for the interface extension for the standalone mode of
 * the FreiStat.
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *  The OLED display of the adafruit has a with of 128 pixels and a height of 
 *  32 pixels.
 *  The height of 32 pixel provides enough space for 4 lines of text 
 *  (8 pixel per line) if the font size is choosen as 1.
 * 
 *  Below the basic structure of the interface is shown below
 * 
 *  [|>| option nr.1   |                            ]
 *  [| |     ...       |                            ]
 *  [| |     ...       |                            ] 
 *  [| |     ...       |                            ] 
 * 
 *  The first row shows the cursor for navigating inside the menus.
 *  The second row shows the name of the option and the last row is free for 
 *  e.g. values of variables. 
 *  The second and the third row can consist in total out of 18 characters if 
 *  fontsize is 1.
 * 
 *  The function of the buttons is as followed:
 *  Button A:   up      /   increase
 *  Button B:   down    /   decrease
 *  Button C:   ok
 * 
 *****************************************************************************/

// Include guard
#ifndef interface_H
#define interface_H

// Include external dependencies
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Include dependencies
#include "../data_storage/constants.cpp"

// Definitions
// Interface buttons
#define BUTTON_A 9
#define BUTTON_B 6
#define BUTTON_C 5

/******************************************************************************
 * @brief: Abstract class for the interface extenstion class
 * 
 * @author: Mark Jasper
 * @version: V 1.0.0
 * @date: 19.01.2022
 * 
 *****************************************************************************/
class C_Interface{
    private:
        // Object pointers
        Adafruit_SSD1306 display;

        // Variables
        char chrarrEcMethods[6][4];

        int8_t iCursorCurrentPos;
        int8_t iCursorPreviousPos;
        int8_t iSelectedMethod;
        int8_t iCurrentMenuPoints;

        // Methods
        void defaultFont();
        void createFrame(int8_t, int8_t);
        void moveCursor(int8_t);

        int8_t menubounds(int8_t);

    public:
        // Constructor
        C_Interface();

        // Starting method
        void begin();

        // Methods
        int8_t navigate();
};
#endif  /* interface_H */
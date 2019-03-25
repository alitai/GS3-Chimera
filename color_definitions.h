//color_definitions.h
//***********************************************************************
// Color choices
//***********************************************************************
#ifdef MCUFRIEND 
// ADAFRUIT Color definitions for MCU_FRIEND
#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F
#endif

char* PWM_Color = ILI9341_RED;
char* PWM_BGColor = ILI9341_MAROON;
char* pressure_Color = ILI9341_CYAN;
char* pressure_BGColor = ILI9341_DARKCYAN;
char* flowVolume_Color = ILI9341_GREEN;
char* flowVolume_BGColor = ILI9341_DARKGREEN;
char* flowRate_Color = ILI9341_YELLOW;
char* flowRate_BGColor = ILI9341_GREENYELLOW;
char* weight_Color = ILI9341_ORANGE;
char* timer_Color = ILI9341_YELLOW;
char* text_light_Color = ILI9341_WHITE;
char* text_dark_Color = ILI9341_WHITE;
char* bg_Color = ILI9341_BLACK;
char* axis_minor_Color = ILI9341_LIGHTGREY;
char* axis_major_Color = ILI9341_WHITE;
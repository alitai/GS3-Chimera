//********************************************************************
//  Print something somewhere....
//
//  Example: printSomething("text here", 0, 14, text_dark_Color, &FreeSans9pt7b, false ); 
//  Use NULL for default font
//
//  COLOR LIST:
//  ILI9341_BLACK     ILI9341_OLIVE       ILI9341_RED
//  ILI9341_NAVY      ILI9341_LIGHTGREY     ILI9341_MAGENTA  
//  ILI9341_DARKGREEN   ILI9341_DARKGREY      ILI9341_YELLOW  
//  ILI9341_DARKCYAN    ILI9341_BLUE        ILI9341_WHITE
//  ILI9341_MAROON      ILI9341_GREEN       ILI9341_ORANGE
//  ILI9341_PURPLE      ILI9341_CYAN        ILI9341_GREENYELLOW 
//  ILI9341_PINK                
//              
//********************************************************************
void printSomething(char* text, unsigned x, unsigned y, char* selectedColor, const GFXfont* selectedFont, boolean clearBackground)
{
	// How to concatenate chars?
	tft.setTextColor(selectedColor);
	tft.setFont(selectedFont);
	if (clearBackground)
	{
		int  x1, y1;
		unsigned w, h;
		tft.getTextBounds(text, x, y, &x1, &y1, &w, &h);
		tft.fillRect(x1, y1, w, h, bg_Color);
	}
	tft.setCursor(x, y);
	tft.print(text);
}

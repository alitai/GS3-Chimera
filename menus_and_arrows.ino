byte menuNavigation()
{
	
	// If there is an incomplete Mode switch (e.g. triggered from 3d5 serial port) ignore any manual input until complete  
	if (g_modeSwitchIncomplete)
		return 0;                
	
	byte softKey = touchpadSenseKey();

	//If Nav arrows (softKey 1 & 4) are pressed or forced - change menu
	if ((softKey == 1 && g_currentMenu == 0) || (softKey == 4 && g_currentMenu ==5)) //throw out of bound clicks out...
		return 99; 
	if (softKey == 1 && g_currentMenu > 0)
		g_currentMenu--;
	if (softKey == 4 && g_currentMenu < 5)
		g_currentMenu++;
		
	return softKey;
}


void menuDrawSoftKeys(char* softKey2Label, boolean isSoftKey2Selected, char* softKey3Label, boolean isSoftKey3Selected)
{
	// set font for menus
	tft.setFont(&FreeSans9pt7b);

	if (isSoftKey2Selected)
	{
		tft.fillRoundRect(31,286,85,28,3,ILI9341_DARKGREEN);
		tft.drawRoundRect(29,284,87,32,3,ILI9341_WHITE);
		tft.setTextColor(text_light_Color);
	} 
	else
	{
		tft.fillRoundRect(31,286,85,28,3,bg_Color);
		tft.drawRoundRect(29,284,87,32,3,ILI9341_DARKGREY);
		tft.setTextColor(text_dark_Color);
	}

	// Print label text centered on soft key 2
	int  x1, y1;
	unsigned w, h;
	tft.getTextBounds(softKey2Label, 0, 200, &x1, &y1, &w, &h); // size of label is used to center label on button
	tft.setCursor(31 + 42 - (w >> 1), 305);
	tft.print(softKey2Label);

	if (isSoftKey3Selected)
	{
		tft.fillRoundRect(126, 286, 85, 28, 3, ILI9341_DARKGREEN);
		tft.drawRoundRect(124, 284, 87, 32, 3, ILI9341_WHITE);  
		tft.setTextColor(text_light_Color);
	}
	else 
	{
		tft.fillRoundRect(126, 286, 85, 28, 3, bg_Color);
		tft.drawRoundRect(124, 284, 87, 32, 3,ILI9341_DARKGREY);
		tft.setTextColor(text_dark_Color);
	} 

	// Print label text centered on soft key 3
	tft.getTextBounds(softKey3Label, 0, 200, &x1, &y1, &w, &h);
	tft.setCursor(126 + 42 - (w >> 1), 305);
	tft.print(softKey3Label);
} 

void menuDrawNavArrows()
{
// display proper navigation arrows
	if (g_currentMenu > 0)
		tft.fillTriangle(9,300,19,308,19,292,ILI9341_GREEN);
	else
	{
		tft.fillTriangle(9,300,19,308,19,292,bg_Color);
		tft.drawTriangle(9,300,19,308,19,292,ILI9341_DARKGREEN);
	}

	if (g_currentMenu < 6)
		tft.fillTriangle(231,300,221,308,221,292,ILI9341_GREEN);
	else
	{
		tft.fillTriangle(231,300,221,308,221,292,bg_Color);
		tft.drawTriangle(231,300,221,308,221,292,ILI9341_DARKGREEN);
	}
}

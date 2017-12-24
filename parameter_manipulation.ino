//  Setup Slayer parameter adjustment screen
void menuSetupSlayerParameters()
{
	int currentPotValue = analogRead(CONTROL_POT); 
	boolean potMoved = wasPotMoved(currentPotValue); 	//check if potentiometer was moved	

	switch (g_selectedParameter)
	{
		case 1: 
			if (potMoved)
			{
// Assaf extended the range from 45 to 70 seconds 			
			//	slayerPIPeriod = map(currentPotValue, 0, 1023, 4, 45); // 5 - 45 seconds
				slayerPIPeriod = map(currentPotValue, 0, 1023, 4, 70); // 5 - 70 seconds
				graphDrawSlayerProfile(); // Updates the graph with the new data...
			}
			break;
		case 2: 
			if (potMoved)
			{ 
				slayerMainPWM = map(currentPotValue, 0, 1023, slayerPIFlowRate, slayerMaxPWM);
				graphDrawSlayerProfile(); // Updates the graph with the new data...
			}
			break;
	}
}

//  Setup parameter adjustment screen
void menuSetupSystemParameters()
{
	int currentPotValue = analogRead(CONTROL_POT); 
	boolean potMoved = wasPotMoved(currentPotValue); 	//check if potentiometer was moved	
  
	if (g_selectedParameter == 1) 
	{
		if (potMoved)
		{
			FLBThreshold = map(currentPotValue, 0, 1023, 0, 50);
			graphDrawCurrentProfiles(); // Updates the graph with the new data... 
			tft.fillRect(120, 0, 120, 10, ILI9341_BLACK);
		}
		printSomething("FLBThreshold = ", 0, 0, ILI9341_WHITE, NULL , false);
	}
	else
		printSomething("FLBThreshold = ", 0, 0, ILI9341_DARKGREY, NULL , false);
	tft.setCursor(120,0);
	tft.print(FLBThreshold);
	tft.print(" Vm in %");

	if (g_selectedParameter == 2) 
	{
		if (potMoved)
		{
			debounceCount = map(currentPotValue, 0, 1023, 0, 20);
			tft.fillRect(120, 12, 120, 10, ILI9341_BLACK);
		}
		printSomething("debounceCount = ", 0, 12, ILI9341_WHITE, NULL , false);
	}
	else
		printSomething("debounceCount = ", 0, 12, ILI9341_DARKGREY, NULL , false);
	tft.setCursor(120,12);
	tft.print(debounceCount);
	
 	if (g_selectedParameter == 3)
	{
		if (potMoved)
		{
			pumpMinPWM = map(currentPotValue, 0, 1023, 0, 255);
			tft.fillRect(120, 24, 120, 10, ILI9341_BLACK);
		}	
		printSomething("pumpMinPWM = ", 0, 24, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("pumpMinPWM = ", 0, 24, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(120,24);
	tft.print(pumpMinPWM);
	 
			
	if (g_selectedParameter == 4) 
	{ 
		if (potMoved)
		{
			pumpMaxPWM = map(currentPotValue, 0, 1023, 0, 255);
			tft.fillRect(120, 36, 120, 10, ILI9341_BLACK);  
		}
		printSomething("pumpMaxPWM = ", 0, 36, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("pumpMaxPWM = ", 0, 36, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(120,36);
	tft.print(pumpMaxPWM);
	
	if (g_selectedParameter == 5) 
	{
		if (potMoved)
		{
			mlPerFlowMeterPulse = ((float)map(currentPotValue, 0, 1023, 0, 10))/1000;
			tft.fillRect(120, 48, 120, 10, ILI9341_BLACK);  
		}
		printSomething("ml/pulse = ", 0, 48, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("ml/pulse = ", 0, 48, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(120,48);
	tft.print(mlPerFlowMeterPulse);
	tft.print(" ml/Pulse"); 
	
	if (g_selectedParameter == 6) 
	{
		if (potMoved)
		{
			unionThreshold = ((float)map(currentPotValue, 0, 1023, 0, 900))/1000;
			tft.fillRect(120, 60, 120, 10, ILI9341_BLACK);
		}
		printSomething("unionThreshold = ", 0 , 60, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("unionThreshold = ", 0 , 60, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(120,60);
	tft.print(unionThreshold);
	tft.print(" bar");  
				
	if (g_selectedParameter == 7) 
	{
		if (potMoved)
		{
			Kpp = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(60, 75, 60, 10, ILI9341_BLACK);
		}
		printSomething("Kpp = ", 0 , 75, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("Kpp = ", 0 , 75, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(60,75);
	tft.print(Kpp);
			
	if (g_selectedParameter == 8) 
	{
		if (potMoved)
		{
			Kpi = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(60, 87, 60, 10, ILI9341_BLACK);
		}
		printSomething("Kpi = ", 0 , 87, ILI9341_WHITE, NULL, false);
		
	}
	else
		printSomething("Kpi = ", 0 , 87, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(60,87);
	tft.print(Kpi);
	
	if (g_selectedParameter == 9) 
	{
		if (potMoved)
		{
			Kpd = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(60, 99, 60, 10, ILI9341_BLACK);  
		}
		printSomething("Kpd = ", 0 , 99, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("Kpd = ", 0 , 99, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(60,99);
	tft.print(Kpd);
	
	if (g_selectedParameter == 10) 
	{
		if (potMoved)
		{
			Kfp = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(180, 75, 60, 10, ILI9341_BLACK);
		}
		printSomething("Kfp = ", 120 , 75, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("Kfp = ", 120 , 75, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(180,75);
	tft.print(Kfp);
	
	if (g_selectedParameter == 11) 
	{
		if (potMoved)
		{
			Kfi = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(180, 87, 60, 10, ILI9341_BLACK);
		}
		printSomething("Kfi = ", 120 , 87, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("Kfi = ", 120 , 87, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(180,87);
	tft.print(Kfi);
	
	if (g_selectedParameter == 12) 
	{	
		if (potMoved)
		{
			Kfd = ((double)map(currentPotValue, 0, 1023, 0, 1000))/50;
			tft.fillRect(180, 99, 60, 10, ILI9341_BLACK);
		}
		printSomething("Kfd = ", 120 , 99, ILI9341_WHITE, NULL, false);
	}
	else
		printSomething("Kfd = ", 120 , 99, ILI9341_DARKGREY, NULL, false);
	tft.setCursor(180,99);
	tft.print(Kfd);

	printSomething("Note: Settings implemented immediately. Press commit to save to EEPROM.", 0, 119, ILI9341_RED, NULL, false);
}

boolean wasPotMoved(int currentPotValue)
{	
	//debounce the move (to reduce flicker)
	int potDelta = 2;
	if ((currentPotValue > g_lastParameterPotValue + potDelta) || (currentPotValue < g_lastParameterPotValue - potDelta))
	{
		g_lastParameterPotValue = currentPotValue;
		return true;
	}
	else
		return false;
}

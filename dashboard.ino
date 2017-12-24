//*********************************************************************
// Dashboard: Setup & update dashboard
//*********************************************************************
	
void initializeDisplay()
{	
	tft.setRotation(0);
	tft.fillScreen(ILI9341_BLACK);
	dashboardSetup();
	graphDrawCurrentProfiles();
	
	g_currentMenu = 0; 
	pullModeSwitching(0); // Setup menu system & switch mode to manual profiling
}

void dashboardSetup()
{
  tft.fillRect(0, 0, 240, 140, ILI9341_BLACK);
  if (!g_debugDisplay)
  {
    printSomething("Vm=", 0, 13, ILI9341_DARKGREY, &FreeSans9pt7b , false);
	printSomething("P=", 120, 13, ILI9341_DARKGREY, &FreeSans9pt7b, false);
	printSomething("Flow=", 0, 40, ILI9341_DARKGREY, &FreeSans9pt7b, false);
    printSomething("Vol=", 120, 40, ILI9341_DARKGREY, &FreeSans9pt7b, false);
    printSomething("Timer:", 0, 97, ILI9341_DARKGREY, &FreeSans9pt7b, false);
#ifdef ACAIA_LUNAR_INTEGRATION
	printSomething("Dose=", 0, 67, ILI9341_DARKGREY, &FreeSans9pt7b, false);
	printSomething("Wt=", 120, 67, ILI9341_DARKGREY, &FreeSans9pt7b, false);
	printSomething("CBR=", 120, 97, ILI9341_DARKGREY, &FreeSans9pt7b, false);	
#endif
  }
}

void dashboardUpdate(byte pumpSpeedByte, int profileIndex, float averagePressure, long lastFlowPulseCount, boolean preInfusion)
{
  if (!g_debugDisplay)
  {
    //PWM setting display
    printSomething(NULL, 42, 13 , PWM_Color, &FreeSans9pt7b , true);
    tft.fillRect(41, 1, 60, 17, ILI9341_BLACK);
    tft.print((unsigned)pumpSpeedByte * 100 / 255);
    tft.setTextColor(ILI9341_DARKGREY);  
    tft.print(" %");
    
    // Water volume
    printSomething(NULL, 155, 40 , flow_Color, &FreeSans9pt7b , true);
    tft.fillRect(154, 26, 54, 17, ILI9341_BLACK);
    tft.print((float)((g_flowPulseCount - g_flowPulseCountPreInfusion) * mlPerFlowMeterPulse + g_flowPulseCountPreInfusion * mlPerFlowMeterPulsePreInfusion), 0); // Volume in ml
    tft.setTextColor(ILI9341_DARKGREY); 
    tft.print(" ml");
    
    // Flow Pulse Counter / min
    // Since there are so few pulses, run a running average of the time between pulses and use that to get flow rate resolution.
    printSomething(NULL, 50, 40 , flowRate_Color, &FreeSans9pt7b , true);
    tft.fillRect(44, 26, 75, 17, ILI9341_BLACK);
	if(preInfusion)
    tft.print((float)mlPerFlowMeterPulsePreInfusion * 60.0 * 1000.0 / (float)g_averageF.mean(), 0);
	else
	tft.print((float)mlPerFlowMeterPulse * 60.0 * 1000.0 / (float)g_averageF.mean(), 0);
    
	tft.setTextColor(ILI9341_DARKGREY); 
    tft.print(" /m");
    
    // Boiler Pressure and Weight display - handled by function displayPressureandWeight();

	
    //Pull Timer display  
    tft.setTextColor(ILI9341_ORANGE);
    tft.setFont(&FreeSans12pt7b);
    if (profileIndex % 2 == 0)
		tft.fillRect(54, 77, 38, 22, ILI9341_BLACK); //reduce flicker
    if (profileIndex /2 < 10)
		tft.setCursor(78,97);
    else if (profileIndex /2 < 100)
		tft.setCursor(66,97);
	else
      tft.setCursor(55,97);
    tft.print(profileIndex / 2);
    tft.setCursor(90,97);
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(ILI9341_DARKGREY);   
    tft.print(" S");
  }
  else
  {
    tft.fillRect(120, 0, 120, 157, ILI9341_BLACK);
	printSomething(CURRENT_VERSION, 200, 10 , ILI9341_WHITE, NULL , false); 
    
    printSomething("Profile Index: ", 0, 10 , ILI9341_WHITE, NULL , false);  
    tft.setCursor(120,10);
    tft.print(profileIndex);

    printSomething("Pressure: ", 0, 30 , ILI9341_WHITE, NULL , false);   
    tft.setCursor(120,30);
    tft.print(averagePressure); 
	tft.print("bar AD: "); 
	tft.print(analogRead(PRESSURE_SENSOR_INPUT));

    printSomething("Flow Pulse Count: ", 0, 40 , ILI9341_WHITE, NULL , false);   
    tft.setCursor(120,40);
    tft.print(g_flowPulseCount);
    
    printSomething(" ...Last Count: ", 0, 50 , ILI9341_WHITE, NULL , false);
    tft.setCursor(120,50);
    tft.print(lastFlowPulseCount);

    printSomething("Profile %PWM: ", 0, 60 , ILI9341_WHITE, NULL , false);    
    tft.setCursor(120,60);
    for (int i=0; i<4 ; i++)
    {
      tft.print(g_PWMProfile[i]);
      PRINT_SPACE;
    }
    tft.print(g_PWMProfile[profileIndex]);
    
    printSomething("Pull %PWM: ", 0, 70 , ILI9341_WHITE, NULL , false); 
    tft.setCursor(120,70);
    for (int i=0; (i < profileIndex && i < 4); i++)
    {
      tft.print(g_first4PWM[i]);
      PRINT_SPACE;
    }

    printSomething("Profile P(bar): ", 0, 80 , ILI9341_WHITE, NULL , false);  
    tft.setCursor(120,80);
    for (int i=0; i<4 ; i++)
    {
      tft.print(g_pressureProfile[i]);
      PRINT_SPACE;
    }
    tft.print(g_pressureProfile[profileIndex]);
    
    printSomething("Pull P(bar): ", 0, 90 , ILI9341_WHITE, NULL , false); 
    tft.setCursor(120,90);
    for (int i=0; (i < profileIndex && i < 4); i++)
    {
      tft.print(g_first4Pressure[i]);
      PRINT_SPACE;
    }

    printSomething("Profile Flow: ", 0, 100 , ILI9341_WHITE, NULL , false); 
    tft.setCursor(120,100);
    for (int i=0; i<4 ; i++)
    {
      tft.print(g_flowProfile[i]);
      PRINT_SPACE;
    }
    tft.print(g_flowProfile[profileIndex]); 

    printSomething("Pull Flow: ", 0, 110 , ILI9341_WHITE, NULL , false);  
    tft.setCursor(120,110);
    for (int i=0; (i < profileIndex && i < 4); i++)
    {
      tft.print(g_first4Flow[i]);
      PRINT_SPACE;
    }
  }
}

void displayPressureandWeight()
{
	if (!g_debugDisplay && g_currentMenu != 6 && millis() > (g_lastMillis +500)) // display pressure twice every second
	{

//display Pressure reading
	printSomething(NULL, 148, 13 , ILI9341_CYAN, &FreeSans9pt7b , true);
		tft.fillRect(146, 1, 40 , 17, ILI9341_BLACK);
#ifdef CALIBRATE_PRESSURE
		tft.print((int)g_averageP.mean());
#else
		tft.print(constrain((float)g_averageP.mean(), 0.0, 13.0) , 1);
#endif
		printSomething("bar",190 , 13 , ILI9341_DARKGREY, &FreeSans9pt7b , false);


#ifdef ACAIA_LUNAR_INTEGRATION
//Display Dose
		if(currentDose != lastCurrentDose)
		{
			printSomething(NULL, 55, 67 , weight_Color , &FreeSans9pt7b , false);
			tft.fillRect(53, 55, 65, 17, ILI9341_BLACK);
			tft.print(currentDose, 1);
			tft.print(" g");
			lastCurrentDose = currentDose;
			sleepTimer = sleepTimerReset(); 
		}

//Display Weight & EBF
		if (!scaleConnected && lastScaleWeight != 9999)
		{
			// gray out wt.
			tft.fillRect(155, 55, 80, 17, ILI9341_BLACK);
			printSomething("NC",156 , 67 , ILI9341_DARKGREY, &FreeSans9pt7b , true);

			// do same for EBF
			tft.fillRect(169, 77, 70, 22, ILI9341_BLACK);
			printSomething("NA",170 , 97 , ILI9341_DARKGREY, &FreeSans9pt7b , true);
			lastScaleWeight = 9999; // OOB value helps avoid flicker on next loop....
		}
		
		if (scaleConnected && scaleWeight != lastScaleWeight)
		{
			tft.fillRect(155, 55, 80, 17, ILI9341_BLACK);
			if (scaleWeight < 0)
				printSomething(NULL, 156, 67 , ILI9341_RED, &FreeSans9pt7b , true);
			else
				printSomething(NULL, 156, 67 , weight_Color, &FreeSans9pt7b , true);
			tft.print(scaleWeight, 1);
			tft.print(" g");

			tft.fillRect(169, 79, 70, 23, ILI9341_BLACK);
			//printSomething(NULL, 170, 97, weight_Color, &FreeSans12pt7b , true);
			
			// Don't show rediculous numbers when measuring th PF...
			if (scaleWeight/currentDose > 8) 
				printSomething("-high-", 170, 97, ILI9341_RED, &FreeSans9pt7b , true);
			else if (scaleWeight/currentDose < 0) 
				printSomething("-neg-", 170, 97, ILI9341_RED, &FreeSans9pt7b , true);
			else
			{
				printSomething("1:", 170, 97, weight_Color , &FreeSans12pt7b , true);
				tft.print(scaleWeight/currentDose, 1);
			}
			lastScaleWeight = scaleWeight; // avoid flicker....
			sleepTimer = sleepTimerReset();
		}
		
#endif
		g_lastMillis = millis();
	
	}
}

//Display Battery
void displayBattery()
{
	if (scaleConnected)
	{
		tft.drawRect(230, 105, 4, 5, ILI9341_DARKGREY); //+210
		tft.drawRect(213, 103, 18, 9, ILI9341_DARKGREY);
		long h = 16 * scaleBattery / 100;
		
		if (scaleBattery > 40)
		{
			tft.drawPixel(216,112,ILI9341_BLACK);
			tft.drawPixel(227,102,ILI9341_BLACK);
			tft.fillRect(214+h, 104, 16-h, 7, ILI9341_BLACK);
			tft.fillRect(214, 104, h, 7, ILI9341_GREEN); 
		}
		else if (scaleBattery > 20)
		{
			tft.drawPixel(216,112,ILI9341_BLACK);
			tft.drawPixel(227,102,ILI9341_BLACK);
			tft.fillRect(214+h, 104, 16-h, 7, ILI9341_BLACK);			
			tft.fillRect(214, 104, h, 7, ILI9341_YELLOW); 
		}
		else if (scaleBattery > 14)
		{
			tft.drawPixel(216,112,ILI9341_BLACK);
			tft.drawPixel(227,102,ILI9341_BLACK);
			tft.fillRect(217, 104, 13, 7, ILI9341_BLACK);			
			tft.fillRect(214, 104, 3, 7, ILI9341_RED);
		}
		else if (scaleBattery > 8)
		{
			tft.drawPixel(216,112,ILI9341_BLACK);
			tft.drawPixel(227,102,ILI9341_BLACK);
			tft.fillRect(216, 104, 14, 7, ILI9341_BLACK);			
			tft.fillRect(214, 104, 2, 7, ILI9341_RED);
		}
		else
		{
			tft.fillRect(216, 104, 14, 7, ILI9341_BLACK);			
			tft.fillRect(214, 104, 2, 7, ILI9341_MAROON);
			tft.drawLine(216,112,227,102, ILI9341_WHITE);
		}
	}
	else
	{
		tft.drawPixel(216,112,ILI9341_BLACK);
		tft.drawPixel(227,102,ILI9341_BLACK);
		tft.fillRect(231, 105, 5, 5, ILI9341_BLACK);
		tft.fillRect(213, 103, 18, 9, ILI9341_BLACK);
		
	}
}

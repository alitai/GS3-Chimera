//*********************************************************************
// Dashboard: Setup & update dashboard
//*********************************************************************
	
void initializeDisplay()
{	
	tft.fillScreen(bg_Color);
	dashboardSetup();
	graphDrawCurrentProfiles();
	g_currentMenu = 0; 
	pullModeSwitching(0); // Setup menu system & switch mode to manual profiling
}

void dashboardSetup()
{
	tft.fillRect(0, 0, 240, 140, bg_Color);
    printSomething("Vm=", 0, 13, text_dark_Color, &FreeSans9pt7b , false);
	printSomething("P=", 120, 13, text_dark_Color, &FreeSans9pt7b, false);
	printSomething("Flow=", 0, 40, text_dark_Color, &FreeSans9pt7b, false);
    printSomething("Vol=", 120, 40, text_dark_Color, &FreeSans9pt7b, false);
    printSomething("Timer:", 0, 97, text_dark_Color, &FreeSans9pt7b, false);
#ifdef ACAIA_LUNAR_INTEGRATION
	printSomething("Dose=", 0, 67, text_dark_Color, &FreeSans9pt7b, false);
	printSomething("Wt=", 120, 67, text_dark_Color, &FreeSans9pt7b, false);
	printSomething("CBR=", 120, 97, text_dark_Color, &FreeSans9pt7b, false);	
#endif
}

void dashboardUpdate(uint16_t pumpPWM, uint16_t profileIndex, float averagePressure, uint16_t long lastFlowPulseCount, bool preInfusion)
{
    //PWM setting display
    printSomething(NULL, 42, 13 , PWM_Color, &FreeSans9pt7b , true);
    tft.fillRect(41, 1, 60, 17, bg_Color);
    tft.print(pumpPWM / 4);
    tft.setTextColor(text_dark_Color);  
    tft.print(" %");
    
    // Water volume
    printSomething(NULL, 155, 40 , flowVolume_Color, &FreeSans9pt7b , true);
    tft.fillRect(154, 26, 54, 17, bg_Color);
	float volume = flowVolume();
	tft.print(volume, 0); // Volume in ml
    tft.setTextColor(text_dark_Color); 
    tft.print(" ml");
    
    // Flow Pulse Counter / min
    // Since there are so few pulses, run a running average of the time between pulses and use that to get flow rate resolution.
    printSomething(NULL, 50, 40 , flowRate_Color, &FreeSans9pt7b , true);
    tft.fillRect(44, 26, 75, 17, bg_Color);
	tft.print(flowRate(preInfusion), 0);
	tft.setTextColor(text_dark_Color); 
    tft.print(" /m");
    
    // Boiler Pressure and Weight display - handled by function displayPressureandWeight();
	
    //Pull Timer display  
    tft.setTextColor(timer_Color);
    tft.setFont(&FreeSans12pt7b);
    if (profileIndex % 2 == 0)
		tft.fillRect(53, 77, 41, 22, bg_Color);//(54, 77, 38, 22, bg_Color); //reduce flicker
    if (profileIndex /2 < 10)
		tft.setCursor(78,97);
    else if (profileIndex /2 < 100)
		tft.setCursor(66,97);
	else
      tft.setCursor(55,97);
    tft.print(profileIndex / 2);
    tft.setCursor(90,97);
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(text_dark_Color);   
    tft.print(" S");
}

void displayPressureandWeight()
{
	if (g_currentMenu != 6 && millis() > (g_lastMillis +500)) // display pressure twice every second
	{

//display Pressure reading
	printSomething(NULL, 148, 13 , ILI9341_CYAN, &FreeSans9pt7b , true);
		tft.fillRect(146, 1, 40 , 17, bg_Color);
#ifdef CALIBRATE_PRESSURE
		tft.print((int)g_averageP.mean());
#else
		tft.print(constrain((float)g_averageP.mean(), 0.0, 13.0) , 1);
#endif
		printSomething("bar",190 , 13 , text_dark_Color, &FreeSans9pt7b , false);
		// megunolinkPlot.SendFloatData("Bar", g_averageP.mean(), 2); // send pressure data when idling


#ifdef ACAIA_LUNAR_INTEGRATION
//Display Dose
		if(currentDose != lastCurrentDose)
		{
			printSomething(NULL, 55, 67 , weight_Color , &FreeSans9pt7b , false);
			tft.fillRect(53, 55, 65, 17, bg_Color);
			tft.print(currentDose, 1);
			tft.print(" g");
			lastCurrentDose = currentDose;
		}

//Display Weight & EBF
		if (!scaleConnected && lastScaleWeight != 9999)
		{
			// gray out wt.
			tft.fillRect(155, 55, 80, 17, bg_Color);
			printSomething("NC",156 , 67 , text_dark_Color, &FreeSans9pt7b , true);

			// do same for EBF
			tft.fillRect(169, 77, 70, 22, bg_Color);
			printSomething("NA",170 , 97 , text_dark_Color, &FreeSans9pt7b , true);
			lastScaleWeight = 9999; // OOB value helps avoid flicker on next loop....
		}
		
		if (scaleConnected && scaleWeight != lastScaleWeight)
		{
			tft.fillRect(155, 55, 80, 17, bg_Color);
			if (scaleWeight < 0)
				printSomething(NULL, 156, 67 , ILI9341_RED, &FreeSans9pt7b , true);
			else
				printSomething(NULL, 156, 67 , weight_Color, &FreeSans9pt7b , true);
			tft.print(scaleWeight, 1);
			tft.print(" g");

			tft.fillRect(169, 79, 70, 23, bg_Color);
			//printSomething(NULL, 170, 97, weight_Color, &FreeSans12pt7b , true);
			
			// Don't show rediculous numbers when measuring the PF...
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
		}
#endif
		g_lastMillis = millis();
	}
}

//*******************************************************************************
// Measure, average, calibrate and display boiler pressure and flow rates
//********************************************************************************

float measurePressure()
{  
#ifdef CALIBRATE_PRESSURE
	float pressure = analogRead(PRESSURE_SENSOR_INPUT); // Just show the integer reading
#else
	float pressure = (float)(map((int)analogRead(PRESSURE_SENSOR_INPUT), LOW_CALIBRATION_PRESSURE_READING, HIGH_CALIBRATION_PRESSURE_READING, LOW_CALIBRATION_PRESSURE, HIGH_CALIBRATION_PRESSURE)) / 10.0 ; // Every loop we measure and average pressure in boiler
#endif
 
 g_averageP.push(pressure); // add the measurement to the rolling average
	return pressure; //we return pressure for the PID PP loop

}

double flowRate(bool preInfusion)
{
	double flowRate;
	if(g_averageF.mean() != 0.0f)
	{
		if(preInfusion)
			flowRate = (double)mlPerFlowMeterPulsePreInfusion * 60.0 * 1000.0 / g_averageF.mean();
		else
			flowRate = (double)mlPerFlowMeterPulse * 60.0 * 1000.0 / g_averageF.mean();
	}
	else
		flowRate = 0.0;
	return flowRate;
}

float flowVolume()
{
    float volume = (g_flowPulseCount - g_flowPulseCountPreInfusion) * mlPerFlowMeterPulse 
				+ g_flowPulseCountPreInfusion * mlPerFlowMeterPulsePreInfusion;
	if (volume < 0 || volume > 250)
		return 0.0;
	else
		return volume;
	
}

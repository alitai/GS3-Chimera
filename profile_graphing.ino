//*********************************************************************
// Profile Graphs: Draw and fill the graph area 
//
//  COLOR LIST:
//  ILI9341_BLACK     ILI9341_OLIVE       ILI9341_RED
//  ILI9341_NAVY      ILI9341_LIGHTGREY     ILI9341_MAGENTA  
//  ILI9341_DARKGREEN   ILI9341_DARKGREY      ILI9341_YELLOW  
//  ILI9341_DARKCYAN    ILI9341_BLUE        ILI9341_WHITE
//  ILI9341_MAROON      ILI9341_GREEN       ILI9341_ORANGE
//  ILI9341_PURPLE      ILI9341_CYAN        ILI9341_GREENYELLOW 
//  ILI9341_PINK                
//*********************************************************************
void selectandDrawProfilebyMode()
{
	if (g_cleanCycle || g_flushCycle)
	{	
		graphDrawEmptyGraph(); // For cleaning cycle - Clear existing graph and draw an empty one
		return;
	}
	switch (g_pullMode)
	{
		case AUTO_PRESSURE_PROFILE_PULL: 
		case AUTO_FLOW_PROFILE_PULL:	
		case AUTO_PWM_PROFILE_PULL:
		case AUTO_UNION_PROFILE_PULL:
			graphDrawCurrentProfiles();
			break;
			
		case MANUAL_PULL:
			graphDrawEmptyGraph(); // Clear existing graph and draw an empty one
			graphDrawFLB(); //Draw line where FLB activates
			clearProfiles();
			break;
			
		case SLAYER_LIKE_PULL:
			graphDrawSlayerProfile();
			break;
	}
}

void graphDrawSlayerProfile()
{
	float perPixel = 1.35; //135 pixels represent 100% signal (or 1.35 pixels / 1 percent) 
	
	graphDrawEmptyGraph();

	// Draw Flow Limited flow line (this isn't a measured number - just shows a diagram that the flow is "low")
	tft.fillRect(0, 177, 33, 80, ILI9341_BLACK);
	printSomething(" PI", 10, 256 - (slayerPIFlowRate * perPixel) , ILI9341_DARKGREY, NULL , false);
	tft.drawFastHLine(34, 260 - (slayerPIFlowRate * perPixel), slayerPIPeriod << 1 , ILI9341_DARKGREY);	

	// Draw the Slayer Main PWM speed (the pressure decay is physical)
	printSomething(NULL, 100 + slayerPIPeriod , 250 - (slayerMainPWM * perPixel) , PWM_BGColor, NULL , false);
	tft.print(slayerMainPWM);
	tft.print("%");
	printSomething("PWM", 10, 256 - (slayerMainPWM * perPixel), ILI9341_DARKGREY, NULL , false);
	tft.drawFastHLine(34 + (slayerPIPeriod << 1), 260 - (slayerMainPWM * perPixel) , 200 - (slayerPIPeriod << 1) , ILI9341_DARKGREY);
	
	// Draw the Slayer PI Period vertical line
	printSomething(NULL, 41 + (slayerPIPeriod << 1), (257 - (slayerPIFlowRate + (((slayerMainPWM - slayerPIFlowRate) >> 1) * perPixel))) , timer_Color , NULL , false);
	tft.print(slayerPIPeriod);
	tft.print("s");
	tft.drawLine(34 + (slayerPIPeriod << 1), 260 - (slayerMainPWM * perPixel) , 34 + (slayerPIPeriod << 1) ,260 - (slayerPIFlowRate * perPixel)  , ILI9341_DARKGREY);
	
	
}

void graphDrawCurrentProfiles()
{
	float perPixel = 1.35; //135 pixels represent 100% signal (or 1.35 pixels / 1 percent) 
	
	graphDrawEmptyGraph();
	graphDrawFLB();
	int pulseSum = 0;
	for (byte i = 0; i < 200; i++)
	{
		pulseSum += g_flowProfile[i]; // g_flowProfile stores pulses - we need to accumulate them...
		graphUpdate(g_PWMProfile[i], i, g_pressureProfile[i], pulseSum, true, false);

		// Stored profile - calculate expected pull end - print pull time and draw a vertical dashed line
		if ((i > 10) && (g_flowProfile[i]== 0)  && (g_PWMProfile[i] == 0) && (g_pressureProfile[i] == 0))
		{ 
			//Print pull time on graph
			printSomething(NULL, 42 + i, 210 , timer_Color, NULL , false);
			tft.print(i / 2);
			tft.print("s");
			for (int j = 0; j < 46; j++)
			tft.drawFastVLine(34+ i, 261 - j * 3,2,ILI9341_DARKGREY);
			return;
		}
	}
}

void graphUpdate(byte pumpSpeedByte, int profileIndex, int averagePressure, unsigned long flowPulseCount, boolean isBackground, boolean preInfusion)
{
	float perPixel = 1.35; //135 pixels represent 100% signal (or 1.35 pixels / 1 percent) 
	
	char* colorPWM = PWM_Color;
	char* colorPressure = pressure_Color;
	char* colorFlow = flow_Color;
	
	if (isBackground) // draws profile in background (muted) colors
	{
		colorPWM = PWM_BGColor;
		colorPressure = pressure_BGColor;
		colorFlow = flow_BGColor;
	}

    // Draw 3 current graph pixels
	int graph_x = 34+profileIndex;
    tft.drawPixel(graph_x,graph_y ((float)pumpSpeedByte * perPixel * 100 / 255) , colorPWM);
	tft.drawPixel(graph_x,graph_y(averagePressure * 1.2 * perPixel),colorPressure);
	if (preInfusion)
		tft.drawPixel(graph_x,graph_y((float)flowPulseCount * mlPerFlowMeterPulsePreInfusion * perPixel),colorFlow); 
	else
		tft.drawPixel(graph_x,graph_y((float)flowPulseCount * mlPerFlowMeterPulse * perPixel),colorFlow); 
  // Assaf - added to draw flow rate
	if (!isBackground && preInfusion)
			tft.drawPixel(graph_x, graph_y((float)mlPerFlowMeterPulsePreInfusion * 60.0 * 1000.0 * 0.66666 / (float)g_averageF.mean()), flowRate_Color);  //150ml/min FR...
	if (!isBackground && !preInfusion)
			tft.drawPixel(graph_x, graph_y((float)mlPerFlowMeterPulse * 60.0 * 1000.0 * 0.66666 / (float)g_averageF.mean()), flowRate_Color);  //150ml/min FR...
}

unsigned graph_y(float y)
{
	return constrain(260-(unsigned)(y),120,260);
}



void graphDrawFLB()
{
	float perPixel = 1.35; //135 pixels represent 100% signal (or 1.35 pixels / 1 percent) 
	// draw Flow Limiter Bypass threshold dashed line
	tft.fillRect(0, 177, 33, 80, ILI9341_BLACK);
	printSomething("FLB", 10, 256-(FLBThreshold * perPixel), ILI9341_DARKGREY, NULL , false);
	for (int i = 0; i < 70; i++)
		tft.drawFastHLine(31 + i * 3, 260 - (FLBThreshold * perPixel) , 2 , ILI9341_DARKGREY);
}  

void graphDrawEmptyGraph()
{
	tft.fillRect(34, 120, 206, 141, ILI9341_BLACK); //redrawing only the graphing area so as to eliminate axis flicker......

	// Write the axis labels
	printSomething("60ml", 0, 115, flow_Color, NULL , false); //-23
	printSomething("100%", 0, 127, PWM_Color, NULL , false);
	printSomething("10bar", 0, 139, pressure_Color , NULL , false);
	printSomething("0", 23, 258, ILI9341_LIGHTGREY, NULL , false);
	printSomething("Time", 115, 270, ILI9341_LIGHTGREY, NULL , false);
	printSomething("100s", 215, 268, ILI9341_LIGHTGREY, NULL , false);
	printSomething("0s", 32, 268, ILI9341_LIGHTGREY, NULL , false);

	// draw the axis
	tft.drawFastHLine(31,262,209,ILI9341_LIGHTGREY);
	tft.fillTriangle(240,262,237,260,237,264,ILI9341_LIGHTGREY);
	tft.drawFastVLine(33,115,147,ILI9341_LIGHTGREY); //134-115 = 19
	tft.fillTriangle(33,115,31,118,35,118,ILI9341_LIGHTGREY);

	//draw axis markers
	tft.drawFastVLine(232,259,6,ILI9341_ORANGE);
	tft.drawFastHLine(30,124 ,6,ILI9341_MAGENTA);  
	tft.drawFastHLine(30,125,6,ILI9341_GREEN);
	tft.drawFastHLine(30,126,6,ILI9341_CYAN);
}

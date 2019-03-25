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
//
//  240 x 320 pixels 
//            
//*********************************************************************
void selectandDrawProfilebyMode()
{
	if (g_flushCycle)
	{	
		// graphDrawEmptyGraph(); // For cleaning cycle leave previous graph - Clear existing graph and draw an empty one
		graphDrawCurrentProfiles(); //???
		return;
	}
	switch (g_pullMode)
	{
		case AUTO_PRESSURE_PROFILE_PULL: 
		case AUTO_FLOW_PROFILE_PULL:	
		case AUTO_PWM_PROFILE_PULL:
			graphDrawCurrentProfiles();
			break;
			
		case MANUAL_PULL:
			graphDrawEmptyGraph(); // Clear existing graph and draw an empty one
			graphDrawFLB(); //Draw line where FLB activates
			//	clearProfiles();
			//graphDrawCurrentProfiles(); //test
			break;
			
		case SLAYER_LIKE_PULL:
			graphDrawSlayerProfile();
			break;
			
		case FLOW_PRESSURE_PROFILE:
		case SLAYER_LIKE_PI_PRESSURE_PROFILE:
			graphDrawEmptyGraph();
			graphDrawUnionThreshold();
	}
}

void graphDrawSlayerProfile() //-30 pixels on X
{
	graphDrawEmptyGraph();
	tft.fillRect(0, GRAPH_Y_0 - GRAPH_Y_SPAN + 8 , GRAPH_X_0, GRAPH_Y_SPAN - 10 , bg_Color); //tft.fillRect(0, 177, 3, 80, bg_Color);
	// Draw Flow Limited flow line (this isn't a measured number - just shows a diagram that the flow is "low")
	
	uint16_t PI_x = graph_x(slayerPIPeriod << 1);
	uint16_t PI_y = graph_y(slayerPIFlowRate * 10, GRAPH_PWM_SPAN * 10);
	printSomething(" PI", GRAPH_X_0 + 5, PI_y - 11 , text_light_Color, NULL , false);
	tft.drawFastHLine(GRAPH_X_0 + 1, PI_y, PI_x , axis_minor_Color);	

	// Draw the Slayer Main PWM speed (the pressure decay is physical)
	uint16_t main_y = graph_y(slayerMainPWM * 10, GRAPH_PWM_SPAN * 10);
	printSomething(NULL, 100 + slayerPIPeriod , main_y - 11 , PWM_Color, NULL , false);
	tft.print(slayerMainPWM/4);
	tft.print("%");
	printSomething("PWM", 10, main_y, text_light_Color, NULL , false);
	tft.drawFastHLine(GRAPH_X_0 + 1 + PI_x, main_y , GRAPH_X_SPAN - PI_x , axis_minor_Color);
	
	// Draw the Slayer PI Period vertical line
	printSomething(NULL, 11 + PI_x, (PI_y + main_y) / 2 , timer_Color , NULL , false);
	tft.print(slayerPIPeriod);
	tft.print("s");
	tft.drawLine(GRAPH_X_0 + 1 + PI_x, main_y , GRAPH_X_0 + 1 + PI_x , PI_y , ILI9341_DARKGREY);
}

void graphDrawCurrentProfiles()
{
	graphDrawEmptyGraph();
	graphDrawFLB();
/*
	int pulseSum = 0;
	for (byte i = 0; i < 200; i++)
	{
		pulseSum += g_flowProfile[i]; // g_flowProfile stores pulses - we need to accumulate them...
		drawGraphPixels(g_PWMProfile[i], i, g_pressureProfile[i], pulseSum, true, false);

		// Stored profile - calculate expected pull end - print pull time and draw a vertical dashed line
		if ((i > 10) && (g_flowProfile[i]== 0)  && (g_PWMProfile[i] == 0) && (g_pressureProfile[i] == 0))
		{ 
			//Print pull time on graph
			printSomething(NULL, 12 + i, 210 , timer_Color, NULL , false);
			tft.print(i / 2);
			tft.print("s");
			for (int j = 0; j < 46; j++)
			tft.drawFastVLine(4+ i, 261 - j * 3,2,axis_minor_Color);
			return;
		}
	}
*/
}

void drawGraphPixels(uint16_t pumpPWM, int profileIndex, float averagePressure, uint16_t long flowPulseCount, bool isBackground, bool preInfusion)
{
	char* colorPWM = PWM_Color;
	char* colorPressure = pressure_Color;
	char* colorFlowVolume = flowVolume_Color;
	char* colorFlowRate = flowRate_Color;
	
	if (isBackground) // draws profile in background (muted) colors
	{
		colorPWM = PWM_BGColor;
		colorPressure = pressure_BGColor;
		colorFlowVolume = flowVolume_BGColor;
		colorFlowRate = flowRate_BGColor;
	}

    // Draw 3 current graph pixels
	uint16_t x = graph_x(profileIndex);
	uint16_t y = graph_y(pumpPWM * 10, GRAPH_PWM_SPAN * 10); // * 10 to increase resolution
	tft.fillRect(x, y, 2, 2, colorPWM);
	y = graph_y(averagePressure * 100, GRAPH_PRESSURE_SPAN * 100); // * 10 to increase resolution to .01 bar
	tft.fillRect(x, y, 2, 2, colorPressure);
	y = graph_y(flowVolume() * 10, GRAPH_FLOW_VOLUME_SPAN * 10);
	tft.fillRect(x, y, 2, 2, colorFlowVolume);
	y = graph_y(flowRate(preInfusion) * 10, GRAPH_FLOW_RATE_SPAN * 10);
	tft.fillRect(x, y, 2, 2, colorFlowRate);
}

uint16_t graph_y(float y, uint16_t value_span)
{
	uint16_t y_out = map(y, 0, value_span, 0 , GRAPH_Y_SPAN); 
	y_out = constrain(GRAPH_Y_0 - 1 - y_out, GRAPH_Y_0 - GRAPH_Y_SPAN, GRAPH_Y_0 - 1);
	return y_out;
}

uint16_t graph_x(uint16_t profileIndex)
{
	uint16_t delta_x = round(MAX_PROFILE_INDEX / GRAPH_X_SPAN); 
	profileIndex = profileIndex * delta_x;
	return GRAPH_X_0 + 1 + profileIndex;
}

/*
GRAPH_X_0 3
GRAPH_Y_0 309
GRAPH_X_SPAN 237
GRAPH_Y_SPAN 194
#define GRAPH_PWM_SPAN 100 // in percent PWM - (0 - x)% usually 0-100%
#define GRAPH_PRESSURE_SPAN 10 //in bar: 0-x bar usually 0-10 bar
#define GRAPH_FLOW_VOLUME_SPAN 90 //in ml/min usually 0-90ml
#define GRAPH_FLOW_RATE_SPAN 400 //in ml/min usually 0-400ml/min
#define MAX_PROFILE_INDEX 250

*/

void graphDrawFLB()
{
	// draw Flow Limiter Bypass threshold dashed line
	uint16_t y = graph_y(FLBThresholdPWM * 10, GRAPH_PWM_SPAN * 10);
	tft.fillRect(0, GRAPH_Y_0 - GRAPH_Y_SPAN + 8 , GRAPH_X_0, GRAPH_Y_SPAN - 10 , bg_Color); 
	printSomething("FLB", 215, y - 11, text_dark_Color, NULL , false);
	for (int x = 0; x < 90; x++)
		tft.drawFastHLine(1 + x * 3, y , 2 , axis_minor_Color);
}  

void graphDrawUnionThreshold()
{
	// draw union threshold dashed line
	uint16_t y = graph_y(unionThreshold * 100, GRAPH_PRESSURE_SPAN * 100);
	tft.fillRect(0, GRAPH_Y_0 - GRAPH_Y_SPAN + 8 , GRAPH_X_0, GRAPH_Y_SPAN - 10 , bg_Color);
	//tft.fillRect(0, 177, 3, 80, bg_Color);
	printSomething("UT", 215, y - 11, text_dark_Color, NULL , false);
	for (int x = 0; x < 90; x++)
		tft.drawFastHLine(1 + x * 3, y , 2 , pressure_Color);
} 

void graphDrawEmptyGraph()
{
	int x0 = GRAPH_X_0;
	int y0 = GRAPH_Y_0;
	int x1 = GRAPH_X_0 + GRAPH_X_SPAN;
	int y1 = GRAPH_Y_0 - GRAPH_Y_SPAN;
	
	clearGraphArea();

	// Write the axis labels
	printSomething("--100%", 135, 115, PWM_Color, NULL , false);
	printSomething("--10bar", 135, 126, pressure_Color , NULL , false);
	printSomething("--100ml", 185, 115, flowVolume_Color, NULL , false); //-23
	printSomething("--400ml/m", 185, 126, flowRate_Color , NULL , false);
	//printSomething("0", 23, 258, text_light_Color, NULL , false);
	printSomething("Time", 110, 313, text_light_Color, NULL , false);
	printSomething("120s", 215, 311, text_light_Color, NULL , false);
	printSomething("0s", 1, 311, text_light_Color, NULL , false);

	// draw the axis
	//tft.drawFastHLine(1,262,239,axis_major_Color);
	tft.drawFastHLine(x0 - 2, y0 , x1 - x0 + 2,axis_major_Color);
	tft.fillTriangle(240,y0,236,311,236,307,axis_major_Color);
	tft.drawFastVLine(x0,y1,194,axis_major_Color); //134-115 = 19
	tft.fillTriangle(x0,y1,x0 - 2,119,x0 + 2,119,axis_major_Color);

	//draw axis markers
/*	tft.drawFastVLine(232,307,6,ILI9341_ORANGE);
	tft.drawFastHLine(0,121 ,6,ILI9341_MAGENTA);  
	tft.drawFastHLine(0,122,6,ILI9341_GREEN);
	tft.drawFastHLine(0,123,6,ILI9341_CYAN);
*/	
}

void clearGraphArea()
{
	tft.fillRect(GRAPH_X_0 + 1, GRAPH_Y_0 - GRAPH_Y_SPAN, GRAPH_X_SPAN, GRAPH_Y_SPAN, bg_Color); //redrawing only the graphing area so as to eliminate axis flicker......	
}	

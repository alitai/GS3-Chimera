//***************************************************************************************************
// Select the proper PID loops based on the current selected pull mode....
//***************************************************************************************************

void selectPIDbyMode()
{

	switch (g_pullMode)
	{
		case AUTO_PRESSURE_PROFILE_PULL:
			Serial.println("AUTO_PRESSURE_PROFILE_PULL");
			startPressurePID();
			break;
		case AUTO_FLOW_PROFILE_PULL:
			Serial.println("AUTO_FLOW_PROFILE_PULL");		
			startFlowPID();
			break;
		case AUTO_PWM_PROFILE_PULL:
			Serial.println("AUTO_PWM_PROFILE_PULL");
			break;
		case MANUAL_PULL:
			Serial.println("MANUAL_PULL");
			break;
		case SLAYER_LIKE_PULL:
			Serial.println("SLAYER_LIKE_PULL");
			break;
		case SLAYER_LIKE_PI_PRESSURE_PROFILE:
			Serial.println("SLAYER_LIKE_PI_PRESSURE_PROFILE");
			startPressurePID();
			break;
		case FLOW_PRESSURE_PROFILE:
			Serial.println("FLOW_PRESSURE_PROFILE");
			startPressurePID();
			startFlowPID();
			break;
	}
}

//*************************************************************************************************
// Execute a PID calc (if it is time to...) and display PID parameters (Input, Output and Setpoint) 
//*************************************************************************************************
unsigned executePID_P(double setValue, unsigned pumpPWM, double currentPressure)
{
	g_PIDInput_P = currentPressure;
	g_PIDSetpoint_P = setValue; 
	if (pressurePID.Compute())
    {
		printSomething("Pressure", 190,188, ILI9341_CYAN, NULL, true);
		tft.fillRect(216, 200, 28, 36, bg_Color);
		printSomething("SP=", 190, 200 , ILI9341_CYAN, NULL , false); 
		tft.setCursor(216,200);
		tft.print(g_PIDSetpoint_P, 1);
		printSomething("In=", 190, 212 , ILI9341_CYAN, NULL , false); 
		tft.setCursor(216,212);
		tft.print(g_PIDInput_P, 1); 
		printSomething("Op=", 190, 224 , ILI9341_CYAN, NULL , false); 
		tft.setCursor(216,224);
		tft.print(g_PIDOutput_P,0);  
		if (g_PIDOutput_P < 0)
			return 0;
		else 
			return (unsigned)g_PIDOutput_P;
    }
	return pumpPWM;
}

unsigned executePID_F(double setValue, unsigned pumpPWM, double currentFlow)
{
	g_PIDInput_F = currentFlow;
	g_PIDSetpoint_F = setValue; 
	if(flowPID.Compute())
	{
		printSomething("Flow    ", 190,188, ILI9341_GREEN, NULL, true);
		tft.fillRect(216, 200, 28, 36, bg_Color);
		printSomething("SP=", 190, 200 , ILI9341_GREEN, NULL , false);  
		tft.setCursor(216,200);
		tft.print(g_PIDSetpoint_F,0);
		printSomething("In=", 190, 212 , ILI9341_GREEN, NULL , false);  
		tft.setCursor(216,212);
		tft.print(g_PIDInput_F,0); 
		printSomething("Op=", 190, 224 , ILI9341_GREEN, NULL , false);  
		tft.setCursor(216,224);
		tft.print(g_PIDOutput_F,0);  
		if (g_PIDOutput_F < 0)
			return 0;
		else 
			return (unsigned)g_PIDOutput_F;
	}
	return pumpPWM;
}  


void startFlowPID()
{
	flowPID.SetSampleTime(PIDSampleTime);
	flowPID.SetMode(AUTOMATIC);
	// The following resets the internal PID accumulators (especially INT)
	flowPID.SetOutputLimits(0.0, 1.0); // Force Integral to 0 Forces minimum up to 0.0
	flowPID.SetOutputLimits(-1.0, 0.0); // Forces maximum down to 0.0
	flowPID.SetOutputLimits((double)FLOW_PID_MIN_PWM, (double)FLOW_PID_MAX_PWM);
}

void startPressurePID()
{
	pressurePID.SetSampleTime(PIDSampleTime);
	pressurePID.SetMode(AUTOMATIC);
	// The following resets the internal PID accumulators (especially INT)
	pressurePID.SetOutputLimits(0.0, 1.0); // Force Integral to 0 Forces minimum up to 0.0
	pressurePID.SetOutputLimits(-1.0, 0.0); // Forces maximum down to 0.0
	pressurePID.SetOutputLimits((double)PRESSURE_PID_MIN_PWM, (double)PRESSURE_PID_MAX_PWM);
} 

void stopPID()
{
	pressurePID.SetMode(MANUAL);
	flowPID.SetMode(MANUAL);
}





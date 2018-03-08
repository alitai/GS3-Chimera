//***************************************************************************************************
// Select the proper PID loops based on the current selected pull mode....
//***************************************************************************************************

void selectPIDbyMode()
{
	if (g_flushCycle)
		return;  		 // Fixed PWM so no need to setup PID for cleaning or flushing
	resetPID();
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
			stopPID(); // PID is not needed for PWM replay
			break;
		case MANUAL_PULL:
			Serial.println("MANUAL_PULL");
			stopPID(); // Turn Off PID for Manual pull
			break;
		case SLAYER_LIKE_PULL:
			Serial.println("SLAYER_LIKE_PULL");
			stopPID(); // PID is not needed for Slayer replay
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
		case AUTO_UNION_PROFILE_PULL:
			//Need to define two PID loops
			Serial.println("AUTO_UNION_PROFILE_PULL");
			startPressurePID();
			startFlowPID();
			break;
	}
}

//*************************************************************************************************
// Execute a PID calc (if it is time to...) and display PID parameters (Input, Output and Setpoint) 
//*************************************************************************************************
unsigned executePID_P(boolean followProfile, unsigned targetValue, int profileIndex, unsigned pumpPWM, double currentPressure, unsigned long pullTimer)
{
	g_PIDInput_P = currentPressure;
	
	// Note: in the profile we store averagePressure as displayed (i.e. multiplied by 100 / 12) so we have to correct the Setpoint by multiplying with reciprocal (12 / 100)
	if (followProfile)
		g_PIDSetpoint_P = (double)g_pressureProfile[profileIndex] * 12.0 / 100.0 + (double)(g_pressureProfile[profileIndex + 1] -
			g_pressureProfile[profileIndex]) * 12.0 / 100.0 * ((double)(pullTimer % 500) /500.0) ; 
    else
		g_PIDSetpoint_P = (double)targetValue; //(double)map(currentPotValue, 0, 1023, 3, 10);
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

unsigned executePID_F(boolean followProfile, unsigned setValue, int profileIndex, unsigned pumpPWM, int sumFlowProfile, unsigned long pullTimer, boolean preInfusion)
{
	
	if (followProfile)
	{
		g_PIDSetpoint_F = (double)sumFlowProfile + (double)(g_flowProfile[profileIndex + 1] >> 1 -
			g_flowProfile[profileIndex] >> 1) * ((double)(pullTimer % 500) /500.0) ;
		g_PIDInput_F = (double)g_flowPulseCount;
	}
	else
	{
		g_PIDInput_F = flowRate(preInfusion);
		g_PIDSetpoint_F = (double)setValue; //(double)map(currentPotValue, 0, 1023, 3, 10);
	}

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
	flowPID.SetOutputLimits(0.0, 1.0); // Force Integral to 0 Forces minimum up to 0.0
	flowPID.SetOutputLimits(-1.0, 0.0); // Forces maximum down to 0.0
	flowPID.SetOutputLimits((double)FLOW_PID_MIN_PWM, (double)FLOW_PID_MAX_PWM * 0.6);
}

void startPressurePID()
{
	pressurePID.SetSampleTime(PIDSampleTime);
	pressurePID.SetMode(AUTOMATIC);
	pressurePID.SetOutputLimits(0.0, 1.0); // Force Integral to 0 Forces minimum up to 0.0
	pressurePID.SetOutputLimits(-1.0, 0.0); // Forces maximum down to 0.0
	pressurePID.SetOutputLimits((double)PRESSURE_PID_MIN_PWM, (double)PRESSURE_PID_MAX_PWM);
} 

void stopPID()
{
	pressurePID.SetMode(MANUAL);
	flowPID.SetMode(MANUAL);
}

void resetPID()
{
	startPressurePID();
	startFlowPID();
	if (pressurePID.GetMode() != MANUAL || flowPID.GetMode() != MANUAL)
		pressurePID.SetMode(MANUAL); // Ensure PID is in MANUAL mode to reset all internal variables
		flowPID.SetMode(MANUAL);
}

/*
//****************************************************************************************************************************
//This PID loop was meant to enable electronic control of PI flow rate - but it is not identical to slayer. On hold therefore.
//****************************************************************************************************************************
void startSlayerFlowPID()
{
	flowPID.SetSampleTime(PIDSampleTime);
	flowPID.SetMode(AUTOMATIC);
	flowPID.SetOutputLimits(0.0, 1.0); // Force Integral to 0 Forces minimum up to 0.0
	flowPID.SetOutputLimits(-1.0, 0.0); // Forces maximum down to 0.0
	double minSetOutputLimit = 0.0d;
	double maxSetOutputLimit = (double)slayerMainPWM;
	flowPID.SetOutputLimits(minSetOutputLimit, maxSetOutputLimit);
}

byte executeSlayerPID_F(int profileIndex, byte pumpPWMByte, int sumFlowProfile, unsigned long pullTimer)
{
	g_PIDSetpoint_F = slayerPIFlowRate ;
	g_PIDInput_F = (double)mlPerFlowMeterPulse * 60.0 * 1000.0 / (double)g_averageF.mean();
  
	if(flowPID.Compute())
		{
			printSomething("Flow    ", 190,188, ILI9341_GREEN, NULL, true);
			tft.fillRect(216, 200, 28, 36, bg_Color);
			printSomething("SP=", 190, 200 , ILI9341_GREEN, NULL , false);  
			tft.setCursor(216,200);
			tft.print((int)g_PIDSetpoint_F);
			printSomething("In=", 190, 212 , ILI9341_GREEN, NULL , false);  
			tft.setCursor(216,212);
			tft.print((int)g_PIDInput_F); 
			printSomething("Op=", 190, 224 , ILI9341_GREEN, NULL , false);  
			tft.setCursor(216,224);
			tft.print((int)g_PIDOutput_F);  
			if (g_PIDOutput_F < 0)
				return 0;
			else 
				return (byte)g_PIDOutput_F;
		}
	return pumpPWMByte;
}  
*/


//***********************************************************************************
// Calculate and return pump speed per pull mode
//***********************************************************************************

byte setPumpSpeedbyMode(int profileIndex, long pullTimer, byte pumpSpeedByte, float currentPressure, boolean preInfusion, int sumFlowProfile, int unionSkew)
{
	if (g_cleanCycle || g_flushCycle)
		pumpSpeedByte = (float) cleanPWM * 2.55;
	else
	{
		switch(g_pullMode)
		{
			case MANUAL_PULL:
				int currentPotValue;
				currentPotValue = analogRead(CONTROL_POT);
#ifdef OTTO_HTWF 
				// For otto Controls HTWF-1A12A22A Hall Effect 0-5V paddle control
				if (currentPotValue < 520)
					pumpSpeedByte = (byte)map(currentPotValue, 100, 520, 0, FLBThreshold * 2.55);
				else
					pumpSpeedByte = (byte)map(currentPotValue, 520, 950, FLBThreshold * 2.55, pumpMaxPercent * 2.55 );
#else
				pumpSpeedByte = (byte)(currentPotValue >> 2); //converts int to byte for standard potentiometer control
#endif
				break;
			case AUTO_PWM_PROFILE_PULL:
				pumpSpeedByte = (byte)(g_PWMProfile[profileIndex]+
					(int)((double)(g_PWMProfile[profileIndex+1] - 
					g_PWMProfile[profileIndex])*((double)(pullTimer % 500)/500.0)));
				break;
			case AUTO_PRESSURE_PROFILE_PULL:
				pumpSpeedByte = executePID_P(profileIndex, pumpSpeedByte, currentPressure, pullTimer);
				break;
			case AUTO_FLOW_PROFILE_PULL:
				pumpSpeedByte = executePID_F(profileIndex, pumpSpeedByte, sumFlowProfile, pullTimer);
				break;
			case SLAYER_LIKE_PULL:
				pumpSpeedByte = (float)slayerMainPWM * 255 / 100; // 0-255 range
				break;			
			case AUTO_UNION_PROFILE_PULL:
				//Need to define two setpoints and two Inputs
				if (preInfusion) //if union is in preInfusion mode, it is running Flow Profiling
					pumpSpeedByte = executePID_F(profileIndex, pumpSpeedByte, sumFlowProfile, pullTimer);
				else
					pumpSpeedByte = executePID_P(profileIndex - unionSkew, pumpSpeedByte, currentPressure, pullTimer);
				break;
		}
	}
	// Action Time - operate pump & operate FLB solenoid
	md.setM1Speed(constrain(((int)((float)pumpSpeedByte * 400.0 / 255.0)), pumpMinPWM, pumpMaxPWM));
	return pumpSpeedByte;
}

//*******************************************************************************
// Calculate FLB cutoff for each pull mode and activate/deactivate solenoid
//*******************************************************************************

boolean setFlowLimitBypass(byte pumpSpeedByte, int profileIndex, boolean preInfusion)
{
	if (g_cleanCycle || g_flushCycle)
		preInfusion = false;
	
	if (preInfusion)
	{
		switch(g_pullMode)
		{
			case MANUAL_PULL:
			case AUTO_PWM_PROFILE_PULL:
			case AUTO_PRESSURE_PROFILE_PULL:
			case AUTO_FLOW_PROFILE_PULL:
				if ((unsigned)pumpSpeedByte * 100 / 255 > FLBThreshold)
					preInfusion = false;
				break;
			case SLAYER_LIKE_PULL:
				if (profileIndex > slayerPIPeriod << 1)
					preInfusion = false;
				break;			
		}
	}

	if (preInfusion) 
	{
#ifdef INVERT_FLB_OUTPUT 
		digitalWrite(FLOW_LIMIT_BYPASS, HIGH);
#else
		digitalWrite(FLOW_LIMIT_BYPASS, LOW);
#endif
		if (profileIndex % 2 == 0)
			digitalWrite(GREEN_LED, HIGH);
		else
			digitalWrite(GREEN_LED, LOW);
	}
	else
	{
#ifdef INVERT_FLB_OUTPUT  
		digitalWrite(FLOW_LIMIT_BYPASS, LOW);
#else
		digitalWrite(FLOW_LIMIT_BYPASS, HIGH);
#endif
		digitalWrite(GREEN_LED, HIGH);
	}

	return preInfusion;	
}


void initFlowLimitBypass()
{
	pinMode(FLOW_LIMIT_BYPASS, OUTPUT);

#ifdef INVERT_FLB_OUTPUT 
	digitalWrite(FLOW_LIMIT_BYPASS, HIGH); // turn off FLB solenoid
#else
	digitalWrite(FLOW_LIMIT_BYPASS, LOW); // turn off FLB solenoid
#endif

}

void flushCycle()	
{
	detachInterrupt(digitalPinToInterrupt(GROUP_SOLENOID));
	g_newPull = false;
	while (digitalRead(GROUP_SOLENOID) == LOW)
		md.setM1Speed(constrain((int)(cleanPWM * 4.0), pumpMinPWM, pumpMaxPWM));
	md.setM1Speed(0);	 //Shut down pump motor 
	g_flushCycle = false;
}
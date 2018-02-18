//***********************************************************************************
// Calculate and return pump speed per pull mode
//***********************************************************************************

unsigned setpumpPWMbyMode(int profileIndex, long pullTimer, unsigned pumpPWM, float currentPressure, boolean preInfusion, int sumFlowProfile, int unionSkew)
{
	if (g_cleanCycle || g_flushCycle)
		pumpPWM = cleanPWM;
	else
	{
		int currentPotValue = analogRead(CONTROL_POT);
		switch(g_pullMode)
		{
			case MANUAL_PULL:	
#ifdef OTTO_HTWF 
				// For otto Controls HTWF-1A12A22A Hall Effect 0-5V paddle control
				if (currentPotValue < 520)
					pumpPWM = constrain(map(currentPotValue, 100, 520, 0, FLBThresholdPWM), pumpMinPWM, FLBThresholdPWM);
				else
					pumpPWM = constrain(map(currentPotValue, 520, 930, FLBThresholdPWM, pumpMaxPWM), FLBThresholdPWM, pumpMaxPWM);
#else
				pumpPWM = (unsigned) currentPotValue * 400 / 1024; //converts int to byte for standard potentiometer control
#endif
				break;
			case AUTO_PWM_PROFILE_PULL:
				pumpPWM = (unsigned) 400 / 255 * ((g_PWMProfile[profileIndex]+
					(g_PWMProfile[profileIndex+1] - 
					g_PWMProfile[profileIndex])*((double)(pullTimer % 500)/500.0)));
				break;
			case AUTO_PRESSURE_PROFILE_PULL:
				pumpPWM = executePID_P(true, 0, profileIndex, pumpPWM, currentPressure, pullTimer);
				break;
			case AUTO_FLOW_PROFILE_PULL:
				pumpPWM = executePID_F(profileIndex, pumpPWM, sumFlowProfile, pullTimer);
				break;
			case SLAYER_LIKE_PULL:
				pumpPWM = slayerMainPWM; 
				break;	
			case SLAYER_LIKE_PI_PRESSURE_PROFILE:	
				pumpPWM = executePID_P(false, currentPotValue, profileIndex, pumpPWM, currentPressure, pullTimer); // calculate always but
				if (pullTimer < 1000 || currentPressure < 3.0)                                                                         // override during PI
					pumpPWM = slayerMainPWM;
				break;
			case AUTO_UNION_PROFILE_PULL:
				//Need to define two setpoints and two Inputs
				if (preInfusion) //if union is in preInfusion mode, it is running Flow Profiling
					pumpPWM = executePID_F(profileIndex, pumpPWM, sumFlowProfile, pullTimer);
				else
					pumpPWM = executePID_P(true, 0, profileIndex - unionSkew, pumpPWM, currentPressure, pullTimer);
				break;
		}
	}
	// Action Time - operate pump & operate FLB solenoid
	md.setM1Speed(constrain(pumpPWM, pumpMinPWM, pumpMaxPWM));
	return pumpPWM;
}

//*******************************************************************************
// Calculate FLB cutoff for each pull mode and activate/deactivate solenoid
//*******************************************************************************

boolean setFlowLimitBypass(unsigned pumpPWM, int profileIndex, boolean preInfusion, float currentPressure)
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
				if (pumpPWM > FLBThresholdPWM + 1)
					preInfusion = false;
				break;
			case SLAYER_LIKE_PULL:
				if (profileIndex > slayerPIPeriod << 1)
					preInfusion = false;
				break;	
			case SLAYER_LIKE_PI_PRESSURE_PROFILE:
				if(profileIndex > 2 && currentPressure > 3.0)
					preInfusion = false;
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
#ifdef INVERT_FLB_OUTPUT  
	digitalWrite(FLOW_LIMIT_BYPASS, LOW);
#else
	digitalWrite(FLOW_LIMIT_BYPASS, HIGH);
#endif
	digitalWrite(GREEN_LED, HIGH); 
	digitalWrite(RED_LED, LOW);
	g_newPull = false;
	while (digitalRead(GROUP_SOLENOID) == LOW)
		md.setM1Speed(constrain(cleanPWM, pumpMinPWM, pumpMaxPWM));
	md.setM1Speed(0);	 //Shut down pump motor
#ifdef INVERT_FLB_OUTPUT  
	digitalWrite(FLOW_LIMIT_BYPASS, HIGH);
#else
	digitalWrite(FLOW_LIMIT_BYPASS, LOW);
#endif	
	g_flushCycle = false;
	digitalWrite(GREEN_LED, LOW); 
	digitalWrite(RED_LED, HIGH);
}
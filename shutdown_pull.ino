#ifdef SERIAL_CONTROL

byte shutdownIfPullOver(byte countOffCycles, int lastProfileIndex)
{
	// If in cleaning - wait for the last cycle to finish by timing 
	// Note - there is no delay for a flush cycle
	if (g_cleanCycle && digitalRead(GROUP_SOLENOID) == HIGH) 
	{
		md.setM1Speed(0); //Shut down pump motor
		digitalWrite(FLOW_LIMIT_BYPASS, LOW); // turn off FLB solenoid
		unsigned long cleanPauseTimer = millis();
			
		while ((millis() - cleanPauseTimer) < 6000) //Pause is about 6 seconds...
		{
			if (Serial2.available() || digitalRead(GROUP_SOLENOID) == LOW) //Get out of wait NOW!
				{
				digitalWrite(GREEN_LED, HIGH);
				digitalWrite(RED_LED, LOW);
				break;
				}			
						
			if (((millis() - cleanPauseTimer))/500 % 2 == 0)
			{
				digitalWrite(GREEN_LED, HIGH);
				digitalWrite(RED_LED, LOW);
			}
			else
			{
				digitalWrite(GREEN_LED, LOW);
				digitalWrite(RED_LED, HIGH);
			}	
		}
		if (Serial2.available() || digitalRead(GROUP_SOLENOID) == HIGH)
		{
			countOffCycles = 0; //This will end the cleaning cycle...
			g_cleanCycle = false;
			g_flushCycle = false;
			selectandDrawProfilebyMode();
			//fastModeSwitch(); // if coming from a flush - return to original mode and screen
		}

	}
	
	if (g_flushCycle && digitalRead(GROUP_SOLENOID) == HIGH)
	{
		countOffCycles = 0; //This will end the cleaning cycle...
		g_cleanCycle = false;
		g_flushCycle = false;
		selectandDrawProfilebyMode();
		//fastModeSwitch(); // if coming from a flush - return to original mode and screen
	}

	if (digitalRead(GROUP_SOLENOID) == HIGH || lastProfileIndex > 195 )  // Check if time to end but avoid spurious shutdowns      
	{
		if (countOffCycles < 1 || lastProfileIndex > 199 || Serial2.available())
		{
			g_cleanCycle = true; // be ready for the next cleaning cycle (not keypad initiated)
			md.setM1Speed(0); //Shut down pump motor 
			stopPID(); //Shut down PID loops
#ifdef INVERT_FLB_OUTPUT 
			digitalWrite(FLOW_LIMIT_BYPASS, HIGH); // turn off FLB solenoid
#else
			digitalWrite(FLOW_LIMIT_BYPASS, LOW); // turn off FLB solenoid
#endif
			digitalWrite(GREEN_LED, LOW); 
			digitalWrite(RED_LED, HIGH);
			g_activePull = false; // Pull is over!
		}  
		else
			countOffCycles--;
	}
	else
		countOffCycles = debounceCount;
	return countOffCycles;
}	

#else

byte shutdownIfPullOver(byte countOffCycles, int lastProfileIndex)
{
	if (digitalRead(GROUP_SOLENOID) == HIGH || lastProfileIndex > 195 )  // Check if time to end but avoid spurious shutdowns      
	{
		if (countOffCycles < 1 || lastProfileIndex > 199)
		{
			g_cleanCycle = false; // cleaning cycle are not automated if serial not available
			md.setM1Speed(0); //Shut down pump motor 
			stopPID(); //Shut down PID loops
#ifdef INVERT_FLB_OUTPUT 
			digitalWrite(FLOW_LIMIT_BYPASS, HIGH); // turn off FLB solenoid
#else
			digitalWrite(FLOW_LIMIT_BYPASS, LOW); // turn off FLB solenoid
#endif
			digitalWrite(GREEN_LED, LOW); 
			digitalWrite(RED_LED, HIGH);
			g_activePull = false; // Pull is over!
		}  

		else
			countOffCycles--;
	}
	else
		countOffCycles = debounceCount;
	return countOffCycles;
}
#endif
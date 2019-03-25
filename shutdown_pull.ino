byte shutdownIfPullOver(byte countOffCycles, uint16_t lastProfileIndex)
{
	if (digitalRead(GROUP_SOLENOID) == LOW && lastProfileIndex < MAX_PROFILE_INDEX) 
		return debounceCount;
	else
		countOffCycles--;
	
	if (countOffCycles < 1 || lastProfileIndex > MAX_PROFILE_INDEX || Serial2.available()) 
	{	
		Serial.println("Shutting down pull...");
#ifdef ACAIA_LUNAR_INTEGRATION
	scale->pauseTimer();
#endif
#ifdef MEGUNOLINK
	//	megunolinkPlot.Clear();
		megunolinkPlot.Run(false);
#endif
		resetSystem();	
	}
	
	return countOffCycles;
}

void resetSystem()
{
	Serial.println("Resetting hardware and interrupts");
	md.setM1Speed(0); //Shut down pump motor 
	stopPID(); //Shut down PID loops
	setFLB(false);
	ledColor('r');
	g_activePull = false; 	// Pull is over!
	g_newPull = false;		//
	g_flushCycle = true; 	// be ready for the next cleaning cycle (not Serial port initiated)
	
	//*****************************************************************************
	// Clear interrupt cache and attach interrupt - and be ready for next pull
    //*****************************************************************************	
	//EIFR = 2; // For Arduino Uno 
	EIFR = _BV (INTF5); // Clear outstanding interrupts 	
    attachInterrupt(digitalPinToInterrupt(GROUP_SOLENOID), pullEspresso, FALLING);
	Serial.println("Chimera is now ready for a new pull. To enter parameter editor type E.");
}	


//  Setup Slayer parameter adjustment screen

uint16_t measurePotValue()
{
	//returns a value of 0-1024 for all types of controls...
	uint16_t currentPotValue = analogRead(CONTROL_POT);
#ifdef OTTO_HTWF 
	// For Otto Controls HTWF-1A12A22A Hall Effect 0-5V paddle control
	currentPotValue = constrain(map(currentPotValue, 95, 925, 0, 1024), 1, 1024);//FLBThresholdPWM), pumpMinPWM, pumpMaxPWM);//FLBThresholdPWM + 150);
#endif
	return(currentPotValue);
}	


void menuSetupSlayerParameters()
{
	uint16_t currentPotValue = measurePotValue(); //analogRead(CONTROL_POT); 
	bool potMoved = wasPotMoved(currentPotValue); 	//check if potentiometer was moved	

	switch (g_selectedParameter)
	{
		case 1: 
			if (potMoved)
			{
// Assaf extended the range from 45 to 70 seconds 			
			//	slayerPIPeriod = map(currentPotValue, 0, 1023, 4, 45); // 4 - 45 seconds
				slayerPIPeriod = map(currentPotValue, 0, 1023, 4, 70); //4 - 70 seconds
				graphDrawSlayerProfile(); // Updates the graph with the new data...
			}
			EEPROM.update(43, slayerPIPeriod);
			break;
		case 2: 
			if (potMoved)
			{ 
				slayerMainPWM = map(currentPotValue, 0, 1023, slayerPIFlowRate, slayerMaxPWM);
				graphDrawSlayerProfile(); // Updates the graph with the new data...
			}
			EEPROM.put(41, slayerMainPWM);
			break;
			
	}
}

bool wasPotMoved(uint16_t currentPotValue)
{	
	//debounce the move (to reduce flicker)
	int potDelta = 5;
	if ((currentPotValue > g_lastParameterPotValue + potDelta) || (currentPotValue < g_lastParameterPotValue - potDelta))
	{
		g_lastParameterPotValue = currentPotValue;
		return true;
	}
	else
		return false;
}

void editParametersOverSerial()
{
	bool edit = true;
	
	while (edit)
	{
		readEEPROMtoSerial();
		
		Serial.println("");
		Serial.println("Chimera is disabled. Edit System Parameters:");
		Serial.println("Press a letter to change the value of its parameter.");
		Serial.println("Press Z for an EEPROM dump. Press X to eXit.");
		Serial.println("Make sure you set your terminal to send Newline line endings");
		Serial.println();

		char command;
		do
		{
			command = toupper (Serial.read ());
		} 
		while 
			(command != 'A' && command != 'B' && command != 'C' && command != 'D' && command != 'E' 
			&& command != 'F' && command != 'G' && command != 'H' && command != 'I'	&& command != 'J' 
			&& command != 'K' && command != 'L' && command != 'M' && command != 'N' && command != 'X' && command != 'Z');

		if (command == 'X')
		{
			Serial.println("Chimera is now ready for a new pull. To enter parameter editor type E.");
			edit = false;
			return;
		}
		
		if (command == 'Z') 
		{	
			readProfilestoSerial();
			Serial.println("Chimera is now ready for a new pull. To enter parameter editor type E.");
			edit = false;
			return;
		}
		
		int flushChar;
		do
		{
			flushChar = Serial.read();
		}	while (flushChar != '\n');
		

		Serial.print("Please enter new value for ");
		switch (command)
		{
			case 'A': 
				Serial.print("FLBThresholdPWM");
				break;
			case 'B':
				Serial.print("debounceCount");
				break;
			case 'C':
				Serial.print("pumpMinPWM");
				break;
			case 'D':
				Serial.print("pumpMaxPWM");
				break;
			case 'E':
				Serial.print("mlPerFlowMeterPulse");
				break;	
			case 'F':
				Serial.print("unionThreshold");
				break;						
			case 'G':
				Serial.print("Kpp");
				break;			
			case 'H':
				Serial.print("Kpi");
				break;			
			case 'I':
				Serial.print("Kpd");
				break;			
			case 'J':
				Serial.print("Kfp");
				break;			
			case 'K':
				Serial.print("Kfi");
				break;			
			case 'L':
				Serial.print("Kfd");
				break;			
			case 'M':
				Serial.print("slayerMainPWM");
				break;			
			case 'N':
				Serial.print("slayerPIPeriod");
				break;
		}
		//Serial.print(command);
		Serial.println(":");
		String inString = "";
		bool newValue = false;
		while (!newValue) 
		{
			while (Serial.available() > 0)
			{
				int inChar = Serial.read();
				if (inChar != '\n') 
					// As long as the incoming byte is not a newline, convert the incoming byte to a char and add it to the string
					inString += (char)inChar;
				else 
					newValue = true;
			}
		}
		
		switch (command)
		{
		case 'A':
			FLBThresholdPWM = inString.toInt();
			Serial.print("New FLBThresholdPWM value is: ");
			Serial.println(FLBThresholdPWM, DEC);
			EEPROM.put(0, FLBThresholdPWM);
			break;
		case 'B':
			debounceCount = inString.toInt();
			Serial.print("New debounceCount value is: ");
			Serial.println(debounceCount, DEC);
			EEPROM.update(2, (byte)debounceCount); //EEPROM.update(2, (byte)debounceCount);
			break;
		case 'C':
			pumpMinPWM = inString.toInt();
			Serial.print("New pumpMinPWM value is: ");
			Serial.println(pumpMinPWM, DEC);
			EEPROM.put(3, pumpMinPWM);
			break;
		case 'D':
			pumpMaxPWM = inString.toInt();
			Serial.print("New pumpMaxPWM value is: ");
			Serial.println(pumpMaxPWM, DEC);
			EEPROM.put(5, pumpMaxPWM);
			break;			
		case 'E':
			mlPerFlowMeterPulse = inString.toFloat();
			Serial.print("New mlPerFlowMeterPulse value is: ");
			Serial.println(mlPerFlowMeterPulse, 3);
			EEPROM.put(8, (float)mlPerFlowMeterPulse);
			break;
		case 'F':
			unionThreshold = inString.toFloat();
			Serial.print("New unionThreshold value is: ");
			Serial.println(unionThreshold, 3);
			EEPROM.put(12, (double)unionThreshold);
			break;			
		case 'G':
			Kpp = inString.toFloat();
			Serial.print("New Kpp value is: ");
			Serial.println(Kpp, 2);
			EEPROM.put(16, Kpp);
			break;
		case 'H':
			Kpi = inString.toFloat();
			Serial.print("New Kpi value is: ");
			Serial.println(Kpi, 2);
			EEPROM.put(20, Kpi);
			break;
		case 'I':
			Kpd = inString.toFloat();
			Serial.print("New Kpd value is: ");
			Serial.println(Kpd, 2);
			EEPROM.put(24, Kpd);
			break;
		case 'J':
			Kfp = inString.toFloat();
			Serial.print("New Kfp value is: ");
			Serial.println(Kfp, 2);
			EEPROM.put(28, Kfp);
			break;
		case 'K':
			Kfi = inString.toFloat();
			Serial.print("New Kfi value is: ");
			Serial.println(Kfi, 2);
			EEPROM.put(32, Kfi);
			break;
		case 'L':
			Kfd = inString.toFloat();
			Serial.print("New Kfd value is: ");
			Serial.println(Kfd, 2);
			EEPROM.put(36, Kfd);
			break;
		case 'M':
			slayerMainPWM = inString.toInt(); 
			Serial.print("New slayerMainPWM value is: ");
			Serial.println(slayerMainPWM, DEC);
			EEPROM.put(41, slayerMainPWM);
			break;
		case 'N':
			slayerPIPeriod = inString.toInt(); // EEPROM.read(38);
			Serial.print("New slayerPIPeriod value is: ");
			Serial.println(slayerPIPeriod, DEC);
			EEPROM.update(43, slayerPIPeriod); //EEPROM.update(43, slayerPIPeriod);
			break;
	
		}
		
		Serial.println("Please enter Y to continue...");

		do
		{
			command = toupper (Serial.read ());
		} 
		while 
			(command != 'Y');
			
		Serial.println("");
	}
}	


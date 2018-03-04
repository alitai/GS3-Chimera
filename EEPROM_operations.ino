//*********************************************************************
// EEPROM management fuctions
//
// Parameter data structure
// Address	Data				Length
//  0		FLBThresholdPWM		byte
//	1		debounceCount		byte
//	2		pumpMinPWM			byte
//	3		pumpMaxPWM			byte
//	4		mlPerFlowMeterPulse
//	8		unionThreshold
//	12		Kpp					double (4 bytes)
//	16		Kpi					double (4 bytes)
//	20		Kpd					double (4 bytes)
//	24		Kfp					double (4 bytes)
//	28		Kfi					double (4 bytes)
//	32		Kfd					double (4 bytes)
//	36		layerPIFlowRate		byte
//	37		slayerMainPWM		byte
//	38		slayerPIPeriod		byte
//  39      slayerMaxPWM		byte
//
// 	Profile data structure
//	97		Size of PWMprofile	byte
//	98		Size of Pressure p.	byte
//	99		Size of Flow prof.	byte
//	100-301	PWM Profile			bytes (2 per second)
//	302-503	Pressure Profile	bytes (2 per second)
//	504-705	Flow Profile		bytes (2 per second)
//
//*********************************************************************

// Store PWM, pressure and flow Profiles in EEPROM
void storeProfilesinEEPROM()
{
  EEPROM.update(97, (byte)sizeof(g_PWMProfile)); // Always 201
  EEPROM.update(98, (byte)sizeof(g_pressureProfile)); //Always 201
  EEPROM.update(99, (byte)sizeof(g_flowProfile)); // Always 201

  int shiftAddr = 100; // Start writing arrays  at 10th byte
  int shiftPP = shiftAddr + sizeof(g_PWMProfile);
  int shiftFP = shiftPP + sizeof(g_pressureProfile);

  // Write PWM Profile 
  for (int addr = 0; addr < sizeof(g_PWMProfile); addr++)
  {
    EEPROM.update(addr + shiftAddr, g_PWMProfile[addr]);
    EEPROM.update(addr + shiftPP, g_pressureProfile[addr]);
    EEPROM.update(addr + shiftFP, g_flowProfile[addr]);
  }
}

// Retrieve PWM, pressure and flow Profiles from EEPROM
void readProfilesfromEEPROM()
{
  int PWMProfileSize = EEPROM.read(97);
  int pressureProfileSize = EEPROM.read(98);
  int flowProfileSize = EEPROM.read(99);

  int shiftAddr = 100; // Start reading arrays at 10th byte (should match similar decleration at storeProfilesinEEPROM()
  int shiftPP = shiftAddr + PWMProfileSize;
  int shiftFP = shiftPP + pressureProfileSize;
  
  for (int addr = 0; addr < PWMProfileSize; addr++) 
  {
    g_PWMProfile[addr] = EEPROM.read(addr + shiftAddr);
    g_pressureProfile[addr] = EEPROM.read(addr + shiftPP);
    g_flowProfile[addr] = EEPROM.read(addr + shiftFP);
  } 
}

// Retrieve software parameters from EEPROM
void readSWParametersfromEEPROM()
{
	Serial.println("Retrieving EEPROM Paramaters");
	EEPROM.get(0, FLBThresholdPWM);
	debounceCount = EEPROM.read(2);
	EEPROM.get(3, pumpMinPWM);
	EEPROM.get(5, pumpMaxPWM);
	EEPROM.get(8, mlPerFlowMeterPulse);
	EEPROM.get(12, unionThreshold);
	EEPROM.get(16, Kpp);
	EEPROM.get(20, Kpi);
	EEPROM.get(24, Kpd);
	EEPROM.get(28, Kfp);
	EEPROM.get(32, Kfi);
	EEPROM.get(36, Kfd);
} 

// Update software parameters in EEPROM
void writeSWParameterstoEEPROM()
{
	Serial.println("Saving EEPROM Paramaters");
	EEPROM.put(0, FLBThresholdPWM);
	EEPROM.update(2, debounceCount);
	EEPROM.put(3, pumpMinPWM);
	EEPROM.put(5, pumpMaxPWM);
	EEPROM.put(8, mlPerFlowMeterPulse);
	EEPROM.put(12, unionThreshold);
	EEPROM.put(16, Kpp);
	EEPROM.put(20, Kpi);
	EEPROM.put(24, Kpd);
	EEPROM.put(28, Kfp);
	EEPROM.put(32, Kfi);
	EEPROM.put(36, Kfd);
} 

// Update Slayer parameters to EEPROM

void readSlayerParametersfromEEPROM()
{
	Serial.println("Retrieving Slayer Paramaters from EEPROM");
	slayerPIFlowRate = EEPROM.read(40);
	EEPROM.get(41, slayerMainPWM);
	slayerPIPeriod = EEPROM.read(43);
} 

void writeSlayerParameterstoEEPROM()
{
	Serial.println("Writing Slayer Paramaters to EEPROM");
	EEPROM.update(40, slayerPIFlowRate);
	EEPROM.put(41, slayerMainPWM);
	EEPROM.update(43, slayerPIPeriod);
	EEPROM.put(44, slayerMaxPWM);
} 

/*
 * EEPROM Read
 *
 * Reads the value of each byte of the EEPROM and prints it
 *
 */

void readEEPROMtoSerial() 
{
	byte value;
	float fpvalue;
	int intValue;

	Serial.println("");
	Serial.println("*****************************************");
	Serial.println("*           System Parameters           *");
	Serial.println("*****************************************");

	EEPROM.get(0, intValue);
	Serial.print("a: FLBThresholdPWM");
	Serial.print("\t");
	Serial.print(intValue, DEC);
	Serial.print("/400 (");
	Serial.print(intValue/4, DEC);
	Serial.println("%)");

	value = EEPROM.read(2);
	Serial.print("b: debounceCount");
	Serial.print("\t");
	Serial.println(value, DEC);

	EEPROM.get(3, intValue);
	Serial.print("c: pumpMinPWM");
	Serial.print("\t\t");
	Serial.println(intValue, DEC);
	Serial.print("/400 (");
	Serial.print(intValue/4, DEC);
	Serial.println("%)");


	EEPROM.get(5, intValue);
	Serial.print("d: pumpMaxPWM");
	Serial.print("\t\t");
	Serial.println(intValue, DEC);
	Serial.print("/400 (");
	Serial.print(intValue/4, DEC);
	Serial.println("%)");


	EEPROM.get(8 , fpvalue);
	Serial.print("e: mlPerFlowMeterPulse");
	Serial.print("\t");
	Serial.println(fpvalue, 3);

	EEPROM.get(12, fpvalue);
	Serial.print("f: unionThreshold");
	Serial.print("\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(16, fpvalue);
	Serial.print("g: Kpp");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(20, fpvalue);
	Serial.print("h: Kpi");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(24, fpvalue);
	Serial.print("i: Kpd");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(28, fpvalue);
	Serial.print("j: Kfp");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(32, fpvalue);
	Serial.print("k: Kfi");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(36, fpvalue);
	Serial.print("l. Kfd");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);
	
	EEPROM.get(41, intValue);
	Serial.print("m: slayerMainPWM");
	Serial.print("\t");
	Serial.println(intValue, DEC);
	Serial.print("/400 (");
	Serial.print(intValue/4, DEC);
	Serial.println("%)");

	
	value = EEPROM.read(38);
	Serial.print("n: slayerPIPeriod");
	Serial.print("\t");
	Serial.println(value, DEC);
	
	Serial.println("*****************************************");
	Serial.println("");
}	

void readProfilestoSerial() 
{
	byte value;
	float fpvalue;	


	int address = 0;

	value = EEPROM.read(97);
	Serial.print("Sizeof PWM Profile");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	value = EEPROM.read(98);
	Serial.print("Sizeof Pressure Profile");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	value = EEPROM.read(99);
	Serial.print("Sizeof Flow Profile");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	for (int i = 100; i < 301; i++)
	{
		value = EEPROM.read(i);
		Serial.print("PWM");
		Serial.print("\t");
		Serial.print(i);
		Serial.print("\t");
		Serial.print(value, DEC);
		Serial.println();
	}
	
		for (int i = 302 ; i < 503; i++)
	{
		value = EEPROM.read(i);
		Serial.print("Pressure");
		Serial.print("\t");
		Serial.print(i);
		Serial.print("\t");
		Serial.print(value, DEC);
		Serial.println();
	}
	
		for (int i = 504; i < 705; i++)
	{
		value = EEPROM.read(i);
		Serial.print("Flow");
		Serial.print("\t");
		Serial.print(i);
		Serial.print("\t");
		Serial.print(value, DEC);
		Serial.println();
	}
}

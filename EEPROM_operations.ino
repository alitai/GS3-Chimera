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
  FLBThresholdPWM = (unsigned) EEPROM.read(0) * 4;
  Serial.print("FLBThresholdPWM = ");
  Serial.print(EEPROM.read(0));
  Serial.print(" ");
  Serial.println(FLBThresholdPWM); 
  debounceCount = EEPROM.read(1);
  pumpMinPWM = (unsigned)((long)EEPROM.read(2) * 400 / 255);
  Serial.print("pumpMinPWM = ");
  Serial.print(EEPROM.read(2));
  Serial.print(" ");
  Serial.println(pumpMinPWM); 
  pumpMaxPWM = (unsigned)((long)EEPROM.read(3) * 400 / 255);
  Serial.print("pumpMaxPWM = ");
  Serial.print(EEPROM.read(3));
  Serial.print(" ");
  Serial.println(pumpMaxPWM); 
  EEPROM.get(4, mlPerFlowMeterPulse);
  Serial.print("mlPerFlowMeterPulse = ");
  Serial.println(mlPerFlowMeterPulse); 
  EEPROM.get(8, unionThreshold);
  EEPROM.get(12, Kpp);
  EEPROM.get(16, Kpi);
  EEPROM.get(20, Kpd);
  EEPROM.get(24, Kfp);
  EEPROM.get(28, Kfi);
  EEPROM.get(32, Kfd);
} 

// Update software parameters in EEPROM
void writeSWParameterstoEEPROM()
{
  EEPROM.update(0, (byte)(FLBThresholdPWM / 4));
  Serial.print("FLBThresholdPWM = ");
  Serial.print(FLBThresholdPWM); 
  Serial.print(" Stored as: ");
  Serial.println(EEPROM.read(0));
  EEPROM.update(1, debounceCount);
  EEPROM.update(2, (byte)((long)pumpMinPWM * 255 / 400));
  Serial.print("pumpMinPWM = ");
  Serial.print(pumpMinPWM); 
  Serial.print(" Stored as: ");
  Serial.println(EEPROM.read(2));
  EEPROM.update(3, (byte)((long)pumpMaxPWM * 255 / 400));
  Serial.print("pumpMaxPWM = ");
  Serial.println(pumpMaxPWM); 
  Serial.print(" Stored as: ");
  Serial.println(EEPROM.read(3));
	EEPROM.put(4, (float)mlPerFlowMeterPulse);
	EEPROM.put(8, (float)unionThreshold);
	EEPROM.put(12, Kpp);
	EEPROM.put(16, Kpi);
	EEPROM.put(20, Kpd);
	EEPROM.put(24, Kfp);
	EEPROM.put(28, Kfi);
	EEPROM.put(32, Kfd);
} 

// Update Slayer parameters to EEPROM

void readSlayerParametersfromEEPROM()
{
	Serial.println("Retrieving Slayer Paramaters from EEPROM");
	slayerPIFlowRate = EEPROM.read(36);
	slayerMainPWM = (unsigned) EEPROM.read(37) * 400 / 255;
	slayerPIPeriod = EEPROM.read(38);
} 

void writeSlayerParameterstoEEPROM()
{
	Serial.println("Writing Slayer Paramaters to EEPROM");
	EEPROM.update(36, slayerPIFlowRate);
	EEPROM.update(37, (byte)((long)slayerMainPWM * 255 / 400));
	EEPROM.update(38, slayerPIPeriod);
	EEPROM.update(39, (byte)((long)slayerMaxPWM * 255 / 400));
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

	Serial.println("");
	Serial.println("*****************************************");
	Serial.println("*           System Parameters           *");
	Serial.println("*****************************************");

	value = EEPROM.read(0) * 4;
	Serial.print("a: FLBThresholdPWM");
	Serial.print("\t");
	Serial.println(value, DEC);

	value = EEPROM.read(1);
	Serial.print("b: debounceCount");
	Serial.print("\t");
	Serial.println(value, DEC);

	value = (unsigned)((long)EEPROM.read(2) * 400 / 255);
	Serial.print("c: pumpMinPWM");
	Serial.print("\t\t");
	Serial.println(value, DEC);

	value = (unsigned)((long)EEPROM.read(3) * 400 / 255);
	Serial.print("d: pumpMaxPWM");
	Serial.print("\t\t");
	Serial.println(value, DEC);

	EEPROM.get(4 , fpvalue);
	Serial.print("e: mlPerFlowMeterPulse");
	Serial.print("\t");
	Serial.println(fpvalue, 3);

	EEPROM.get(8, fpvalue);
	Serial.print("f: unionThreshold");
	Serial.print("\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(12, fpvalue);
	Serial.print("g: Kpp");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(16, fpvalue);
	Serial.print("h: Kpi");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(20, fpvalue);
	Serial.print("i: Kpd");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(24, fpvalue);
	Serial.print("j: Kfp");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(28, fpvalue);
	Serial.print("k: Kfi");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);

	EEPROM.get(32, fpvalue);
	Serial.print("l. Kfd");
	Serial.print("\t\t\t");
	Serial.println(fpvalue, 2);
	
	value = (unsigned)EEPROM.read(37) * 400 / 255;
	Serial.print("m: slayerMainPWM");
	Serial.print("\t");
	Serial.println(value, DEC);
	
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

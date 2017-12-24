/*
 * EEPROM Read
 *
 * Reads the value of each byte of the EEPROM and prints it
 *
 */

void readEEPROMtoSerial() 
{
	int address = 0;
	byte value;
	float fpvalue;	
	
	// initialize serial and wait for port to open:
	Serial.begin(9600);
	while (!Serial) {}
	// wait for serial port to connect. Needed for native USB port only

	value = EEPROM.read(0);
	Serial.print("FLBThreshold");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	value = EEPROM.read(1);
	Serial.print("dbounceCount");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	value = EEPROM.read(2);
	Serial.print("pumpMinPWM");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	value = EEPROM.read(3);
	Serial.print("pumpMaxPWM");
	Serial.print("\t");
	Serial.print(value, DEC);
	Serial.println();

	EEPROM.get(4 , fpvalue);
	Serial.print("mlPerFlowMeterPulse");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();

	EEPROM.get(8, fpvalue);
	Serial.print("unionThreshold");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();

	EEPROM.get(12, fpvalue);
	Serial.print("Kpp");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();	  

	EEPROM.get(16, fpvalue);
	Serial.print("Kpi");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();

	EEPROM.get(20, fpvalue);
	Serial.print("Kpd");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();	 

	EEPROM.get(24, fpvalue);
	Serial.print("Kfp");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();	  

	EEPROM.get(28, fpvalue);
	Serial.print("Kfi");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();

	EEPROM.get(32, fpvalue);
	Serial.print("Kfd");
	Serial.print("\t");
	Serial.print(fpvalue, 2);
	Serial.println();	 

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

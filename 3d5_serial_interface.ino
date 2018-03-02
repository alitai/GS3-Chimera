//************************************************************************************
//
// These functions receive the buttons pressed on the GS/3 so we can assign different  
// pull modes to the machine's buttons.
//
// This works with the Gicar Dosatori 3d5 Deluxe 2ptk Bi-Power board I have running 
// 1.16 version firmware. It is not guaranteed to work with another card, or one that 
// uses a different version of firmware.    
//
// Pinout of CN8: 
// Pin 1 GND 
// Pin 2 Rx
// Pin 3 Tx
// Pin 4 16V ----------	BE EXTRA CAREFUL - THE VOLTAGE ON PINS 2 & 3 is 5V TTL LEVEL; 
// 						EXCEEDING 5V MAY DESTROY THE 3D5
// 
// A short is affixed between Pin 2 and Pin 3 otherwise the buttons pushed will be ignored.
// 
// 3d5 Port Settings:
// 1200 baud
// Even Parity
// 8 bit
// 2 stop bits
//
// 3d5 Port sends out the following:
// Pin number on CN2 (1GR)	Button Label	RS232 Hex Code	Pin number of CN4 (2GR)
// 		2					One Shot				11h			1
// 		4					Two Shot				12h			2
// 		6					One Mug					13h			3
// 		8					Two Mug					14h			4
// 		10					Function				19h			5
// 		16					Power/Tea				15h			8
//
//************************************************************************************* 

void serialControl() 
{
	if (!Serial2.available()) return;
	
	int incomingByte = Serial2.read(); // read the incoming byte
		
	switch(incomingByte)
	{
		case 0x11: 	// This is the 1 shot button
			g_pullMode = SLAYER_LIKE_PULL;
			g_currentMenu = 3;
			g_flushCycle = false;
			g_modeSwitchIncomplete = true;        // This variable ensures the mode is fully initialized even if the interrupt comes quickly! 
			break;
	    case 0x12:  // This is the 2 shot button
			g_pullMode = SLAYER_LIKE_PI_PRESSURE_PROFILE;//AUTO_PRESSURE_PROFILE_PULL;
			g_currentMenu = 1;
			g_flushCycle = false;
			g_modeSwitchIncomplete = true; 
			break;
		case 0x13: // This is the 1 mug button
			g_pullMode = AUTO_FLOW_PROFILE_PULL;
			g_currentMenu = 2;
			g_flushCycle = false;
			g_modeSwitchIncomplete = true; 
			break;
		case 0x14: // This is the 2 mug button
			g_pullMode = MANUAL_PULL;
			g_currentMenu = 0;
			g_flushCycle = false;
			g_modeSwitchIncomplete = true; 
			break;
		case 0x19: // This is the Fn button
			g_flushCycle = true;
			break;
		default:
			break;
	}		
	
	Serial2.write(incomingByte); //This line authorizes the 3d5 to start the pull; we now wait for group solenoid interrupt....
	sleepTimer = sleepTimerReset(); 
}


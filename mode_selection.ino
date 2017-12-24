//********************************************************************
// Menus & mode switching
//
// Can force a selection by passing a keySelected during function call; 0 for polling the touch screen
// Display selected g_pullMode and Menus (truth table)
//    softKey2 Condition                  softKey3  Condition
//  0 Manual  (g_pullMode == MANUAL_PULL)         Auto UP   (g_pullMode == AUTO_UNION_PROFILE_PULL)
//  1 Auto PWM(g_pullMode == AUTO_PWM_PROFILE_PULL)     Auto PP   (g_pullMode == AUTO_PRESSURE_PROFILE_PULL)
//  2 Auto FP (g_pullMode == AUTO_FLOW_PROFILE_PULL)    
// 	3 Slayer (g_pullMode == SLAYER_LIKE_PULL) 			Modify/Next/Commit
//  4 Load  N/A                     Save    N/A
//  5 Normal  (!g_debugDisplay)             Debug   (g_debugDisplay)
//  6   Next                        Commit

//  Proposed Layout...
//  0 Man PWM  	(g_pullMode == MANUAL_PWM_PULL)    		Auto PWM   	(g_pullMode == AUTO_PWM_PROFILE_PULL)
//  1 Man PP   	(g_pullMode == MANUAL_PRESSURE_PULL)  	Auto PP   	(g_pullMode == AUTO_PRESSURE_PROFILE_PULL)
//  2 Man FP 	(g_pullMode == MANUAL_FLOW_PULL)    	Auto FP  	(g_pullMode == AUTO_FLOW_PROFILE_PULL)
// 	3 Slayer 	(g_pullMode == SLAYER_LIKE_PULL) 		Modify/Next/Commit
//  4 Load  N/A                     Save    N/A
//  5 Normal  (!g_debugDisplay)             Debug   (g_debugDisplay)
//  6   Next                        Commit


//
//*******************************************************************

void pullModeSwitching(byte softKey)
{
		
	if (g_selectedParameter != 0)    
		if (g_currentMenu == 3 )
			menuSetupSlayerParameters();
		else if (g_currentMenu == 6)
			menuSetupSystemParameters();
		else g_selectedParameter = 0;
		
	if(softKey == 99)    //return code of "99" means no valid input - quit
		return;
	
	menuDrawNavArrows(); // draw the arrows

	switch(g_currentMenu)
	{
		case 0:
			if (softKey == 2)
				g_pullMode = MANUAL_PULL;
			else if  (softKey == 3)
				g_pullMode = AUTO_UNION_PROFILE_PULL;
			menuDrawSoftKeys("Manual", (g_pullMode == MANUAL_PULL), "Auto-UP", (g_pullMode == AUTO_UNION_PROFILE_PULL));
			break;

		case 1:
			if (softKey == 2)
				g_pullMode = AUTO_PWM_PROFILE_PULL;
			else if (softKey == 3)
				g_pullMode = AUTO_PRESSURE_PROFILE_PULL;
			menuDrawSoftKeys("AutoPWM", (g_pullMode == AUTO_PWM_PROFILE_PULL), "Auto-PP", (g_pullMode == AUTO_PRESSURE_PROFILE_PULL));
			break;

		case 2:
			if (softKey == 1)// Clear Slayer Parameter editing display and go back to standard operation mode.....
			{
				dashboardSetup();
				graphDrawCurrentProfiles();
			}
			if (softKey == 2)
				g_pullMode = AUTO_FLOW_PROFILE_PULL;
			else if (softKey == 3)
			{}
			menuDrawSoftKeys("Auto-FP", (g_pullMode == AUTO_FLOW_PROFILE_PULL), "N/A", false);
			break;

		case 3:
			if (softKey == 4 || softKey == 1) // Enter setupParameter mode...
			{
				g_selectedParameter = 0;
				graphDrawSlayerProfile();
			}
			else if (softKey == 2)
			{
				g_pullMode = SLAYER_LIKE_PULL;
			}
			else if (softKey == 3)
			// Switch between 3 parameters: slayerPIFlowRate; slayerPIPeriod; slayerMainPWM
			{
				g_selectedParameter++;
				if (g_selectedParameter > 2)
				{
					g_selectedParameter = 0;
					writeSlayerParameterstoEEPROM();
				}
				g_lastParameterPotValue = analogRead(CONTROL_POT);
				menuSetupSlayerParameters();
			}
			if (g_selectedParameter == 0)
				menuDrawSoftKeys("Slayer", (g_pullMode == SLAYER_LIKE_PULL), "Modify",false);
			else if (g_selectedParameter < 2)
				menuDrawSoftKeys("Slayer", (g_pullMode == SLAYER_LIKE_PULL), "Next",false);
			else if (g_selectedParameter = 2)
				menuDrawSoftKeys("Slayer", (g_pullMode == SLAYER_LIKE_PULL), "Commit",false);
			break;

		case 4:
			if (softKey == 4)// Clear setupParameter editing display and go back to standard operation mode.....
			{
				dashboardSetup();
				graphDrawCurrentProfiles();
			}
			if (softKey == 2)
			{
				readProfilesfromEEPROM();
				graphDrawCurrentProfiles();
				menuDrawSoftKeys("Loaded", true, "Save", false);
			}
			else if (softKey == 3)  
			{
				storeProfilesinEEPROM();
				menuDrawSoftKeys("Load", false, "Saved", true);
			}
			else
				menuDrawSoftKeys("Load", false, "Save", false);
			break;

		case 5:
			if (softKey == 1) // Clear setupParameter editing display and go back to standard operation mode.....
			{
				tft.fillRect(0, 0, 240, 140, ILI9341_BLACK);
				dashboardSetup();
			}
			if (softKey == 2)
			{
				g_debugDisplay = false;
				dashboardSetup();
			}
			else if (softKey == 3)
			{
				g_debugDisplay = true;
				tft.fillRect(0, 0, 240, 140, ILI9341_BLACK);
			}
			menuDrawSoftKeys("Normal", !g_debugDisplay, "Debug", g_debugDisplay);
			break;

		case 6:
			if (softKey == 4) // Enter setupParameter mode...
			{
				tft.fillRect(0, 0, 240, 140, ILI9341_BLACK);
				g_selectedParameter = 0;
				menuSetupSystemParameters();
			}
			else if (softKey == 2)
			{
				if (g_selectedParameter > 11)
					g_selectedParameter = 0;
				g_selectedParameter++;
				g_lastParameterPotValue = analogRead(CONTROL_POT);
				menuSetupSystemParameters();
				menuDrawSoftKeys("Next", false, "Commit",false);
			}
			else if (softKey == 3)
			{
				writeSWParameterstoEEPROM();
				g_selectedParameter = 0;
				menuSetupSystemParameters();
				menuDrawSoftKeys("Select", false, "Saved",true);
			}
			//else
				menuDrawSoftKeys("Select", false, "Commit",false);
			break;
	}
	g_modeSwitchIncomplete = false;
}

void fastModeSwitch()
{	
	switch(g_pullMode)
	{
		case SLAYER_LIKE_PULL:
			g_currentMenu = 3;
			g_cleanCycle = false;
			g_modeSwitchIncomplete = true;
			break;
	
		case AUTO_PRESSURE_PROFILE_PULL:
			g_currentMenu = 1;
			g_cleanCycle = false;
			g_modeSwitchIncomplete = true;
			break;
			
		case AUTO_FLOW_PROFILE_PULL:
			g_currentMenu = 2;
			g_cleanCycle = false;
			g_modeSwitchIncomplete = true; 
			break;
	
		case MANUAL_PULL:
			g_currentMenu = 0;
			g_cleanCycle = false;
			g_modeSwitchIncomplete = true; 
			break;

		case AUTO_UNION_PROFILE_PULL:	
		default:
			break;
	}
}

void gotoSleep()
{
	tft.fillScreen(ILI9341_BLACK);
	tft.drawRect(0,0,240,320, ILI9341_GREENYELLOW); // 240x320
	tft.drawRect(1,1,238,318, ILI9341_GREENYELLOW);
	tft.drawRect(2,2,236,316, ILI9341_YELLOW);
	tft.drawRect(3,3,234,314, ILI9341_YELLOW);
	printSomething("GS/3 - Mod Chimera",36, 110, ILI9341_YELLOW, &FreeSans12pt7b, false );
	printSomething("Touch screen or", 54, 180, ILI9341_DARKGREY, &FreeSans9pt7b, false );
	printSomething("GS/3 button to wake",41, 210, ILI9341_DARKGREY, &FreeSans9pt7b, false );

#ifdef ACAIA_LUNAR_INTEGRATION	
	disconnectScale();
#endif
	while (!ts.touched()) //wait until ts touched or 3d5 button pressed...
	{
#ifdef SERIAL_CONTROL
		if (Serial2.available()) //wait until 3d5 button pressed...
		{
			byte serialCommand = Serial2.read();                  //flush whatever button was pressed...
			if(serialCommand == 0x15 || serialCommand == 0x19)    //unless it is hot water - which should be dispensed immediately...
				Serial2.write(serialCommand);                     // If just water no need to wake up...
			else
				break;                                             // Wake up...
		}
#endif
	}

#ifdef ACAIA_LUNAR_INTEGRATION
	connectScale();	
#endif
	initializeDisplay();	
}

double sleepTimerReset()
{
 sleepTimer = millis() + (SLEEP_TIMER_MINUTES * 60000);
}

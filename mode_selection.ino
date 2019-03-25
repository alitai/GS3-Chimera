//********************************************************************
// Menus & mode switching
//
// Can force a selection by passing a keySelected during function call; 0 for polling the touch screen
// Display selected g_pullMode and Menus (truth table)
//    softKey2 Condition                  softKey3  Condition
//  0 Manual  (g_pullMode == MANUAL_PULL)         Auto UP   (g_pullMode == FLOW_PRESSURE_PROFILE)
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
				g_pullMode = FLOW_PRESSURE_PROFILE;
			menuDrawSoftKeys("Manual", (g_pullMode == MANUAL_PULL), "Auto-UP", (g_pullMode == FLOW_PRESSURE_PROFILE));
			break;

		case 1:
			if (softKey == 2)
				g_pullMode = AUTO_PWM_PROFILE_PULL;
			else if (softKey == 3)
				g_pullMode = SLAYER_LIKE_PI_PRESSURE_PROFILE; // AUTO_PRESSURE_PROFILE_PULL;
			menuDrawSoftKeys("AutoPWM", (g_pullMode == AUTO_PWM_PROFILE_PULL), "Auto-PP", (g_pullMode == SLAYER_LIKE_PI_PRESSURE_PROFILE));//AUTO_PRESSURE_PROFILE_PULL));
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
/*#ifdef OTTO_HTWF 
			// For Otto Controls HTWF-1A12A22A Hall Effect 0-5V paddle control
				g_lastParameterPotValue = constrain(map(analogRead(CONTROL_POT), 100, 930, 0, 1024), 0, 1024);//FLBThresholdPWM), pumpMinPWM, pumpMaxPWM);//FLBThresholdPWM + 150);
			
#else*/
				g_lastParameterPotValue = measurePotValue();
//#endif
				
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
	}
	g_modeSwitchIncomplete = false;
}



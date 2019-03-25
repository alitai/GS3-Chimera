/*
	GS/3 "Chimera"  
	Electronic "Paddle" and profile replay control application for La Marzocco GS/3 AV
	
	This software and associated Arduino based hardware converts a GS/3 into a powerful pressure 
	profiling machine. Furthermore it can mimic (hence "Chimera") all sorts of machines, such as 
	Slayer like slow preinfusion and abrupt pressure change, etc.
   
    New versions can be downloaded from https://github.com/alitai/GS3-Transformer
   
	Modes supported:
    	Manual (virtual) Paddle - based on potentiometer
		Flow profile replay
		Pump voltage (PWM) profile replay
 	 	Pressure profile replay
 		Union Profile (FP preinfusion and PP handoff)
 	
  	Chimera modes: 
		Slayer Pull 
		Strada EP Pull 
		Strada MP Pull
		
	Technical capabilities:
		Can connect to 3d5 for pass through control (using a serial connection)
		Can identify cleaning cycles (if using serial connection)
		Profiles & system parameters stored on EEPROM
		Extensive menu system
		Potentiometer for paddle control; also used to modify parameters
		Full graphing of all parameter data and pull data
		Dashboard shows instant by instant parameters such as flow, pressure and motor current
		FIFO based smoothing
		Interrupt based start of pull
		Time based calculation of flow for enhanced resolution and accuracy
				
	Hardware:
		Runs on an Arduino mega 2560 with a prototype shield 
		Display from Adafruit with a touch panel, 
		Pololu VNH5019 powerful motor driver breakout PCB
		Pressure sensor + amplification
		Flowmeter connection (with optocoupler isolation)
		3-way connection
		Optional connection to the 3d5 using a serial cable. 
			See 3d5_serial_interface.ino for details.
		Medium orifice metering valve
		MG204 gear pump from FOT
		Fittings
		Red & Green Status LEDs
		
	v0.27
		Added Nicolas' support for HM10 BT breakout board and Acaia Lunar scale
		Added weight, dose and EBF support
		
	v0.30
		Removed Motor Current
		Condensed Dashboard view
		Enlarged graph view
		
	v0.33
		Added support for Otto Controls HTWF Hall Effect control paddle with center indent aligned to FLB threshold (PI until center indent, main pull beyond it)
	
	v0.34
		Added color variable support
		Corrected some Acaia Lunar support enable/disable issues
		Added support for ITEAD and other non Adafruit displays (larger, but the displays aren't of similar quality)
		Corrected Flush behavior
		
	v0.35
		Corrected some bad pumpPWM casting
		
	v0.36
		Added Combo mode 7: Slayer like PI and subsequent Pressure Profile... SLAYER_LIKE_PI_PRESSURE_PROFILE
		
	v0.37
		configuration file
		
	v0.38
		Refactored shutdown sequences
		Refactored flush sequences
		Added optional MQTT support
		Removed non-Serial support (should be replaced with defaults)
		Moved parameter manipulation to Serial port & ESP8266 Telnet port
		Removed parameter manipulation from UI
		Removed debug code
		
	v0.39
		Added SINGLE_PUMP mode (per TH)
		Store PWM as Int (no more byte casting)
		
	v0.40
		Added telemetry (Megunolink)
		
	v0.41
		Added FLOW_PRESSURE_PROFILE mode - FP: PID Flow (left side of paddle) up to unionThreshold. Then PP PID Pressure (right side of paddle)
		
	v0.42
		Bugfixes and refactoring

	v0.43
		Corrected a situation where pull would stall and the ml/min would remain stuck
		
	v0.44
		Added a directive to enable/disable sleep AUTO_SLEEP (silly feature removed)
		
	v0.50
		Added MAX_PROFILE_INDEX
		Set timer timeout to MAX_PROFILE_INDEX
		Set profiles to length of MAX_PROFILE_INDEX+1
		Fixed many castings
		Added "linearizing" overlap for FLB on Manual and OTTO control & FLB dead zone (to prevent solenoid rattling)
		Removed sleep timer
		Serial console now added a Z command to dump the EEPROM profiles
		Auto start for Acaia timer
		
		
*/
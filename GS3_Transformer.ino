#define CURRENT_VERSION "V0.33"

/*
	GS/3 "Chimera"  
	Electronic "Paddle" and profile replay control application for La Marzocco GS/3 AV
	
	https://github.com/alitai/GS3-Transformer
 
	This software and associated Arduino based hardware converts a GS/3 into a powerful pressure 
	profiling machine. Furthermore it can mimic (hence "Chimera") all sorts of machines, such as 
	Slayer like slow preinfusion and abrupt pressure change, etc.
   
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
		
*/
  
// For debugging, this sends the entire EEPROM content to the serial monitor
//#define EEPROM_SERIAL_DOWNLOAD

// If the output relay inverts (like some modules do) uncomment the next line
#define INVERT_FLB_OUTPUT

// If using Otto Controls HTWF-1A12A22A Hall Effect 0-5V paddle control instead of potentiometer (same connections)
// Aligns center indent of paddle with FLB control and Preinfusion
#define OTTO_HTWF

// Allows control of the unit via CN8 on the 3d5 
#define SERIAL_CONTROL 

// Allows measuring of weight using a Lunar
// From https://github.com/npouvesle/ACAIAScale_Arduino
#define ACAIA_LUNAR_INTEGRATION
#define DEFAULT_DOSE_FOR_EBF 18.0
#define EMPTY_PORTAFILTER_WEIGHT 537.1

// In device_HM10.h please set the proper scale MAC address as below
// const char * mac = "001C971267DF"; 
// #define ACAIA_SCALE_MAC "001C971267DF"

// Sleep timer puts the GUI (but not the Arduino or peripherals to sleep (in minutes)
#define SLEEP_TIMER_MINUTES 5  


//*******************************************************************************
// Calibrate boiler pressure readings 
//********************************************************************************
// The CALIBRATE_PRESSURE directive displays the 10-bit reading on the analog pressure input pin. 
// Assuming the pressure sensor is sufficiently linear, we will need two points to calibrate the 
// readings (to pressure in bars): 
// A low range pressure input and its associated reading, and a high pressure source and its 
// associated reading.
// So for calibration of the pressure measurement loop enable the line below. 
// Use manual pull mode and a blind basket to:
// 1. Set a constant 3 bar, and write down the displayed A/D value (between 0 to 1023) 
// 2. Do the same for 9 bar and write down the displayed value.
//
// Notes: 
// 1. Some espresso machine manometers are not very accurate. A Scace device or a Fluke 
// pressure gauge are usually more accurate.
// 2. Wait for the pressure to stabilize. if pressure exceeds the intended pressure it may 
// not be possible to reduce the pressure so if that happens run a new pull. 
// 3. It is recommended to run the test a few times to ensure consistency of the results.
// 4. Pressure sensors (like all resistive bridge based transducers) have a high tempco. Some, but not all, 
// are accurately compensated for temperature. Recommended to wait for the machine to heat up and for the
// sensor temperature to stabilize before calibration.
//
//#define CALIBRATE_PRESSURE

// Write the low end calibration figure below
//#define LOW_CALIBRATION_PRESSURE_READING 287 // 10-bit AD reading when low (3.0 bar) pressure is applied (between 0-1023)
#define LOW_CALIBRATION_PRESSURE_READING 320 // 10-bit AD reading when low (3.0 bar) pressure is applied (between 0-1023)

// Write the high end Calibration figure below
//#define HIGH_CALIBRATION_PRESSURE_READING 791 // 10-bit AD reading when high (9.0 bar) pressure is applied (between 0-1023)
#define HIGH_CALIBRATION_PRESSURE_READING 891 // 10-bit AD reading when high (9.0 bar) pressure is applied (between 0-1023)

// If your calibration point are different than 3.0 and 9.0 bar - set the relevant pressures below
// Note: This setting is provided for sensors that are linear within a narrower pressure range - so calibration
// can be performed within the linear range of the transducer; or if your reference transducer supports different 
// reading points (e.g. a maximum of 8 bar).
#define LOW_CALIBRATION_PRESSURE 30 // x10 in bar - so for 3.0 bar write 30
#define HIGH_CALIBRATION_PRESSURE 90 // x10 in bar - so for 9.0 bar write 90

#include <Average.h> 
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Adafruit_STMPE610.h"
#include "VNH5019MotorShieldMega.h"
#include <PID_v1_GS3_EP.h>
#include <EEPROM.h>

//***********************************************************************
// Global Parameters and Variables & Arduino Mega R3 Pins
//***********************************************************************
// Pins
#define CS1 A0 // A0 - VNH5019 Motor driver current default
#define CONTROL_POT A1 // A1 - Control potentiometer is on analog pin 1
#define PRESSURE_SENSOR_INPUT A2 // A2 - 0-5V Pressure Input
// D0 and D1 are serial port
#define FLOW_COUNT_INPUT 2 // D2 - INT0 Flow counter interrupt pin
#define GROUP_SOLENOID 3 // D3 - INT1 Group solenoid 220v detector is on pin 3 (an interrupt pin)
#define INB1 4 // D4 - VNH5019 Motor driver default
#define RED_LED 5 // D5 - LED lights RED during standby
#define EN1DIAG1 6 // D6 - VNH5019 Motor driver default
#define GREEN_LED 7 // D7 - LED lights GREEN during a pull
#define STMPE_CS 8 //  D8 - Adafruit TS CS
#define TFT_DC 9 // D9 - Adafruit TFT DC
#define TFT_CS 10 // D10 - Adafruit TFT CS
//  _PWM1 = 11      // D11 - VNH5019 Motor driver default set in VNH5019MotorShieldMega.h file
#define INA1 12  // VNH5019 Motor driver - Move INA1 from D2 to D12 to allow pin 2 (INT0) use as an interrupt for the flow meter
#define FLOW_LIMIT_BYPASS 13 // D13 - Digital output to flow control bypass solenoid (0V - flow limited; 5V - bypass enabled)

// Touchpad driver
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
// Graphics driver
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// Motor PWM controller is an automotive VNH5019 bridge. 20kHz PWM freauency is used as below. Please note the libray was modified to support  
// a single VNH5019 chip (instead of dusal chips)
// https://forum.pololu.com/t/modified-vnh5019-shield-library-for-20khz-pwm-with-mega/5178/2
VNH5019MotorShieldMega md = VNH5019MotorShieldMega(INA1, INB1, EN1DIAG1, CS1);

// Defaults for system paramters
// NOTE: These are default parameters that are only used for new Arduinos (in which case they get written into the EEPROM as defaults). 
// Once the system is in operation they will get overwritten by readSWParametersfromEEPROM() 
// 
// Pump PWM (min, max, slayer, FLB Threshold): 0-255 correlates to 0-100% PWM or 0-400 on the motor driver library
// Slayer PI Flow rate ml/minute (for display only!)
// Slayer PI Period in seconds

byte FLBThreshold = 15, pumpMaxPWM = 210, pumpMinPWM = 0, pumpMaxPercent = 45, debounceCount = 3, slayerPIFlowRate = 5, slayerMainPWM = 28, slayerPIPeriod = 15, slayerMaxPWM = 45; 
float mlPerFlowMeterPulse = 0.48f; // 0.42f;// ml/pulse
float mlPerFlowMeterPulsePreInfusion = 0.34f; // 0.42f;// ml/pulse
double unionThreshold = 3.0d; // in bar - at this point the system will switch from FP to PP
double Kpp = 5, Kpi = 5, Kpd = 1, Kfp = 5, Kfi = 5, Kfd = 1; 

// Define 2 PID loops
double g_PIDSetpoint_F, g_PIDInput_F, g_PIDOutput_F, g_PIDInput_P, g_PIDOutput_P, g_PIDSetpoint_P;
PID pressurePID(&g_PIDInput_P, &g_PIDOutput_P, &g_PIDSetpoint_P,Kpp,Kpi,Kpd, DIRECT);
PID flowPID(&g_PIDInput_F, &g_PIDOutput_F, &g_PIDSetpoint_F,Kfp,Kfi,Kfd, DIRECT);

// The following system parameters are constant and will not change
const unsigned PIDSampleTime = 100; // in mSec
const byte cleanPWM = 42; // %PWM for cleaning cycle
// Removed FP and PP can correct up to y percent lower than profile PWM (double)
//#define PWM_TRACK_UPPER_BOUND 15.0 // FP and PP can correct up to x percent higher than profile PWM (double)
//#define PWM_TRACK_LOWER_BOUND 90.0 // FP and PP can correct up to y percent lower than profile PWM (double)

// Initialize FIFO averages for Pressure and Flow Rate 
Average<float> g_averageP(6);
Average<unsigned long> g_averageF(4); // Pulse rate can be as low as 4-5 per scond. So we select 4 to ensure that the update rates are reasonable.

// Current pull profiles - 200 byte arrays of measurements - each bin is 500mSec
// PWM Voltage Profile (as sent to the VNH5019 is kept in a 200 byte array)
// Brew boiler pressure log is the average pressure by the end of the 500mSec period
// Flow log - each bin has the number of pulses received by the end of the 500mSec period
byte g_PWMProfile[201] , g_pressureProfile[201], g_flowProfile[201];  
byte g_first4Flow[4], g_first4Pressure[4], g_first4PWM[4];  //Array to save first 4 measured entries for debugDisplay

unsigned long g_lastMillis = 0;
					   
#define PRINT_SPACE tft.print(" ") // for convenience

// Menu system, mode selection and state machine variables  
// Available Pull Modes:
#define MANUAL_PULL 0
#define AUTO_PWM_PROFILE_PULL 1
#define AUTO_PRESSURE_PROFILE_PULL 2
#define AUTO_FLOW_PROFILE_PULL 3
#define AUTO_UNION_PROFILE_PULL 4            // Union Profile does FP and switches to PP
#define SLAYER_LIKE_PULL 5
#define LEVER_LIKE_PULL 6
unsigned char g_pullMode; 
int g_currentMenu, g_selectedParameter = 0, g_lastParameterPotValue;
boolean g_debugDisplay, g_modeSwitchIncomplete = false; 
#ifdef SERIAL_CONTROL
boolean g_flushCycle = false, g_cleanCycle = true;
#else
boolean g_flushCycle = false, g_cleanCycle = false;
#endif
volatile boolean g_activePull = false, g_newPull; // Used by interrupts
volatile unsigned long g_currentFlowPulseMillis, g_flowPulseCount, g_flowPulseCountPreInfusion; // Used by interrupts
#define UNION_PRESSURE 30

double sleepTimer;
// Settings for Acaia Scale
//
//
#ifdef ACAIA_LUNAR_INTEGRATION
#include "Scale.h"
Scale *scale = NULL;
unsigned long startTime;
int state = 0;
bool isRunning = false;
float scaleWeight;
float lastScaleWeight;
unsigned char scaleBattery;
float currentDose;
float lastCurrentDose;
boolean scaleConnected = false;
unsigned long scaleReconnectionTimer;
unsigned long scaleIdleTimer; 
#endif


// Colors:

//  COLOR LIST:
//  ILI9341_BLACK     ILI9341_OLIVE       ILI9341_RED
//  ILI9341_NAVY      ILI9341_LIGHTGREY     ILI9341_MAGENTA  
//  ILI9341_DARKGREEN   ILI9341_DARKGREY      ILI9341_YELLOW  
//  ILI9341_DARKCYAN    ILI9341_BLUE        ILI9341_WHITE
//  ILI9341_MAROON      ILI9341_GREEN       ILI9341_ORANGE
//  ILI9341_PURPLE      ILI9341_CYAN        ILI9341_GREENYELLOW 
//  ILI9341_PINK                
//*********************************************************************
char* PWM_Color = ILI9341_RED;
char* PWM_BGColor = ILI9341_MAROON;
char* pressure_Color = ILI9341_CYAN;
char* pressure_BGColor = ILI9341_DARKCYAN;
char* flow_Color = ILI9341_GREEN;
char* flow_BGColor = ILI9341_DARKGREEN;
char* flowRate_Color = ILI9341_GREENYELLOW;
char* weight_Color = ILI9341_ORANGE;
char* timer_Color = ILI9341_YELLOW;




//********************************************************************
// Motor Error
//********************************************************************
void stopIfFault()
{
	if (md.getM1Fault())
	{
		Serial.println("M1 fault");
		while (1);
	}
}

//*************************************************************************
// Interrupt handlers 
// 1. When Solenoid is activated (ignore spurious triggers if pull is active)
// 2. Flowmeter pulse counter
//*************************************************************************
void pullEspresso()  // Checks if group solenoid is powered up and if so triggers a new pull
{
	if (!g_activePull)
		g_newPull = true; // interrupt effective only if not actively pulling a shot!
}  

void flowPulseReceived(boolean preInfusion) // receives flow pulses from the Gicar flow sensor
{
	g_flowPulseCount++;
	if (preInfusion) 
		g_flowPulseCountPreInfusion++; //
	g_currentFlowPulseMillis = millis(); 
}

//********************************************************************
// Setup...
//********************************************************************
void setup() 
{
	pinMode(GROUP_SOLENOID, INPUT_PULLUP);
	pinMode(FLOW_COUNT_INPUT, INPUT_PULLUP);
	pinMode(GREEN_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	digitalWrite(RED_LED, HIGH);
	
	initFlowLimitBypass();

	// Initialize hardware
	md.init(); // Initialize VNH5019 motor driver
	tft.begin(); // Initialize Adafruit ILI9341 based 2.8" display
	ts.begin(); // Initialize Adafruit STMPE610 based resistive touchscreen

	//Read parameters and saved profiles from EEPROM. Initialize EEPROM if new...
	if (EEPROM.read(99) == 201 && EEPROM.read(98) == 201 && EEPROM.read(97) == 201) //EEPROM has valid information
	{
		readSWParametersfromEEPROM();
		readProfilesfromEEPROM();
		readSlayerParametersfromEEPROM();
	}
	else // initialize EEPROM (if Arduino is new)
	{
		writeSWParameterstoEEPROM();
		writeSlayerParameterstoEEPROM();
	}
	
	// Create default dashboard, graph and menus 
	initializeDisplay();
	
#ifdef SERIAL_CONTROL
	Serial2.begin(1200, SERIAL_8E2);	// opens serial port, sets data rate to 1200 bps, 8 bits even parity, 2 stop bits
#endif

#ifdef EEPROM_SERIAL_DOWNLOAD
	readEEPROMtoSerial();
#endif

#ifdef ACAIA_LUNAR_INTEGRATION
	connectScale();
#endif
}

//********************************************************************
// Main Loop
//********************************************************************
void loop(void) 
{
	int profileIndex, lastProfileIndex;
	byte countOffCycles;
	long lastFlowPulseCount;
	unsigned long pullStartTime, pullTimer, lastFlowPulseMillis;
	byte pumpSpeedByte;
	int sumFlowProfile;
	int unionSkew; // alignment delta between pressure profile and union threshold point
	boolean preInfusion = false;
#ifdef ACAIA_LUNAR_INTEGRATION		
	while(currentDose != DEFAULT_DOSE_FOR_EBF && scaleWeight > 10 && scaleConnected && !ts.touched()) //Pause next pull until demitasse removed 
		{
			updateWeight();
			measurePressure();
			displayPressureandWeight();
			if (Serial2.available())
			{
				if (Serial2.read() == 0x16)
					Serial2.write(0x16);
				else
					break;
			}
		}
#endif
	currentDose = DEFAULT_DOSE_FOR_EBF;
	lastCurrentDose = 0;
	lastScaleWeight = 0;
	sleepTimerReset();

	//*****************************************************************************
	// Clear interrupt cache and attach interrupt - and be ready for next pull
    //*****************************************************************************	
	//EIFR = 2; // For Arduino Uno 
	EIFR = _BV (INTF5); // Clear outstanding interrupts 	
    attachInterrupt(digitalPinToInterrupt(GROUP_SOLENOID), pullEspresso, FALLING);
	
	//*****************************************************************************
	// When not pulling an espresso poll menu system and wait for Serial commands
	//*****************************************************************************
	while (!g_newPull && !g_activePull)
	{
#ifdef SERIAL_CONTROL
		serialControl();
#endif		
		pullModeSwitching(menuNavigation());
		measurePressure(); // display pressure always (even when not pulling a shot)
// The following code connects (if needed for the first time) and updates the Bluetooth weight scale 		
#ifdef ACAIA_LUNAR_INTEGRATION	
		if (!scaleConnected) 
			manageScaleConnection();
		else
			updateDoseWeight();
		displayBattery();
#endif
		displayPressureandWeight();
	
		if (millis() > sleepTimer)
		{
				gotoSleep();
				return;
		}
	}

	//****************************************************************************
	// Pull an Espresso 
	//***************************************************************************
	if (g_newPull)
	{
		detachInterrupt(digitalPinToInterrupt(GROUP_SOLENOID)); //no triggers until pull ends
		g_newPull = false; // debounce...
		pullStartTime = millis();

// The following code tares the scale for a new pull
#ifdef ACAIA_LUNAR_INTEGRATION		
		if(scaleConnected)	scale->tare();
#endif
    
		// Clear Gicar flowmeter counters and attach interrupt 
		g_flowPulseCount = 0; 
		g_flowPulseCountPreInfusion = 0;
		lastFlowPulseCount = 0;
		EIFR = _BV (INTF4); // clear a cached interrupt (not very important)
		attachInterrupt(digitalPinToInterrupt(FLOW_COUNT_INPUT), flowPulseReceived, FALLING);

		// set initial parameters
		countOffCycles = debounceCount;
		profileIndex = 0;
		lastProfileIndex = 1; // force a first screen update
		pumpSpeedByte = 0;
		sumFlowProfile = 0;

		// if system did not finish prepping a serial pull - do it now!
		if (g_modeSwitchIncomplete)
			pullModeSwitching(0);
		
		// Initialize the different manual and automated pull modes 
		selectPIDbyMode();
		selectandDrawProfilebyMode();
		if (!g_cleanCycle && !g_flushCycle)
			preInfusion = true; // start in preinfusion mode (in Union mode start with flow profiling preinfusion mode)
		else 
			preInfusion = false;
		
		// Odds and ends for union mode... 
		// Calculate the profileIndex in which the profile exceeds the unionThreshold setting
		if (g_pullMode == AUTO_UNION_PROFILE_PULL)
			for (int i = 0; i < 200; i++) // find the profileIndex where the pressure setting exceeds the unionThreshold - it will be used to calculate the time skew
			{
				if (g_pressureProfile[i+2] > g_pressureProfile [i+1] 
					&& g_pressureProfile[i+1] > g_pressureProfile [i] 
					&& g_pressureProfile [i] > UNION_PRESSURE) // Verify it is in a rising pressure curve 
					// && g_pressureProfile [i] > unionThreshold)
					// Assaf: problem: unionthreshold is in PWM so irrelevant for comparison with pressureprofile.
				{
					unionSkew = i ;
					break;
				}
			} 
		
		digitalWrite(RED_LED, LOW);
		digitalWrite(GREEN_LED, HIGH);
		lastFlowPulseMillis = millis();
		g_activePull = true; // Time to rock & roll
	}    
		
	while (g_activePull)
	{
		// Time the pull and calculate the profile index (there are two profile points per second)
		pullTimer = millis() - pullStartTime;
		profileIndex = pullTimer / 500; // index advances in 500mSec steps
		
		// Measure current pressure in boiler and flow rate 
		float currentPressure = measurePressure(); // We need the current pressure for the PID loop and stored for rolling average; and displayed
	    if (g_currentFlowPulseMillis > lastFlowPulseMillis) // If there is a new flow meter pulse send the timing for flow rate calculation
		{	
			g_averageF.push(g_currentFlowPulseMillis - lastFlowPulseMillis);
			lastFlowPulseMillis = g_currentFlowPulseMillis;
		}

// Weight the shot using Bluetooth
#ifdef ACAIA_LUNAR_INTEGRATION		
		updateWeight();
#endif
		displayPressureandWeight();
		
		//Odds & ends for PID loops
		//Flow PID's target is the accumulated number of pulses. However, in profile we store pulses per 500mSec. So we accumulate.
		if (profileIndex != lastProfileIndex)
			sumFlowProfile += g_flowProfile[profileIndex] >> 1;
		// For Union mode, figure out if we need to switch from flow to pressure. Also avoid spurious switches...
		if (g_pullMode == AUTO_UNION_PROFILE_PULL && preInfusion && (currentPressure > unionThreshold) && (profileIndex > 10))
		{
			preInfusion = false;
			unionSkew = profileIndex - unionSkew; //Calculate skew once!
		}	
			
		// calculate pump speed & preinfusion for all pull modes...
		pumpSpeedByte = setPumpSpeedbyMode(profileIndex, pullTimer, pumpSpeedByte, currentPressure, preInfusion, sumFlowProfile, unionSkew);
		preInfusion = setFlowLimitBypass(pumpSpeedByte, profileIndex, preInfusion); // check preinfusion status & set flow limit bypass solenoid and Slayer style preInfusion
		//stopIfFault(); // What to do with this??? Make an error display???
		
		// Update graphs, dashboards and profiles
		if (profileIndex != lastProfileIndex) // display update & document profile every 500mSec
		{
			if (g_pullMode == MANUAL_PULL && !g_cleanCycle && !g_flushCycle) //update profile array if manual && not cleaning...
				updateProfiles(profileIndex, pumpSpeedByte, lastFlowPulseCount);
			
			if (g_debugDisplay && profileIndex < 4)   // For debug display
				updateDebugFirstFour(profileIndex, pumpSpeedByte, lastFlowPulseCount);
	
			// Update dashboard and graph
			dashboardUpdate(pumpSpeedByte, profileIndex, g_averageP.mean(), lastFlowPulseCount, preInfusion);
			graphUpdate(pumpSpeedByte, profileIndex, g_averageP.mean() * 100 / 12.0, g_flowPulseCount, false, preInfusion);
			
			//reset variables for next 500mSec period... 
			lastFlowPulseCount = g_flowPulseCount; 
			lastProfileIndex = profileIndex;
		}
		
		//Checks if pull is over and if so shuts down the solenoids, pumps, and LEDs...
		countOffCycles = shutdownIfPullOver(countOffCycles, lastProfileIndex);
	}
}

#define CURRENT_VERSION "V0.35"

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
		
*/
// Menu system, mode selection and state machine variables  
// Available Pull Modes:
#define MANUAL_PULL 0
#define AUTO_PWM_PROFILE_PULL 1
#define AUTO_PRESSURE_PROFILE_PULL 2
#define AUTO_FLOW_PROFILE_PULL 3
#define AUTO_UNION_PROFILE_PULL 4            // Union Profile does FP and switches to PP
#define SLAYER_LIKE_PULL 5
#define LEVER_LIKE_PULL 6
#define SLAYER_LIKE_PI_PRESSURE_PROFILE 7   // Low flow PI & pressure profiling

// For debugging, this sends the entire EEPROM content to the serial monitor
//#define EEPROM_SERIAL_DOWNLOAD

// Quote the following line to force parameter rewrite from code defaults below into EEPROM
//#define READ_EEPROM_PARAMETERS

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
// The CALIBRATE_PRESSURE directive displays the 10-bit reading on the analog pressure input pin 
// (instead of the 12 bar scale). Assuming the pressure sensor is sufficiently linear, we will need 
// two points to calibrate the readings (to pressure in bars): 
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
#define LOW_CALIBRATION_PRESSURE_READING 290 // 10-bit AD reading when low (3.0 bar) pressure is applied (between 0-1023)
//#define LOW_CALIBRATION_PRESSURE_READING 320 // (blown Mega) 10-bit AD reading when low (3.0 bar) pressure is applied (between 0-1023)

// Write the high end Calibration figure below
#define HIGH_CALIBRATION_PRESSURE_READING 788 // 10-bit AD reading when high (9.0 bar) pressure is applied (between 0-1023)
//#define HIGH_CALIBRATION_PRESSURE_READING 891 // (blown MEGA) 10-bit AD reading when high (9.0 bar) pressure is applied (between 0-1023)

// If your calibration point are different than 3.0 and 9.0 bar - set the relevant pressures below
// Note: This setting is provided for sensors that are linear within a narrower pressure range - so calibration
// can be performed within the linear range of the transducer; or if your reference transducer supports different 
// reading points (e.g. a maximum of 8 bar).
#define LOW_CALIBRATION_PRESSURE 30 // x10 in bar - so for 3.0 bar write 30
#define HIGH_CALIBRATION_PRESSURE 90 // x10 in bar - so for 9.0 bar write 90

//***********************************************************************
// Touchpad driver
//***********************************************************************
#define TFT_TOUCH // A simple XPT2046 library for Arduino https://github.com/Bodmer/TFT_Touch
//#define TS_STMPE //The Adafruit STMPE610 library 

//***********************************************************************
// Graphics Drivers
//***********************************************************************
//#define ADAFRUIT_ILI9341
#define MCUFRIEND //MCU Friend allows 16 bit parallel: https://github.com/prenticedavid/MCUFRIEND_kbv

/*
#define LCD_CS 38 // Chip Select goes to Analog 3
#define LCD_CD 39 // Command/Data goes to Analog 2
#define LCD_WR 40 // LCD Write goes to Analog 1
#define LCD_RD 41 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
*/

#include <Average.h> 
#include "SPI.h"
#include "Adafruit_GFX.h"
#ifdef ADAFRUIT_ILI9341
#include "Adafruit_ILI9341.h"
#endif
#ifdef MCUFRIEND 
#include <MCUFRIEND_kbv.h>
// ADAFRUIT Color definitions for MCU_FRIEND
#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

#endif

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
//  _PWM1 = 11      // D11 - VNH5019 Motor driver default set in VNH5019MotorShieldMega.h file
#define INA1 12  // VNH5019 Motor driver - Move INA1 from D2 to D12 to allow pin 2 (INT0) use as an interrupt for the flow meter
#define FLOW_LIMIT_BYPASS 13 // D13 - Digital output to flow control bypass solenoid (0V - flow limited; 5V - bypass enabled)

#ifdef TS_STMPE
#define TS_CS 8 //  D8 - TouchScreen CS
#endif
#ifdef TFT_TOUCH
// These are the pins used to interface between the 2046 touch controller and Arduino Mega
#define DOUT 44 // 3  /* Data out pin (T_DO) of touch screen */
#define DIN  46 //4  /* Data in pin (T_DIN) of touch screen */
#define DCS  8  /* Chip select pin (T_CS) of touch screen */
#define DCLK 48 //6  /* Clock pin (T_CLK) of touch screen */
#endif

#ifdef ADAFRUIT_ILI9341
#define TFT_DC 9 // D9 - Adafruit TFT DC
#define TFT_CS 10 // D10 - Adafruit TFT CS
#endif

/*
If parallel like ITEAD 3.2" MEGA shield pinout using MCUFRIEND is as follows. Remember to configure MCUFRIEND properly using special.h file.
D2 - T_IRQ   - Disconnected
D3 - T_DOUT  - Modified to pin 44
D4 - T_DIN   - Modified to pin 46
D5 - T_CS    - Modified to pin 8
D6 - T_CLK   - Modified to pin 48
D22 DB8      
D23 DB9 
D24 DB10 
D25 DB11 
D26 DB12 
D27 DB13 
D28 DB14 
D29 DB15 
D30 DB7 
D31 DB6 
D32 DB5 
D33 DB4 
D34 DB3 
D35 DB2 
D36 DB1 
D37 DB0 
D38 RS 
D39 WR 
D40 CS 
D41 RST 
D50 SD_MISO 
D51 SD_MOSI 
D52 SD_SCK 
D53 SD_NSS 
*/

#ifdef TFT_TOUCH
#include <TFT_Touch.h>
// These are the default min and maximum values, set to 0 and 4095 to test the screen
#define HMIN 0
#define HMAX 4095
#define VMIN 0
#define VMAX 4095
#define XYSWAP 0 // 0 or 1

// This is the screen size for the raw to coordinate transformation
// width and height specified for landscape orientation
#define HRES 320 /* Default screen resulution for X axis */
#define VRES 320 /* Default screen resulution for Y axis */

/* Create an instance of the touch screen library */
TFT_Touch ts = TFT_Touch(DCS, DCLK, DIN, DOUT);

#endif

#ifdef TS_STMPE
Adafruit_STMPE610 ts = Adafruit_STMPE610(TS_CS);
#endif

TS_Point p;

// Graphics driver
#ifdef MCUFRIEND
MCUFRIEND_kbv tft;
#endif
#ifdef ADAFRUIT_ILI9341
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#endif

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

byte debounceCount = 3, slayerPIFlowRate = 5, slayerPIPeriod = 15; 

float mlPerFlowMeterPulse = 0.48f; // 0.42f;// ml/pulse
float mlPerFlowMeterPulsePreInfusion = 0.34f; // 0.42f;// ml/pulse
double unionThreshold = 3.0d; // in bar - at this point the system will switch from FP to PP
//double Kpp = 5, Kpi = 5, Kpd = 1, Kfp = 5, Kfi = 5, Kfd = 1; 
double Kpp = 100, Kpi = 10, Kpd = 0, Kfp = 5, Kfi = 5, Kfd = 1; 

// Define 2 PID loops
double g_PIDSetpoint_F, g_PIDInput_F, g_PIDOutput_F, g_PIDInput_P, g_PIDOutput_P, g_PIDSetpoint_P;
PID pressurePID(&g_PIDInput_P, &g_PIDOutput_P, &g_PIDSetpoint_P,Kpp,Kpi,Kpd, DIRECT);
PID flowPID(&g_PIDInput_F, &g_PIDOutput_F, &g_PIDSetpoint_F,Kfp,Kfi,Kfd, DIRECT);

// The following system parameters are constant and will not change
const unsigned PIDSampleTime = 100; // in mSec


// PWM speed is from -400 to 400. Pumps cannot be driven in reverse, so from 0-400. 
//const byte cleanPWM = 42; // %PWM for cleaning cycle
//byte FLBThresholdPWM = 15, pumpMaxPWM = 210, pumpMinPWM = 0, pumpMaxPercent = 50
unsigned FLBThresholdPWM = 60,  pumpMaxPWM = 220, pumpMinPWM = 0, pumpMaxPercent = pumpMaxPWM / 4, slayerMainPWM = 112, slayerMaxPWM = 190;
const unsigned cleanPWM = 168; // 42%PWM for cleaning cycle
// Removed FP and PP can correct up to y percent lower than profile PWM (double)
//#define PWM_TRACK_UPPER_BOUND 15.0 // FP and PP can correct up to x percent higher than profile PWM (double)
//#define PWM_TRACK_LOWER_BOUND 90.0 // FP and PP can correct up to y percent lower than profile PWM (double)

// Initialize FIFO averages for Pressure and Flow Rate 
Average<float> g_averageP(6);
Average<unsigned long> g_averageF(4); // Pulse rate can be as low as 4-5 per second. So we select 4 to ensure that the update rates are reasonable.

// Current pull profiles - 200 byte arrays of measurements - each bin is 500mSec
// PWM Voltage Profile (as sent to the VNH5019 is kept in a 200 byte array)
// Brew boiler pressure log is the average pressure by the end of the 500mSec period
// Flow log - each bin has the number of pulses received by the end of the 500mSec period
byte g_PWMProfile[201] , g_pressureProfile[201], g_flowProfile[201];  
byte g_first4Flow[4], g_first4Pressure[4], g_first4PWM[4];  //Array to save first 4 measured entries for debugDisplay

unsigned long g_lastMillis = 0;
					   
#define PRINT_SPACE tft.print(" ") // for convenience

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
char* text_light_Color = ILI9341_WHITE;
char* text_dark_Color = ILI9341_WHITE;
char* bg_Color = ILI9341_BLACK;
char* axis_minor_Color = ILI9341_LIGHTGREY;
char* axis_major_Color = ILI9341_WHITE;


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
	Serial.begin(38400);
	Serial.println("Chimera started... ");
	
	initFlowLimitBypass();

	// Initialize hardware
	md.init(); // Initialize VNH5019 motor driver
	tft.begin(); // Initialize Adafruit ILI9341 based 2.8" display

#ifdef TS_STMPE	
    ts.begin(); // Initialize Adafruit STMPE610 based resistive touchscreen
#endif

#ifdef TFT_TOUCH
    ts.setCal(HMIN, HMAX, VMIN, VMAX, HRES, VRES, XYSWAP); // Raw xmin, xmax, ymin, ymax, width, height
    ts.setRotation(1);
#endif

#ifdef READ_EEPROM_PARAMETERS
	//Read parameters and saved profiles from EEPROM. Initialize EEPROM if new...
	if (EEPROM.read(99) == 201 && EEPROM.read(98) == 201 && EEPROM.read(97) == 201) //EEPROM has valid information
	{
		readSWParametersfromEEPROM();
		readProfilesfromEEPROM();
		readSlayerParametersfromEEPROM();
	}
	else // initialize EEPROM (if Arduino is new) 
#endif
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
	unsigned pumpPWM;
	int sumFlowProfile;
	int unionSkew; // alignment delta between pressure profile and union threshold point
	boolean preInfusion = false;
#ifdef ACAIA_LUNAR_INTEGRATION		
	while(currentDose != DEFAULT_DOSE_FOR_EBF && scaleWeight > 10 && scaleConnected /*&& !ts.touched()*/ && ts.Pressed()) //Pause next pull until demitasse removed 
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
	currentDose = DEFAULT_DOSE_FOR_EBF;
	lastCurrentDose = 0;
	lastScaleWeight = 0;
#endif
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

    // Flush Cycle
	if (g_newPull && g_flushCycle)
		flushCycle();
	
	//****************************************************************************
	// Pull an Espresso 
	//***************************************************************************
	if (g_newPull && !g_flushCycle)
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
		pumpPWM = 0;
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
		pumpPWM = setpumpPWMbyMode(profileIndex, pullTimer, pumpPWM, currentPressure, preInfusion, sumFlowProfile, unionSkew);
		preInfusion = setFlowLimitBypass(pumpPWM, profileIndex, preInfusion, currentPressure); // check preinfusion status & set flow limit bypass solenoid and Slayer style preInfusion
		//stopIfFault(); // What to do with this??? Make an error display???
		
		// Update graphs, dashboards and profiles
		if (profileIndex != lastProfileIndex) // display update & document profile every 500mSec
		{
			if (g_pullMode == MANUAL_PULL && !g_cleanCycle && !g_flushCycle) //update profile array if manual && not cleaning...
				updateProfiles(profileIndex, pumpPWM, lastFlowPulseCount);
			
			if (g_debugDisplay && profileIndex < 4)   // For debug display
				updateDebugFirstFour(profileIndex, pumpPWM, lastFlowPulseCount);
	
			// Update dashboard and graph
			dashboardUpdate(pumpPWM, profileIndex, g_averageP.mean(), lastFlowPulseCount, preInfusion);
			graphUpdate(pumpPWM, profileIndex, g_averageP.mean() * 100 / 12.0, g_flowPulseCount, false, preInfusion);
			
			//reset variables for next 500mSec period... 
			lastFlowPulseCount = g_flowPulseCount; 
			lastProfileIndex = profileIndex;
		}
		
		//Checks if pull is over and if so shuts down the solenoids, pumps, and LEDs...
		countOffCycles = shutdownIfPullOver(countOffCycles, lastProfileIndex);
	}
}

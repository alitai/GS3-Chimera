//configuration.h

// Menu system, mode selection and state machine variables  
// Available Pull Modes:
#define MANUAL_PULL 0							// Manual PWM set by paddle/pot. FLB OFF until PWM exceeds FLBThresholdPWM value
#define AUTO_PWM_PROFILE_PULL 1					// Not recommended
#define AUTO_PRESSURE_PROFILE_PULL 2			// Not recommended
#define AUTO_FLOW_PROFILE_PULL 3				// Not recommended
#define SLAYER_LIKE_PULL 4						// Single speed pump - low-flow PI by FLB-OFF for set time followed by FLB ON 
#define LEVER_LIKE_PULL 5						// Not currently implemented
#define SLAYER_LIKE_PI_PRESSURE_PROFILE 6   	// Low flow PI (FLB OFF) until pressure reaches 4 bar aand then manual pressure profiling
#define FLOW_PRESSURE_PROFILE 7					// FP: PID Flow (left side of paddle) up to unionThreshold. Then PP PID Pressure (right side of paddle)
#define FLUSH 10

// 3d5 buttons - choose the code above to change the pull modes
#define BUTTON_1 4                               // Leftmost button (single shot)
#define BUTTON_2 6								 // Second button (double shot)
#define BUTTON_3 7								 // Third button (single mug)
#define BUTTON_4 0								 // 4th button (double mug)


// For debugging, this sends the entire EEPROM content to the serial monitor
#define EEPROM_SERIAL_DOWNLOAD

// Define MQTT channel
//#define MQTT

// Megunolink telemetry
// For use over an ESP8266 also needs the com0com com2tcp adapter. See https://hackaday.io/page/1304-virtual-serial-port-tunnel-to-use-with-esp-link.
//#define MEGUNOLINK

// If the output relay inverts (like some modules do) uncomment the next line
#define INVERT_FLB_OUTPUT

// If using Otto                                    Controls HTWF-1A12A22A Hall Effect 0-5V paddle control instead of potentiometer (same connections)
// Aligns center indent of paddle with FLB control and Preinfusion
#define OTTO_HTWF

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

//Flowmeter Selection
//#define GICAR_FLOWMETER
#define DIGMESA_FLOWMETER // #9NB-0100/01A http://www.digmesa.com/wp-content/uploads/9NB-01xx_01_x_GB.pdf
#ifdef GICAR_FLOWMETER
float mlPerFlowMeterPulse = 0.48f; // 0.42f;// ml/pulse
float mlPerFlowMeterPulsePreInfusion = 0.34f; // 0.42f;// ml/pulse
#endif
#ifdef DIGMESA_FLOWMETER
float mlPerFlowMeterPulse = 0.0240f; // Calibrated values.... Spec is 0.025ml/pulse
float mlPerFlowMeterPulsePreInfusion = 0.0230f;
#endif

//#define SINGLE_PUMP // Used if the gear pump is also used for autofill

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

#define MCUFRIEND //MCU Friend allows 16 bit parallel: https://github.com/prenticedavid/MCUFRIEND_kbv
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

//#define ADAFRUIT_ILI9341
/*
#define LCD_CS 38 // Chip Select goes to Analog 3
#define LCD_CD 39 // Command/Data goes to Analog 2
#define LCD_WR 40 // LCD Write goes to Analog 1
#define LCD_RD 41 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
*/



//***********************************************************************
// PINS: Global Parameters and Variables & Arduino Mega R3 Pins
//***********************************************************************
#define CS1 A0 // A0 - VNH5019 Motor driver current default
#define CONTROL_POT A1 // A1 - Control potentiometer is on analog pin 1
#define PRESSURE_SENSOR_INPUT A2 // A2 - 0-5V Pressure Input
// Pins D0 and D1 are serial port
#define FLOW_COUNT_INPUT 2 // D2 - INT0 Flow counter interrupt pin
#define GROUP_SOLENOID 3 // D3 - INT1 Group solenoid 220v detector is on pin 3 (an interrupt pin)
#define INB1 4 // D4 - VNH5019 Motor driver default
#ifndef SINGLE_PUMP
#define RED_LED 5 // D5 - LED lights RED during standby
#endif
#define EN1DIAG1 6 // D6 - VNH5019 Motor driver default
#define GREEN_LED 7 // D7 - LED lights GREEN during a pull
//  _PWM1 = 11      // D11 - VNH5019 Motor driver default set in VNH5019MotorShieldMega.h file
#define INA1 12  // VNH5019 Motor driver - Move INA1 from D2 to D12 to allow pin 2 (INT0) use as an interrupt for the flow meter
#define FLOW_LIMIT_BYPASS 13 // D13 - Digital output to flow control bypass solenoid (0V - flow limited; 5V - bypass enabled)
#define STROBE_RELAY 10 // D10 - Will simulate a 3d5 button push 

#ifdef SINGLE_PUMP
#define PUMP_RELAY 5 // Pump on detection to control filling of tank and tea water
#define RED_LED 22 // D22 - LED lights RED during standby (TH: changed from 5 to 22 to release one pin for pump on detection)
#endif

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


#ifdef TFT_TOUCH
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
#endif

//***********************************************************************
// Defaults for system parameters
// NOTE: These are default parameters that are only used for new Arduinos (in which case they get written into the EEPROM as defaults). 
// Once the system is in operation they will get overwritten by readSWParametersfromEEPROM() 
// To overwrite and force parameter rewrite from code unquote the following line
//***********************************************************************
//#define OVERWRITE_EEPROM_WITH_DEFAULTS

byte debounceCount = 3, slayerPIPeriod = 15, slayerPIFlowRate = 5; 
double unionThreshold = 4.0d; // in bar - at this point the system will switch from FP to PP

//************************************************************************
// PID parameters
//************************************************************************
const unsigned PIDSampleTime = 25; // in mSec

// Pressure PID loop (pressure profiling)
double Kpp = 60, Kpi = 20, Kpd = 3; 
#define PID_MIN_PRESSURE 4.0
#define PID_MAX_PRESSURE 10.0
#define PRESSURE_PID_MIN_PWM 0 
#define PRESSURE_PID_MAX_PWM 220

// Flow PID loop (flow profiling)
double Kfp = 5, Kfi = 5, Kfd = 1;
#define PID_MIN_FLOW 0
#define PID_MAX_FLOW 150  // maximum debit in ml/min while in PI
#define FLOW_PID_MIN_PWM 10
#define FLOW_PID_MAX_PWM 150

//***********************************************************************
// Pump Speeds (in PWM)
// 0-400 correlates to 0-100% PWM 
// PWM speed is from -400 to 400. Pumps cannot be driven in reverse, so from 0-400. 
//***********************************************************************
unsigned FLBThresholdPWM = 60;
unsigned  pumpMaxPWM = 220, pumpMinPWM = 0; 
const unsigned slayerMaxPWM = 190; // Maximum - minimum pump PWM (0-400)
unsigned slayerMainPWM = 160;
const unsigned flushPWM = 168; // 42% PWM for flush and cleaning cycle

// Removed FP and PP can correct up to y percent lower than profile PWM (double)
//#define PWM_TRACK_UPPER_BOUND 15.0 // FP and PP can correct up to x percent higher than profile PWM (double)
//#define PWM_TRACK_LOWER_BOUND 90.0 // FP and PP can correct up to y percent lower than profile PWM (double)
// Lunar MAC address is defined in device_HM10.h file.
// Programming with ESP8266: 
// bash -c "/mnt/d/Users/assaf.HOME/Documents/ESP8266/esp-link-3.2.47.alpha/megaflash.sh  192.168.200.55 /mnt/d/Users/assaf.HOME/Documents/Arduino/GS3_Transformer/GS3_Transformer.ino.mega.hex"

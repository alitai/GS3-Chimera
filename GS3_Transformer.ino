#define CURRENT_VERSION "V0.42"

#include "VNH5019MotorShieldMega.h"
#include "configuration.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#ifdef ADAFRUIT_ILI9341
#include "Adafruit_ILI9341.h"
#endif

#ifdef MCUFRIEND 
#include "MCUFRIEND_kbv.h"
#endif

#include "color_definitions.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Adafruit_STMPE610.h"
#include "PID_v1_GS3_EP.h"
#include "EEPROM.h"
#include "Average.h" 

#ifdef MEGUNOLINK
#include "MegunoLink.h"
TimePlot megunolinkPlot;
#endif

#ifdef TFT_TOUCH
#include <TFT_Touch.h>
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

// Motor PWM controller is an automotive VNH5019 bridge. 20kHz PWM frequency is used as below. Please note the library was modified 
// to support a single VNH5019 chip (instead of a dual chip carrier)
// https://forum.pololu.com/t/modified-vnh5019-shield-library-for-20khz-pwm-with-mega/5178/2
VNH5019MotorShieldMega md = VNH5019MotorShieldMega(INA1, INB1, EN1DIAG1, CS1);

// Define 2 PID loops - one for pressure and one for flow....
double g_PIDSetpoint_F, g_PIDInput_F, g_PIDOutput_F, g_PIDInput_P, g_PIDOutput_P, g_PIDSetpoint_P;
PID pressurePID(&g_PIDInput_P, &g_PIDOutput_P, &g_PIDSetpoint_P,Kpp,Kpi,Kpd, DIRECT);
PID flowPID(&g_PIDInput_F, &g_PIDOutput_F, &g_PIDSetpoint_F,Kfp,Kfi,Kfd, DIRECT);

// Initialize cyclical averages for Pressure and Flow Rate 
Average<float> g_averageP(6);
#ifdef GICAR_FLOWMETER
Average<unsigned long> g_averageF(4); // Pulse rate can be as low as 4-5 per second. So we select 4 to ensure that the update rates are reasonable.
#endif
#ifdef DIGMESA_FLOWMETER
Average<unsigned long> g_averageF(10); // Pulse rate is 20x higher which can also be noisier
#endif

#ifdef MQTT
#include <ELClient.h>
#include <ELClientCmd.h>
#include <ELClientMqtt.h>

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize CMD client (for GetTime)
ELClientCmd cmd(&esp);

// Initialize the MQTT client
ELClientMqtt mqtt(&esp);

// Callback made from esp-link to notify of wifi status changes
// Here we just print something out for grins
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);
    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
    }
  }
}

bool connected;
// Callback when MQTT is connected
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");
  mqtt.subscribe("/esp-link/1");
  mqtt.subscribe("/hello/world/#");
  //mqtt.subscribe("/esp-link/2", 1);
  //mqtt.publish("/esp-link/0", "test1");
  connected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  connected = false;
}

// Callback when an MQTT message arrives for one of our subscriptions
void mqttData(void* response) {
  ELClientResponse *res = (ELClientResponse *)response;
  Serial.print("Received: topic=");
  String topic = res->popString();
  Serial.println(topic);
  Serial.print("data=");
  String data = res->popString();
  Serial.println(data);
}

void mqttPublished(void* response) {
  Serial.println("MQTT published");
}

static int count;
static uint32_t last;
#endif

// Current pull profiles - 200 byte arrays of measurements - each bin is 500mSec
// PWM Voltage Profile (as sent to the VNH5019 is kept in a 200 byte array)
// Brew boiler pressure log is the average pressure by the end of the 500mSec period
// Flow log - each bin has the number of pulses received by the end of the 500mSec period
byte g_PWMProfile[201] , g_pressureProfile[201], g_flowProfile[201];  

unsigned long g_lastMillis = 0;

#define PRINT_SPACE tft.print(" ") // for convenience

unsigned char g_pullMode; 
int g_currentMenu, g_selectedParameter = 0, g_lastParameterPotValue;
boolean g_modeSwitchIncomplete = false; 
boolean g_flushCycle = true, g_activePull; //flush cycled DO NOT imply a Serial port signal. Hence it is default!
volatile boolean g_newPull; // Used by interrupts
volatile unsigned long g_flowPulseMillis, g_flowPulseCount;  // Used by interrupts
unsigned long g_flowPulseCountPreInfusion, g_lastFlowPulseCount;
double sleepTimer;

// Settings for Acaia Scale
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
	g_flowPulseMillis = millis();
}

//********************************************************************
// Setup...
//********************************************************************

void setup() 
{
	// Initialize hardware
	md.init(); // Initialize VNH5019 pump driver
	tft.begin(); // Initialize displays
	tft.setRotation(2);
	
	Serial.begin(115200);
	Serial.println(" ");
	Serial.println("Chimera is starting....");

	pinMode(GROUP_SOLENOID, INPUT_PULLUP);
	pinMode(FLOW_COUNT_INPUT, INPUT_PULLUP);
	pinMode(GREEN_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	pinMode(STROBE_RELAY, OUTPUT);
#ifdef SINGLE_PUMP
	pinMode(PUMP_RELAY, INPUT_PULLUP);
#endif

	ledColor('r');
	Serial2.begin(1200, SERIAL_8E2);	// opens serial port for Gicar 3d5, sets data rate to 1200 bps, 8 bits even parity, 2 stop bits

	initFlowLimitBypass();

// Initialize touch screens
#ifdef TS_STMPE	
    ts.begin(); // Initialize Adafruit STMPE610 based resistive touchscreen
#endif

#ifdef TFT_TOUCH
    ts.setCal(HMIN, HMAX, VMIN, VMAX, HRES, VRES, XYSWAP); // Raw xmin, xmax, ymin, ymax, width, height
    ts.setRotation(1);
#endif

#ifdef OVERWRITE_EEPROM_WITH_DEFAULTS
	// initialize EEPROM (if Arduino is new) 
	writeSWParameterstoEEPROM();
#else 
	//Read parameters and saved profiles from EEPROM. Initialize EEPROM if new...
	readSWParametersfromEEPROM();
	readProfilesfromEEPROM();
#endif
	
	// Create default dashboard, graph and menus 
	initializeDisplay();
#ifdef EEPROM_SERIAL_DOWNLOAD
	readEEPROMtoSerial();
	readProfilestoSerial()
#endif

#ifdef ACAIA_LUNAR_INTEGRATION
	connectScale();
#endif

#ifdef MQTT
	// Sync-up with esp-link, this is required at the start of any sketch and initializes the
	// callbacks to the wifi status change callback. The callback gets called with the initial
	// status right after Sync() below completes.
	esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
	bool ok;
	do 
	{
		ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
		if (!ok) 
			Serial.println("EL-Client sync failed!");
	} 
	while(!ok);
	Serial.println("EL-Client synced!");

	// Set-up callbacks for events and initialize with es-link.
	mqtt.connectedCb.attach(mqttConnected);
	mqtt.disconnectedCb.attach(mqttDisconnected);
	mqtt.publishedCb.attach(mqttPublished);
	mqtt.dataCb.attach(mqttData);
	mqtt.setup();
	Serial.println("EL-MQTT ready");
#endif

	//Megunolink 
#ifdef MEGUNOLINK
	megunolinkPlot.SetTitle("GS/3 Pull Telemetry");
	megunolinkPlot.SetXlabel("Time");
	megunolinkPlot.SetYlabel("Value");
	//megunolinkPlot.SetSeriesProperties("ADCValue", Plot::Magenta, Plot::Solid, 2, Plot::Square);
#endif

	resetSystem(); // reset hardware states and interrupts
}

//********************************************************************
// Main Loop
//********************************************************************

void loop(void) 
{
	unsigned profileIndex, lastProfileIndex;
	byte countOffCycles;
	unsigned long lastFlowPulseCount;
	unsigned long pullStartTime, pullTimer, lastFlowPulseMillis;
	unsigned pumpPWM;
	unsigned sumFlowProfile;
	boolean preInfusion = false;

#ifdef ACAIA_LUNAR_INTEGRATION		
	while(currentDose != DEFAULT_DOSE_FOR_EBF && scaleWeight > 10 && scaleConnected /*&& !ts.touched()*/ && ts.Pressed()) //Pause next pull until demitasse removed 
		{
			Serial.println("B");
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

//****************************************************************************
// Idle - Wait for Serial trigger or Group Solenoid interrupt... 
//***************************************************************************
	while (!g_newPull && !g_activePull)
	{
		serialControl();
		char editNow=Serial.read();
		if (editNow == 'E' || editNow == 'e') 
			editParametersOverSerial(); //ruins the display at this point...
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

#ifdef SINGLE_PUMP		
		 // Check if pump relay by 3D5 is switched on - if yes assume tank fill cycle
		while (digitalRead(PUMP_RELAY) == LOW)
		{
			md.setM1Speed(constrain(flushPWM, pumpMinPWM, pumpMaxPWM));
		}
		md.setM1Speed(0); //Shut down pump motor

#endif
		if (millis() > sleepTimer)
		{
				gotoSleep();
				return;
		}

#ifdef MQTT		
		esp.Process();
#endif

	}

//****************************************************************************
// Flush or clean group
//***************************************************************************	
	if (g_newPull && g_flushCycle && !g_activePull)
	{
		Serial.println("Flushing group....");
		flushCycle();
	}

//****************************************************************************
// Pull an Espresso - Setup
//***************************************************************************
	if (g_newPull && !g_flushCycle && !g_activePull)
	{
		Serial.println("Starting new pull...");
		detachInterrupt(digitalPinToInterrupt(GROUP_SOLENOID)); //no triggers until pull ends
		g_newPull = false; // debounce...
		pullStartTime = millis();

// The following code tares the scale for a new pull
#ifdef ACAIA_LUNAR_INTEGRATION		
		if(scaleConnected)	scale->tare();
#endif

		// Clear flowmeter counters and attach interrupt 
		g_flowPulseCount = 0; 
		g_flowPulseCountPreInfusion = 0;
		lastFlowPulseCount = 0;

		// Attach flowmeter interrupt...
		EIFR = _BV (INTF4); // clear a cached interrupt (not very important)
		attachInterrupt(digitalPinToInterrupt(FLOW_COUNT_INPUT), flowPulseReceived, FALLING);

		// Set initial parameters
		countOffCycles = debounceCount;
		profileIndex = 0;
		lastProfileIndex = 1; // force a first screen update
		pumpPWM = 0;
		sumFlowProfile = 0;

		// Clear existing averages in the stack...
		g_averageF.clear();
		g_averageP.clear();
		
		// Megunolink graphing
#ifdef MEGUNOLINK
		megunolinkPlot.Clear();
#endif

		// if system did not finish prepping a serial pull - do it now!
		if (g_modeSwitchIncomplete)
			pullModeSwitching(0);

		// Initialize the different manual and automated pull modes 
		selectPIDbyMode();
		selectandDrawProfilebyMode();

		//if (!g_cleanCycle)
		preInfusion = true; // start in preinfusion mode
	
		ledColor('g');
		lastFlowPulseMillis = millis();
		g_activePull = true; // Time to rock & roll
	}    

//****************************************************************************
// Pull an Espresso - Percolation
//***************************************************************************
	while (g_activePull)
	{
		// Time the pull and calculate the profile index (there are two profile points per second)
		pullTimer = millis() - pullStartTime;
		profileIndex = pullTimer / 500; // index advances in 500mSec steps

		// Measure current pressure in boiler and flow rate 
		double currentPressure = measurePressure(); // We need the current pressure for the PID loop and stored for rolling average; and displayed

		// flow measurement processing - capture two volatile variables before they change 
		unsigned long capturedFlowPulseCount = g_flowPulseCount; 
		long capturedFlowPulseMillis = g_flowPulseMillis;
#ifdef GICAR_FLOWMETER
	    //Gicar flowmeters have very low pulse rates. To enhance resolution we will calculate the reciprocal of the time between pulses (1/T).
		if (capturedFlowPulseMillis > lastFlowPulseMillis) // If there is a new flow meter pulse send the timing for flow rate calculation
		{	
			if (preInfusion) 
				g_flowPulseCountPreInfusion = capturedFlowPulseCount; //
			g_averageF.push(capturedFlowPulseMillis - lastFlowPulseMillis);
			lastFlowPulseMillis = capturedFlowPulseMillis;
		}
#endif
#ifdef DIGMESA_FLOWMETER
		//Digmesa Flowmeters have 20x the pulse rates of Gicar flowmeters. We will count the number of pulses per each 500mSec cycle.
		if(capturedFlowPulseCount > lastFlowPulseCount)
		{
			if (preInfusion)
				g_flowPulseCountPreInfusion = capturedFlowPulseCount; 
			g_averageF.push((capturedFlowPulseMillis - lastFlowPulseMillis)
				/(capturedFlowPulseCount - lastFlowPulseCount)); //calculate number of millisec per pulse
			lastFlowPulseCount = capturedFlowPulseCount;
			lastFlowPulseMillis = capturedFlowPulseMillis;
		}
		// how to detect and what to do with a stalled flow? 
		// else - perhaps artificially exponent the flow down?  
		// g_averageF.push(1.01 * g_averageF.mean()); // if there are no pulses received we have to allow the display to show the exponential drop in flowrate (even if "fictional".)
		
#endif
		
// Weight the shot using Bluetooth
#ifdef ACAIA_LUNAR_INTEGRATION		
		updateWeight();
#endif

		displayPressureandWeight();

		//Flow PID's target is the accumulated number of pulses. However, in profile we store pulses per 500mSec. So we accumulate.
		if (profileIndex != lastProfileIndex)
			sumFlowProfile += g_flowProfile[profileIndex] >> 1;

		// calculate pump speed & preinfusion for all pull modes...
		pumpPWM = setPumpPWMbyMode(profileIndex, pullTimer, pumpPWM, currentPressure, preInfusion, sumFlowProfile);
		preInfusion = setFlowLimitBypass(pumpPWM, profileIndex, preInfusion, currentPressure); // check preinfusion status & set flow limit bypass solenoid and Slayer style preInfusion
		//stopIfFault(); // What to do with this??? Make an error display???

		// Update graphs, dashboards and profiles
		if (profileIndex != lastProfileIndex) // display update & document profile every 500mSec
		{
			if (g_pullMode == MANUAL_PULL) //update profile array if manual && not cleaning...
				updateProfiles(profileIndex, pumpPWM, lastFlowPulseCount);
	
			// Update dashboard and graph
			dashboardUpdate(pumpPWM, profileIndex, g_averageP.mean(), lastFlowPulseCount, preInfusion);
			
			// Telemetry graphing
#ifdef MEGUNOLINK
			megunolinkPlot.SendFloatData("Bar", g_averageP.mean(), 1);
			megunolinkPlot.SendData("PWM", pumpPWM);
			megunolinkPlot.SendData("pulses", lastFlowPulseCount);
			megunolinkPlot.SendFloatData("ml/min", flowRate(preInfusion), 1);
			megunolinkPlot.SendFloatData("gr", scaleWeight, 1);
#endif 

#ifdef MQTT
//			mqtt.publish("/gs3/pressure", g_averageP.mean());
			mqtt.publish("/gs3/flow", profileIndex);
			mqtt.publish("/gs3/flow", lastFlowPulseCount);
			mqtt.publish("/gs3/PWM", profileIndex);
			mqtt.publish("/gs3/PWM", pumpPWM);
#endif			

			graphUpdate(pumpPWM, profileIndex, g_averageP.mean() * 100 / 12.0, g_flowPulseCount, false, preInfusion);

			//reset variables for next 500mSec period... 
			lastFlowPulseCount = g_flowPulseCount; 
			lastProfileIndex = profileIndex;
		}

		//Checks if pull is over and if so shuts down the solenoids, pumps, and LEDs...
		countOffCycles = shutdownIfPullOver(countOffCycles, lastProfileIndex);
	}
}
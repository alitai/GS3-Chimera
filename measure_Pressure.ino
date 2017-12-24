//*******************************************************************************
// Measure, average, calibrate and display boiler pressure
//********************************************************************************

float measurePressure()
{  
#ifdef CALIBRATE_PRESSURE
	float pressure = analogRead(PRESSURE_SENSOR_INPUT); // Just show the integer reading
#else
	float pressure = (float)(map((int)analogRead(PRESSURE_SENSOR_INPUT), LOW_CALIBRATION_PRESSURE_READING, HIGH_CALIBRATION_PRESSURE_READING, LOW_CALIBRATION_PRESSURE, HIGH_CALIBRATION_PRESSURE)) / 10.0 ; // Every loop we measure and average pressure in boiler
#endif
 

	g_averageP.push(pressure); // add the measurement to the rolling average
	return pressure; //we return pressure for the PID PP loop
}


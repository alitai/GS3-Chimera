void clearProfiles()
{
	memset(g_PWMProfile, 0, sizeof(g_PWMProfile)); // integer is more than 1 byte
	memset(g_pressureProfile, 0, sizeof(g_pressureProfile)); // integer is more than 1 byte
	memset(g_flowProfile, 0, sizeof(g_flowProfile)); // integer is more than 1 byte
}

void updateProfiles(int profileIndex, unsigned pumpPWM, long lastFlowPulseCount)
{
	g_PWMProfile[profileIndex] = (byte) ((long) pumpPWM * 255 / 400); // store PWM setting 0-255 
	g_pressureProfile[profileIndex] = (byte)(g_averageP.mean() * 100.0 / 12.0); // store pressure average (at higher resolution of 0-100)
	g_flowProfile[profileIndex] = (byte)(g_flowPulseCount - lastFlowPulseCount);
}

void updateDebugFirstFour(int profileIndex, unsigned pumpPWM, long lastFlowPulseCount)
{
	g_first4PWM[profileIndex] = (byte) ((long) pumpPWM * 255 / 400);
	g_first4Pressure[profileIndex] = (byte)(g_averageP.mean() * 100.0 / 12.0);
	g_first4Flow[profileIndex] = (byte)(g_flowPulseCount - lastFlowPulseCount);
}
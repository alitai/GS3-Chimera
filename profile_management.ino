void clearProfiles()
{
	memset(g_PWMProfile, 0, sizeof(g_PWMProfile)); // integer is more than 1 byte
	memset(g_pressureProfile, 0, sizeof(g_pressureProfile)); // integer is more than 1 byte
	memset(g_flowProfile, 0, sizeof(g_flowProfile)); // integer is more than 1 byte
}

void updateProfiles(uint16_t profileIndex, uint16_t pumpPWM, uint32_t lastFlowPulseCount)
{
	g_PWMProfile[profileIndex] = (byte) ((uint32_t) pumpPWM * 255 / 400); // store PWM setting 0-255 
	g_pressureProfile[profileIndex] = (byte)(g_averageP.mean() * 100.0 / 12.0); // store pressure average (at higher resolution of 0-100)
	g_flowProfile[profileIndex] = (byte)(g_flowPulseCount - lastFlowPulseCount);
}

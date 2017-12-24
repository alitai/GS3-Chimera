void manageScaleConnection()
{
	if (!scaleConnected)
	{
		if (millis() > scaleReconnectionTimer + 1000) 
		{	
			//connectScale();
			scaleConnected = scale->isScaleConnected(); 
			scaleReconnectionTimer = millis();
		}
	}
	else 
	{
	scaleConnected = scale->isScaleConnected();
	scaleReconnectionTimer = millis();
	}			
}

	
/*
Autopower off
else if(scaleActive && millis() > 10000 + scaleIdleTimer ) 
{	
	scale->disconnect();
}
*/

//check if there is a Portafilter to do a custom dose.
void updateDoseWeight()
{			
	if(scaleConnected)
		{
		updateWeight();
		int doseStabilityCounter = 8;
		float previousWeight = scaleWeight;
		float epsilon = 0.05;
		while (previousWeight > EMPTY_PORTAFILTER_WEIGHT)
		{
			//scaleWeight = measureWeight(previousWeight);
			if (updateWeight()) 
			{
				displayPressureandWeight();
				if (abs(scaleWeight - previousWeight) < epsilon)
					doseStabilityCounter--;
				else
					doseStabilityCounter=8;
			}
			previousWeight = scaleWeight;
			if (doseStabilityCounter == 0)
			{
				currentDose = scaleWeight - EMPTY_PORTAFILTER_WEIGHT;
				break;
			}
		}
	}
}
		
boolean updateWeight()
{  
	if (scaleConnected)
	{
		manageScaleConnection();
		scale->update();
		if (scale->hasWeightChanged()) 
		{
			float a = scale->getWeight(); // get new weight and convert to the nearest 1/10
			int b = a * 10;
			scaleWeight = float(b / 10.0); 
			scaleBattery = scale->getBattery();
			return true;
		}
	}
	return false; 
}

void disconnectScale()
{
	scale->disconnect();
	scaleConnected = scale->isScaleConnected();
}

void connectScale()
{
	scale = new Scale();
	scale->connect();
	scaleConnected = scale->isScaleConnected();
	scaleReconnectionTimer = millis();
}

byte touchpadSenseKey()
{	
	byte softKey = 99;     // 99 is the code for "not touched"

//	TS_Point p;
#ifdef TS_STMPE
	if (!ts.touched())
		return softKey;
	while (ts.touched()) 
		p = ts.getPoint();
#endif
#ifdef TFT_TOUCH
	if (!ts.Pressed())
		return softKey;       //Touchpad was "not touched"

	while (ts.Pressed())
	{
		p.x = HMAX - ts.RawX();
		p.y = VMAX - ts.RawY();
	}

#endif

/*	int delta = 75;
	if ( p.x != 0 && (p.x - delta) < ts.RawX()  && ts.RawX() < (p.x + delta) )
	{
		Serial.println (ts.RawX());
		Serial.println (p.x - delta);
		Serial.println (p.x + delta);
		return softKey;
	}
	
	p.x = ts.RawX();
	p.y = ts.RawY();
*/
	if (10 < p.y < 800)    // menu area
	{
		if (p.x > 3300)
			softKey = 4;
		else if (p.x > 2000)
			softKey = 3;
		else if (p.x > 700)
			softKey = 2;
		else if (p.x > 10)
			softKey = 1;
		delay(250);
	}
	if (p.x >10 && p.y >10)
	{
		Serial.print("Raw x,y = ");
		Serial.print(p.x);
		Serial.print(",");
		Serial.println(p.y);
	}

#ifdef ADAFRUIT_STMPE	
	ts.writeRegister8(STMPE_INT_STA, 0xFF);
#endif
	return softKey;
}
	
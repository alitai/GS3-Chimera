byte touchpadSenseKey()
{	
	byte softKey = 99;     // 99 is the code for "not touched"

	TS_Point p;
	if (!ts.touched())
		return softKey;       //Touchpad was "not touched"
	while (ts.touched()) 
		p = ts.getPoint();
		
	if (p.y >3400)    // menu area
	{
		if (p.x > 3400)
			softKey = 4;
		else if (p.x > 2000)
			softKey = 3;
		else if (p.x > 900)
			softKey = 2;
		else if (p.x < 900)
			softKey = 1;
	}

	ts.writeRegister8(STMPE_INT_STA, 0xFF);
	sleepTimerReset();  //extend sleep timer after activity... 
	return softKey;
}
	
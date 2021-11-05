#pragma once

class Stepper
{
private:
	uint8_t pins[4];
	uint8_t currStep = 0;

	uint32_t lastStep = 0;
	uint32_t stepInterval = 1000;

	void doStep(bool reverse)
	  {
		if(reverse)
		  {
      digitalWrite(pins[1], HIGH);
		  }
		else
		  {
      digitalWrite(pins[1], LOW);
		  }
    //do the step
    digitalWrite(pins[0], HIGH);
    digitalWrite(pins[0], LOW);
	  }

public:

	Stepper(uint8_t stepPin, uint8_t dirPin)
	  {
		pins[0] = stepPin;
		pins[1] = dirPin;
	  }

	void init()
	  {
		for(uint8_t i = 0; i < 2; i++)
		  {
			pinMode(pins[i], OUTPUT);
		  }
	  }

	void setSpeed(uint16_t stepsPerSecond)
	  {
		stepInterval = (uint32_t)1000 / stepsPerSecond;
	  }

	void step(int32_t n)
	{
		bool reverse = false;
		if(n < 0)
		{
			n = -n;
			reverse = true;
		}

		while(n > 0)
		{
			uint32_t now = millis();
			while(lastStep + stepInterval > now)
			{
				yield();
				now = millis();
			}

			doStep(reverse);
			lastStep = now;
			n--;
		}
	}

	void singleStep(bool reverse)
	{
		uint32_t now = millis();
		while(lastStep + stepInterval > now)
		{
			yield();
			now = millis();
		}

		doStep(reverse);
		lastStep = now;
	}
};

/*
  ServoShield.h - Decade counter driven Servo library for Arduino using one 8 bit timer and 4 DIO to control up to 16 servos
  Revision 1.1
  Copyright (c) 2009 Adriaan Swanepoel.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "ServoShield.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <Arduino.h> 

volatile uint16_t counter1servopositions[10];
volatile uint16_t *counter1currentservo;
volatile uint16_t *counter1deadperiod;
volatile uint16_t counter1current;

uint16_t servosmax[16];
uint16_t servosmin[16];

int outputmap[] = {0, 1, 4, 5, 2, 6, 3, 7, 8, 3, 0, 4, 1, 5, 2, 6};


uint16_t CalcCounterValue(int Time)
// timer preload
//current servo is time value (counter1servopositions)
{ 
   return (uint16_t)(65535 - (Time*8) + 1); 
    //return -Time;
} 

ISR(TIMER1_OVF_vect) 
{
	counter1cntport |= _BV(counter1cntpin); 
	*counter1deadperiod -= *counter1currentservo;
	TCNT1 = CalcCounterValue(*counter1currentservo++); 
	counter1cntport &= ~_BV(counter1cntpin);	
	
	if (counter1currentservo > &counter1servopositions[9])
	{
		counter1currentservo = &counter1servopositions[0];
		*counter1deadperiod = deadperiod;
	}		
}




void timer_init()
{

	
	//Need to clear first
	TIMSK1 = 0;  
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	
	TCCR1B = (1 << CS10);
	TIMSK1 = (1 << TOIE1);
	TCNT1 = CalcCounterValue(*counter1currentservo++); 
	

}

ServoShield::ServoShield()
{
	//Set all servos to default center
	for (int servo = 0; servo < 10; servo++) 
	{
		counter1servopositions[servo] = 1500;
		servosmax[servo] = 2000;
		servosmin[servo] = 1000;
	}
	
	//Setup pin modes
	pinMode(counter1resetpin, OUTPUT);

	counter1cntddr |= _BV(counter1cntpin); 

}

int ServoShield::setposition(int servo, int position)
{
	if ((position >= servosmin[servo]) && (position <= servosmax[servo]))
	{
		//Servo 0 to 8 on counter 1
		if ((servo >= 0) && (servo < 9))
		{
			counter1servopositions[outputmap[servo]] = position;
			return 0;
		}

		
		return 1;
	}
	
	return 1;
}

int ServoShield::getposition(int servo)
{
	//Servo 0 to 8 on counter 1
	if ((servo >= 0) && (servo < 9))
		return counter1servopositions[outputmap[servo]];
	
	return -1;
}

int ServoShield::setbounds(int servo, int minposition, int maxposition)
{
	if (servo < 16)
	{
		servosmax[servo] = maxposition;
		servosmin[servo] = minposition;
		return 0;
	}
	
	return 1;
}

int ServoShield::start()
{
	//Reset counters
	digitalWrite(counter1resetpin, HIGH);  
	counter1current = 0;
	
	//Set servo pointers
	counter1currentservo = &counter1servopositions[0];
	counter1deadperiod = &counter1servopositions[9];
	
	//Set dead periods
	*counter1deadperiod = deadperiod;

	
	timer_init();
	
	digitalWrite(counter1resetpin, LOW); 
}

int ServoShield::stop()
{
		TIMSK1 &= ~(1 << TOIE1); //enable local interrupt
}
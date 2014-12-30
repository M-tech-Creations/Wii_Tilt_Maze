/*
 *Created 12/21/14 by Mario Avenoso of Mtech Creations for Mario.mtechcreations.com
 *Code is for a tilt Maze game controled by a WiiChuck remote
 *servo control code based on the Adafruit_PWMServoDriver.h example code
 *written by Limor Fried/Ladyada for Adafruit Industries
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)

#define ACCLXLOW 82 //this is the value of the x accel at its lowest (expected)
#define ACCLXHIGH 185 //this is the value of the x accel at its highest (expected)

#define ACCLYLOW 73 //this is the value of the y accel at its lowest (expected)
#define ACCLYHIGH 190 //this is the value of the y accel at its highest (expected
//numbers found thought experimentation

#define XSERVO 0 //used to set servo control using pwm board
#define YSERVO 1 //used to set servo control using pwm board

static uint8_t Wii_buf[6];   // array to store nunchuck data,
//[0] joy x; [1] joy y; [2] accel x;[3] accel y;[4] accel z;[5] buttons

long ptimew = 0;        // will store last time there was an updated for wiichuck
long ptimeb = 0;        //will store last time there was an updated for C button
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 10;           // interval at which to blink (milliseconds)
int onoff = 1; //used to stored state of the Wiichuck controler input

void setup()
{
	//Serial.begin (19200); Was used for debuging
	Wire.begin();
	Wii_start();
	 pwm.begin();
	 
	 pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
}

void loop()
{
	unsigned long ctimew = millis(); //Set current time for update check
	unsigned long ctimeb = millis(); //Set current time for update check
	
	if(ctimew - ptimew > interval && onoff == 1) {//update platform location at interval time
		// save the last time you updated
		ptimew = ctimew;
		updateServo();
		//print();
	}
	
	if(ctimeb - ptimeb > interval+5) {//used to pasue the game / platform movement
		// save the last time you updated
		ptimeb = ctimeb;
		WII_get_data();
		if (Wii_CB())
		{
			if (onoff == 1)
			onoff = 0;
			else
			onoff = 1;
		}
	}
	
	//print();//was used for debuging
	
	

}
//updates servo posistion based on Wii input
void updateServo()
{   
	WII_get_data();
	double X = mapWiiacclx(Wii_Accelx());
	double Y = mapWiiaccly(Wii_Accely());
	pwm.setPWM(XSERVO, 0, mapPulse(X));
	pwm.setPWM(YSERVO, 0, mapPulse(Y));
}
//takes the output of the wii accel and turns it into a 1-180 degree
double mapWiiacclx(int d)
{
	return map(d, ACCLXLOW,ACCLXHIGH,0,180);
}
//takes the output of the wii accel and turns it into a 1-180 degree
double mapWiiaccly(int d)
{
	return map(d, ACCLYLOW+10,ACCLYHIGH-10,0,180);
}
//Will map a degree as an int to a pulse frequency number
double mapPulse(int d)
{
	return map(d, 0, 180, SERVOMIN, SERVOMAX);
}

static void Wii_start()
{
	Wire.beginTransmission (0x52);	// transmit to device 0x52
	Wire.write((uint8_t)0x40);// sends memory address
	Wire.write((uint8_t)0x00);// sends sent a zero.
	Wire.endTransmission ();	// stop transmitting
}

//would be send_zero()
static void WII_data_request() //sends zero byte to controller so it will send data back
{
	Wire.beginTransmission(0x52);
	Wire.write((uint8_t)0x00);// sends one byte
	Wire.endTransmission();// stop transmitting
}

static int WII_get_data()
{
	int count = 0;
	
	Wire.requestFrom (0x52, 6);// request data from Controller
	
	while(Wire.available())
	{
		Wii_buf[count] = Wii_decode_byte(Wire.read());
		count++;
	}
	WII_data_request();//send zero
	if (count>=5)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// Encode data to format that most wiimote drivers except
// only needed if you use one of the regular wiimote drivers
char Wii_decode_byte (char x)
{
	x = (x ^ 0x17) + 0x17;
	return x;
}


// returns Z button state: 1 = pressed, 0 = not pressed
static int Wii_ZB()
{
	return ((Wii_buf[5] >> 0) & 1) ? 0 : 1;  // voodoo
}

// returns C button state: 1 = pressed, 0 = not pressed
static int Wii_CB()
{
	return ((Wii_buf[5] >> 1) & 1) ? 0 : 1;  // voodoo
}

// returns value of x-axis joystick
static int Wii_JX()
{
	return Wii_buf[0];
}

// returns value of y-axis joystick
static int Wii_JY()
{
	return Wii_buf[1];
}

// returns value of the x-axis accelerator
static int Wii_Accelx()
{
	return Wii_buf[2];
}
// returns value of the x-axis accelerator
static int Wii_Accely()
{
	return Wii_buf[3];
}
// returns value of the x-axis accelerator
static int Wii_Accelz()
{
	return Wii_buf[4];
}
//shows raw data of the WiiChuck, used for debuging and code writting
void print ()
{
	
	Serial.print ("Joy X: ");
	Serial.print (Wii_JX(), DEC);
	Serial.print ("\t");

	Serial.print ("Joy Y: ");
	Serial.print (Wii_JY(), DEC);
	Serial.print ("\t");

	Serial.print ("Acc X: ");
	Serial.print (Wii_Accelx(), DEC);
	Serial.print ("\t");

	Serial.print ("Acc Y:");	
	Serial.print (Wii_Accely(), DEC);
	Serial.print ("\t");

	Serial.print ("Acc Z: ");
	Serial.print (Wii_Accelz(), DEC);
	Serial.print ("\t");

	Serial.print ("BUTT Z: ");
	Serial.print (Wii_ZB(), DEC);
	Serial.print ("\t");

	Serial.print ("BUTT C: ");
	Serial.print (Wii_CB(), DEC);
	Serial.print ("\t");

	Serial.print ("\r\n");
}
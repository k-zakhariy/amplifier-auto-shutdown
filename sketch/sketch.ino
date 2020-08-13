#include <avr/sleep.h>

class AmplifierState
{
	private:
	bool isAmplifierOn;
	bool isNowPlaying;
	unsigned long lastIdle; // Preserves the millis() read when idle was detected
	unsigned long idleTimeToTurnOff; // The time to wait to shut down amplifier

	public:
	void Init(unsigned long max_idle);
	bool IsAmplifierOn() { return isAmplifierOn; }
	bool IsNowPlaying() { return isNowPlaying; }
	bool IsIdle() { return (isNowPlaying == false) && isAmplifierOn; }
	void SetAmplifierSample(bool mp);
};


// Initializes the class, max_idle is in minutes
void AmplifierState::Init(unsigned long max_idle)
{
	idleTimeToTurnOff = max_idle;
    Serial.print("\t Idle time: "); Serial.println(idleTimeToTurnOff, DEC);
}

// Should be called on every sample of the audio signal. mp is true if music detected
void AmplifierState::SetAmplifierSample(bool mp)
{
	unsigned long now = millis(); // Take current time
	unsigned long diff;

	if (mp)
	{
		// Music is playing, act accordingly
		isAmplifierOn = true;
		isNowPlaying = true;
		lastIdle = 0;
	}
	else
	{
		// No music is playing
		if (isAmplifierOn)
		{
			isNowPlaying = false;
			if (lastIdle != 0)
			{
				diff = 0;
				if (now < lastIdle) // Overflow of millis()
					diff = now + (0xFFFFFFFFL - lastIdle);
				else
					diff = now - lastIdle;
				Serial.print("Idle for ");
				Serial.println(diff, DEC);
				if (diff > idleTimeToTurnOff)
				{
					// Turn off the Amplifier
					isAmplifierOn = false;
					Serial.println(">>> Amplifier turned off <<<");
				}
			}
			else
				lastIdle = now; // Now is first time we see idle, save the time.
		}
	}
}


/* ===============================================================
 *  Global Variables
 * ===============================================================
 */
/* Timing constants
 * maxIdleMinutes - Longer than this, amplifier will be turned off
 * sampleSleeTime - Every 2,000 mSec we sample and process
 */
const int maxIdleMinutes = 5;
const int sampleSleepTime = 2000;
/* Wiring information
 * powerPin is always on after setup() is complete. Blinks off each sample
 * playingPin indicates that music was detected
 * waitingPin indicates that no music is playing, on the way to turn off
 * amplifierPin indicates the status of the amplifier as we understand it
 */
const int powerPin = 13;
const int playingPin = 12;
const int waitingPin = 11;
const int amplifierOff = 10;
const int leftInput = A0;
const int rightInput = A1;

AmplifierState NovaAmplifier;	// And the AmplifierState class
#define ANALOG_SAMPLES 20
const int analogThreashold = 2; // Less than this is noise

void DisplayStatus()
{
	Serial.print("System Status:\n\tPlaying\t= ");
	Serial.print(NovaAmplifier.IsNowPlaying(), DEC);
	Serial.print("\n\tIdle\t= ");
	Serial.print(NovaAmplifier.IsIdle(), DEC);
	Serial.print("\n\tAmp. \t= ");
	Serial.print(NovaAmplifier.IsAmplifierOn(), DEC);
	Serial.println("");

	digitalWrite(playingPin, NovaAmplifier.IsNowPlaying());
	digitalWrite(waitingPin, NovaAmplifier.IsIdle());
	digitalWrite(amplifierOff, NovaAmplifier.IsAmplifierOn());
}


bool IsMusicPlaying()
{
	int maxRead = 0;
	int samples[ANALOG_SAMPLES];
	int i;

	Serial.print("Reading audio information:  Left ");
	digitalWrite(powerPin, LOW);

	// Take samples on right channel
	for(i=0; i < (sizeof(samples)/sizeof(samples[0])); i++)
	{
		samples[i] = analogRead(leftInput);
		delay(2 * i);
	}
	for(i=0; i < (sizeof(samples)/sizeof(samples[0])); i++)
		if (samples[i] > maxRead)
			maxRead = samples[i];

	Serial.print(maxRead, DEC);
	Serial.print("\tTotal ");

	// Take samples on left channel
	for(i=0; i < (sizeof(samples)/sizeof(samples[0])); i++)
	{
		samples[i] = analogRead(leftInput);
		delay(2 * i);
	}
	for(i=0; i < (sizeof(samples)/sizeof(samples[0])); i++)
		if (samples[i] > maxRead)
			maxRead = samples[i];

	digitalWrite(powerPin, HIGH);
	Serial.println(maxRead, DEC);

	return ((maxRead*2) > analogThreashold);
}


void setup()
{
	Serial.begin(9600); // You can use faster if you like...

	// Setup the LEDs status display
	pinMode(powerPin, OUTPUT);
	pinMode(playingPin, OUTPUT);
	pinMode(waitingPin, OUTPUT);
	pinMode(amplifierOff, OUTPUT);

	NovaAmplifier.Init((unsigned long)(maxIdleMinutes * 60L * 1000L));
	digitalWrite(powerPin, HIGH);
}



void loop()
{
	NovaAmplifier.SetAmplifierSample(IsMusicPlaying());
	DisplayStatus();
	delay(sampleSleepTime);
}

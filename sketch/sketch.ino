#include <avr/sleep.h>


class AmplifierState
{
  private:
  bool isAmplifierOn;
  bool isNowPlaying;
  unsigned long lastIdle; // Preserves the millis() read when idle was detected
  unsigned long idleTimeToTurnOff; // The time to wait to shut down amplifier

  public:
  void init(unsigned long max_idle);
  bool isOn() { return isAmplifierOn; }
  bool isNotPlaying() { return isNowPlaying; }
  bool isIdle() { return (isNowPlaying == false) && isAmplifierOn; }
  void setSample(bool mp);
};

// Initializes the class, max_idle is in minutes
void AmplifierState::init(unsigned long max_idle)
{
  idleTimeToTurnOff = max_idle;
  Serial.print("\t Idle time: "); Serial.println(idleTimeToTurnOff, DEC);
}

// Should be called on every sample of the audio signal. mp is true if music detected
void AmplifierState::setSample(bool mp)
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

AmplifierState Amplifier; // And the AmplifierState class
#define ANALOG_SAMPLES 20
const int analogThreshold = 2; // Less than this is noise

void updateStatus()
{
  Serial.print("System Status:\n\tPlaying\t= ");
  Serial.print(Amplifier.isNotPlaying(), DEC);
  Serial.print("\n\tIdle\t= ");
  Serial.print(Amplifier.isIdle(), DEC);
  Serial.print("\n\tAmp. \t= ");
  Serial.print(Amplifier.isOn(), DEC);
  Serial.println("");

  digitalWrite(playingPin, Amplifier.isNotPlaying());
  digitalWrite(waitingPin, Amplifier.isIdle());
  digitalWrite(amplifierOff, Amplifier.isOn());
}


bool isMusicPlaying()
{
  int maxRead = 0;
  int samples[ANALOG_SAMPLES];
  int i;

  Serial.print("Reading audio information:  Left ");
  digitalWrite(powerPin, LOW);

  // Take samples on right channel
  for(i=0; i < (sizeof(samples)/sizeof(samples[0])); i++)
  {
    samples[i] = analogRead(rightInput);
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

  return (maxRead > analogThreshold);
}


void setup()
{
  Serial.begin(9600); // You can use faster if you like...

  // Setup the LEDs status display
  pinMode(powerPin, OUTPUT);
  pinMode(playingPin, OUTPUT);
  pinMode(waitingPin, OUTPUT);
  pinMode(amplifierOff, OUTPUT);

  Amplifier.init((unsigned long)(maxIdleMinutes * 60L * 1000L));
  digitalWrite(powerPin, HIGH);
}



void loop()
{
  Amplifier.setSample(isMusicPlaying());
  updateStatus();
  delay(sampleSleepTime);
}
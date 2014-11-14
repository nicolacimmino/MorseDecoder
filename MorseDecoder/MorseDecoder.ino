// MorseDetector implements a decoder for morse code.
//  Copyright (C) 2014 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//
// I referred to https://github.com/jacobrosenthal/Goertzel for the Goertzel 
//  implementation.

// Pin powering the mic pre-amp
#define AUDIO_IN_PIN A0
#define LED_GREEN_PIN 2
#define LED_RED_PIN 3
#define AMPLI_PWR_PIN A3

// Sampling rate in samples/s
#define SAMPLING_RATE 5000

// Length of one dot. Assumes constant WPM at the moment
#define DOT_LEN 1

#define THRESHOLD 200

// Precalculated coefficient for our goertzel filter.
float goetzelCoeff=0;

// Length of the goertzel filter
#define GOERTZEL_N 128

// The samples taken from the A/D. The A/D results are
// 10-bits.
int sampledData[GOERTZEL_N];

enum statuses
{
    none =0,
    dot,
    dash,
    intersymbol,
    interchar,
    interword
};

statuses currentStatus=none;

int statusCounter=0;

char* lookupString = ".EISH54V.3UF....2ARL.....WP..J.1TNDB6.X..KC..Y..MGZ7.Q..O.8..90.";
byte currentDecoderIndex = 0;
byte currentDashJump = 64;
char currentAssumedChar='\0';

void setup()
{
  Serial.begin(115200); 
  
  // Set ADC prescaler to 16
  _SFR_BYTE(ADCSRA) |=  _BV(ADPS2); // Set ADPS2
  _SFR_BYTE(ADCSRA) &= ~_BV(ADPS1); // Clear ADPS1
  _SFR_BYTE(ADCSRA) &= ~_BV(ADPS0); // Clear ADPS0
  
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  
  // Power up the amplifier.
  pinMode(AMPLI_PWR_PIN, OUTPUT);
  digitalWrite(AMPLI_PWR_PIN,HIGH);
  
  float omega = (2.0 * PI * 700) / SAMPLING_RATE;
  goetzelCoeff = 2.0 * cos(omega);
}

void loop()
{
  // Sample at 4KHz 
  int32_t dcOffset=0;
  for (int ix = 0; ix < GOERTZEL_N; ix++)
  {
    sampledData[ix] = analogRead(AUDIO_IN_PIN); // 17uS
    delayMicroseconds(233); // total 250uS -> 4KHz
    dcOffset+=sampledData[ix];
  } 
  dcOffset=dcOffset/GOERTZEL_N;
  
  // Remove DC offset (center signal around zero).
  // Calculate RMS signal level.
  int32_t rmsLevel=0;
  for (int ix = 0; ix < GOERTZEL_N; ix++)
  {
    sampledData[ix] -= dcOffset;
    rmsLevel += abs(sampledData[ix]);
  }
  rmsLevel= rmsLevel/GOERTZEL_N;
  
  // AGC scale to have RMS at about 100
  for (int ix = 0; ix < GOERTZEL_N; ix++)
  {
    if(rmsLevel>5)
    {
      sampledData[ix] = sampledData[ix] * (100.0f/rmsLevel);
    }
    else
    {
      sampledData[ix] = 0;
    }
  }
  
  // Apply goertzel filter and get amplitude of  
  float Q2 = 0;
  float Q1 = 0;
  for (int ix = 0; ix < GOERTZEL_N; ix++)
  {
      float Q0 = goetzelCoeff * Q1 - Q2 + (float) (sampledData[ix]);
      Q2 = Q1;
      Q1 = Q0;  
  }
  
  int magnitude = sqrt(Q1*Q1 + Q2*Q2 - goetzelCoeff*Q1*Q2);
  //Serial.println(magnitude);
  
  if(currentStatus==none && magnitude>THRESHOLD)
  {
    currentStatus=dot;
    statusCounter=0;  
  }
  else if(currentStatus==dot && magnitude>THRESHOLD && statusCounter>DOT_LEN*2)
  {
    currentStatus=dash;
  }
  else if(currentStatus==intersymbol && magnitude<THRESHOLD && statusCounter>DOT_LEN*3)
  {
    currentStatus=interchar;
  }
  else if(currentStatus==interchar && magnitude<THRESHOLD && statusCounter>DOT_LEN*8)
  {
    currentStatus=interword;
  }
  else if(currentStatus!=none && currentStatus<intersymbol && magnitude<THRESHOLD)
  {
    //Serial.print((currentStatus==dot)?".":"-");
    digitalWrite((currentStatus==dot)?LED_GREEN_PIN:LED_RED_PIN,HIGH);
    currentAssumedChar=lookup((currentStatus==dot)?'.':'-');
    currentStatus=intersymbol;
    statusCounter=0;
    //currentStatus=none;
  }
  else if(currentStatus>dash && (magnitude>THRESHOLD || currentStatus==interword))
  {
    //Serial.print((currentStatus==intersymbol)?"":" ");
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    
    if(currentStatus==interchar)
    {
      Serial.print(currentAssumedChar);
      lookup('\0');
    }
    else if(currentStatus==interword)
    {
      Serial.print(currentAssumedChar);
      Serial.print(" ");
      lookup('\0');
    }
    
    currentStatus=none;
  }
  
  statusCounter++;
}


char lookup(char currentMark)
{
    currentDashJump = floor(currentDashJump / 2.0f);
    if (currentMark == '.')
    {                
        currentDecoderIndex++;
    }
    else if (currentMark == '-')
    {
        currentDecoderIndex += currentDashJump;
    }
    else if (currentMark == '\0')
    {
        currentDecoderIndex = 0;
        currentDashJump = 64;
        return '\0';
    }
    return lookupString[currentDecoderIndex];
}


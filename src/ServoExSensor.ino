/*
  Jeti Sensor EX Telemetry C++ Library

  Main program
  --------------------------------------------------------------------

  Copyright (C) 2017 Bernd Wokoeck

  *** Extended notice on additional work and copyrights, see header of JetiExProtocol.cpp ***

  Wiring:

    Arduino Mini  TXD-Pin 0 <-- Receiver "Ext." input (orange cable)

  Ressources:
    Uses built in UART of Arduini Mini Pro 328

  Version history:
  0.90   11/22/2015  created
  0.95   12/23/2015  new sample sensors for GPS and date/time
  0.96   02/21/2016  comPort number as optional parameter for Teensy in Start(...)
                     sensor device id as optional parameter (SetDeviceId(...))
  0.99   06/05/2016  max number of sensors increased to 32 (set MAX_SENSOR to a higher value in JetiExProtocol.h if you need more)
                     bug with TYPE_6b removed
                     DemoSensor delivers 18 demo values now
  1.00   01/29/2017  Some refactoring:
                     - Bugixes for Jetibox keys and morse alarms (Thanks to Ingmar !)
                     - Optimized half duplex control for AVR CPUs in JetiExHardwareSerialInt class (for improved Jetibox key handling)
                     - Reduced size of serial transmit buffer (128-->64 words)
                     - Changed bitrates for serial communication for AVR CPUs (9600-->9800 bps)
                     - EX encryption removed, as a consequence: new manufacturer ID: 0xA409
                       *** Telemetry setup in transmitter must be reconfigured (telemetry reset) ***
                     - Delay at startup increased (to give receiver more init time)
                     - New HandleMenu() function in JetiexSensor.ini (including more alarm test)
                     - JETI_DEBUG and BLOCKING_MODE removed (cleanup)
  1.0.1  02/15/2017  Support for ATMega32u4 CPU in Leonardo/Pro Micro
                     GetKey routine optimized
  1.02   03/28/2017  New sensor memory management. Sensor data can be located in PROGMEM
  1.03   07/14/2017  Allow all jetibox key combinations (thanks to ThomasL)
                     Disable RX at startup to prevent reception of receiver identification or other junk chars
                     Send dictionary already in serial initialization for the 1st time
                       in order to improve behaviour on telemetry reset
  1.04   07/18/2017  dynamic sensor de-/activation

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

**************************************************************/

#include "JetiExProtocol.h"
#include "ServoSensor.h"
#include <EEPROM.h>

// #define JETIEX_DEBUG

JetiExProtocol jetiEx;
ServoSensor    servoSensor;

void HandleMenu();

enum
{
  ID_SERVO = 1,
  ID_SERVO_PERCENT = 2,
  ID_SERVO_FREQ = 3
};

// sensor definition (max. 31 for DC/DS-16)
// name plus unit must be < 20 characters
// precision = 0 --> 0, precision = 1 --> 0.0, precision = 2 --> 0.00
JETISENSOR_CONST sensors[] PROGMEM =
{
  // id                name         unit         data type             precision
  { ID_SERVO,         "Position",   "us",        JetiSensor::TYPE_14b, 0 },
  { ID_SERVO_PERCENT, "Percent",    "%",         JetiSensor::TYPE_14b, 0 },
  { ID_SERVO_FREQ,    "Frequency",  "Hz",        JetiSensor::TYPE_14b, 0 },
  { 0 } // end of array
};

void setup()
{
#ifdef JETIEX_DEBUG
#if defined (CORE_TEENSY) || (__AVR_ATmega32U4__)
  Serial.begin( 9600 );
  while ( !Serial )
    ;
#endif
#endif

  // jetiEx.SetSensorActive( ID_VAL11, false, sensors ); // disable sensor

  jetiEx.SetDeviceId( 0x76, 0x32 ); // 0x3276
  jetiEx.Start( "Servo", sensors, JetiExProtocol::SERIAL2 );

  jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Servo Telemetry" );
  jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "" );

  /* add your setup code here */
  servoSensor.Init();
}

void loop()
{
  /* add your main program code here */

  jetiEx.SetSensorValue( ID_SERVO,          servoSensor.GetServoPosition() );
  jetiEx.SetSensorValue( ID_SERVO_PERCENT,  servoSensor.GetServoPositionPercent() );
  jetiEx.SetSensorValue( ID_SERVO_FREQ,     servoSensor.GetServoFrequency() );

  HandleMenu();

  jetiEx.DoJetiSend();
}

void HandleMenu()
{
  static char _buffer[ 17 ];
  static int _x = 0, _y = 0;

  uint8_t c = jetiEx.GetJetiboxKey();

  if ( c == 0 )
    return;

#ifdef JETIEX_DEBUG
#if defined (CORE_TEENSY) || (__AVR_ATmega32U4__)
  Serial.println( c );
#endif
#endif

  if ( c == JetiExProtocol::DOWN )
  {
    _y++;
    if ( _y > 2 ) _y = 0;
  }
  else if ( c == JetiExProtocol::UP )
  {
    if ( _y > 0 ) _y--;
  }
  else if ( c == JetiExProtocol::RIGHT )
  {
    if ( _y == 1 ) servoSensor.SetPercentMin( servoSensor.GetPercentMin() + 10 );
    if ( _y == 2 ) servoSensor.SetPercentMax( servoSensor.GetPercentMax() + 10 );
  }
  else if ( c == JetiExProtocol::LEFT )
  {
    if ( _y == 1 ) servoSensor.SetPercentMin( servoSensor.GetPercentMin() - 10 );
    if ( _y == 2 ) servoSensor.SetPercentMax( servoSensor.GetPercentMax() - 10 );
  }

  if ( _y == 0 )
  {
    jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Servo" );
    jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "Telemetry" );
  }
  else if ( _y == 1 )
  {
    jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Minimum" );

    sprintf( _buffer, "%d", servoSensor.GetPercentMin());
    jetiEx.SetJetiboxText( JetiExProtocol::LINE2, _buffer );
  }
  else if ( _y == 2 )
  {
    jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Maximum" );

    sprintf( _buffer, "%d", servoSensor.GetPercentMax());
    jetiEx.SetJetiboxText( JetiExProtocol::LINE2, _buffer );
  }
}

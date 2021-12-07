
#ifndef SERVOSENSOR_H
#define SERVOSENSOR_H

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

class ServoData
{
public:
  const int m_pwmPIN = 7; 
  const byte m_pwmPIN_reg = 0b10000000;

  volatile int m_PW = 0;                    // pulsewidth measurements
  volatile int m_Period = 0;                // Time between L->H (servo update frequency)
  volatile boolean m_prev_pinState = false; // used to determine whether a pin has gone low-high or high-low
  volatile unsigned long m_pwmTimer = 0;    // store the start time of each PWM pulse
};

class ServoSensor
{
public:
  ServoSensor();

  long GetServoPosition();
  long GetServoPositionPercent();
  long GetServoFrequency();

  void Init();

  int GetPercentMin() { return m_PWPcntMin; }
  void SetPercentMin(int pcntMin);

  int GetPercentMax() { return m_PWPcntMax; }
  void SetPercentMax(int pcntMax);

protected:
  ServoData m_servoData;

  const int m_PWPcntMinDef = 1000;
  const int m_PWPcntMaxDef = 2000;
  const int m_address = 0x10;

  int m_PWPcntMin;
  int m_PWPcntMax;

  void PciSetup(byte pin);
};

#endif // SERVOSENSOR

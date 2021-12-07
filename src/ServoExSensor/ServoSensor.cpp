
#include "ServoSensor.h"
#include <EEPROM.h>

ServoData* _pServoData;

// READ INTERRUPTS ON PINS D0-7: ISR routine detects which pin has changed, and returns PWM pulse width, and pulse repetition period.
ISR(PCINT2_vect)
{
  unsigned long pciTime = micros();

  if (!_pServoData->m_prev_pinState && (PIND & _pServoData->m_pwmPIN_reg))      // LOW to HIGH (start of pulse)
  {
    digitalWrite( 13, HIGH );
    
    _pServoData->m_prev_pinState = true;
    _pServoData->m_Period = pciTime - _pServoData->m_pwmTimer;
    _pServoData->m_pwmTimer = pciTime;
  }
  else if (_pServoData->m_prev_pinState && !(PIND & _pServoData->m_pwmPIN_reg)) // HIGH to LOW (end of pulse) 
  {
    digitalWrite( 13, LOW );
    
    _pServoData->m_prev_pinState = false;    
    _pServoData->m_PW = pciTime - _pServoData->m_pwmTimer;   
  }
}

ServoSensor::ServoSensor()
{
  _pServoData = &m_servoData;
}

long ServoSensor::GetServoPosition()
{
  int PW = m_servoData.m_PW;
  return PW;
}

long ServoSensor::GetServoPositionPercent() 
{
  long PW = GetServoPosition();
  if( PW < m_PWPcntMin ) return 0;
  return 100 * (PW - m_PWPcntMin) / (m_PWPcntMax - m_PWPcntMin);
}

long ServoSensor::GetServoFrequency() 
{
  int period = m_servoData.m_Period;
  if( period == 0) return 0;
  return 1e6 / period;
}

void ServoSensor::SetPercentMin(int pcntMin)
{
  m_PWPcntMin = max(pcntMin, 0);
  EEPROM.put(m_address, pcntMin);
}

void ServoSensor::SetPercentMax(int pcntMax)
{
  EEPROM.put(m_address + sizeof(m_PWPcntMin), pcntMax);
}

void ServoSensor::Init() 
{
  PciSetup(m_servoData.m_pwmPIN);

  int address = m_address;
  EEPROM.get(address, m_PWPcntMin);
  if (m_PWPcntMin == 0xffff) m_PWPcntMin = m_PWPcntMinDef;
  
  address += sizeof(m_PWPcntMin);
  EEPROM.get(address, m_PWPcntMax);
  if (m_PWPcntMax == 0xffff) m_PWPcntMax = m_PWPcntMaxDef;
}

void ServoSensor::PciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin));                   // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin));                   // enable interrupt for the group
}

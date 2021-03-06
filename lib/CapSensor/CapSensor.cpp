#include "CapSensor.h"
#include "Timer.h"
#include "Arduino.h"
#include <Adafruit_CAP1188.h>

const int  CapSensor::s_defaultKeyPollTime = 50;

//-----------------------------------------------------------------------------

class MyDebounceTimerAdatper : public TimerAdapter
{
private:
  CapSensor* m_capSensor;
  unsigned int m_capSensingDelayMs;
  uint8_t m_lastTouchValue;
  Timer m_capSensingDelayTimer;
  
public:
  MyDebounceTimerAdatper(CapSensor* capSensor, unsigned int capSensingDelay)
  : m_capSensor(capSensor)
  , m_lastTouchValue(0)
  , m_capSensingDelayMs(capSensingDelay)
  , m_capSensingDelayTimer()
  { }
  
  void timeExpired()
  {
    if (0 != m_capSensor)
    {
      uint8_t currentTouchValue = m_capSensor->wasTouched();
      if (m_lastTouchValue != currentTouchValue)
      {
        m_lastTouchValue = currentTouchValue;

        if(!m_capSensingDelayTimer.isRunning())
        {
          m_capSensor->notifyTouched(currentTouchValue);
          m_capSensingDelayTimer.startTimer(m_capSensingDelayMs);
        }
      } 
    }
  }
};

//-----------------------------------------------------------------------------

CapSensor::CapSensor(CapSensorAdapter* adapter, unsigned int capSensingDelayMs )
: m_debounceTimer(new Timer(new MyDebounceTimerAdatper(this, capSensingDelayMs), Timer::IS_RECURRING, s_defaultKeyPollTime))
, m_adapter(adapter)
, m_adaCap(new Adafruit_CAP1188())
{ 
   // init cap sensor with i2c
   if(!m_adaCap->begin())
   {
     Serial.println("CAP1188 not found");
     while (1);
   }
   m_adaCap->writeRegister(0x1F, 0x6F);
}

CapSensor::~CapSensor()
{
  delete m_debounceTimer->adapter();
  delete m_debounceTimer; m_debounceTimer = 0;
  delete m_adaCap; m_adaCap = 0;
}

CapSensorAdapter* CapSensor::adapter()
{
  return m_adapter;
}

void CapSensor::attachAdapter(CapSensorAdapter* adapter)
{
  m_adapter = adapter;
}

uint8_t CapSensor::wasTouched()
{
  uint8_t touched = 0;
  if(0 != m_adaCap)
  {
    touched = m_adaCap->touched();
  }
  return touched;
}

void CapSensor::notifyTouched(uint8_t currentTouchValue)
{
  if(0 != m_adapter)
  {
    m_adapter->notifyCapTouched(currentTouchValue);
  }
}

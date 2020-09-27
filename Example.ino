#include "MotorDriver.hpp"

const unsigned int pwmPin = 3;
const unsigned int posPin = 5;
const unsigned int negPin = 4;

HBridge driver(posPin, negPin, pwmPin);
  
void setup()
{
  /* Nothing to do */
}

void loop() {

  int i = 0;
  for(i ; i < 256 ; ++i)
  {
    driver(i);
    delay(25);
  }

  for(i ; i > -255 ; --i)
  {
    driver(i);
    delay(25);
  }

  for(i ; i < 0 ; ++i)
  {
    driver(i);
    delay(25);
  }

}


#include <PanelsController.h>


PanelsController dev;

void setup()
{
  dev.setup();

  delay(10);

  dev.writeFramesToCard();
}

void loop()
{
  Serial.println("frames written!")
  delay(1);
}

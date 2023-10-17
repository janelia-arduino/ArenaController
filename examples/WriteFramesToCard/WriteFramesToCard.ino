#include <PanelsController.hpp>


PanelsController dev;
uint32_t duration;

void setup()
{
  dev.setup();

  Serial.println("waiting to write...");
  delay(10);
  Serial.println("writing...");

  duration = millis();
  dev.writeFramesToCard();
  duration = millis() - duration;
}

void loop()
{
  Serial.print("frames written in ");
  Serial.print(duration);
  Serial.println(" milliseconds!");
  delay(4);
}

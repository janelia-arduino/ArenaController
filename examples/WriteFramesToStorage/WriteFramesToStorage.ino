#include <ArenaController.hpp>


ArenaController dev;
uint32_t duration;

void setup()
{
  dev.setup();

  Serial.println("waiting to write...");
  delay(10);
  Serial.println("writing...");

  duration = millis();
  dev.writeFramesToStorage();
  duration = millis() - duration;
}

void loop()
{
  Serial.print("frames written in ");
  Serial.print(duration);
  Serial.println(" milliseconds!");
  delay(4);
}
